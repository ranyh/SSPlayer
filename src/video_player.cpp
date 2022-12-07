#include "video_player.h"

#include "application.h"
#include <iostream>
#include <condition_variable>


namespace playos {

static const char *playerBackend = "ffmpeg";

VideoPlayer::VideoPlayer(Application *app):
        m_app(app)
{
    const char *b = getenv("SS_PLAYER_BACKEND");
    if (b) {
        playerBackend = b;
    }
}

VideoPlayer::~VideoPlayer()
{
    m_player->stop();
    m_player->setHandler(nullptr);
}

void VideoPlayer::onEvent(Event &event)
{
    Scene::onEvent(event);

    if(event.type == Event::KEY && event.state == KEY_PRESSED) {
        switch (event.value) {
        case KEY_SPACE:
            if (m_player->isPlaying()) {
                m_player->pause();
            } else {
                m_player->play();
            }
            break;
        }
    } else if (event.type == Event::WINDOW_RESIZE) {
        m_videoView->setSize(width(), height());
        m_controller->setSize(width(), m_controller->height());
    }
}

void VideoPlayer::setPlayList(std::vector<std::string> &playList)
{
    m_playList = std::move(playList);
    if (m_controller) {
        m_controller->setPlayList(&m_playList);
    }

    // TODO:
    if (m_playlistView) {
        auto a = m_playList;
        m_playlistView->setList(a);
    }
}

void VideoPlayer::onAudioCallback(uint8_t **stream, int len)
{
    player::AudioFrame *frame = m_player->getAudioFrame();
    if (frame) {
        memcpy(stream[0], frame->data[0], len);
        m_player->putFrame(frame);
    } else {
        memset(stream[0], 0, len);
    }
}

void VideoPlayer::onInit(UIContext *context)
{
    int dWidth = context->drawableWidth();
    int dHeight = context->drawableHeight();

    m_player = std::unique_ptr<player::Player>(new player::Player(playerBackend));
    m_videoView = std::unique_ptr<VideoView>(new VideoView(context->createResourceContext(), this));
    m_controller = std::unique_ptr<Controller>(new Controller(this));
    m_playlistView = std::unique_ptr<Playlist>(new Playlist(this));
    m_playlistView->setVisibility(false);

    m_videoView->setSize(dWidth, dHeight);
    m_player->setHandler(this);
    m_controller->setListener(this);
    m_controller->setPlayList(&m_playList);

    // TODO:
    auto a = m_playList;
    m_playlistView->setList(a);

    add(m_videoView.get());
    add(m_controller.get());
    add(m_playlistView.get());
}

void VideoPlayer::onPlay()
{
    m_player->play();
}

void VideoPlayer::onPause()
{
    m_player->pause();
}

void VideoPlayer::onStop()
{
    m_player->stop();
}

void VideoPlayer::onSeek(int64_t i)
{
    m_player->seek(i);
}

void VideoPlayer::onSetUri(const std::string &uri)
{
    m_player->setUri(uri);
}

bool VideoPlayer::isPlaying()
{
    return m_player->isPlaying();
}

void VideoPlayer::onShowPlaylist()
{
    m_playlistView->setVisibility(true);
}

void VideoPlayer::onReady(std::shared_ptr<player::VideoInfo> info)
{
    m_controller->setProgress(0);
    m_controller->setDuration(info->duration);
    m_controller->setVideoTitle(info->tags[0].title);
}

void VideoPlayer::onEOS()
{
    m_controller->next();
}

void VideoPlayer::onFrameInfo(std::shared_ptr<player::VideoFrameInfo> frameInfo)
{
    m_frameInfo = frameInfo;
    m_app->exec([&]() {
        m_videoView->prepareTextures(m_frameInfo);

        m_audioSink.init(std::bind(&VideoPlayer::onAudioCallback, this, std::placeholders::_1, std::placeholders::_2));
        m_audioSink.start();
    });
}

void VideoPlayer::onFrame(player::Frame *frame)
{
    if (frame->type == player::Frame::Video) {
        static std::mutex mux;
        static std::condition_variable cv;
        bool finish = false;

        m_app->exec([&, frame]() {
            player::VideoFrame *vFrame = (player::VideoFrame *)(frame);
            if (vFrame->map(player::VideoFrame::ReadOnly)) {
                m_videoView->load(vFrame);
                vFrame->unmap();
            }
            m_controller->setProgress(frame->duration);

            finish = true;
            cv.notify_all();
        });

        std::unique_lock<std::mutex> lk(mux);
        cv.wait(lk, [&]{return finish;});
    } else if (frame->type == player::Frame::Audio) {
        player::AudioFrame *aFrame = (player::AudioFrame *)(frame);
        // TODO:
        m_audioSink.queue((const uint8_t **)aFrame->data, aFrame->samples * 2 * 2);
    }
}

}
