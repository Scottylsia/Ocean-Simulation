#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#define  GLFW_INCLUDE_GLU
#define STB_IMAGE_IMPLEMENTATION    
#include "stb_image.h"
#include "Shader.h"
#include "Time.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>

GLFWwindow* initWindow();
GLFWwindow* initFullScreen();

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
unsigned int loadTexture(const char* texturePath, GLenum format);

const int screenWidth = 1920;//ширина экрана
const int screenHeight = 1080;//высота экрана 


const int circlesAmount = 32;//колличество кружков

//манипуляция вращением камеры
float yaw = -90.0f;
float pitch = 0.0f;

//задаем начальную позицию мыши
float lastX = screenWidth / 2.0f;
float lastY = screenHeight / 2.0f;

//проверяем, первый ли раз обрабатываются координаты мыши
bool firstMouse = true;

//Позиция времени
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//устанавливаем позицию/векторы камеры
glm::vec3 cameraPos = glm::vec3(3.0f, 0.0f, 14.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);


int main()
{
    GLFWwindow* window = initFullScreen();
    if (window == NULL)
    {
        return -1;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    float vertexes[] = {
    -0.1f, -0.1f, -0.3f,  0.0f, 0.0f,
     0.1f, -0.1f, -0.3f,  1.0f, 0.0f,
     0.1f,  0.1f, -0.3f,  1.0f, 1.0f,
    };

    //выделяем память под трансформацию объектов
    glm::vec3** vertexesTransformation;
    vertexesTransformation = new glm::vec3* [circlesAmount];
    for (unsigned int i = 0; i < circlesAmount; i++)
    {
        vertexesTransformation[i] = new glm::vec3 [circlesAmount];
    }
    for (unsigned int i = 0; i < circlesAmount; i++)
    {
        for (unsigned int j = 0; j < circlesAmount; j++)
        {
            vertexesTransformation[i][j] = glm::vec3(i + (rand() % 10 - 4) * 0.1f, 0.0f + ((rand() % 10 - 4) * 0.1f), j + (rand() % 10 - 4) * 0.1f);
        }
    }

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexes), vertexes, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //переворачивает изображение вверх ногами
    stbi_set_flip_vertically_on_load(true);
    unsigned int texture1 = loadTexture("circle.png", GL_RGBA);

    Shader shader("shader.vs", "shader.fs", "shader.gs");
    shader.use();
    Time fpsSpeed("");
    glEnable(GL_DEPTH_TEST);
    shader.setInt("texture1", 0);

    
    float *kBig = new float[circlesAmount];
    float *omega = new float[circlesAmount];
    float *amplitude = new float[circlesAmount];
    float *phases = new float[circlesAmount];
    float *alpha = new float[circlesAmount * circlesAmount];
    
    float period = 2.0f;
    float k = 2.0f * 3.14f / 10.0f;
    kBig[0] = 0.3f;
    omega[0] = 0.5f * 3.14f / period;
    amplitude[0] = 0.4f;
    for ( int i = 1; i < circlesAmount; i++) 
    {
             kBig[i] = kBig[i - 1] + 0.01f;
             omega[i] = omega[i - 1];
             amplitude[i] = amplitude[i - 1] + 0.01f;
    }

    for (int i = 0; i < circlesAmount * circlesAmount; i++)
    {
        alpha[i] = (rand() % 100) / 110.0f + 0.2f;
    }


    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);//всключает отслеживание курсора
        processInput(window);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glBindVertexArray(VAO);

        //перспективная матрица
        glm::mat4 projection;
        projection = glm::perspective(glm::radians(45.0f), (float)screenWidth / screenHeight, 0.1f, 100.f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 model(1.0f);
        
        std::map<float, glm::vec4> sorted;
        for (int i = 0; i < circlesAmount; i++)
            for (int j = 0; j < circlesAmount; j++)
            {
                float x = vertexesTransformation[i][j].x;
                float distance = glm::length(cameraPos - vertexesTransformation[i][j]);
                sorted[distance] = glm::vec4(x - (kBig[j] / k) * amplitude[j] * sin(kBig[j] * x - omega[j] * currentFrame), amplitude[j] * cos(kBig[j] * x - omega[j] * currentFrame), vertexesTransformation[i][j].z - (kBig[j] / k) * amplitude[j] * sin(kBig[j] * vertexesTransformation[i][j].z - omega[j] * currentFrame), i * circlesAmount + j);
            }

        for (std::map<float, glm::vec4>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
        {
            glm::mat4 model(1.0f);
            model = glm::translate(model, glm::vec3(it->second.x, it->second.y, it->second.z));
            shader.setMat4("model", model);
            shader.setFloat("alpha", alpha[(int)it->second.w]);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }

        //манипуляция вращением камеры
        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(direction);

        fpsSpeed.CalculateFrameRate(currentFrame);

        shader.setFloat("screenRatio", float(screenWidth) / screenHeight);
        shader.setMat4("projection", projection);
        shader.setFloat3("cameraPos", cameraPos.x, cameraPos.y, cameraPos.z);
        shader.setMat4("view", view);
        shader.use();
        glfwSwapBuffers(window);
        glfwPollEvents();
       
        cameraPos.y += fpsSpeed.getSpeed() * deltaTime;
    
    }
    glfwTerminate();
    return 0;
}

GLFWwindow* initFullScreen()
{   
    glfwInit();
    
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode =  glfwGetVideoMode(monitor);

    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    //создаем окно 
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "My window", monitor, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return window;
    }
   
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    //инициализация glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return window;
    }

    //параметры просмотра окна 
    glViewport(0, 0, screenWidth, screenHeight);

    //callback
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    return window;
}
   
GLFWwindow* initWindow()
{
    //параметры окна
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //создаем окно 
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Project", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return window;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    //инициализация glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return window;
    }

    //параметры просмотра окна 
    glViewport(0, 0, screenWidth, screenHeight);

    //callback
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    return window;
}

//callback
void framebuffer_size_callback(GLFWwindow * window, int width, int height)
{
        glViewport(0, 0, width, height);
}

//управление
void processInput(GLFWwindow* window)
{

    float cameraSpeed = 5.0f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        cameraSpeed *= 10.0f;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        cameraSpeed *= 0.1f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraUp;
    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraUp;
   /* if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
       window=initFullScreen();
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        window=initWindow();
    }*/
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos;//обратно, так как y определяется снизу вверх
    lastX = (float)xpos;
    lastY = (float)ypos;

    const float sensitivity = 0.05f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    //налево вправо, рыскание
    yaw += xoffset;
    //вверх вниз, тангаж
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;
}

unsigned int loadTexture(const char* texturePath, GLenum format)
{
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(texturePath, &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    return texture;
}