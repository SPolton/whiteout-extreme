#include "parser.hpp"

#include "utils/logger.h"

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

#include <fstream>
#include <sstream>

namespace parser::json {

    std::string readTextFile(const std::string& filePath)
    {
        std::ifstream input(filePath, std::ios::in | std::ios::binary);
        if (!input.is_open()) {
            return {};
        }

        std::ostringstream buffer;
        buffer << input.rdbuf();
        return buffer.str();
    }

    bool readStringField(const rapidjson::Value& value, const char* fieldName, std::string& outValue)
    {
        if (!value.HasMember(fieldName) || !value[fieldName].IsString()) {
            return false;
        }

        outValue = value[fieldName].GetString();
        return true;
    }

    bool readBoolField(const rapidjson::Value& value, const char* fieldName, bool defaultValue)
    {
        if (!value.HasMember(fieldName) || !value[fieldName].IsBool()) {
            return defaultValue;
        }

        return value[fieldName].GetBool();
    }

    float readFloatField(const rapidjson::Value& value, const char* fieldName, float defaultValue)
    {
        if (!value.HasMember(fieldName) || !value[fieldName].IsNumber()) {
            return defaultValue;
        }

        return static_cast<float>(value[fieldName].GetDouble());
    }

} // namespace parser::json

namespace parser {

    bool loadSoundRegistry(const std::string& manifestPath, json::SoundMap& soundMap)
    {
        soundMap.clear();

        std::string manifestText = json::readTextFile(manifestPath);
        if (manifestText.empty()) {
            logger::error("Failed to open audio json registry '{}'", manifestPath);
            return false;
        }

        rapidjson::Document document;
        document.Parse(manifestText.c_str());
        if (document.HasParseError()) {
            logger::error(
                "Failed to parse audio json in '{}' at offset {}: {}",
                manifestPath,
                document.GetErrorOffset(),
                rapidjson::GetParseError_En(document.GetParseError()));
            return false;
        }

        if (!document.IsObject() || !document.HasMember("sounds") || !document["sounds"].IsArray()) {
            logger::error("Audio json registry '{}' must contain an array field named 'sounds'", manifestPath);
            return false;
        }

        const auto& sounds = document["sounds"].GetArray();
        for (const auto& entry : sounds)
        {
            if (!entry.IsObject()) {
                logger::warning("Skipping non-object sound entry in '{}'", manifestPath);
                continue;
            }

            std::string soundId;
            if (!json::readStringField(entry, "id", soundId)) {
                logger::warning("Sound entry is missing string field 'id'");
                continue;
            }

            if (soundMap.find(soundId) != soundMap.end()) {
                logger::warning("Duplicate sound id '{}'", soundId);
                continue;
            }

            json::SoundDefinition definition;
            if (!json::readStringField(entry, "file", definition.filePath)) {
                logger::warning("Sound '{}' is missing string field 'file'", soundId);
                continue;
            }

            definition.is3d = json::readBoolField(entry, "3d", definition.is3d);
            definition.looping = json::readBoolField(entry, "looping", definition.looping);
            definition.stream = json::readBoolField(entry, "stream", definition.stream);
            definition.defaultVolumeDb = json::readFloatField(entry, "volumeDb", definition.defaultVolumeDb);

            soundMap.emplace(soundId, definition);
        }

        return true;
    }

} // namespace parser
