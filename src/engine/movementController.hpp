#pragma once

#include "vGameObject.hpp"
#include "vWindow.hpp"
#include "EntityStore.hpp"
#include "EntityHandle.hpp"

namespace v
{
    class MovementController
    {
      public:
        struct KeyMappings
        {
            int moveLeft = GLFW_KEY_A;
            int moveRight = GLFW_KEY_D;
            int moveForward = GLFW_KEY_W;
            int moveBackward = GLFW_KEY_S;
            int moveUp = GLFW_KEY_E;
            int moveDown = GLFW_KEY_Q;
            int lookLeft = GLFW_KEY_LEFT;
            int lookRight = GLFW_KEY_RIGHT;
            int lookUp = GLFW_KEY_UP;
            int lookDown = GLFW_KEY_DOWN;
        };

        void scrollMoved(GLFWwindow *window, double yOffset);
        void moveRelative(GLFWwindow *window, float deltaTime, ecs::EntityHandle &entityHandle);
        void mouseMoved(GLFWwindow *window, double xpos, double ypos, ecs::EntityHandle &entityHandle);
        void hotkeys(GLFWwindow *window, int &lineMode);

      private:
        KeyMappings keys{};
        float moveSpeed{3.f};
        float sensitivity{0.1f};
        double lastX, lastY;

        bool releasedLast = true;
    };
}