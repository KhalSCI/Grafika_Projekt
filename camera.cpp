#include "camera.h"
#include "constants.h"
#include <algorithm>
#include <cmath>

Camera::Camera() : target(0.0f, 1.0f, 0.0f), up(0.0f, 1.0f, 0.0f),
                   radius(8.0f), theta(0.0f), phi(PI/4.0f),
                   min_radius(2.0f), max_radius(20.0f),
                   min_phi(PI/8.0f), max_phi(PI*3.0f/4.0f),
                   mouse_pressed(false), last_mouse_x(0.0), last_mouse_y(0.0) {
    // Calculate initial position
    position.x = target.x + radius * sin(phi) * cos(theta);
    position.y = target.y + radius * cos(phi);
    position.z = target.z + radius * sin(phi) * sin(theta);
}

Camera::~Camera() {
}

void Camera::update(GLFWwindow* window) {
    processKeyInput(window);
    
    // Update position based on spherical coordinates
    position.x = target.x + radius * sin(phi) * cos(theta);
    position.y = target.y + radius * cos(phi);
    position.z = target.z + radius * sin(phi) * sin(theta);
}

void Camera::processMouseInput(GLFWwindow* window, double xpos, double ypos) {
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if (!mouse_pressed) {
            mouse_pressed = true;
            last_mouse_x = xpos;
            last_mouse_y = ypos;
        } else {
            double dx = xpos - last_mouse_x;
            double dy = ypos - last_mouse_y;
            
            theta += dx * 0.01f;
            phi += dy * 0.01f;
            
            // Clamp phi to prevent flipping
            phi = std::max(min_phi, std::min(max_phi, phi));
            
            last_mouse_x = xpos;
            last_mouse_y = ypos;
        }
    } else {
        mouse_pressed = false;
    }
}

void Camera::processScrollInput(GLFWwindow* window, double xoffset, double yoffset) {
    radius -= yoffset * 0.5f;
    radius = std::max(min_radius, std::min(max_radius, radius));
}

void Camera::processKeyInput(GLFWwindow* window) {
    float speed = 0.05f;
    
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        radius -= speed;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        radius += speed;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        theta -= speed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        theta += speed;
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        phi -= speed;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        phi += speed;
    }
    
    // Clamp values
    radius = std::max(min_radius, std::min(max_radius, radius));
    phi = std::max(min_phi, std::min(max_phi, phi));
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, target, up);
}

void Camera::setRadius(float new_radius) {
    radius = std::max(min_radius, std::min(max_radius, new_radius));
}
