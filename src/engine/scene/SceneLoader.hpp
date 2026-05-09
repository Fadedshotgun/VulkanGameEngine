#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "EntityStore.hpp"
#include "vDevice.hpp"
#include "vDescriptorSetLayout.hpp"
#include "vDescriptorPool.hpp"

namespace scene
{
    class SceneLoader
    {
      public:
        SceneLoader(v::vDevice &device, v::vDescriptorSetLayout &textureSetLayout, v::vDescriptorPool &textureDescriptorPool) : device{device}, textureSetLayout{textureSetLayout}, textureDescriptorPool{textureDescriptorPool} {}

        void loadScene(const std::string &sceneFilePath, ecs::EntityStore &store);

      private:
        v::vDevice &device;
        v::vDescriptorSetLayout &textureSetLayout;
        v::vDescriptorPool &textureDescriptorPool;
        std::filesystem::path sceneDirectory{};

        void loadSceneFromFileStream(std::ifstream &stream, ecs::EntityStore &store);
        void parseComponent(const std::string &typeName, const nlohmann::json &data, ecs::EntityHandle entity);

        glm::vec3 readVec3(const nlohmann::json &data, const char *primaryKey, glm::vec3 fallback);
        std::string convertToAbsolutePath(const std::filesystem::path &sceneDirectory, const std::string &path);
    };
}