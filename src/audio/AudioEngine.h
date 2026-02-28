
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

    void Update();

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
class CAudioEngine {
public:
    static void Init();
    static void Update();
    static void Shutdown();
    static int ErrorCheck(FMOD_RESULT result);

    void LoadSound(const string& strSoundName, bool b3d = true, bool bLooping = false, bool bStream = false);
    void UnLoadSound(const string& strSoundName);
    int PlaySounds(const string& strSoundName, const Vector3& vPos = Vector3{ 0, 0, 0 }, float fVolumedB = 0.0f);
    void PauseChannel(int nChannelId);
    void ResumeChannel(int nChannelId);
    void SetChannel3dPosition(int nChannelId, const Vector3& vPosition);
    void SetChannelVolume(int nChannelId, float fVolumedB);
    float dbToVolume(float db);
    float VolumeTodB(float volume);
    FMOD_VECTOR VectorToFmod(const Vector3& vPosition);

    // not used?
    void StopChannel(int nChannelId);
    bool IsPlaying(int nChannelId) const;
    void LoadBank(const string& strBankName, FMOD_STUDIO_LOAD_BANK_FLAGS flags);
    void LoadEvent(const string& strEventName);
    void Set3dListenerAndOrientation(const Vector3& vPos = Vector3{ 0, 0, 0 }, float fVolumedB = 0.0f);
    void PlayEvent(const string& strEventName);
    void StopEvent(const string& strEventName, bool bImmediate = false);
    void GetEventParameter(const string& strEventName, const string& strEventParameter, float* parameter);
    void SetEventParameter(const string& strEventName, const string& strParameterName, float fValue);
    void StopAllChannels();
    bool IsEventPlaying(const string& strEventName) const;
};

#endif
