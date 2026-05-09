#include "app.hpp"

#include "CameraComponent.hpp"
#include "CameraSystem.hpp"
#include "SceneLoader.hpp"
#include "vTexture.hpp"

#define MAX_FRAME_TIME .1f

namespace v
{
    static int frameCount = 0;
    static float timeSinceLastFrameCount = 0;

    static void theme()
    {
        ImGuiStyle &style = ImGui::GetStyle();
        style.TabRounding = 0;

        ImVec4 *colors = style.Colors;
        colors[ImGuiCol_WindowBg] = ImVec4(0.01f, 0.01f, 0.01f, 1.00f);
        colors[ImGuiCol_Border] = ImVec4(0.06f, 0.06f, 0.06f, 0.50f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.01f, 0.01f, 0.01f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.01f, 0.01f, 0.01f, 1.00f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.53f, 0.53f, 0.53f, 0.67f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.95f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.27f, 0.27f, 0.27f, 0.80f);
        colors[ImGuiCol_Tab] = ImVec4(0.06f, 0.06f, 0.06f, 0.00f);
        colors[ImGuiCol_TabSelected] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
        colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_TabDimmed] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.48f, 0.00f, 0.00f, 0.80f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.80f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_DockingPreview] = ImVec4(0.48f, 0.11f, 0.11f, 0.70f);
    }

    void vApp::updateViewport(auto commandBuffer, ImGuiID dockspaceId)
    {
        ImGuiDockNode *centralNode = ImGui::DockBuilderGetCentralNode(dockspaceId);
        if (centralNode)
        {
            ImVec2 pos = centralNode->Pos;
            ImVec2 size = centralNode->Size;
            ImGuiViewport *viewport = ImGui::GetMainViewport();

            float x = centralNode->Pos.x - viewport->Pos.x;
            float y = centralNode->Pos.y - viewport->Pos.y;
            float w = centralNode->Size.x;
            float h = centralNode->Size.y;

            aspectRatio = w / h;

            renderer.editRenderArea(commandBuffer, glm::vec4(x, y, w, h));
        }
    }

    void vApp::gui()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowStyleEditor();

        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);

        ImGuiWindowFlags dockspaceFlags =
            ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus |
            ImGuiWindowFlags_NoBackground;

        ImGui::Begin("DockSpace", nullptr, dockspaceFlags);
        ImGui::PopStyleVar(2);

        dockspaceId = ImGui::GetID("MainDockSpace");
        ImGui::DockSpace(dockspaceId, ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);

        ImGui::End();

        ImGui::Begin("Scene Hierarchy");

        for (Entity entity : entityRegistry.getEntities())
        {
            std::string name = entityRegistry.getName(entity);

            if (ImGui::Selectable(name.c_str(), false))
            {
                std::cout << "Selected entity " << name << "\n";
            }
        }

        ImGui::End();

        ImGui::Begin("Debug");
        ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
        ImGui::End();
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
    }

    vApp::~vApp()
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void vApp::initImGui()
    {
        imguiDescriptorPool = vDescriptorPool::Builder{device}
                                  .setMaxSets(1000)
                                  .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
                                  .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, 100)
                                  .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100)
                                  .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100)
                                  .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100)
                                  .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100)
                                  .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100)
                                  .build();

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForVulkan(window.getGLFWwindow(), true);

        ImGui_ImplVulkan_InitInfo initInfo{};
        initInfo.Instance = device.getInstance();
        initInfo.PhysicalDevice = device.getPhysicalDevice();
        initInfo.Device = device.device();
        initInfo.Queue = device.graphicsQueue();
        initInfo.QueueFamily = device.getGraphicsQueueFamily();
        initInfo.DescriptorPool = imguiDescriptorPool->getDescriptorPool();
        initInfo.MinImageCount = vSwapChain::MAX_FRAMES_IN_FLIGHT;
        initInfo.ImageCount = vSwapChain::MAX_FRAMES_IN_FLIGHT;

        ImGui::GetStyle().WindowMenuButtonPosition = ImGuiDir_None;

        ImGui_ImplVulkan_PipelineInfo pipelineInfo{};
        pipelineInfo.RenderPass = renderer.getSwapChainRenderPass();

        initInfo.PipelineInfoMain = pipelineInfo;

        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        ImGui_ImplVulkan_Init(&initInfo);

        theme();
    }

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

        initImGui();
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

        while (!window.shouldClose())
        {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            frameTime = glm::min(frameTime, MAX_FRAME_TIME); // CAP MINIMUM FPS TO 10
            float simulationFrameTime = frameTime;

            if (glfwGetKey(window.getGLFWwindow(), GLFW_KEY_R) == GLFW_PRESS)
            {
                simulationFrameTime = 0;
            }

            movementController.moveRelative(window.getGLFWwindow(), frameTime, entityRegistry, cameraEntity);
            movementController.mouseMoved(window.getGLFWwindow(), window.mouseMovementX, window.mouseMovementY, entityRegistry, cameraEntity);
            movementController.scrollMoved(window.getGLFWwindow(), window.scrollY, entityRegistry, cameraEntity);
            movementController.hotkeys(window.getGLFWwindow(), renderMode);

            ecs::CameraSystem::update(entityRegistry, aspectRatio);

            if (auto commandBuffer = renderer.beginFrame())
            {
                int frameIndex = renderer.getFrameIndex();
                FrameInfo frameInfo{frameIndex, simulationFrameTime, commandBuffer, currentCamera, globalDescriptorSets[frameIndex], entityRegistry, renderMode};

                gui();

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

                updateViewport(commandBuffer, dockspaceId);

                // if (glfwGetKey(window.getGLFWwindow(), GLFW_KEY_H) == GLFW_PRESS)
                // {
                //     minus = 50;
                //     std::cout << "minus: " << minus << "\n";
                //     renderer.editRenderArea(renderer.getCurrentCommandBuffer(), glm::vec4(minus, minus, minus, minus));
                // }

                // render solid before semi transparent
                renderSystem.renderGameObjects(frameInfo);
                pointLightRenderSystem.render(frameInfo);

                ImGui::Render();
                ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

                renderer.endSwapChain(commandBuffer);
                renderer.endFrame();
            }

            window.resetWindowInfo();
        }

        vkDeviceWaitIdle(device.device());
    }

    // void vApp::loadGameObjects()
    // {
    //     auto floor = entityStore.createEntity();
    //     floor.addComponent<ecs::MeshRendererComponent>({vModel::createSharedModelFromFile(
    //         device,
    //         std::string(PROJECT_ROOT) + "models/quad.obj",
    //         std::string(PROJECT_ROOT) + "textures/huhdog.jpg",
    //         *textureSetLayout,
    //         *textureDescriptorPool)});
    //     floor.addComponent<ecs::TransformComponent>({.translation = {0.f, 0.f, 0.f}, .scale = {1.f, 1.f, 1.f}});

    //     auto vase = entityStore.createEntity();
    //     vase.addComponent<ecs::MeshRendererComponent>({vModel::createSharedModelFromFile(
    //         device,
    //         std::string(PROJECT_ROOT) + "models/smooth_vase.obj",
    //         *textureSetLayout,
    //         *textureDescriptorPool)});
    //     vase.addComponent<ecs::TransformComponent>({.translation = {1.f, 0.f, 0.f}, .scale = {1.f, 1.f, 1.f}});

    //     auto vase2 = entityStore.createEntity();
    //     vase2.addComponent<ecs::MeshRendererComponent>({vModel::createSharedModelFromFile(
    //         device,
    //         std::string(PROJECT_ROOT) + "models/flat_vase.obj",
    //         *textureSetLayout,
    //         *textureDescriptorPool)});
    //     vase2.addComponent<ecs::TransformComponent>({.translation = {-1.f, 0.f, 0.f}, .scale = {1.f, 1.f, 1.f}});

    //     // std::string path = std::string(PROJECT_ROOT) + "models/smooth_vase.obj";
    //     // std::shared_ptr<vModel> model = vModel::createModelFromFile(device, path);

    //     // std::string path2 = std::string(PROJECT_ROOT) + "models/flat_vase.obj";
    //     // std::shared_ptr<vModel> model2 = vModel::createModelFromFile(device, path2);

    //     // std::string path3 = std::string(PROJECT_ROOT) + "models/quad.obj";
    //     // std::string path4 = std::string(PROJECT_ROOT) + "textures/huhdog.jpg";
    //     // std::shared_ptr<vModel> model3 = vModel::createModelFromFile(device, path3, path4);
    //     // model3->createTextureDescriptor(*textureSetLayout, *textureDescriptorPool);

    //     // vGameObject gameObject = vGameObject::createGameObject();
    //     // gameObject.model = model;
    //     // gameObject.transform.translation = {1.f, 0.f, 0.f};
    //     // gameObject.transform.scale = {1.f, 1.f, 1.f};

    //     // vGameObject gameObject2 = vGameObject::createGameObject();
    //     // gameObject2.model = model2;
    //     // gameObject2.transform.translation = {-0.f, 0.f, 0.f};
    //     // gameObject2.transform.scale = {1.f, 1.f, 1.f};

    //     // vGameObject floor = vGameObject::createGameObject();
    //     // floor.model = model3;
    //     // floor.transform.translation = {0.f, 0.f, 0.f};
    //     // floor.transform.scale = {1.f, 1.f, 1.f};

    //     std::vector<glm::vec3> lightColors{
    //         {1.f, .1f, .1f},
    //         {.1f, .1f, 1.f},
    //         {.1f, 1.f, .1f},
    //         {1.f, 1.f, .1f},
    //         {.1f, 1.f, 1.f},
    //         {1.f, 1.f, 1.f} //
    //     };

    //     for (int i = 0; i < lightColors.size(); i++)
    //     {
    //         auto rotateLight = glm::rotate(glm::mat4(1.f), (float)i / lightColors.size() * glm::two_pi<float>(), {0.f, 1.f, 0.f});
    //         auto translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));

    //         auto pointLight = entityStore.createEntity();
    //         pointLight.addComponent<ecs::PointLightComponent>({.color = lightColors[i]});
    //         pointLight.addComponent<ecs::TransformComponent>({.translation = translation, .scale = {.1f, .1f, .1f}});
    //     }
    // }
}