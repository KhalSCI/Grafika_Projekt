/*
Simple Tree Generation Demo
Stripped down to focus only on tree generation logic.
*/

#define GLM_FORCE_RADIANS

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <cmath>
#include "constants.h"
#include "shaderprogram.h"
#include "tree_simple.h"
#include "camera.h"  // Add camera header
#include "lodepng.h"
#include <iostream> // Include iostream for std::cout and std::endl
#include "myCube.h"
#include "myCube2.h"

// Global variables
float aspectRatio = 1.0f;
ShaderProgram *sp;
Tree tree;
Camera camera;  // Add camera instance
double lastTime = 0.0;
GLuint barkTex;
GLuint leafTex;
GLuint grassTex;
GLuint sunTex;

// Custom ground normals - all pointing up for proper lighting
float groundNormals[] = {
    // All 36 vertices get upward normals (0, 1, 0)
    0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  // Triangle 1
    0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  // Triangle 2
    0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  // Triangle 3
    0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  // Triangle 4
    0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  // Triangle 5
    0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  // Triangle 6
    0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  // Triangle 7
    0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  // Triangle 8
    0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  // Triangle 9
    0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  // Triangle 10
    0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  // Triangle 11
    0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f   // Triangle 12
};

GLuint readTexture(const char* filename) {
   GLuint tex;
   glActiveTexture(GL_TEXTURE0);
   
   std::vector<unsigned char> image;
   unsigned width, height;
   
   unsigned error = lodepng::decode(image, width, height, filename);
   if (error) {
    std::cout << "PNG load error: " << lodepng_error_text(error) << std::endl;
}
   
   glGenTextures(1, &tex);
   glBindTexture(GL_TEXTURE_2D, tex);
   
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*) image.data());
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   
   return tex;
}

// Error handling callback
void error_callback(int error, const char* description) {
    fputs(description, stderr);
}

// Window resize callback
void windowResizeCallback(GLFWwindow* window, int width, int height) {
    if (height == 0) return;
    aspectRatio = (float)width / (float)height;
    glViewport(0, 0, width, height);
}

// Key callback - ESC to exit, camera controls
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }
    }
    
    // Let camera handle key input
    camera.processKeyInput(window);
}

// Mouse callback for camera control
void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    camera.processMouseInput(window, xpos, ypos);
}

// Scroll callback for camera zoom
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.processScrollInput(window, xoffset, yoffset);
}

// Simple render function for geometry
void renderGeometry(const std::vector<GLfloat>& vertices, int vertexCount) {
    if (vertices.empty() || vertexCount == 0) return;
    int stride = 9 * sizeof(GLfloat);
    glEnableVertexAttribArray(sp->a("vertex"));
    glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, stride, vertices.data());
    int texAttrib = sp->a("texcoord");
    if (texAttrib >= 0) {
        glEnableVertexAttribArray(texAttrib);
        glVertexAttribPointer(texAttrib, 2, GL_FLOAT, false, stride, vertices.data() + 4);
    }
    int normAttrib = sp->a("normal");
    if (normAttrib >= 0) {
        glEnableVertexAttribArray(normAttrib);
        glVertexAttribPointer(normAttrib, 3, GL_FLOAT, false, stride, vertices.data() + 6);
    }
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    glDisableVertexAttribArray(sp->a("vertex"));
    if (texAttrib >= 0) glDisableVertexAttribArray(texAttrib);
    if (normAttrib >= 0) glDisableVertexAttribArray(normAttrib);
}

// Initialization
void initOpenGLProgram(GLFWwindow* window) {
    glClearColor(0.5f, 0.7f, 0.9f, 1.0f); // Light blue background
    glEnable(GL_DEPTH_TEST);
    
    glfwSetWindowSizeCallback(window, windowResizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mouseCallback);     // Add mouse callback
    glfwSetScrollCallback(window, scrollCallback);       // Add scroll callback
    
    sp = new ShaderProgram("v_simplest.glsl", NULL, "f_simplest.glsl");
    
    // Initialize tree
    tree.generate();
    barkTex = readTexture("bark.png");
    leafTex = readTexture("leaf.png");
    grassTex = readTexture("grass3.png");
    sunTex = readTexture("sun_yellow.png");
}

// Cleanup
void freeOpenGLProgram(GLFWwindow* window) {
    delete sp;
}

