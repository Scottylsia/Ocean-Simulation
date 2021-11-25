#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#define GLFW_INCLUDE_GLU
#define STB_IMAGE_IMPLEMENTATION    
#include "stb_image.h"
#include "Shader.h"
#include "Time.h"
#include "Textures.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>
#include <thread>
#include <mutex>

GLFWwindow* initWindow();
GLFWwindow* initFullScreen();

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
float* getVertexes(std::string filePath, int size);
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
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

std::mutex freeze;
std::map<float, glm::vec4> tempSorted;

Time fpsSpeed;

class Circle
{
public:
    glm::vec3** vertexesTransformation;
    float* kBig;
    float* omega;
    float* amplitude;
    float* phases;
    float* alpha;
    float period;
    float k;
    //выделяем память под трансформацию объектов
    Circle()
    {
        vertexesTransformation = new glm::vec3* [circlesAmount];
        for (unsigned int i = 0; i < circlesAmount; i++)
        {
            vertexesTransformation[i] = new glm::vec3[circlesAmount];
        }
        for (unsigned int i = 0; i < circlesAmount; i++)
        {
            for (unsigned int j = 0; j < circlesAmount; j++)
            {
                if (i < circlesAmount / 2)
                {
                    vertexesTransformation[i][j] = glm::vec3(i + (rand() % 10 - 4) * 0.1f, 0.0f + ((rand() % 10 - 4) * 0.1f), j + (rand() % 10 - 4) * 0.1f);
                }
                else
                {
                    vertexesTransformation[i][j] = glm::vec3(i - circlesAmount / 2 + (rand() % 10 - 4) * 0.1f, -1.5f + ((rand() % 10 - 4) * 0.1f), j + (rand() % 10 - 4) * 0.1f);
                }
            }
        }
        kBig = new float[circlesAmount];
        omega = new float[circlesAmount];
        amplitude = new float[circlesAmount];
        phases = new float[circlesAmount];
        alpha = new float[circlesAmount * circlesAmount];
        period = 2.0f;
        k = 2.0f * 3.14f / 10.0f;
        kBig[0] = 0.3f;
        omega[0] = 0.5f * 3.14f / period;
        amplitude[0] = 0.4f;
        for (int i = 1; i < circlesAmount; i++)
        {
            kBig[i] = kBig[i - 1] + 0.01f;
            omega[i] = omega[i - 1];
            amplitude[i] = amplitude[i - 1] + 0.01f;
        }

        for (int i = 0; i < circlesAmount * circlesAmount; i++)
        {
            alpha[i] = (rand() % 100) / 110.0f + 0.2f;
        }
    }

    void calculateOrder()
    {
        while (1) {
            std::map<float, glm::vec4> sorted;
            float currentFrame = (float)glfwGetTime();
            for (int i = 0; i < circlesAmount; i++)
                for (int j = 0; j < circlesAmount; j++)
                {

                    float x = vertexesTransformation[i][j].x;
                    float distance = glm::length(cameraPos - vertexesTransformation[i][j]);
                    sorted[distance] = glm::vec4(x - (kBig[j] / k) * amplitude[j] * sin(kBig[j] * x - omega[j] * currentFrame), // x
                                                 vertexesTransformation[i][j].y + amplitude[j] * cos(kBig[j] * x - omega[j] * currentFrame), // y
                                                 vertexesTransformation[i][j].z - (kBig[j] / k) * amplitude[j] * sin(kBig[j] * vertexesTransformation[i][j].z - omega[j] * currentFrame), // z
                                                 i * circlesAmount + j); // w
                }
            freeze.lock();
            tempSorted = sorted;
            freeze.unlock();
            //std::cout << "completedThreadsFunc" << ++completedThreadsFunc << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 / fpsSpeed.globalFPS));
        }
    }
};

class Skybox {
public:
    unsigned int VAO, VBO;
    Shader shaderSkybox;
    std::vector<std::string> faces
    {
        "skybox/right.jpg",
            "skybox/left.jpg",
            "skybox/top.jpg",
            "skybox/bottom.jpg",
            "skybox/front.jpg",
            "skybox/back.jpg"
    };
    unsigned int textureCubeMap = loadCubemap(faces);

