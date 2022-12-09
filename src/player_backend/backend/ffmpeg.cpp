#include "ffmpeg.h"

#include <errno.h>
#include <stdio.h>
#include <chrono>
#include <unistd.h>

#include "utils/time.h"

extern "C" {
#include <libavutil/pixdesc.h>
#include <libavutil/dict.h>
#include <libavutil/hwcontext.h>
#include <libavutil/opt.h>
#include <libavutil/avassert.h>
#include <libavutil/imgutils.h>
#include <libavutil/hwcontext_drm.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
}

namespace playos {
namespace player {

static const char *defaultHWdev = "drm";

static std::unordered_map<AVPixelFormat, PixelFormat> s_pixelFormatMap = {
    { AV_PIX_FMT_YUV420P, YUV420P },
    { AV_PIX_FMT_NV12, NV12 },
    { AV_PIX_FMT_DRM_PRIME, DRM },
    { AV_PIX_FMT_RGB24, RGB },
};

static std::unordered_map<AVSampleFormat, AudioSampleFormat> s_audioSampleFormatMap = {
    { AV_SAMPLE_FMT_S16, AudioS16 },
};


static PixelFormat AVPixFmtToPixelFormat(AVPixelFormat fmt)
{
    if (s_pixelFormatMap.count(fmt))
        return s_pixelFormatMap[fmt];

    return RGB;
}

static AudioSampleFormat AVSampleFormatToAudioSampleFormat(AVSampleFormat f)
{
    if (s_audioSampleFormatMap.count(f))
        return s_audioSampleFormatMap[f];

    return AudioUnkown;
}

static int64_t getAVFrameDuration(AVFrame *frame)
{
    return frame->pts == AV_NOPTS_VALUE ? 0 : (
            frame->pkt_duration * av_q2d(frame->time_base) +
            frame->best_effort_timestamp * av_q2d(frame->time_base)) * 1000;
}

FFVideoFrame::FFVideoFrame(std::shared_ptr<VideoFrameInfo> videoInfo, const AVPixFmtDescriptor *pixFmtDesc):
            VideoFrame(videoInfo), m_frame(nullptr), m_pixFmtDesc(pixFmtDesc)
{
    m_frame = av_frame_alloc();
}

void FFVideoFrame::init(AVFrame *frame) {
    deinit();

    av_frame_ref(m_frame, frame);

    duration = getAVFrameDuration(frame);
    // spdlog::debug("FFVideoFrame duration: {}, pts: {}, pkt_duration: {}, time_base: {}, best_effort_timestamp: {}",
    //         duration, frame->pts, frame->pkt_duration, av_q2d(frame->time_base), frame->best_effort_timestamp);
    if (m_frame->format == AV_PIX_FMT_DRM_PRIME) {
        AVDRMFrameDescriptor *m_drmFd = reinterpret_cast<AVDRMFrameDescriptor*>(m_frame->data[0]);

        planes = m_drmFd->nb_objects;
        m_frameInfo->drmFourcc = m_drmFd->layers[0].format;

        for (int i = 0; i < planes; ++i) {
            data[i] = (void *)&m_drmFd->objects[i].fd;
            size[i] = m_drmFd->objects[i].size;
            stride[i] = m_drmFd->layers[0].planes[i].pitch;
            offset[i] = m_drmFd->layers[0].planes[i].offset;
        }
    } else {
        planes = av_pix_fmt_count_planes(AVPixelFormat(m_frame->format));

        for (int i = 0; i < planes; ++i) {
            data[i] = m_frame->data[plane(i)];
            stride[i] = m_frame->linesize[plane(i)];
            // FIXME
            size[i] = stride[i] * (i > 0 ? m_frame->height / 2 : m_frame->height);
        }
    }
}

FFVideoFrame::~FFVideoFrame()
{
    deinit();
    av_frame_free(&m_frame);
}

int FFVideoFrame::plane(int index)
{
    return m_pixFmtDesc->comp[index].plane;
}

void FFVideoFrame::deinit()
{
    for (int i = 0; i < MAX_PLANE; ++i) {
        data[i] = nullptr;
        stride[i] = 0;
        size[i] = 0;
        offset[i] = 0;
    }

    av_frame_unref(m_frame);
}

AVChannelLayout FFAudioFrame::s_channelLayout = AV_CHANNEL_LAYOUT_STEREO;

FFAudioFrame::FFAudioFrame(): m_maxSamples(0)
{
    m_frame = av_frame_alloc();
    if (!m_frame) {
        spdlog::error("Could not allocate avframe for FFAudioFrame");
        exit(1);
    }

    m_swrCtx = swr_alloc();
    if (!m_swrCtx) {
        spdlog::error("Could not allocate resampler context");
        exit(1);
    }
}

FFAudioFrame::~FFAudioFrame()
{
    deinit();
    swr_free(&m_swrCtx);
    av_frame_free(&m_frame);

}

void FFAudioFrame::init(AVFrame *frame)
{
    av_channel_layout_copy(&m_frame->ch_layout, &s_channelLayout);
    m_frame->format = AV_SAMPLE_FMT_S16;
    m_frame->sample_rate = 48000;
    m_frame->time_base = (AVRational){1, m_frame->sample_rate};
    duration = getAVFrameDuration(frame);
    // spdlog::debug("FFAudioFrame duration: {}, {}, {}, {}, {}", duration, frame->pts, 
    //         frame->pkt_duration, av_q2d(m_tb), frame->best_effort_timestamp);
    resampling(frame, m_frame);

    channels = m_frame->ch_layout.nb_channels;
    samples = m_frame->nb_samples;
    format = AVSampleFormatToAudioSampleFormat(AVSampleFormat(m_frame->format));
    freq = m_frame->sample_rate;
    planes = 1;

    if (planes > 1) {
        for (int i = 0; i < channels; ++i) {
            data[i] = m_frame->data[i];
        }
    } else {
        data[0] = m_frame->data[0];
    }
}

void FFAudioFrame::deinit()
{
    // av_frame_unref(m_frame);

    for (int i = 0; i < MAX_PLANE; ++i) {
        data[i] = nullptr;
    }
}

void FFAudioFrame::resampling(AVFrame *src, AVFrame *dst)
{
    int ret;
    int dst_linesize;

    /* set options */
    av_opt_set_chlayout(m_swrCtx, "in_chlayout",    &src->ch_layout, 0);
    av_opt_set_int(m_swrCtx, "in_sample_rate",       src->sample_rate, 0);
    av_opt_set_sample_fmt(m_swrCtx, "in_sample_fmt", AVSampleFormat(src->format), 0);

    av_opt_set_chlayout(m_swrCtx, "out_chlayout",    &dst->ch_layout, 0);
    av_opt_set_int(m_swrCtx, "out_sample_rate",       dst->sample_rate, 0);
    av_opt_set_sample_fmt(m_swrCtx, "out_sample_fmt", AVSampleFormat(dst->format), 0);

    if (swr_is_initialized(m_swrCtx)) {
        swr_close(m_swrCtx);
    }

    /* initialize the resampling context */
    if ((ret = swr_init(m_swrCtx)) < 0) {
        spdlog::error("Failed to initialize the resampling context");
        return;
    }

    dst->nb_samples =
            av_rescale_rnd(src->nb_samples, dst->sample_rate, src->sample_rate, AV_ROUND_UP);
    if (dst->nb_samples > m_maxSamples) {
        // FIXME: memory leak
        ret = av_frame_get_buffer(dst, 0);
        if (ret < 0) {
            spdlog::error("Could not allocate audio data buffers");
            return;
        }
        m_maxSamples = dst->nb_samples;
    }

    ret = swr_convert(m_swrCtx, (uint8_t **)&dst->data, dst->nb_samples,
            (const uint8_t **)&src->data, src->nb_samples);
    if (ret < 0) {
        spdlog::error("Error while converting");
    }
}

FFmpegBackend::FFmpegBackend():
        m_enableHWAccel(true), m_fmtCtx(nullptr),
        m_videoDecCtx(nullptr), m_audioDecCtx(nullptr),
        m_swsContext(nullptr),
        m_videoStreamIdx(-1), m_audioStreamIdx(-1),
        m_state(CREATED), m_pixFmt(AV_PIX_FMT_NONE)
{
    const char *hw = getenv("SS_PLAYER_HWACCEL");
    if (hw != NULL) {
        m_enableHWAccel = strcmp(hw, "no") != 0;
    }

    const char *level = getenv("SS_PLAYER_LOG_LEVEL");
    int avLogLevel = AV_LOG_WARNING;
    if (level != NULL) {
        if (level[0] >= '0' && level[0] <= '9') {
            switch (level[0]) {
            case '0':
                avLogLevel = AV_LOG_PANIC;
                break;
            case '1':
                avLogLevel = AV_LOG_FATAL;
                break;
            case '2':
                avLogLevel = AV_LOG_ERROR;
                break;
            case '3':
                avLogLevel = AV_LOG_WARNING;
                break;
            case '4':
                avLogLevel = AV_LOG_INFO;
                break;
            case '5':
                avLogLevel = AV_LOG_VERBOSE;
                break;
            case '6':
                avLogLevel = AV_LOG_DEBUG;
                break;
            case '7':
                avLogLevel = AV_LOG_TRACE;
                break;
            }
        }

    }

    av_log_set_level(avLogLevel);
}

FFmpegBackend::~FFmpegBackend()
{
    stop();
}

AVPixelFormat FFmpegBackend::getHWFmt(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts)
{
    const enum AVPixelFormat *p;

    for (p = pix_fmts; *p != -1; p++) {
        if (*p == m_pixFmt)
            return *p;
    }

    spdlog::error("Failed to get HW surface format.");
    return AV_PIX_FMT_NONE;
}
 
int FFmpegBackend::openCodecContext(int *stream_idx, AVCodecContext **dec_ctx, enum AVMediaType type)
{
    int ret, stream_index;
    AVStream *st;
    const AVCodec *dec = NULL;
    int err = 0;
 
    ret = av_find_best_stream(m_fmtCtx, type, -1, -1, &dec, 0);
    if (ret < 0) {
        spdlog::warn("Could not find {} stream in input file '{}'",
                av_get_media_type_string(type), m_uri);
        return ret;
    }

    stream_index = ret;
    st = m_fmtCtx->streams[stream_index];

    if (dec == nullptr) {
        dec = avcodec_find_decoder(st->codecpar->codec_id);
        if (!dec) {
            spdlog::error("Failed to find {} codec", av_get_media_type_string(type));
            return AVERROR(EINVAL);
        }
    }

    if (type == AVMEDIA_TYPE_VIDEO && m_enableHWAccel) {
        const char *decoder = nullptr;
#if defined(__linux__) && defined(__aarch64__)
        if (st->codecpar->codec_id == AV_CODEC_ID_H264) {
            decoder = "h264_v4l2m2m";
        } else if (st->codecpar->codec_id == AV_CODEC_ID_H265) {
            decoder = "h265_v4l2m2m";
        }
#endif

        if (decoder != nullptr) {
            if ((dec = avcodec_find_decoder_by_name(decoder)) == NULL) {
                spdlog::error("Cannot find the v4l2m2m decoder");
            } else {
                m_pixFmt = AV_PIX_FMT_DRM_PRIME;
            }
        }
    }

    /* Allocate a codec context for the decoder */
    *dec_ctx = avcodec_alloc_context3(dec);
    if (!*dec_ctx) {
        spdlog::error("Failed to allocate the {} codec context",
                av_get_media_type_string(type));
        return AVERROR(ENOMEM);
    }

    /* Copy codec parameters from input stream to output codec context */
    if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0) {
        spdlog::error("Failed to copy {} codec parameters to decoder context",
                av_get_media_type_string(type));
        return ret;
    }