// Main drawing procedure
void drawScene(GLFWwindow* window, float time) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Calculate delta time
    double currentTime = glfwGetTime();
    float deltaTime = (float)(currentTime - lastTime);
    lastTime = currentTime;
    
    // Update camera
    camera.update(window);
    
    // Update tree growth with actual delta time
    tree.updateGrowth(deltaTime);
    
    // Debug: Print growth progress every few seconds
    static double lastDebugTime = 0.0;
    if (currentTime - lastDebugTime > 5.0) {  // Every 5 seconds
        printf("Time: %.1fs, Growth Progress: %.1f%%, Branches visible: %d\n", 
               currentTime, tree.getGrowthProgress() * 100.0f, tree.getBranchVertexCount() / 24);  // Rough branch count
        lastDebugTime = currentTime;
    }
    
    // Set up matrices
    glm::mat4 P = glm::perspective(glm::radians(50.0f), aspectRatio, 1.0f, 50.0f);
    glm::mat4 V = camera.getViewMatrix();
    glm::mat4 M = glm::mat4(1.0f);

    sp->use();
    glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
    glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));
    glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M));

    // --- MOVING SUN LIGHT (DEBUG & DYNAMIC PATH) ---
    float sun_radius = 10.0f;
    float sun_height = 8.0f;
    float sun_speed = 0.25f; // radians per second
    float sun_angle = sun_speed * (float)glfwGetTime();
    // Sun moves in a circular path above the scene
    glm::vec3 sunPos = glm::vec3(sun_radius * cos(sun_angle), sun_height, sun_radius * sin(sun_angle));
    glUniform3fv(sp->u("lightPos"), 1, glm::value_ptr(sunPos));
    
    // Set up texture samplers and bind all textures at once
    glUniform1i(sp->u("textureMap0"), 0); // Bark texture on unit 0
    glUniform1i(sp->u("textureMap1"), 1); // Leaf texture on unit 1
    glUniform1i(sp->u("textureMap2"), 2); // Grass texture on unit 2
    glUniform1i(sp->u("textureMap3"), 3); // Sun texture on unit 3
    
    // Bind all textures at once for all objects
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, barkTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, leafTex);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, grassTex);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, sunTex);
    
    // Debug: Print sun position and light position every second
    static double lastSunDebug = 0.0;
    if (currentTime - lastSunDebug > 1.0) {
        printf("[DEBUG] SunPos: (%.2f, %.2f, %.2f)\n", sunPos.x, sunPos.y, sunPos.z);
        lastSunDebug = currentTime;
    }
    // --- END MOVING SUN LIGHT ---

    // --- DRAW SUN CUBE (MODERN OPENGL) ---
    glm::mat4 sunModel = glm::translate(glm::mat4(1.0f), sunPos) * glm::scale(glm::mat4(1.0f), glm::vec3(0.6f));
    glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(sunModel));
    glUniform1i(sp->u("useBarkTex"), 0);
    glUniform1i(sp->u("useLeafTex"), 0);
    glUniform1i(sp->u("useGroundTex"), 0);
    glUniform1i(sp->u("useSunTex"), 1); // Use sun texture
    
    int sunPosAttrib = sp->a("vertex");
    int sunTexAttrib = sp->a("texcoord");
    int sunNormAttrib = sp->a("normal");
    if (sunPosAttrib >= 0) {
        glEnableVertexAttribArray(sunPosAttrib);
        glVertexAttribPointer(sunPosAttrib, 4, GL_FLOAT, false, 0, myCubeVertices);
    }
    if (sunTexAttrib >= 0) {
        glEnableVertexAttribArray(sunTexAttrib);
        glVertexAttribPointer(sunTexAttrib, 2, GL_FLOAT, false, 0, myCubeTexCoords);
    }
    if (sunNormAttrib >= 0) {
        glEnableVertexAttribArray(sunNormAttrib);
        glVertexAttribPointer(sunNormAttrib, 3, GL_FLOAT, false, 0, myCubeNormals);
    }
    glDrawArrays(GL_TRIANGLES, 0, myCubeVertexCount);
    if (sunPosAttrib >= 0) glDisableVertexAttribArray(sunPosAttrib);
    if (sunTexAttrib >= 0) glDisableVertexAttribArray(sunTexAttrib);
    if (sunNormAttrib >= 0) glDisableVertexAttribArray(sunNormAttrib);
    glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M));
    // --- END DRAW SUN CUBE ---

    
    glm::mat4 groundModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -2.0f, 0.0f))
        * glm::scale(glm::mat4(1.0f), glm::vec3(8.0f, 2.0f, 8.0f));
    glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(groundModel));
    glUniform1i(sp->u("useBarkTex"), 0);
    glUniform1i(sp->u("useLeafTex"), 0);
    glUniform1i(sp->u("useGroundTex"), 1); // Use ground texture
    glUniform1i(sp->u("useSunTex"), 0); // Not using sun texture
    int groundPosAttrib = sp->a("vertex");
    int groundTexAttrib = sp->a("texcoord");
    int groundNormAttrib = sp->a("normal");
    if (groundPosAttrib >= 0) {
        glEnableVertexAttribArray(groundPosAttrib);
        glVertexAttribPointer(groundPosAttrib, 4, GL_FLOAT, false, 0, myCubeVertices);
    }
    if (groundTexAttrib >= 0) {
        glEnableVertexAttribArray(groundTexAttrib);
        glVertexAttribPointer(groundTexAttrib, 2, GL_FLOAT, false, 0, myCubeTexCoords);
    }
    if (groundNormAttrib >= 0) {
        glEnableVertexAttribArray(groundNormAttrib);
        glVertexAttribPointer(groundNormAttrib, 3, GL_FLOAT, false, 0, groundNormals);
    }
    glDrawArrays(GL_TRIANGLES, 0, myCubeVertexCount);
    if (groundPosAttrib >= 0) glDisableVertexAttribArray(groundPosAttrib);
    if (groundTexAttrib >= 0) glDisableVertexAttribArray(groundTexAttrib);
    if (groundNormAttrib >= 0) glDisableVertexAttribArray(groundNormAttrib);
    glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M));
    // --- END DRAW GROUND CUBE ---

    // Render tree branches with bark texture
    glUniform1i(sp->u("useBarkTex"), 1); // Use bark texture
    glUniform1i(sp->u("useLeafTex"), 0); // Not using leaf texture
    glUniform1i(sp->u("useGroundTex"), 0); // Not using ground texture
    glUniform1i(sp->u("useSunTex"), 0); // Not using sun texture
    // Set up attributes: position (4), texcoord (2), normal (3)
    const std::vector<GLfloat>& branchVerts = tree.getBranchVertices();
    int stride = 9 * sizeof(GLfloat);
    int posAttrib = sp->a("vertex");
    int texAttrib = sp->a("texcoord");
    int normAttrib = sp->a("normal");
    if (posAttrib >= 0) {
        glEnableVertexAttribArray(posAttrib);
        glVertexAttribPointer(posAttrib, 4, GL_FLOAT, false, stride, branchVerts.data());
    }
    if (texAttrib >= 0) {
        glEnableVertexAttribArray(texAttrib);
        glVertexAttribPointer(texAttrib, 2, GL_FLOAT, false, stride, branchVerts.data() + 4);
    }
    if (normAttrib >= 0) {
        glEnableVertexAttribArray(normAttrib);
        glVertexAttribPointer(normAttrib, 3, GL_FLOAT, false, stride, branchVerts.data() + 6);
    }
    glDrawArrays(GL_TRIANGLES, 0, tree.getBranchVertexCount());
    if (posAttrib >= 0) glDisableVertexAttribArray(posAttrib);
    if (texAttrib >= 0) glDisableVertexAttribArray(texAttrib);
    if (normAttrib >= 0) glDisableVertexAttribArray(normAttrib);

    // Render tree leaves (9 floats per vertex: position + texcoord + normal)
    glUniform1i(sp->u("useBarkTex"), 0);
    glUniform1i(sp->u("useLeafTex"), 1);
    glUniform1i(sp->u("useGroundTex"), 0);
    glUniform1i(sp->u("useSunTex"), 0); // Not using sun texture
    const std::vector<GLfloat>& leafVerts = tree.getLeafVertices();
    if (posAttrib >= 0) {
        glEnableVertexAttribArray(posAttrib);
        glVertexAttribPointer(posAttrib, 4, GL_FLOAT, false, stride, leafVerts.data());
    }
    if (texAttrib >= 0) {
        glEnableVertexAttribArray(texAttrib);
        glVertexAttribPointer(texAttrib, 2, GL_FLOAT, false, stride, leafVerts.data() + 4);
    }
    if (normAttrib >= 0) {
        glEnableVertexAttribArray(normAttrib);
        glVertexAttribPointer(normAttrib, 3, GL_FLOAT, false, stride, leafVerts.data() + 6);
    }
    glDrawArrays(GL_TRIANGLES, 0, tree.getLeafVertexCount());
    if (posAttrib >= 0) glDisableVertexAttribArray(posAttrib);
    if (texAttrib >= 0) glDisableVertexAttribArray(texAttrib);
    if (normAttrib >= 0) glDisableVertexAttribArray(normAttrib);
    
    glfwSwapBuffers(window);
}

int main(void) {
    GLFWwindow* window;
    
    glfwSetErrorCallback(error_callback);
    
    if (!glfwInit()) {
        fprintf(stderr, "Can't initialize GLFW.\n");
        exit(EXIT_FAILURE);
    }
    
    window = glfwCreateWindow(800, 600, "Tree Generation Demo", NULL, NULL);
    
    if (!window) {
        fprintf(stderr, "Can't create window.\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    
    glfwMakeContextCurrent(window);
    
    glfwSwapInterval(1);
    
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Can't initialize GLEW.\n");
        exit(EXIT_FAILURE);
    }
    
    initOpenGLProgram(window);
    
    glfwSetTime(0);
    lastTime = glfwGetTime();  // Initialize lastTime
    
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        drawScene(window, glfwGetTime());
        glfwPollEvents();
    }
    
    freeOpenGLProgram(window);
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
