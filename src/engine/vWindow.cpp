#include "vWindow.hpp"

namespace v
{
    vWindow::vWindow(int w, int h, std::string title) : width(w), height(h), windowTitle(title)
    {
        initWindow();
    }

    vWindow::~vWindow()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void vWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface)
    {
        glfwCreateWindowSurface(instance, window, nullptr, surface);
    }

    void vWindow::cursorCallback(GLFWwindow *window, double posx, double posy)
    {
        vWindow *v_window = reinterpret_cast<vWindow *>(glfwGetWindowUserPointer(window));

        if (v_window->firstMouse)
        {
            v_window->mouseX = posx;
            v_window->mouseY = posy;
            v_window->firstMouse = false;
        }

        v_window->mouseMovementX = posx - v_window->mouseX;
        v_window->mouseMovementY = posy - v_window->mouseY;

        v_window->mouseX = posx;
        v_window->mouseY = posy;
    }

    void vWindow::scrollCallback(GLFWwindow *window, double posx, double posy)
    {
        vWindow *v_window = reinterpret_cast<vWindow *>(glfwGetWindowUserPointer(window));

        v_window->scrollY = posy;
    }

    void vWindow::initWindow()
    {
        if (!glfwInit())
        {
            throw std::runtime_error("Failed to initialize GLFW");
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window = glfwCreateWindow(width, height, windowTitle.c_str(), nullptr, nullptr);

        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, frameBufferResizeCallback);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        glfwSetCursorPosCallback(window, cursorCallback);
        glfwSetScrollCallback(window, scrollCallback);

        // glfwSetWindowRefreshCallback(window, windowRefreshCallback); UPDATES WHILE RESIZING
    }

    void vWindow::frameBufferResizeCallback(GLFWwindow *window, int width, int height)
    {
        vWindow *v_window = reinterpret_cast<vWindow *>(glfwGetWindowUserPointer(window));
        v_window->frameBufferResized = true;
        v_window->width = width;
        v_window->height = height;
    }

    void vWindow::resetWindowInfo()
    {
        mouseMovementX = 0;
        mouseMovementY = 0;
        scrollY = 0;
    }
}