    if (type == AVMEDIA_TYPE_VIDEO && m_enableHWAccel) {
        for (int i = 0; ; i++) {
            const AVCodecHWConfig *config = avcodec_get_hw_config(dec, i);
            if (!config) {
                spdlog::error("Decoder {} does not support device type {}.",
                        dec->name, av_hwdevice_get_type_name(m_hwType));
                break;
            }

            if (config->methods & AV_CODEC_HW_CONFIG_METHOD_INTERNAL) {
                if (m_pixFmt != AV_PIX_FMT_NONE) {
                    (*dec_ctx)->pix_fmt = m_pixFmt;
                }

                (*dec_ctx)->opaque = this;
                (*dec_ctx)->get_format = [](AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts)
                        -> AVPixelFormat {
                    auto ff = reinterpret_cast<FFmpegBackend *>(ctx->opaque);
                    return ff->getHWFmt(ctx, pix_fmts);
                };
                break;
            }

            if (config->device_type == m_hwType) {
                (*dec_ctx)->pix_fmt = config->pix_fmt;

                if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX) {
                    if ((err = av_hwdevice_ctx_create(&(*dec_ctx)->hw_device_ctx,
                            m_hwType, NULL, NULL, 0)) < 0) {
                        spdlog::error("Failed to create specified HW device: {}.", av_err2str(err));
                        return err;
                    }
                }

                break;
            }
        }
    }

