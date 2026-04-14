#include "AudioEngine.h"

#include "utils/logger.h"

Implementation* sgpImplementation = nullptr;

constexpr const char* kDefaultSoundRegistryPath = "assets/audio/sounds.json";

// Implementation struct methods
//=============================================================================================================//
// Implementation constructor
Implementation::Implementation() {
    mpStudioSystem = NULL;
    mnNextChannelId = 0;
    mJsonRegistryPath = kDefaultSoundRegistryPath;
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
    std::vector<ChannelMap::iterator> pStoppedChannels;
    for (auto it = mChannels.begin(), itEnd = mChannels.end(); it != itEnd; ++it)
    {
        bool bIsPlaying = false;
        it->second->isPlaying(&bIsPlaying);
        bool bIsPaused = false;
        it->second->getPaused(&bIsPaused);

        if (!bIsPlaying && !bIsPaused)
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

bool AudioEngine::loadSoundRegistry(const std::string& manifestPath)
{
    if (!sgpImplementation) {
        return false;
    }

    sgpImplementation->mJsonRegistryLoadAttempted = true;
    sgpImplementation->mJsonRegistryPath = manifestPath;
    sgpImplementation->mJsonRegistryLoaded = false;

    if (!parser::loadSoundRegistry(manifestPath, sgpImplementation->mJsonSounds)) {
        return false;
    }

    sgpImplementation->mJsonRegistryLoaded = true;
    return true;
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
    auto tFoundIt = sgpImplementation->mSounds.find(strSoundName);
    if (tFoundIt == sgpImplementation->mSounds.end())
    {
        loadSound(strSoundName);
        tFoundIt = sgpImplementation->mSounds.find(strSoundName);
        if (tFoundIt == sgpImplementation->mSounds.end())
        {
            return sgpImplementation->mnNextChannelId++;
        }
    }

    return playSoundByName(strSoundName, vPos, fVolumeDB, false);
}

int AudioEngine::playSoundByName(const std::string& strSoundName, const Vector3& vPos, float fVolumeDB, bool startPaused)
{
    int nChannelId = sgpImplementation->mnNextChannelId++;
    auto tFoundIt = sgpImplementation->mSounds.find(strSoundName);
    if (tFoundIt == sgpImplementation->mSounds.end())
    {
        return nChannelId;
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
        AudioEngine::errorCheck(pChannel->setPaused(startPaused));
        sgpImplementation->mChannels[nChannelId] = pChannel;
    }

    return nChannelId;
}

int AudioEngine::jsonSound(const std::string& soundId, bool startPaused)
{
    if (!sgpImplementation) {
        return -1;
    }

    if (!sgpImplementation->mJsonRegistryLoaded && !sgpImplementation->mJsonRegistryLoadAttempted) {
        loadSoundRegistry(sgpImplementation->mJsonRegistryPath.empty() ? kDefaultSoundRegistryPath : sgpImplementation->mJsonRegistryPath);
    }

    auto definitionIt = sgpImplementation->mJsonSounds.find(soundId);
    if (definitionIt == sgpImplementation->mJsonSounds.end()) {
        logger::error("Unknown sound id '{}'", soundId);
        return -1;
    }

    const parser::json::SoundDefinition& definition = definitionIt->second;
    loadSound(definition.filePath, definition.is3d, definition.looping, definition.stream);

    return playSoundByName(definition.filePath, Vector3{ 0, 0, 0 }, definition.defaultVolumeDb, startPaused);
}

int AudioEngine::jsonSound(const std::string& soundId, const Vector3& vPos, bool startPaused)
{
    if (!sgpImplementation) {
        return -1;
    }

    if (!sgpImplementation->mJsonRegistryLoaded && !sgpImplementation->mJsonRegistryLoadAttempted) {
        loadSoundRegistry(sgpImplementation->mJsonRegistryPath.empty() ? kDefaultSoundRegistryPath : sgpImplementation->mJsonRegistryPath);
    }

    auto definitionIt = sgpImplementation->mJsonSounds.find(soundId);
    if (definitionIt == sgpImplementation->mJsonSounds.end()) {
        logger::error("Unknown sound id '{}'", soundId);
        return -1;
    }

    const parser::json::SoundDefinition& definition = definitionIt->second;
    loadSound(definition.filePath, definition.is3d, definition.looping, definition.stream);

    return playSoundByName(definition.filePath, vPos, definition.defaultVolumeDb, startPaused);
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

// based on given channel, stop playing the sound
void AudioEngine::stopChannel(int nChannelId)
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
        // stop audio channel
        AudioEngine::errorCheck(pChannel->stop());
        // remove from channel map
        sgpImplementation->mChannels.erase(tFoundIt);
    }
}

void AudioEngine::restartSound(int nChannelId) {
    auto tFoundIt = sgpImplementation->mChannels.find(nChannelId);
    if (tFoundIt != sgpImplementation->mChannels.end()) {
        AudioEngine::errorCheck(tFoundIt->second->setPosition(0, FMOD_TIMEUNIT_MS));
        AudioEngine::errorCheck(tFoundIt->second->setPaused(false));
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
    (void)fVolumeDB;
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
        logger::error("FMOD ERROR {}", static_cast<int>(result));
        return 1;
    }
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
