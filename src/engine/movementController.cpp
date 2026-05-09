#include "movementController.hpp"

namespace v
{
    void MovementController::mouseMoved(GLFWwindow *window, double xpos, double ypos, ecs::EntityRegistry &entityRegistry, Entity entity)
    {
        if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL)
        {
            return;
        }

        glm::vec3 rotate{0};

        rotate.y += xpos * sensitivity;
        rotate.x -= ypos * sensitivity;

        // if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS)
        //     rotate.y += 1.f;
        // if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS)
        //     rotate.y -= 1.f;
        // if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS)
        //     rotate.x += 1.f;
        // if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS)
        //     rotate.x -= 1.f;

        if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
        {
            ecs::TransformComponent &transform = entityRegistry.getComponent<ecs::TransformComponent>(entity);
            transform.rotation += sensitivity * rotate;
        }
    }

    void MovementController::scrollMoved(GLFWwindow *window, double yOffset, ecs::EntityRegistry &entityRegistry, Entity entity)
    {
        ecs::TransformComponent &transform = entityRegistry.getComponent<ecs::TransformComponent>(entity);
        transform.translation += 2 * static_cast<float>(yOffset) * glm::vec3{transform.mat4()[2]};
    }

    void MovementController::moveRelative(GLFWwindow *window, float deltaTime, ecs::EntityRegistry &entityRegistry, Entity entity)
    {

        ecs::TransformComponent &transform = entityRegistry.getComponent<ecs::TransformComponent>(entity);

        transform.rotation.x = glm::clamp(transform.rotation.x, -1.5f, 1.5f);
        transform.rotation.y = glm::mod(transform.rotation.y, glm::two_pi<float>());

        float yaw = transform.rotation.y;
        const glm::vec3 forwardDirection{transform.mat4()[2]};
        const glm::vec3 rightDirection = {transform.mat4()[0]};
        const glm::vec3 upDirection = {transform.mat4()[1]};

        glm::vec3 moveDirection{0};

        if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS)
            moveDirection += forwardDirection;
        if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS)
            moveDirection -= forwardDirection;
        if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS)
            moveDirection += upDirection;
        if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS)
            moveDirection -= upDirection;
        if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS)
            moveDirection += rightDirection;
        if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS)
            moveDirection -= rightDirection;

        if (glm::dot(moveDirection, moveDirection) > std::numeric_limits<float>::epsilon())
        {
            transform.translation += moveSpeed * deltaTime * glm::normalize(moveDirection);
        }
    }

    void MovementController::hotkeys(GLFWwindow *window, int &lineMode)
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }

        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && releasedLast)
        {
            lineMode += 1;
            if (lineMode > 2)
            {
                lineMode = 0;
            }
            releasedLast = false;
        }
        else if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE)
        {
            releasedLast = true;
        }

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}