#include "app.hpp"

#include "CameraComponent.hpp"
#include "CameraSystem.hpp"
#include "SceneLoader.hpp"
#include "editorUI.hpp"
#include "vTexture.hpp"
#include "vTextureManager.hpp"

#include "ParticleEmitterSystem.hpp"

#define MAX_FRAME_TIME .1f

namespace v
{
    static int frameCount = 0;
    static float timeSinceLastFrameCount = 0;

    static bool focusedOnEditor = false;

    vApp::vApp()
    {
        globalDescriptorPool = vDescriptorPool::Builder{device}
                                   .setMaxSets(vSwapChain::MAX_FRAMES_IN_FLIGHT)
                                   .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, vSwapChain::MAX_FRAMES_IN_FLIGHT)
                                   //.setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
                                   .build();

        textureDescriptorPool = vDescriptorPool::Builder{device}
                                    .setMaxSets(128)
                                    .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 128)
                                    .build();

        textureSetLayout = vDescriptorSetLayout::Builder{device}
                               .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                               .build();
    }

    vApp::~vApp() {}

    void vApp::run()
    {
        scene::SceneLoader sceneLoader{device, *textureSetLayout, *textureDescriptorPool};
        sceneLoader.loadScene(std::string(PROJECT_ROOT) + "testScene.json", entityRegistry);

        std::vector<std::unique_ptr<vBuffer>> uniformBuffers{vSwapChain::MAX_FRAMES_IN_FLIGHT};
        for (int i = 0; i < vSwapChain::MAX_FRAMES_IN_FLIGHT; i++)
        {
            uniformBuffers[i] = std::make_unique<vBuffer>(
                device,
                sizeof(UniformBufferObject),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            uniformBuffers[i]->map();
        }

        auto globalSetLayout = vDescriptorSetLayout::Builder{device}
                                   .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                                   .build();

        std::vector<VkDescriptorSet> globalDescriptorSets(vSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (size_t i = 0; i < globalDescriptorSets.size(); i++)
        {
            auto bufferInfo = uniformBuffers[i]->descriptorInfo();
            vDescriptorWriter(*globalSetLayout, *globalDescriptorPool)
                .writeBuffer(0, &bufferInfo)
                .build(globalDescriptorSets[i]);
        }

        const std::string defaultTexturePath = std::string(PROJECT_ROOT) + "textures/white.bmp";

        vTextureManager textureManager{device, *textureSetLayout, *textureDescriptorPool};
        editor::EditorUI editorUI{device, window, renderer, textureManager};

        auto [defaultTextureSet2, defaultTexture2] = textureManager.loadTexture(defaultTexturePath);

        int renderMode = 0;
        DefaultRenderSystem renderSystem = DefaultRenderSystem{device, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout(), textureSetLayout->getDescriptorSetLayout(), defaultTextureSet2};
        PointLightRenderSystem pointLightRenderSystem = PointLightRenderSystem{device, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        ParticleRenderSystem particleRenderSystem = ParticleRenderSystem{device, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};

        auto currentTime = std::chrono::high_resolution_clock::now();

        // initImGui();
        aspectRatio = renderer.getAspectRatio();

        MovementController movementController{};
        Entity cameraEntity;

        entityRegistry.forEach<ecs::CameraComponent>([&](auto entity, auto &cameraComponent)
            {
            if (cameraComponent.active)
            {
                cameraEntity = entity;
            } });

        int minus = 0;

        vCamera &currentCamera = entityRegistry.getComponent<ecs::CameraComponent>(cameraEntity).camera;

        int multiplier = 1;

        while (!window.shouldClose())
        {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            frameTime = glm::min(frameTime, MAX_FRAME_TIME); // CAP MINIMUM FPS TO 10
            float simulationFrameTime = frameTime * multiplier;

            if (!focusedOnEditor)
            {
                movementController.moveRelative(window.getGLFWwindow(), frameTime, entityRegistry, cameraEntity);
                movementController.mouseMoved(window.getGLFWwindow(), window.mouseMovementX, window.mouseMovementY, entityRegistry, cameraEntity);
                movementController.scrollMoved(window.getGLFWwindow(), window.scrollY, entityRegistry, cameraEntity);
                movementController.hotkeys(window.getGLFWwindow(), renderMode, multiplier);
            }

            ecs::CameraSystem::update(entityRegistry, aspectRatio);
            // ecs::updateParticleEmitters(entityRegistry, frameTime);

            if (auto commandBuffer = renderer.beginFrame())
            {
                int frameIndex = renderer.getFrameIndex();
                FrameInfo frameInfo{frameIndex, simulationFrameTime, commandBuffer, currentCamera, globalDescriptorSets[frameIndex], entityRegistry, renderMode};

                focusedOnEditor = editorUI.drawUI(entityRegistry, multiplier);

                // update
                UniformBufferObject uboData{};
                uboData.projection = currentCamera.getProjection();
                uboData.view = currentCamera.getView();
                uboData.inverseView = currentCamera.getInverseView();

                pointLightRenderSystem.update(frameInfo, uboData);

                uniformBuffers[frameIndex]->writeToBuffer(&uboData);
                uniformBuffers[frameIndex]->flush();

                // render
                renderer.beginSwapChain(commandBuffer);

                editorUI.updateView(commandBuffer, aspectRatio);

                // render solid
                renderSystem.renderGameObjects(frameInfo);
                // particleRenderSystem.render(frameInfo);

                // render semi transparent
                pointLightRenderSystem.render(frameInfo);

                editorUI.render(commandBuffer);

                renderer.endSwapChain(commandBuffer);
                renderer.endFrame();
            }

            window.resetWindowInfo();
        }

        vkDeviceWaitIdle(device.device());
    }
}