#include "video_player.h"

#include "application.h"


namespace playos {

VideoPlayer::VideoPlayer(Application *app):
        m_app(app), m_texture(nullptr)
{
}

VideoPlayer::~VideoPlayer()
{
    m_player->stop();
    m_player->setExecutor(nullptr);
    m_player->setHandler(nullptr);
}

void VideoPlayer::onEvent(Event &event)
{
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

void VideoPlayer::onInit(UIContext *context)
{
    int dWidth = context->drawableWidth();
    int dHeight = context->drawableHeight();

    m_player = std::unique_ptr<player::Player>(new player::Player(m_app));
    m_videoView = std::unique_ptr<VideoView>(new VideoView(this));
    m_controller = std::unique_ptr<Controller>(new Controller(this));
    m_texture = std::make_shared<Texture>();

    m_videoView->setTexture(m_texture);
    m_videoView->setSize(dWidth, dHeight);
    m_player->setHandler(this);

    m_player->setUri("file:///Users/rany/Movies/70aaa3ac13c691c93dae0237fc9ff6e7.mp4");

    add(m_videoView.get());
    add(m_controller.get());
}

void VideoPlayer::onFrameInfo(std::shared_ptr<player::VideoFrameInfo> frameInfo)
{
    m_frameInfo = frameInfo;
    m_videoView->play();
}

void VideoPlayer::onFrame(std::shared_ptr<player::Frame> frame)
{
    if (frame->vFrame->map(player::VideoFrame::ReadOnly)) {
        m_texture->load(frame->vFrame->data(), m_frameInfo->width, m_frameInfo->height);
        frame->vFrame->unmap();
    }
}

}
