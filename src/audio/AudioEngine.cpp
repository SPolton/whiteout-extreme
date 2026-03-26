#include "AudioEngine.h"

// Implementation struct methods
//=============================================================================================================//
// Implementation constructor
Implementation::Implementation() {
    mpStudioSystem = NULL;
    mnNextChannelId = 0;
    // check that all FMOD calls are successful
    AudioEngine::errorCheck(FMOD::Studio::System::create(&mpStudioSystem));
    AudioEngine::errorCheck(mpStudioSystem->initialize(32, FMOD_STUDIO_INIT_LIVEUPDATE, FMOD_INIT_PROFILE_ENABLE, NULL));
    
    mpSystem = NULL;
    AudioEngine::errorCheck(mpStudioSystem->getCoreSystem(&mpSystem));
}


// implementation deconstructor, cleans up FMOD
Implementation::~Implementation() {
    AudioEngine::errorCheck(mpStudioSystem->unloadAll());
    AudioEngine::errorCheck(mpStudioSystem->release());
}

// checks if channel has stop playing, then destroy
// updates event sounds
void Implementation::update() {
    vector<ChannelMap::iterator> pStoppedChannels;
    for (auto it = mChannels.begin(), itEnd = mChannels.end(); it != itEnd; ++it)
    {
        bool bIsPlaying = false;
        it->second->isPlaying(&bIsPlaying);
        if (!bIsPlaying)
        {
            pStoppedChannels.push_back(it);
        }
    }
    for (auto& it : pStoppedChannels)
    {
        mChannels.erase(it);
    }
    AudioEngine::errorCheck(mpStudioSystem->update());
    AudioEngine::errorCheck(mpSystem->update());
}

Implementation* sgpImplementation = nullptr;

// Audio Engine methods
//=============================================================================================================//
void AudioEngine::init() {
    if (sgpImplementation) {
        return;
    }
    sgpImplementation = new Implementation;
}

void AudioEngine::update() {
    sgpImplementation->update();
}

// to load sounds with filename
void AudioEngine::loadSound(const std::string& strSoundName, bool b3d, bool bLooping, bool bStream)
{
    auto tFoundIt = sgpImplementation->mSounds.find(strSoundName);
    if (tFoundIt != sgpImplementation->mSounds.end())
        return;

    FMOD_MODE eMode = FMOD_DEFAULT;
    eMode |= b3d ? FMOD_3D : FMOD_2D;
    eMode |= bLooping ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
    eMode |= bStream ? FMOD_CREATESTREAM : FMOD_CREATECOMPRESSEDSAMPLE;

    FMOD::Sound* pSound = nullptr;
    AudioEngine::errorCheck(sgpImplementation->mpSystem->createSound(strSoundName.c_str(), eMode, nullptr, &pSound));
    if (pSound) {
        sgpImplementation->mSounds[strSoundName] = pSound;
    }
}

// sound unloader to free memory
void AudioEngine::unLoadSound(const std::string& strSoundName)
{
    auto tFoundIt = sgpImplementation->mSounds.find(strSoundName);
    if (tFoundIt == sgpImplementation->mSounds.end())
        return;

    AudioEngine::errorCheck(tFoundIt->second->release());
    sgpImplementation->mSounds.erase(tFoundIt);
}

