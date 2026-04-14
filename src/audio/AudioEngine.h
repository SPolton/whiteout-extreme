
#ifndef _AUDIO_ENGINE_H_
#define _AUDIO_ENGINE_H_
// checks if audio engine was defined

#include "fmod_studio.hpp"
#include "fmod.hpp"
#include <string>
#include <map>
#include <vector>
#include <math.h>
#include <iostream>

// make life easier, no need to use std::
using namespace std;

struct Vector3 {
    float x;
    float y;
    float z;
};

// initailizes and shutsdown FMOD
// will have map of all sounds and events
struct Implementation {
    Implementation();
    ~Implementation();

    void update();

    FMOD::Studio::System* mpStudioSystem;
    FMOD::System* mpSystem;

    int mnNextChannelId;

    typedef map<string, FMOD::Sound*> SoundMap;
    typedef map<int, FMOD::Channel*> ChannelMap;
    typedef map<string, FMOD::Studio::EventInstance*> EventMap;
    typedef map<string, FMOD::Studio::Bank*> BankMap;

    BankMap mBanks;
    EventMap mEvents;
    SoundMap mSounds;
    ChannelMap mChannels;
};

// Audio engine
/*
* calls to Implementation struct to start, stop, and update FMOD
* handles basic things like loading, playing, stopping, and updating information on sounds and events
*/
class AudioEngine {
public:
    static void init();
    static void update();
    static void shutdown();
    static int errorCheck(FMOD_RESULT result);

    void loadSound(const string& strSoundName, bool b3d = true, bool bLooping = false, bool bStream = false);
    void unLoadSound(const string& strSoundName);
    int playSounds(const string& strSoundName, const Vector3& vPos = Vector3{ 0, 0, 0 }, float fVolumeDB = 0.0f);
    void pauseChannel(int nChannelId);
    void resumeChannel(int nChannelId);
    void stopChannel(int nChannelId);
    void restartSound(int nChannelId);

    void setChannel3dPosition(int nChannelId, const Vector3& vPos);
    void set3dListenerAndOrientation(const Vector3& vPos = Vector3{ 0, 0, 0 }, float fVolumeDB = 0.0f);
    void setChannelVolume(int nChannelId, float fVolumeDB);
    float dbToVolume(float dB);
    float volumeToDB(float volume);
    FMOD_VECTOR vectorToFmod(const Vector3& vPos);

};

#endif