    if ((ret = avcodec_open2(*dec_ctx, dec, NULL)) < 0) {
        spdlog::error("Failed to open {} codec", av_get_media_type_string(type));
        return ret;
    }
    *stream_idx = stream_index;
 
    return 0;
}

void FFmpegBackend::decodeVideo()
{
    int ret = 0;
    bool stop = false;
    AVPacket *pkt = nullptr;
    AVFrame *frame = av_frame_alloc();
    if (!frame) {
        spdlog::error("Could not allocate frame");
        errno = AVERROR(ENOMEM);
        return;
    }

    for (; m_state != STOPPED && !stop;) {
        if (m_state != PLAYING) {
            std::this_thread::yield();
            continue;
        }

        m_videoState.mux.lock();
        if (m_videoState.que.empty()) {
            m_videoState.mux.unlock();
            std::this_thread::yield();
            continue;
        }

        pkt = m_videoState.que.front();
        m_videoState.que.pop();
        m_videoState.queDuration -= pkt->duration;
        m_videoState.mux.unlock();

        ret = avcodec_send_packet(m_videoDecCtx, pkt);
        av_packet_unref(pkt);
        if (ret < 0) {
            spdlog::error("Error submitting a video packet for decoding ({})", av_err2str(ret));

            onError("Error submitting a video packet for decoding");
            m_state = STOPPED;
            break;
        }

        while (ret >= 0 && m_state != STOPPED) {
            ret = avcodec_receive_frame(m_videoDecCtx, frame);
             if (ret == AVERROR(EAGAIN)) {
                break;
            } else if (ret == AVERROR_EOF) {
                stop = true;
                m_handler->onEOS();
                break;
            } else if (ret < 0) {
                spdlog::error("Error during video decoding ({})", av_err2str(ret));

                onError("Error during video decoding");
                m_state = STOPPED;
                break;
            }

            FFVideoFrame *vf = m_videoFramePool->waitForFrame();
            if (!vf) {
                av_frame_unref(frame);
                break;
            }

            frame->time_base = m_videoStream->time_base;

            if (s_pixelFormatMap.count(AVPixelFormat(frame->format))) {
                vf->init(frame);
            } else {
                AVFrame *tmpFrame, *swFrame;
                if (m_videoDecCtx->hw_device_ctx) {
                    swFrame = av_frame_alloc();
                    if ((ret = av_hwframe_transfer_data(swFrame, frame, 0)) < 0) {
                        onError("Error transferring the data to system memory\n");
                    }
                    tmpFrame = swFrame;
                } else {
                    tmpFrame = frame;
                }

                AVFrame *dstFrame = av_frame_alloc();
                if (dstFrame) {
                    if (m_swsContext == nullptr) {
                        m_swsContext = sws_getContext(m_width, m_height, AVPixelFormat(tmpFrame->format),
                                m_width, m_height, AV_PIX_FMT_RGB24, SWS_BICUBIC,
                                NULL, NULL, NULL);
                    }

                    if ((ret = sws_scale_frame(m_swsContext, dstFrame, tmpFrame)) < 0) {
                        spdlog::error("Failed to convert pixel format: {}", av_err2str(ret));
                    }

                    av_frame_copy_props(dstFrame, frame);
                    vf->init(dstFrame);
                    av_frame_free(&dstFrame);
                }

                av_frame_free(&swFrame);
            }

            m_handler->onFrame(vf);
            av_frame_unref(frame);
        }
    }
    av_frame_free(&frame);
}

