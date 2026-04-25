#define PL_MPEG_IMPLEMENTATION
#include "VideoPlayer.hpp"
#include <vector>

void on_video_frame(plm_t* plm, plm_frame_t* frame, void* user) {
    (void)plm;
    VideoPlayer* player = static_cast<VideoPlayer*>(user);

    std::vector<uint8_t> rgb_data(frame->width * frame->height * 3);
    plm_frame_to_rgb(frame, rgb_data.data(), frame->width * 3);

    glBindTexture(GL_TEXTURE_2D, player->getTextureID());
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->width, frame->height, GL_RGB, GL_UNSIGNED_BYTE, rgb_data.data());
}

VideoPlayer::VideoPlayer() {}

VideoPlayer::~VideoPlayer() {
    if (plm) plm_destroy(plm);
    if (textureID_RGB) glDeleteTextures(1, &textureID_RGB);
}

bool VideoPlayer::load(const std::string& filename) {
    plm = plm_create_with_filename(filename.c_str());
    if (!plm) return false;

    width = plm_get_width(plm);
    height = plm_get_height(plm);

    plm_set_video_decode_callback(plm, on_video_frame, this);

    glGenTextures(1, &textureID_RGB);
    glBindTexture(GL_TEXTURE_2D, textureID_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return true;
}

void VideoPlayer::rewind() {
    if (plm) {
        plm_rewind(plm);
        setPaused(false);
        wasRewind = true;
    }
}

void VideoPlayer::update(float dt) {
    if (plm && !isPaused) {
        plm_decode(plm, (double)dt);

        if (isLooping && plm_has_ended(plm)) {
            rewind();
        }
    }
}

bool VideoPlayer::isFinished() const {
    return plm ? plm_has_ended(plm) : true;
}