// check if sound exists, if not, then load
// find open channel and play sound
int AudioEngine::playSounds(const std::string& strSoundName, const Vector3& vPos, float fVolumeDB)
{
    int nChannelId = sgpImplementation->mnNextChannelId++;
    auto tFoundIt = sgpImplementation->mSounds.find(strSoundName);
    if (tFoundIt == sgpImplementation->mSounds.end())
    {
        loadSound(strSoundName);
        tFoundIt = sgpImplementation->mSounds.find(strSoundName);
        if (tFoundIt == sgpImplementation->mSounds.end())
        {
            return nChannelId;
        }
    }
    FMOD::Channel* pChannel = nullptr;
    AudioEngine::errorCheck(sgpImplementation->mpSystem->playSound(tFoundIt->second, nullptr, true, &pChannel));
    if (pChannel)
    {
        FMOD_MODE currMode;
        tFoundIt->second->getMode(&currMode);
        if (currMode & FMOD_3D) {
            FMOD_VECTOR position = vectorToFmod(vPos);
            AudioEngine::errorCheck(pChannel->set3DAttributes(&position, nullptr));
        }
        AudioEngine::errorCheck(pChannel->setVolume(dbToVolume(fVolumeDB)));
        AudioEngine::errorCheck(pChannel->setPaused(false));
        sgpImplementation->mChannels[nChannelId] = pChannel;
    }
    return nChannelId;
}

// based on given channel, pauses the sound
void AudioEngine::pauseChannel(int nChannelId)
{
    // initialize var for pointer
    FMOD::Channel* pChannel = nullptr;

    // look for channel with id
    auto tFoundIt = sgpImplementation->mChannels.find(nChannelId);
    // if found and not at end of map
    if (tFoundIt != sgpImplementation->mChannels.end())
    {
        // retrieve pointer
        pChannel = tFoundIt->second;
    }

    // if channel exists
    if (pChannel)
    {
        // pause audio
        AudioEngine::errorCheck(pChannel->setPaused(true));
    }
}

// based on given channel, resume playing the sound
void AudioEngine::resumeChannel(int nChannelId)
{
    // initialize var for pointer
    FMOD::Channel* pChannel = nullptr;

    // look for channel with id
    auto tFoundIt = sgpImplementation->mChannels.find(nChannelId);
    // if found and not at end of map
    if (tFoundIt != sgpImplementation->mChannels.end())
    {
        // retrieve pointer
        pChannel = tFoundIt->second;
    }

    // if channel exists
    if (pChannel)
    {
        // resume audio
        AudioEngine::errorCheck(pChannel->setPaused(false));
    }
}

// set volume and position of sound
void AudioEngine::setChannel3dPosition(int nChannelId, const Vector3& vPos)
{
    auto tFoundIt = sgpImplementation->mChannels.find(nChannelId);
    if (tFoundIt == sgpImplementation->mChannels.end())
        return;

    FMOD_VECTOR position = vectorToFmod(vPos);
    AudioEngine::errorCheck(tFoundIt->second->set3DAttributes(&position, NULL));
}

// set listener position
void AudioEngine::set3dListenerAndOrientation(const Vector3& vPos, float fVolumeDB)
{
    FMOD_VECTOR position = vectorToFmod(vPos);
};

void AudioEngine::setChannelVolume(int nChannelId, float fVolumeDB)
{
    auto tFoundIt = sgpImplementation->mChannels.find(nChannelId);
    if (tFoundIt == sgpImplementation->mChannels.end())
        return;

    AudioEngine::errorCheck(tFoundIt->second->setVolume(dbToVolume(fVolumeDB)));
}

// Helper functions (converters and error checking)
//=============================================================================================================//
FMOD_VECTOR AudioEngine::vectorToFmod(const Vector3& vPos) {
    FMOD_VECTOR fVec;
    fVec.x = vPos.x;
    fVec.y = vPos.y;
    fVec.z = vPos.z;
    return fVec;
}

float  AudioEngine::dbToVolume(float dB)
{
    return powf(10.0f, 0.05f * dB);
}

float  AudioEngine::volumeToDB(float volume)
{
    return 20.0f * log10f(volume);
}

int AudioEngine::errorCheck(FMOD_RESULT result) {
    if (result != FMOD_OK) {
        cout << "FMOD ERROR " << result << endl;
        return 1;
    }
    // cout << "FMOD all good" << endl;
    return 0;
}

// engine shutdown
void AudioEngine::shutdown() {
    if (!sgpImplementation) {
        return;
    }
    delete sgpImplementation;
    sgpImplementation = nullptr;
}
