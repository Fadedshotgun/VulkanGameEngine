#include "movementController.hpp"

namespace v
{
    void MovementController::mouseMoved(GLFWwindow *window, double xpos, double ypos, vGameObject &gameObject)
    {
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
            gameObject.transform.rotation += sensitivity * rotate;
        }
    }

    void MovementController::scrollMoved(GLFWwindow *window, double yOffset)
    {
        moveSpeed = glm::max(0.0, moveSpeed + (yOffset / 2));
    }

    void MovementController::moveRelative(GLFWwindow *window, float deltaTime, vGameObject &gameObject)
    {

        gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
        gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

        float yaw = gameObject.transform.rotation.y;
        const glm::vec3 forwardDirection{sin(yaw), 0.f, cos(yaw)};
        const glm::vec3 rightDirection = {forwardDirection.z, 0.f, -forwardDirection.x};
        const glm::vec3 upDirection = {0.f, -1.f, 0.f};

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
            gameObject.transform.translation += moveSpeed * deltaTime * glm::normalize(moveDirection);
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
    }
}