void FFmpegBackend::decodeAudio()
{
    int ret = 0;
    bool stop = false;
    AVPacket *pkt = nullptr;
    AVFrame *frame = av_frame_alloc();
    if (!frame) {
        spdlog::error("Could not allocate frame");
        errno = AVERROR(ENOMEM);
        return;
    }

    for (; m_state != STOPPED && !stop;) {
        if (m_state != PLAYING) {
            std::this_thread::yield();
            continue;
        }

        m_audioState.mux.lock();
        if (m_audioState.que.empty()) {
            m_audioState.mux.unlock();
            std::this_thread::yield();
            continue;
        }

        pkt = m_audioState.que.front();
        m_audioState.que.pop();
        m_audioState.queDuration -= pkt->duration;
        m_audioState.mux.unlock();

        ret = avcodec_send_packet(m_audioDecCtx, pkt);
        if (ret < 0) {
            av_packet_unref(pkt);
            m_state = STOPPED;

            onError("Error submitting a audio packet for decoding");
            spdlog::error("Error submitting a audio packet for decoding ({})", av_err2str(ret));
            break;
        }

        while (ret >= 0 && m_state != STOPPED) {
            ret = avcodec_receive_frame(m_audioDecCtx, frame);
            if (ret == AVERROR(EAGAIN)) {
                break;
            } else if (ret == AVERROR_EOF) {
                stop = true;
                m_handler->onEOS();
                break;
            } else if (ret < 0) {
                spdlog::error("Error during audio decoding ({})", av_err2str(ret));
                m_state = STOPPED;
                onError("Error during audio decoding");
                break;
            }

            frame->time_base = m_audioStream->time_base;

            FFAudioFrame *af = m_audioFramePool->waitForFrame();
            if (af) {
                af->init(frame);
                m_handler->onFrame(af);
            }

            av_frame_unref(frame);
        }

        av_packet_unref(pkt);
    }
    av_frame_free(&frame);
}

