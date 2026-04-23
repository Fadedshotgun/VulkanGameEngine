#include "app.hpp"

#include "vTexture.hpp"

#define MAX_FRAME_TIME .1f

namespace v
{
    static int frameCount = 0;
    static float timeSinceLastFrameCount = 0;

    static void countFps(float frameTime, DefaultRenderSystem &currentRenderSystem)
    {
        timeSinceLastFrameCount += frameTime;
        frameCount += 1;
        if (timeSinceLastFrameCount > 1)
        {
            timeSinceLastFrameCount -= 1;
            std::cout << frameCount << " frames per second\n";
            // std::cout << currentRenderSystem.triangleCount << " triangles drawn last frame\n";
            frameCount = 0;
        }
    }

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

        loadGameObjects();
    }

    vApp::~vApp() {}

    void vApp::run()
    {
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
        VkDescriptorSet defaultTextureSet = VK_NULL_HANDLE;
        auto defaultTexture = std::make_shared<vTexture>(device, defaultTexturePath);
        VkDescriptorImageInfo defaultImageInfo{};
        defaultImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        defaultImageInfo.imageView = defaultTexture->getImageView();
        defaultImageInfo.sampler = defaultTexture->getSampler();

        vDescriptorWriter(*textureSetLayout, *textureDescriptorPool)
            .writeImage(0, &defaultImageInfo)
            .build(defaultTextureSet);

        int renderMode = 0;
        DefaultRenderSystem renderSystem = DefaultRenderSystem{device, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout(), textureSetLayout->getDescriptorSetLayout(), defaultTextureSet};
        PointLightRenderSystem pointLightRenderSystem = PointLightRenderSystem{device, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};

        auto currentTime = std::chrono::high_resolution_clock::now();

        MovementController movementController{};
        vCamera currentCamera{};
        vGameObject cameraObject = vGameObject::createGameObject();
        cameraObject.transform.translation.z = -2.5f;
        cameraObject.transform.translation.y = -1.f;

        while (!window.shouldClose())
        {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            frameTime = glm::min(frameTime, MAX_FRAME_TIME); // CAP MINIMUM FPS TO 10
            countFps(frameTime, renderSystem);

            movementController.moveRelative(window.getGLFWwindow(), frameTime, cameraObject);
            movementController.mouseMoved(window.getGLFWwindow(), window.mouseMovementX, window.mouseMovementY, cameraObject);
            movementController.scrollMoved(window.getGLFWwindow(), window.scrollY);
            movementController.hotkeys(window.getGLFWwindow(), renderMode);

            currentCamera.setViewYXZ(cameraObject.transform.translation, cameraObject.transform.rotation);

            float aspect = renderer.getAspectRatio();
            currentCamera.setPerspectiveProjection(glm::radians(80.f), aspect, .1f, 1000.f);

            if (auto commandBuffer = renderer.beginFrame())
            {
                int frameIndex = renderer.getFrameIndex();
                FrameInfo frameInfo{frameIndex, frameTime, commandBuffer, currentCamera, globalDescriptorSets[frameIndex], entityStore, renderMode};

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
                // render solid before semi transparent
                renderSystem.renderGameObjects(frameInfo);
                pointLightRenderSystem.render(frameInfo);

                renderer.endSwapChain(commandBuffer);
                renderer.endFrame();
            }

            window.resetWindowInfo();
        }

        vkDeviceWaitIdle(device.device());
    }

    void vApp::loadGameObjects()
    {
        auto floor = entityStore.createEntity();
        floor.addComponent<ecs::MeshRendererComponent>({
            vModel::createSharedModelFromFile(
                device,
                std::string(PROJECT_ROOT) + "models/quad.obj",
                std::string(PROJECT_ROOT) + "textures/huhdog.jpg",
                *textureSetLayout,
                *textureDescriptorPool)
        });
        floor.addComponent<ecs::TransformComponent>({.translation = {0.f, 0.f, 0.f}, .scale = {1.f, 1.f, 1.f}});

        auto vase = entityStore.createEntity();
        vase.addComponent<ecs::MeshRendererComponent>({
            vModel::createSharedModelFromFile(
                device,
                std::string(PROJECT_ROOT) + "models/smooth_vase.obj",
                *textureSetLayout,
                *textureDescriptorPool)
        });
        vase.addComponent<ecs::TransformComponent>({.translation = {1.f, 0.f, 0.f}, .scale = {1.f, 1.f, 1.f}});

        auto vase2 = entityStore.createEntity();
        vase2.addComponent<ecs::MeshRendererComponent>({
            vModel::createSharedModelFromFile(
                device,
                std::string(PROJECT_ROOT) + "models/flat_vase.obj",
                *textureSetLayout,
                *textureDescriptorPool)
        });
        vase2.addComponent<ecs::TransformComponent>({.translation = {-1.f, 0.f, 0.f}, .scale = {1.f, 1.f, 1.f}});

        // std::string path = std::string(PROJECT_ROOT) + "models/smooth_vase.obj";
        // std::shared_ptr<vModel> model = vModel::createModelFromFile(device, path);

        // std::string path2 = std::string(PROJECT_ROOT) + "models/flat_vase.obj";
        // std::shared_ptr<vModel> model2 = vModel::createModelFromFile(device, path2);

        // std::string path3 = std::string(PROJECT_ROOT) + "models/quad.obj";
        // std::string path4 = std::string(PROJECT_ROOT) + "textures/huhdog.jpg";
        // std::shared_ptr<vModel> model3 = vModel::createModelFromFile(device, path3, path4);
        // model3->createTextureDescriptor(*textureSetLayout, *textureDescriptorPool);

        // vGameObject gameObject = vGameObject::createGameObject();
        // gameObject.model = model;
        // gameObject.transform.translation = {1.f, 0.f, 0.f};
        // gameObject.transform.scale = {1.f, 1.f, 1.f};

        // vGameObject gameObject2 = vGameObject::createGameObject();
        // gameObject2.model = model2;
        // gameObject2.transform.translation = {-0.f, 0.f, 0.f};
        // gameObject2.transform.scale = {1.f, 1.f, 1.f};

        // vGameObject floor = vGameObject::createGameObject();
        // floor.model = model3;
        // floor.transform.translation = {0.f, 0.f, 0.f};
        // floor.transform.scale = {1.f, 1.f, 1.f};

        std::vector<glm::vec3> lightColors{
            {1.f, .1f, .1f},
            {.1f, .1f, 1.f},
            {.1f, 1.f, .1f},
            {1.f, 1.f, .1f},
            {.1f, 1.f, 1.f},
            {1.f, 1.f, 1.f} //
        };

        for (int i = 0; i < lightColors.size(); i++)
        {
            auto rotateLight = glm::rotate(glm::mat4(1.f), (float)i / lightColors.size() * glm::two_pi<float>(), {0.f, 1.f, 0.f});
            auto translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));

            auto pointLight = entityStore.createEntity();
            pointLight.addComponent<ecs::PointLightComponent>({.color = lightColors[i]});
            pointLight.addComponent<ecs::TransformComponent>({.translation = translation, .scale = {.1f, .1f, .1f}});
        }
    }
}