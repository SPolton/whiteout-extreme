#pragma once

#include <map>
#include <string>

#include "rapidjson/document.h"

namespace parser::json {

    struct SoundDefinition {
        std::string filePath;
        bool is3d = true;
        bool looping = false;
        bool stream = false;
        float defaultVolumeDb = 0.0f;
    };

    using SoundMap = std::map<std::string, SoundDefinition>;

    std::string readTextFile(const std::string& filePath);
    bool readStringField(const rapidjson::Value& value, const char* fieldName, std::string& outValue);
    bool readBoolField(const rapidjson::Value& value, const char* fieldName, bool defaultValue);
    float readFloatField(const rapidjson::Value& value, const char* fieldName, float defaultValue);

} // namespace parser::json


namespace parser {

    bool loadSoundRegistry(const std::string& manifestPath, json::SoundMap& soundMap);

} // namespace parser
