#ifndef __PLAYOS_VIDEO_PLAYER_H__
#define __PLAYOS_VIDEO_PLAYER_H__

#include "ui/controller.h"
#include "ui/scene.h"
#include "ui/video_view.h"
#include "player/player.h"

namespace playos {

class Application;

class VideoPlayer: public Scene, public player::PlayerCallback {
public:
    VideoPlayer(Application *app);
    ~VideoPlayer();

    void onEvent(Event &event) override;

protected:
    void onInit(UIContext *context) override;

protected:
    void onFrameInfo(std::shared_ptr<player::VideoFrameInfo> frameInfo) override;
    void onFrame(std::shared_ptr<player::Frame> frame) override;

private:
    Application *m_app;
    std::shared_ptr<Texture> m_texture;
    std::shared_ptr<player::VideoFrameInfo> m_frameInfo;

    std::unique_ptr<VideoView> m_videoView;
    std::unique_ptr<Controller> m_controller;
    std::unique_ptr<player::Player> m_player;
};

}

#endif
