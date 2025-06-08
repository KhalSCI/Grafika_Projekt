#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

class Camera {
private:
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;
    
    float radius;
    float theta; // Horizontal angle
    float phi;   // Vertical angle
    
    float min_radius;
    float max_radius;
    float min_phi;
    float max_phi;
    
    bool mouse_pressed;
    double last_mouse_x;
    double last_mouse_y;
    
public:
    Camera();
    ~Camera();
    
    void update(GLFWwindow* window);
    void processMouseInput(GLFWwindow* window, double xpos, double ypos);
    void processScrollInput(GLFWwindow* window, double xoffset, double yoffset);
    void processKeyInput(GLFWwindow* window);
    
    glm::mat4 getViewMatrix() const;
    glm::vec3 getPosition() const { return position; }
    glm::vec3 getTarget() const { return target; }
    
    void setTarget(glm::vec3 new_target) { target = new_target; }
    void setRadius(float new_radius);
};

#endif
