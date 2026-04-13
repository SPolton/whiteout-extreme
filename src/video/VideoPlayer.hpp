#pragma once
#include <string>
#include <glad/glad.h>
#include "../libraries/pl_mpeg/pl_mpeg.h"

class VideoPlayer {
public:
    VideoPlayer();
    ~VideoPlayer();

    bool load(const std::string& filename);
    void update(float dt);

    GLuint getTextureID() const { return textureID_RGB; }
    bool isFinished() const;

private:
    plm_t* plm = nullptr;
    GLuint textureID_RGB = 0;
    int width = 0;
    int height = 0;

    friend void on_video_frame(plm_t* plm, plm_frame_t* frame, void* user);
};