void FFmpegBackend::pushPacket(DecodeState &state, AVPacket *pkt)
{
    AVPacket *p = av_packet_alloc();

    if (!p) {
        av_packet_unref(pkt);
        return;
    }

    av_packet_move_ref(p, pkt);

    state.mux.lock();
    state.queDuration += p->duration;
    state.que.push(p);
    state.mux.unlock();
}

void FFmpegBackend::tryInitHWType()
{
    const char *hwDev = defaultHWdev;
    if (getenv("SS_PLAYER_HW_DEV") != nullptr) {
        hwDev = getenv("SS_PLAYER_HW_DEV");
    }

    m_hwType = av_hwdevice_find_type_by_name(hwDev);
    if (m_hwType == AV_HWDEVICE_TYPE_NONE) {
        spdlog::warn("Device type {} is not supported.", hwDev);
        spdlog::warn("Available device types:");
        while((m_hwType = av_hwdevice_iterate_types(m_hwType)) != AV_HWDEVICE_TYPE_NONE)
            spdlog::warn("     {}", av_hwdevice_get_type_name(m_hwType));
    }
}

bool FFmpegBackend::doInit()
{
    return true;
}

bool FFmpegBackend::play()
{
    if (m_state == PAUSED) {
        m_state = PLAYING;
    } else if (m_state == STOPPED || m_state == CREATED) {
        m_state = PLAYING;

        tryInitHWType();

        if (avformat_open_input(&m_fmtCtx, m_uri, NULL, NULL) < 0) {
            spdlog::error("Could not open source file {}", m_uri);
            return false;
        }
    
        if (avformat_find_stream_info(m_fmtCtx, NULL) < 0) {
            spdlog::error("Could not find stream information");
            return false;
        }

        m_videoInfo = VideoInfo::create();
        m_videoInfo->tags.emplace_back();
        m_videoInfo->duration = m_fmtCtx->duration / 1000;

        const AVDictionaryEntry *tag = NULL;
         while ((tag = av_dict_get(m_fmtCtx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
            printf("%s=%s", tag->key, tag->value);

        if (openCodecContext(&m_videoStreamIdx, &m_videoDecCtx, AVMEDIA_TYPE_VIDEO) >= 0) {
            m_videoStream = m_fmtCtx->streams[m_videoStreamIdx];
            if (m_videoDecCtx->time_base.num == 0) {
                m_videoDecCtx->time_base = m_videoStream->time_base;
            }
            if (m_videoDecCtx->pkt_timebase.num == 0) {
                m_videoDecCtx->pkt_timebase = m_videoStream->time_base;
            }
    
            m_width = m_videoDecCtx->width;
            m_height = m_videoDecCtx->height;
            m_pixFmt = m_videoDecCtx->pix_fmt;

            if (m_pixFmt == AV_PIX_FMT_DRM_PRIME) {
                m_videoFrameInfo = VideoFrameInfo::create(m_width, m_height, DRM);
            } else if (m_pixFmt == AV_PIX_FMT_YUV420P) {
                m_videoFrameInfo = VideoFrameInfo::create(m_width, m_height, YUV420P);
            } else if (m_pixFmt == AV_PIX_FMT_NV12) {
                m_videoFrameInfo = VideoFrameInfo::create(m_width, m_height, NV12);
            } else {
                m_videoFrameInfo = VideoFrameInfo::create(m_width, m_height, RGB);
                if (m_videoDecCtx->sw_pix_fmt == AV_PIX_FMT_NONE) {
                    m_videoDecCtx->sw_pix_fmt = AV_PIX_FMT_RGB24;
                }
            }

            m_pixFmtDesc = av_pix_fmt_desc_get(m_pixFmt);
            spdlog::debug("Video raw data pix fmt: {}, sw pix fmt: {}",
                    av_get_pix_fmt_name(m_pixFmt), av_get_pix_fmt_name(m_videoDecCtx->sw_pix_fmt));
            if (m_videoDecCtx->sw_pix_fmt != AV_PIX_FMT_NONE) {
                m_videoFrameInfo->planars = av_pix_fmt_count_planes(m_videoDecCtx->sw_pix_fmt);
                m_videoFrameInfo->swFormat = AVPixFmtToPixelFormat(m_videoDecCtx->sw_pix_fmt);
            } else {
                m_videoFrameInfo->planars = av_pix_fmt_count_planes(m_videoDecCtx->pix_fmt);
                m_videoFrameInfo->swFormat = AVPixFmtToPixelFormat(m_videoDecCtx->pix_fmt);
            }

            m_handler->onReady(m_videoInfo, m_videoFrameInfo);
        }
    
        if (openCodecContext(&m_audioStreamIdx, &m_audioDecCtx, AVMEDIA_TYPE_AUDIO) >= 0) {
            m_audioStream = m_fmtCtx->streams[m_audioStreamIdx];
        }
    
        /* dump input information to stderr */
        av_dump_format(m_fmtCtx, 0, m_uri, 0);
    
        if (!m_videoStream && !m_audioStream) {
            spdlog::error("Could not find audio or video stream in the input, aborting");
            errno = -1;

            return false;
        }
    
        m_pkt = av_packet_alloc();
        if (!m_pkt) {
            spdlog::error("Could not allocate packet");
            errno = AVERROR(ENOMEM);
            return false;
        }

        m_videoFramePool = FramePool<FFVideoFrame>::create(24, [&]() {
            return new FFVideoFrame(m_videoFrameInfo, m_pixFmtDesc);
        });
        m_audioFramePool = FramePool<FFAudioFrame>::create(24, [&]() {
            return new FFAudioFrame();
        });

        if (m_videoStreamIdx != -1) {
            m_videoState.decodeThread = std::thread(&FFmpegBackend::decodeVideo, this);
            m_videoState.queDuration = 0;
        }

        if (m_audioStreamIdx != -1) {
            m_audioState.decodeThread = std::thread(&FFmpegBackend::decodeAudio, this);
            m_audioState.queDuration = 0;
        }

        m_readThread = std::thread(&FFmpegBackend::readThread, this);
    }

    return true;
}

bool FFmpegBackend::pause()
{
    m_state = PAUSED;

    return true;
}

bool FFmpegBackend::stop()
{
    if (m_state != STOPPED) {
        m_state = STOPPED;

        if (m_readThread.joinable()) {
            m_readThread.join();
        }

        m_audioFramePool = nullptr;
        if (m_audioState.decodeThread.joinable()) {
            m_audioState.decodeThread.join();
        }

        m_videoFramePool = nullptr;
        if (m_videoState.decodeThread.joinable()) {
            m_videoState.decodeThread.join();
        }

        if (m_swsContext) {
            sws_freeContext(m_swsContext);
        }

        avcodec_free_context(&m_videoDecCtx);
        avcodec_free_context(&m_audioDecCtx);
        avformat_close_input(&m_fmtCtx);
        av_packet_free(&m_pkt);
    }

    return true;
}

int FFmpegBackend::seek(int64_t pos)
{
    return true;
}

static bool checkQueue(FFmpegBackend::DecodeState &state, AVStream *stream)
{
    return state.que.size() > 25 && state.queDuration * av_q2d(stream->time_base) > 1.0;
}

bool FFmpegBackend::readThread()
{
    for (; m_state != STOPPED; ) {
        int ret = 0;
        if (m_state != PLAYING || (checkQueue(m_audioState, m_audioStream) &&
                checkQueue(m_videoState, m_videoStream))) {
            spdlog::trace("Waiting, videoFramePool size: {}, audioFramePool size: {}, "
                    "audioState queue size: {}, videoState queue size: {}",
                    m_videoFramePool->size(), m_audioFramePool->size(),
                    m_audioState.que.size(), m_videoState.que.size());
            usleep(1000*10);
            continue;
        }

        if (av_read_frame(m_fmtCtx, m_pkt) >= 0) {
            if (m_pkt->stream_index == m_videoStreamIdx) {
                pushPacket(m_videoState, m_pkt);
            } else if (m_pkt->stream_index == m_audioStreamIdx) {
                pushPacket(m_audioState, m_pkt);
            }

            av_packet_unref(m_pkt);
        } else {
            if (m_videoStreamIdx != -1) {
                pushPacket(m_videoState, m_pkt);
            }

            if (m_audioStreamIdx != -1) {
                pushPacket(m_audioState, m_pkt);
            }

            break;
        }
    }

    return true;
}

void FFmpegBackend::putFrame(Frame *frame)
{
    if (frame->type == Frame::Video) {
        FFVideoFrame *f = (FFVideoFrame *)frame;
        f->deinit();
        m_videoFramePool->putFrame(f);
    } else {
        FFAudioFrame *f = (FFAudioFrame *)frame;
        f->deinit();
        m_audioFramePool->putFrame(f);
    }
}

}
}
