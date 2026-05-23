#pragma once

#include <glad/glad.h>
#include <pl_mpeg.h>
#include <string>

class VideoPlayer {
public:
    VideoPlayer();
    ~VideoPlayer();

    bool load(const std::string& filename);
    void update(float dt);

    GLuint getTextureID() const { return textureID_RGB; }
    bool isFinished() const;

    void setLoop(bool loop) { isLooping = loop; }
    void rewind();
    bool wasRewind = false;

    void setPaused(bool paused) { isPaused = paused; }
    bool getPaused() const { return isPaused; }
    void togglePause() { isPaused = !isPaused; }

private:
    plm_t* plm = nullptr;
    GLuint textureID_RGB = 0;
    int width = 0;
    int height = 0;
    bool isPaused = false;
    bool isLooping = false;

    friend void on_video_frame(plm_t* plm, plm_frame_t* frame, void* user);
};