    Skybox() : shaderSkybox("shaderCubeMap.vs", "shaderCubeMap.fs") {
        float cubeMapVertixes[] = { -1.0f, 1.0f, -1.0f,
           -1.0f, -1.0f, -1.0f,
           1.0f, -1.0f, -1.0f,
           1.0f, -1.0f, -1.0f,
           1.0f, 1.0f, -1.0f,
           -1.0f, 1.0f, -1.0f,

           -1.0f, -1.0f, 1.0f,
           -1.0f, -1.0f, -1.0f,
           -1.0f, 1.0f, -1.0f,
           -1.0f, 1.0f, -1.0f,
           -1.0f, 1.0f, 1.0f,
           -1.0f, -1.0f, 1.0f,

           1.0f, -1.0f, -1.0f,
           1.0f, -1.0f, 1.0f,
           1.0f, 1.0f, 1.0f,
           1.0f, 1.0f, 1.0f,
           1.0f, 1.0f, -1.0f,
           1.0f, -1.0f, -1.0f,

           -1.0f, -1.0f, 1.0f,
           -1.0f, 1.0f, 1.0f,
           1.0f, 1.0f, 1.0f,
           1.0f, 1.0f, 1.0f,
           1.0f, -1.0f, 1.0f,
           -1.0f, -1.0f, 1.0f,

           -1.0f, 1.0f, -1.0f,
           1.0f, 1.0f, -1.0f,
           1.0f, 1.0f, 1.0f,
           1.0f, 1.0f, 1.0f,
           -1.0f, 1.0f, 1.0f,
           -1.0f, 1.0f, -1.0f,

           -1.0f, -1.0f, -1.0f,
           -1.0f, -1.0f, 1.0f,
           1.0f, -1.0f, -1.0f,
           1.0f, -1.0f, -1.0f,
           -1.0f, -1.0f, 1.0f,
           1.0f, -1.0f, 1.0f
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeMapVertixes), &cubeMapVertixes, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        shaderSkybox.use();
        shaderSkybox.setInt("skybox", 0);
    }

    void draw(glm::mat4 view, glm::mat4 projection)
    {
        glDepthFunc(GL_LEQUAL);
        shaderSkybox.use();
        shaderSkybox.setMat4("view", glm::mat4(glm::mat3(view)));
        shaderSkybox.setMat4("projection", glm::mat4(projection));

        glBindVertexArray(VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureCubeMap);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);
    }

};


int main()
{
    GLFWwindow* window = initFullScreen();
    if (window == NULL)
    {
        return -1;
    }

    Skybox skybox;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    float vertexes[] = {
    -0.1f, -0.1f, -0.3f,  0.0f, 0.0f,
    };

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
    unsigned int textureBlueCircle = loadTexture("circle_blue.png", GL_RGBA);

    Shader shader("shader.vs", "shader.fs", "shader.gs");
    shader.use();
    
    glBindAttribLocation(shader.ID, 0, "aPos");
    glBindAttribLocation(shader.ID, 1, "aTexCoord");
    glEnable(GL_DEPTH_TEST);
    shader.setInt("texture1", 0);
    shader.setInt("textureBlueCircle", 1);
    
    shader.setFloat("screenRatio", float(screenWidth) / screenHeight);

    Circle circleParameters;
    std::map<float, glm::vec4> sorted;
    std::thread t1([&]()
        {
            circleParameters.calculateOrder();
        });
    t1.detach();

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame; 

        //перспективная матрица
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)screenWidth / screenHeight, 0.1f, 100.f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 model(1.0f);
        //манипуляция вращением камеры
        glm::vec3 direction;
        // glm::cos(projection);
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(direction);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);//всключает отслеживание курсора
        processInput(window);

       // glDepthMask(GL_FALSE);
        // ... задание видовой и проекционной матриц   
        //glDepthMask(GL_TRUE);
        skybox.draw(view, projection);
        shader.use();

        glBindVertexArray(0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureBlueCircle);
        glBindVertexArray(VAO);

        if (tempSorted.size() != 0)
        {
            freeze.lock();
            sorted = tempSorted;
            freeze.unlock();
        }
        //std::cout << "completedThreadsMain " << ++completedThreadsMain << std::endl;
        // for (std::map<float, glm::vec4>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
        for (auto it = sorted.rbegin(); it != sorted.rend(); ++it)
        {
            glm::mat4 model(1.0f);
            model = glm::translate(model, glm::vec3(it->second.x, it->second.y, it->second.z));
            shader.setMat4("model", model);
            shader.setFloat("alpha", circleParameters.alpha[(int)it->second.w]);
            glDrawArrays(GL_POINTS, 0, 1);
        }

        fpsSpeed.CalculateFrameRate(currentFrame);

        shader.setMat4("projection", projection);
        shader.setFloat3("cameraPos", cameraPos.x, cameraPos.y, cameraPos.z);
        shader.setMat4("view", view);

        
        glfwSwapBuffers(window);
        glfwPollEvents();

        //cameraPos.y += fpsSpeed.getSpeed() * deltaTime;
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

float* getVertexes(std::string filePath, int size)
{
    std::ifstream vertexFile;
    float* vertexData = new float[size];
    vertexFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        vertexFile.open(filePath);
        for (int i = 0; i < size; i++)
        {
            vertexFile >> vertexData[i];
            vertexFile.get();
            vertexFile.get();
        }
        vertexFile.close();
    }
    catch (std::ifstream::failure e)
     {
        std::cout << "ERROR::VERTEXES::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }
    return vertexData;
}

