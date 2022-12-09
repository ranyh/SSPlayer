#ifndef __PLAYOS_PLAYER_FFMPEG_H__
#define __PLAYOS_PLAYER_FFMPEG_H__

#include "backend.h"
#include "frame_pool.h"
#include "state.h"

#include <thread>
#include <queue>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

#ifdef av_err2str
#undef av_err2str
#include <string>
av_always_inline std::string av_err2string(int errnum) {
    char str[AV_ERROR_MAX_STRING_SIZE];
    return av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum);
}
#define av_err2str(err) av_err2string(err).c_str()
#endif  // av_err2str


namespace std {
template <> struct hash<AVSampleFormat> {
    size_t operator() (const AVSampleFormat& s) const { return size_t(s); }
};
template <> struct hash<AVPixelFormat> {
    size_t operator() (const AVPixelFormat& s) const { return size_t(s); }
};
}

namespace playos {
namespace player {

class FFVideoFrame: public VideoFrame, public FrameLink {
public:
    FFVideoFrame(std::shared_ptr<VideoFrameInfo> videoInfo, const AVPixFmtDescriptor *pixFmtDesc);
    ~FFVideoFrame();
    void init(AVFrame *frame);
    void deinit();

private:
    int plane(int index);

private:
    AVFrame *m_frame;
    const AVPixFmtDescriptor *m_pixFmtDesc;
};

class FFAudioFrame: public AudioFrame, public FrameLink {
public:
    FFAudioFrame();
    ~FFAudioFrame();
    void init(AVFrame *frame);
    void deinit();

private:
    void resampling(AVFrame *src, AVFrame *dst);

private:
    AVFrame *m_frame;
    SwrContext *m_swrCtx;
    int m_maxSamples;

    static AVChannelLayout s_channelLayout;
};

class FFmpegBackend: public Backend {
public:
    struct DecodeState {
        std::thread decodeThread;
        std::queue<AVPacket *> que;
        std::mutex mux;
        int64_t queDuration;
    };
public:
    FFmpegBackend();
    ~FFmpegBackend();
    AVPixelFormat getHWFmt(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts);

private:
    int openCodecContext(int *stream_idx,
                        AVCodecContext **dec_ctx,
                        enum AVMediaType type);
    bool readThread();
    void decodeVideo();
    void decodeAudio();

    void pushPacket(DecodeState &state, AVPacket *pkt);
    void tryInitHWType();

private:
    bool doInit() override;

    bool play() override;
    bool pause() override;
    bool stop() override;
    int seek(int64_t pos) override;
    void putFrame(Frame *frame) override;

private:
    AVFormatContext *m_fmtCtx;
    AVCodecContext *m_videoDecCtx;
    AVCodecContext *m_audioDecCtx;
    SwsContext *m_swsContext;
    int m_width, m_height;
    enum AVHWDeviceType m_hwType;
    enum AVPixelFormat m_pixFmt;
    AVStream *m_videoStream;
    AVStream *m_audioStream;

    int m_videoStreamIdx;
    int m_audioStreamIdx;
    AVPacket *m_pkt;

    std::thread m_readThread;
    DecodeState m_videoState;
    DecodeState m_audioState;

    volatile PlayerState m_state;

    std::shared_ptr<VideoInfo> m_videoInfo;
    std::shared_ptr<VideoFrameInfo> m_videoFrameInfo;
    const AVPixFmtDescriptor *m_pixFmtDesc;
    std::unique_ptr<FramePool<FFVideoFrame>> m_videoFramePool;
    std::unique_ptr<FramePool<FFAudioFrame>> m_audioFramePool;
    bool m_enableHWAccel;
};

}
}

#endif
