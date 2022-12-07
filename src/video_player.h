#ifndef __PLAYOS_VIDEO_PLAYER_H__
#define __PLAYOS_VIDEO_PLAYER_H__

#include "ui/controller.h"
#include "ui/scene.h"
#include "ui/video_view.h"
#include "player_backend/player.h"
#include "ui/playlist.h"
#include "sound/audio_sink.h"


namespace playos {

class Application;

class VideoPlayer: public Scene, public player::PlayerCallback,
        public Controller::Listener {
public:
    VideoPlayer(Application *app);
    ~VideoPlayer();

    void onEvent(Event &event) override;

    // Set and token the playList owership
    void setPlayList(std::vector<std::string> &playList);
    void onAudioCallback(uint8_t **stream, int len);

protected:
    void onInit(UIContext *context) override;

    // Controller listener
    void onPlay() override;
    void onPause() override;
    void onStop() override;
    void onSeek(int64_t i) override;
    void onSetUri(const std::string &uri) override;
    bool isPlaying() override;
    void onShowPlaylist() override;

protected:
    // Player listener
    void onReady(std::shared_ptr<player::VideoInfo> info) override;
    void onEOS() override;
    void onFrameInfo(std::shared_ptr<player::VideoFrameInfo> frameInfo) override;
    void onFrame(player::Frame *frame) override;

private:
    Application *m_app;
    std::shared_ptr<player::VideoFrameInfo> m_frameInfo;
    std::vector<std::string> m_playList;
    AudioSink m_audioSink;

    std::unique_ptr<VideoView> m_videoView;
    std::unique_ptr<Controller> m_controller;
    std::unique_ptr<player::Player> m_player;
    std::unique_ptr<Playlist> m_playlistView;
};

}

#endif
