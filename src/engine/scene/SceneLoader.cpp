#include "SceneLoader.hpp"

#include "Components.hpp"
#include "vModel.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace scene
{
    std::string SceneLoader::convertToAbsolutePath(const std::filesystem::path &sceneDirectory, const std::string &path)
    {
        if (path.empty())
        {
            return path;
        }

        std::filesystem::path filePath{path};
        if (filePath.is_relative())
        {
            filePath = sceneDirectory / filePath;
        }

        return filePath.string();
    }

    glm::vec3 SceneLoader::readVec3(const nlohmann::json &data, const char *primaryKey, glm::vec3 fallback)
    {
        const nlohmann::json *arrayJson = nullptr;

        if (data.contains(primaryKey))
        {
            arrayJson = &data[primaryKey];
        }

        if (arrayJson == nullptr || !arrayJson->is_array() || arrayJson->size() != 3)
        {
            return fallback;
        }

        return {
            (*arrayJson)[0].get<float>(),
            (*arrayJson)[1].get<float>(),
            (*arrayJson)[2].get<float>(),
        };
    }

    void SceneLoader::parseComponent(const std::string &typeName, const nlohmann::json &data, ecs::EntityRegistry &registry, Entity entity)
    {
        if (typeName == "Transform")
        {
            const glm::vec3 translation = readVec3(data, "translation", {0.f, 0.f, 0.f});
            const glm::vec3 rotation = readVec3(data, "rotation", {0.f, 0.f, 0.f});
            const glm::vec3 scale = readVec3(data, "scale", {1.f, 1.f, 1.f});

            registry.addComponent<ecs::TransformComponent>(entity, {translation, scale, rotation});
        }
        else if (typeName == "Camera")
        {
            ecs::CameraComponent camera{};
            camera.fovDegrees = data.value("fovDegrees", camera.fovDegrees);
            camera.nearPlane = data.value("nearPlane", camera.nearPlane);
            camera.farPlane = data.value("farPlane", camera.farPlane);
            camera.active = data.value("active", camera.active);

            registry.addComponent<ecs::CameraComponent>(entity, camera);
        }
        else if (typeName == "MeshRenderer")
        {
            std::string modelPath = data.value("model", std::string{});
            if (modelPath.empty())
            {
                modelPath = data.value("path", std::string{});
            }
            std::string texturePath = data.value("texture", std::string{});
            bool active = data.value("active", true);

            if (modelPath.empty())
            {
                std::cerr << "MeshRenderer component missing model path, skipping\n";
                return;
            }

            try
            {
                std::shared_ptr<v::vModel> model;
                const std::string absoluteModelPath = convertToAbsolutePath(sceneDirectory, modelPath);
                const std::string absoluteTexturePath = convertToAbsolutePath(sceneDirectory, texturePath);
                if (texturePath.empty())
                {
                    model = v::vModel::createSharedModelFromFile(device, absoluteModelPath, textureSetLayout, textureDescriptorPool);
                }
                else
                {
                    model = v::vModel::createSharedModelFromFile(device, absoluteModelPath, absoluteTexturePath, textureSetLayout, textureDescriptorPool);
                }

                registry.addComponent<ecs::MeshRendererComponent>(entity, {model, active});
            }
            catch (const std::exception &error)
            {
                std::cerr << "Failed to load MeshRenderer model '" << modelPath << "': " << error.what() << '\n';
            }
        }
        else if (typeName == "PointLight")
        {
            const glm::vec3 color = readVec3(data, "color", {1.f, 1.f, 1.f});
            const float intensity = data.value("intensity", 1.f);

            registry.addComponent<ecs::PointLightComponent>(entity, {color, intensity});
        }
        else if (typeName == "ParticleEmitter")
        {
            const float emissionRate = data.value("emissionRate", 0.f);
            const glm::vec3 emissionDirection = readVec3(data, "emissionDirection", {0.f, 1.f, 0.f});
            const glm::vec3 color = readVec3(data, "color", {1.f, 1.f, 1.f});
            const float lifetime = data.value("lifetime", 0.f);
            const float speed = data.value("speed", 0.f);
            const float size = data.value("size", 1.f);
            const float angle1 = data.value("angle1", 0.f);

            registry.addComponent<ecs::ParticleEmitterComponent>(entity, {emissionRate, lifetime, speed, size, angle1, emissionDirection, color});
        }
        else
        {
            std::cerr << "Unknown component type: " << typeName << '\n';
        }
    }

    void SceneLoader::loadSceneFromFileStream(std::ifstream &stream, ecs::EntityRegistry &registry)
    {
        nlohmann::json root;
        stream >> root;

        auto entityList = root["entities"];

        for (const auto &entityJson : entityList)
        {
            if (!entityJson.contains("components") || !entityJson["components"].is_array())
            {
                continue;
            }

            const auto &components = entityJson["components"];
            std::string name = entityJson.value("name", std::string{});
            Entity entity = registry.createEntity();
            registry.setName(entity, name);

            for (const auto &compJson : components)
            {
                if (!compJson.contains("type") || !compJson["type"].is_string())
                {
                    continue;
                }

                const std::string typeName = compJson["type"].get<std::string>();
                const nlohmann::json data = compJson.value("data", nlohmann::json::object());
                parseComponent(typeName, data, registry, entity);
            }
        }
    }

    void SceneLoader::loadScene(const std::string &sceneFilePath, ecs::EntityRegistry &registry)
    {
        try
        {
            sceneDirectory = std::filesystem::path{sceneFilePath}.parent_path();
            if (sceneDirectory.empty())
            {
                sceneDirectory = std::filesystem::current_path();
            }

            std::ifstream stream(sceneFilePath);
            if (!stream.is_open())
            {
                throw std::runtime_error("Unable to open scene file: " + sceneFilePath);
            }

            loadSceneFromFileStream(stream, registry);
        }
        catch (const std::exception &e)
        {
            throw std::runtime_error(std::string("Error loading scene: ") + e.what());
        }
    }
}