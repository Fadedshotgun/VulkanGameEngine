#include "editorUI.hpp"
#include "theme.hpp"

#include "Components.hpp"
#include "vDescriptorSetLayout.hpp"
#include "vDescriptorWriter.hpp"
#include "vSwapChain.hpp"
#include "vTexture.hpp"

#include <cstring>
#include <string>

namespace editor
{

    EditorUI::EditorUI(v::vDevice &device, v::vWindow &window, v::vRenderer &renderer, v::vTextureManager &textureManager) : renderer(renderer), textureManager(textureManager)
    {
        imguiDescriptorPool = v::vDescriptorPool::Builder{device}
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
        if (!ImGui_ImplGlfw_InitForVulkan(window.getGLFWwindow(), true))
        {
            throw std::runtime_error("Failed to initialize ImGui GLFW backend");
        }

        ImGui_ImplVulkan_InitInfo initInfo{};
        initInfo.Instance = device.getInstance();
        initInfo.PhysicalDevice = device.getPhysicalDevice();
        initInfo.Device = device.device();
        initInfo.Queue = device.graphicsQueue();
        initInfo.QueueFamily = device.getGraphicsQueueFamily();
        initInfo.DescriptorPool = imguiDescriptorPool->getDescriptorPool();
        initInfo.MinImageCount = v::vSwapChain::MAX_FRAMES_IN_FLIGHT;
        initInfo.ImageCount = v::vSwapChain::MAX_FRAMES_IN_FLIGHT;

        ImGui::GetStyle().WindowMenuButtonPosition = ImGuiDir_None;

        ImGui_ImplVulkan_PipelineInfo pipelineInfo{};
        pipelineInfo.RenderPass = renderer.getSwapChainRenderPass();

        initInfo.PipelineInfoMain = pipelineInfo;

        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        if (!ImGui_ImplVulkan_Init(&initInfo))
        {
            throw std::runtime_error("Failed to initialize ImGui Vulkan backend");
        }

        textureManager.loadTexture(std::string(PROJECT_ROOT) + "textures/FOLDER.png");

        editor::applyTheme();
    }

    EditorUI::~EditorUI()
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void EditorUI::updateView(VkCommandBuffer commandBuffer, float &aspectRatio)
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

    void EditorUI::render(VkCommandBuffer commandBuffer)
    {
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    }

