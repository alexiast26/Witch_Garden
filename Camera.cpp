#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;

        //TODO - Update the rest of camera parameters
        this->cameraFrontDirection = glm::normalize(this->cameraPosition - this->cameraTarget);
        this->cameraRightDirection = -glm::normalize(glm::cross(this->cameraFrontDirection, cameraUp));
        this->cameraUpDirection = glm::cross(this->cameraRightDirection, this->cameraFrontDirection);

    }

    //Getter functions
    glm::vec3 Camera::getCameraPosition() {
        return cameraPosition;
    }
    glm::vec3 Camera::getCameraFront() {
        return cameraFrontDirection;
    }
    glm::vec3 Camera::getCameraRight() {
        return cameraRightDirection;
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        // since cameraFrontDirection points from camera to target and our
        // v = normalize(c-l), we need -v
        glm::vec3 actualTarget = cameraPosition - cameraFrontDirection;
        return glm::lookAt(cameraPosition, actualTarget, cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        //TODO
        glm::vec3 frontHorizontal = glm::vec3(cameraFrontDirection.x, 0.0f, cameraFrontDirection.z);
        switch (direction) {
        case MOVE_FORWARD:
            this->cameraPosition += speed * (-frontHorizontal);
            break;
        case MOVE_BACKWARD:
            this->cameraPosition += speed * frontHorizontal;
            break;
        case MOVE_LEFT:
            this->cameraPosition += speed * (this->cameraRightDirection);
            break;
        case MOVE_RIGHT:
            this->cameraPosition += speed * (-this->cameraRightDirection);
        }
        

    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        //TODO
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
        rotation = glm::rotate(rotation, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));

        glm::vec4 newFront = rotation * glm::vec4(0.0f, 00.0f, -1.0f, 0.0f);
        this->cameraFrontDirection = glm::normalize(glm::vec3(newFront));

        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
        this->cameraUpDirection = glm::cross(this->cameraRightDirection, this->cameraFrontDirection);
    }
}