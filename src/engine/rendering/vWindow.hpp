#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <functional>
#include <iostream>
#include <string>

namespace v
{

    class vWindow
    {
      public:
        vWindow(int w, int h, std::string title);
        ~vWindow();

        bool shouldClose() { return glfwWindowShouldClose(window); };
        VkExtent2D getExtent() { return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; };
        bool wasFrameBufferResized() { return frameBufferResized; };
        void resetResizedFlag() { frameBufferResized = false; };

        void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

        void resetWindowInfo();

        GLFWwindow *getGLFWwindow() const { return window; };

        vWindow(const vWindow &) = delete;
        vWindow &operator=(const vWindow &) = delete;

        double mouseX{0}, mouseY{0};
        double scrollY{0};
        double mouseMovementX{0}, mouseMovementY{0};
        bool firstMouse{true};

      private:
        void initWindow();
        static void cursorCallback(GLFWwindow *window, double posx, double posy);
        static void scrollCallback(GLFWwindow *window, double posx, double posy);
        static void frameBufferResizeCallback(GLFWwindow *window, int width, int height);

        int width;
        int height;
        bool frameBufferResized = false;

        std::string windowTitle;
        GLFWwindow *window;
    };
}