    void EditorUI::inspector(ecs::EntityRegistry &entityRegistry)
    {

        ImGui::Begin("Inspector");

        if (currentlySelectedEntity == 0)
        {
            ImGui::End();
            return;
        }

        ImGui::AlignTextToFramePadding();
        ImGui::Text("Name");
        ImGui::SameLine();

        const char *og = entityRegistry.getName(currentlySelectedEntity).c_str();
        std::string str(og);
        static char text[30] = "";
        std::strncpy(text, str.c_str(), IM_ARRAYSIZE(text) - 1);

        text[IM_ARRAYSIZE(text) - 1] = '\0';
        if (ImGui::InputText("##Name", text, IM_ARRAYSIZE(text), ImGuiInputTextFlags_EnterReturnsTrue))
        {
            entityRegistry.setName(currentlySelectedEntity, text);
        }

        ImGui::Separator();

        auto transformComponent = entityRegistry.tryGetComponent<ecs::TransformComponent>(currentlySelectedEntity);
        if (transformComponent != nullptr)
        {
            if (ImGui::CollapsingHeader("Transform"))
            {
                ImGui::Text("Position");
                ImGui::PushItemWidth(-1.0f);
                ImGui::DragFloat3("##Position", &transformComponent->translation.x, 0.1f);
                ImGui::PopItemWidth();

                ImGui::Text("Rotation");
                ImGui::PushItemWidth(-1.0f);
                ImGui::DragFloat3("##Rotation", &transformComponent->rotation.x, 0.1f);
                ImGui::PopItemWidth();

                ImGui::Text("Scale");
                ImGui::PushItemWidth(-1.0f);
                ImGui::DragFloat3("##Scale", &transformComponent->scale.x, 0.1f);
                ImGui::PopItemWidth();
            }
        }

        auto pointLightComponent = entityRegistry.tryGetComponent<ecs::PointLightComponent>(currentlySelectedEntity);
        if (pointLightComponent != nullptr)
        {
            if (ImGui::CollapsingHeader("Point Light"))
            {
                ImGui::Text("Color");
                ImGui::PushItemWidth(-1.0f);
                ImGui::ColorEdit3("##Color", &pointLightComponent->color.x);
                ImGui::PopItemWidth();

                ImGui::Text("Intensity");
                ImGui::PushItemWidth(-1.0f);
                ImGui::DragFloat("##Intensity", &pointLightComponent->intensity, .1f, 1.f, 0.f);
                ImGui::PopItemWidth();
            }
        }

        auto meshRendererComponent = entityRegistry.tryGetComponent<ecs::MeshRendererComponent>(currentlySelectedEntity);
        if (meshRendererComponent != nullptr)
        {
            if (ImGui::CollapsingHeader("Mesh Renderer"))
            {
                ImGui::Text("Model: %s", meshRendererComponent->model ? meshRendererComponent->model->mypath.c_str() : "None");
                ImGui::Checkbox("Active", &meshRendererComponent->active);
            }
        }

        auto particleEmitterComponent = entityRegistry.tryGetComponent<ecs::ParticleEmitterComponent>(currentlySelectedEntity);
        if (particleEmitterComponent != nullptr)
        {
            if (ImGui::CollapsingHeader("Particle Emitter"))
            {
                ImGui::Text("Emission Rate");
                ImGui::PushItemWidth(-1.0f);
                ImGui::DragFloat("##EmissionRate", &particleEmitterComponent->emissionRate, .1f, 0.f);
                ImGui::PopItemWidth();

                ImGui::Text("Emission Direction");
                ImGui::PushItemWidth(-1.0f);
                ImGui::DragFloat3("##EmissionDirection", &particleEmitterComponent->emissionDirection.x, .1f, 0.f);
                ImGui::PopItemWidth();

                ImGui::Text("Color");
                ImGui::PushItemWidth(-1.0f);
                ImGui::ColorEdit3("##Color", &particleEmitterComponent->color.x);
                ImGui::PopItemWidth();

                ImGui::Text("Speed");
                ImGui::PushItemWidth(-1.0f);
                ImGui::DragFloat("##Speed", &particleEmitterComponent->speed, .1f, 0.f);
                ImGui::PopItemWidth();

                ImGui::Text("Size");
                ImGui::PushItemWidth(-1.0f);
                ImGui::DragFloat("##Size", &particleEmitterComponent->size, .1f, 0.f);
                ImGui::PopItemWidth();

                ImGui::Text("Lifetime");
                ImGui::PushItemWidth(-1.0f);
                ImGui::DragFloat("##Lifetime", &particleEmitterComponent->lifetime, .1f, 0.f);
                ImGui::PopItemWidth();

                ImGui::Text("RandomAngle");
                ImGui::PushItemWidth(-1.0f);
                ImGui::DragFloat("##Angle", &particleEmitterComponent->angle1, .1f, 0.f);
                ImGui::PopItemWidth();
            }
        }
        ImGui::End();
    }

    static int id = 0;
    static int itemsInLine = 0;

    // void EditorUI::buttonTest()
    // {
    //     ImGui::BeginGroup();
    //     ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
    //     ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 0.f));
    //     ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.f, 0.f, 0.f, 0.f));
    //     ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(20, ImGui::GetStyle().ItemSpacing.y));

    //     itemsInLine++;
    //     id++;
    //     ImGui::ImageButton(std::to_string(id).c_str(), (ImTextureID)textureManager.getTexture(std::string(PROJECT_ROOT) + "textures/FOLDER.png").first, ImVec2(32, 32));

    //     char *label = (char *)("Folder " + std::to_string(id)).c_str();
    //     float text_width = ImGui::CalcTextSize(label).x;
    //     ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (32 - text_width) * 0.5f);

    //     ImGui::Text(label);
    //     ImGui::EndGroup();
    //     ImGui::PopStyleColor(3);

    //     if (ImGui::IsItemHovered())
    //     {
    //         ImGui::SetTooltip("FOLDER!!!");
    //     }

    //     if (ImGui::IsItemClicked())
    //     {
    //         std::cout << "Clicked!" << std::endl;
    //     }

    //     float spacing = ImGui::GetStyle().ItemSpacing.x + 5;
    //     float availWidth = ImGui::GetContentRegionAvail().x;

    //     if (availWidth > (64 + spacing) * itemsInLine)
    //     {
    //         ImGui::SameLine();
    //     }
    //     else
    //     {
    //         itemsInLine = 0;
    //     }

    //     ImGui::PopStyleVar();
    // }

    void EditorUI::explorer()
    {
        ImGui::Begin("Explorer");

        // id = 0;
        // itemsInLine = 0;
        // buttonTest();

        ImGui::End();
    }

    bool EditorUI::drawUI(ecs::EntityRegistry &entityRegistry, int multiplier)
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
                selectEntity(entity);
            }
        }

        ImGui::End();

        inspector(entityRegistry);

        ImGui::Begin("Debug");
        ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);

        ImGui::Text("State: %s", multiplier == 1 ? "Running" : "Paused");
        ImGui::End();

        explorer();

        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}