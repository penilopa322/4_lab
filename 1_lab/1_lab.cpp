#define GLEW_DLL
#define GLFW_DLL
#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

class Shader {
public:
    GLuint ID;
    Shader(const char* vertexPath, const char* fragmentPath) {
        std::string vertexCode, fragmentCode;
        std::ifstream vFile, fFile;
        vFile.open(vertexPath);
        fFile.open(fragmentPath);
        std::stringstream vStream, fStream;
        vStream << vFile.rdbuf();
        fStream << fFile.rdbuf();
        vFile.close();
        fFile.close();
        vertexCode = vStream.str();
        fragmentCode = fStream.str();
        const char* vCode = vertexCode.c_str();
        const char* fCode = fragmentCode.c_str();
        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &vCode, NULL);
        glCompileShader(vs);
        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &fCode, NULL);
        glCompileShader(fs);
        ID = glCreateProgram();
        glAttachShader(ID, vs);
        glAttachShader(ID, fs);
        glLinkProgram(ID);
        glDeleteShader(vs);
        glDeleteShader(fs);
    }
    void use() {
        glUseProgram(ID);
    }
    void setVec4(const char* name, float x, float y, float z, float w) {
        glUniform4f(glGetUniformLocation(ID, name), x, y, z, w);
    }
    void setMat4(const char* name, const glm::mat4& mat) {
        glUniformMatrix4fv(glGetUniformLocation(ID, name), 1, GL_FALSE, glm::value_ptr(mat));
    }
};

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float yaw = -90.0f;
float pitch = 0.0f;
bool firstMouse = true;
float lastX = 400.0f;
float lastY = 400.0f;

void mouseCallback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;
    yaw += xoffset;
    pitch += yoffset;
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

int main() {
    GLFWwindow* window;
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not start GLFW3.\n");
        return 1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(800, 800, "Mainwindow", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    GLenum ret = glewInit();
    if (GLEW_OK != ret) {
        fprintf(stderr, "ERROR: %s\n", glewGetErrorString(ret));
        return 1;
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouseCallback);
    float points[] = {
     0.5f,  0.0f, 0.0f,
     0.25f, 0.433f, 0.0f,
    -0.25f, 0.433f, 0.0f,
    -0.5f,  0.0f, 0.0f,
    -0.25f,-0.433f, 0.0f,
     0.25f,-0.433f, 0.0f
    };

    unsigned int indices[] = {
    0, 1, 2,
    0, 2, 3,
    0, 3, 4,
    0, 4, 5
    };
    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    Shader shader("vertex.glsl", "fragment.glsl");

    while (!glfwWindowShouldClose(window)) {
        const float cameraSpeed = 0.01f;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPos += cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPos -= cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 800.0f, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 transform = glm::mat4(1.0f);

        float timeValue = (float)glfwGetTime();
        float red = (cos(timeValue) + 1.0f) / 2.0f;
        float green = (cos(timeValue + 2.0f) + 1.0f) / 2.0f;
        float blue = (cos(timeValue + 4.0f) + 1.0f) / 2.0f;

        shader.use();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        shader.setMat4("model", model);
        shader.setMat4("transform", transform);
        shader.setVec4("ourColor", red, green, blue, 1.0f);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}
