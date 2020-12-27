#include <iostream>
#include <cmath>
#include <string>
#include <map>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "stb_image.h"

//====================GLOBAL==========================
// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;
//keyboard related
bool keys[1024];
//camera related
Camera camera(glm::vec3(0.0f, 1.0f, 3.0f));
GLfloat lastX = (GLfloat)WIDTH / 2.0;
GLfloat lastY = (GLfloat)HEIGHT / 2.0;
bool firstMouse = true;
//lighting
glm::vec3 directLightPos(-11.0f, -2.0f, -5.0f);
bool globalSpotlightSwitch = false;
bool showLampsAndTheirLight = false;
const int numberOfPointLights = 2;
// Deltatime-time between current frame and last frame
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
//====================================================
//======================================FUNCTIONS======================================================================================================================================================
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    std::cout<<key<<std::endl;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS) {
            keys[key] = true;
            if (key == GLFW_KEY_F)
                globalSpotlightSwitch = !globalSpotlightSwitch;     //DAMN CRUTCH
        }
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
}

void do_movements(){
    if (keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (keys[GLFW_KEY_R]) {
        camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f));   //this is the vector from the first camera
                                                        //constructor so this is VERY bad thing to
                                                        //do(also known as THE CRUTCH)
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to left
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void drawFloor(const glm::mat4 projectionMat, const unsigned int planeVAO, Shader myShader, const unsigned int floorTexture)
{
    glm::mat4 modelMat = glm::mat4(1.0f);
    glm::mat4 viewMat = camera.GetViewMatrix();

    glStencilMask(0x00);

    myShader.setMat4("viewMat", viewMat);
    myShader.setMat4("projectionMat", projectionMat);
    myShader.Use();
    glBindVertexArray(planeVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, floorTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, floorTexture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, 0);
    modelMat = glm::translate(modelMat, glm::vec3(0.0f, -0.01f, 0.0f));
    myShader.setMat4("modelMat", modelMat);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, 0);

    glStencilMask(0xFF);
}

void drawCubesAndOutline(const glm::mat4 projectionMat, const unsigned int containerVAO, Shader myShader, Shader outlineShader, glm::vec3* cubePositions,
    const unsigned int diffuseMap, const unsigned int specularMap, const unsigned int emissionMap)
{
    glm::mat4 viewMat = camera.GetViewMatrix();

    myShader.Use();
    myShader.setMat4("viewMat", viewMat);
    myShader.setMat4("projectionMat", projectionMat);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, specularMap);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, emissionMap);

    //Draw figures
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);

    glBindVertexArray(containerVAO);
    for (unsigned int i = 0; i < 5; i++)
    {
        glm::mat4 modelMat = glm::mat4(1.0f);
        modelMat = glm::translate(modelMat, cubePositions[i]);
        myShader.setMat4("modelMat", modelMat);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glBindVertexArray(0);

    //draw outline
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);
    //glDisable(GL_DEPTH_TEST);
    outlineShader.Use();
    float scale = 1.005f;

    outlineShader.setVec3("outlineColor", glm::vec3(1.0f, 0.0f, 0.0f));
    outlineShader.setMat4("viewMat", viewMat);
    outlineShader.setMat4("projectionMat", projectionMat);

    glBindVertexArray(containerVAO);
    for (unsigned int i = 0; i < 5; i++)
    {
        glm::mat4 modelMat = glm::mat4(1.0f);
        modelMat = glm::translate(modelMat, cubePositions[i]);
        modelMat = glm::scale(modelMat, glm::vec3(scale));
        outlineShader.setMat4("modelMat", modelMat);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glBindVertexArray(0);

    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);
}

void drawLamps(const glm::mat4 projectionMat, const unsigned int lightVAO, Shader lampShader, const glm::vec3* pointLightPositions, const glm::vec3 ambientColor,
    const glm::vec3 diffuseColor)
{
    glm::mat4 viewMat = camera.GetViewMatrix();
    glm::mat4 modelMat = glm::mat4(1.0f);

    lampShader.Use();
    lampShader.setMat4("viewMat", viewMat);
    lampShader.setMat4("projectionMat", projectionMat);
    glBindVertexArray(lightVAO);
    for (unsigned int i = 0; i < 2; i++)
    {
        lampShader.setVec3("ambient", ambientColor);
        lampShader.setVec3("diffuse", diffuseColor);
        lampShader.setVec3("specular", glm::vec3(1.0f));
        modelMat = glm::mat4(1.0f);
        modelMat = glm::translate(modelMat, pointLightPositions[i]);
        modelMat = glm::scale(modelMat, glm::vec3(0.2f));
        lampShader.setMat4("modelMat", modelMat);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glBindVertexArray(0);
}

void drawSkyboxAndMirrorCube(const glm::mat4 projectionMat, const unsigned int skyboxVAO, const unsigned int mirrorVAO, Shader skyboxShader, Shader mirrorShader,
    const unsigned int cubemapTexture)
{
    glm::mat4 viewMat = glm::mat4(1.0f);
    glm::mat4 modelMat = glm::mat4(1.0f);

    //draw skybox
    glDepthFunc(GL_LEQUAL);
    skyboxShader.Use();
    viewMat = glm::mat4(glm::mat3(camera.GetViewMatrix()));     //we will F' up view matrix to get rid of translation, but we will only do it for skybox
    skyboxShader.setMat4("viewMat", viewMat);
    skyboxShader.setMat4("projectionMat", projectionMat);
    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
    viewMat = camera.GetViewMatrix();               //here we are "restoring" the "right" view matrix

    //draw mirror cube
    mirrorShader.Use();
    glm::mat4 mirrorModelMat = glm::mat4(1.0f);
    mirrorModelMat = glm::translate(mirrorModelMat, glm::vec3(-5.0f, 3.0f, 1.0f));
    mirrorModelMat = glm::scale(mirrorModelMat, glm::vec3(0.7f));
    mirrorShader.setMat4("modelMat", mirrorModelMat);
    mirrorShader.setMat4("viewMat", viewMat);
    mirrorShader.setMat4("projectionMat", projectionMat);
    mirrorShader.setVec3("cameraPos", camera.Position);
    glBindVertexArray(mirrorVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void drawWindows(const glm::mat4 projectionMat, const unsigned int transparentVAO, Shader windowShader, std::vector<glm::vec3> windows,
    const unsigned int windowTexture)
{
    glm::mat4 viewMat = camera.GetViewMatrix();
    glm::mat4 modelMat = glm::mat4(1.0f);

    //sorting windows by distance
    std::map<float, glm::vec3> sortedWindows;
    for (unsigned int i = 0; i < windows.size(); i++)
    {
        float distance = glm::length(camera.Position - windows[i]);
        sortedWindows[distance] = windows[i];
    }

    //draw windows
    windowShader.Use();
    glBindVertexArray(transparentVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, windowTexture);
    windowShader.setMat4("viewMat", viewMat);
    windowShader.setMat4("projectionMat", projectionMat);
    for (std::map<float, glm::vec3>::reverse_iterator it = sortedWindows.rbegin(); it != sortedWindows.rend(); ++it)
    {
        modelMat = glm::mat4(1.0f);
        modelMat = glm::translate(modelMat, it->second);
        windowShader.setMat4("modelMat", modelMat);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    glBindVertexArray(0);
}

//=====================================================================================================================================================================================================

int main()
{
    //Init GLFW
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Graphics", NULL, NULL);
    if (window == NULL)
    {
        std::cout<<"Failed to create GLFW window"<<std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //Build and compile our shader programs
    Shader myShader("../shaders/default.ver", "../shaders/default.frag");
    Shader outlineShader("../shaders/outline.ver", "../shaders/outline.frag");
    Shader lampShader("../shaders/lamp.ver", "../shaders/lamp.frag");
    Shader windowShader("../shaders/window.ver", "../shaders/window.frag");
    Shader skyboxShader("../shaders/skybox.ver", "../shaders/skybox.frag");
    Shader mirrorShader("../shaders/mirrorCube.ver", "../shaders/mirrorCube.frag");
    Shader simpleDepthShader("../shaders/shadow_mapping.ver", "../shaders/shadow_mapping.frag");

    float skyboxVertices[] = {
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
    };

    float vertices[] = {
         //Position            //TextureCoord    //Normals
        -0.5f, -0.5f, -0.5f,    0.0f, 0.0f,    0.0f, 0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,    1.0f, 1.0f,    0.0f, 0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,    1.0f, 0.0f,    0.0f, 0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,    1.0f, 1.0f,    0.0f, 0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, 0.0f,    0.0f, 0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,    0.0f, 1.0f,    0.0f, 0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,    0.0f, 0.0f,     0.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,    1.0f, 0.0f,     0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,    1.0f, 1.0f,     0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,    1.0f, 1.0f,     0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,    0.0f, 1.0f,     0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,    0.0f, 0.0f,     0.0f, 0.0f, 1.0f,

        -0.5f,  0.5f,  0.5f,    1.0f, 0.0f,    -1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,    1.0f, 1.0f,    -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, 1.0f,    -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, 1.0f,    -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,    0.0f, 0.0f,    -1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,    1.0f, 0.0f,    -1.0f, 0.0f, 0.0f,

         0.5f,  0.5f,  0.5f,    1.0f, 0.0f,     1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,    0.0f, 1.0f,     1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,    1.0f, 1.0f,     1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,    0.0f, 1.0f,     1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,    1.0f, 0.0f,     1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,    0.0f, 0.0f,     1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,    0.0f, 1.0f,    0.0f, -1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,    1.0f, 1.0f,    0.0f, -1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,    1.0f, 0.0f,    0.0f, -1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,    1.0f, 0.0f,    0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,    0.0f, 0.0f,    0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, 1.0f,    0.0f, -1.0f, 0.0f,

        -0.5f,  0.5f, -0.5f,    0.0f, 1.0f,     0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,    1.0f, 0.0f,     0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,    1.0f, 1.0f,     0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,    1.0f, 0.0f,     0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,    0.0f, 1.0f,     0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,    0.0f, 0.0f,     0.0f, 1.0f, 0.0f
    };

    float planeVertices[] = {
        //Coordinates         //TextureCoord    //Normals
         5.0f, -0.5f,  5.0f,   2.0f, 0.0f,     0.0f, 1.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,   0.0f, 2.0f,     0.0f, 1.0f, 0.0f,
        -5.0f, -0.5f,  5.0f,   0.0f, 0.0f,     0.0f, 1.0f, 0.0f,

         5.0f, -0.5f,  5.0f,   2.0f, 0.0f,     0.0f, 1.0f, 0.0f,
         5.0f, -0.5f, -5.0f,   2.0f, 2.0f,     0.0f, 1.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,   0.0f, 2.0f,     0.0f, 1.0f, 0.0f
    };

    float transparentVertices[] = {
        //Coordinates        //TextureCoords
         0.0f,  0.5f,  0.0f,   0.0f, 0.0f,
         0.0f, -0.5f,  0.0f,   0.0f, 1.0f,
         1.0f, -0.5f,  0.0f,   1.0f, 1.0f,

         0.0f,  0.5f,  0.0f,   0.0f, 0.0f,
         1.0f, -0.5f,  0.0f,   1.0f, 1.0f,
         1.0f,  0.5f,  0.0f,   1.0f, 0.0f
    };
    //different WORLD posistions for cubes
    glm::vec3 cubePositions[] = {
        glm::vec3(0.0f,  0.0f,  0.0f),
        //glm::vec3(2.0f,  5.0f, -15.0f),
        //glm::vec3(-1.5f, -2.2f, -2.5f),
        //glm::vec3(-3.8f, -2.0f, -12.3f),
        //glm::vec3(2.4f, -0.4f, -3.5f),
        //glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3(1.3f, -2.0f, -2.5f),
        glm::vec3(1.5f,  2.0f, -2.5f),
        glm::vec3(1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };
    //and lights
    glm::vec3 pointLightPositions[] = {
        glm::vec3(0.7f,  0.2f,  2.0f),
        glm::vec3(0.0f,  0.0f, -3.0f),
        glm::vec3(-4.0f,  2.0f, -12.0f),
        glm::vec3(2.3f, -3.3f, -4.0f)
    };
    //and windows
    std::vector<glm::vec3> windows
    {
        glm::vec3(-1.5f, 1.3f, -0.48f),
        glm::vec3(1.5f, 3.0f, 0.51f),
    };
    //skybox locatoins and load
    std::vector<std::string> skyboxFaces
    {
        "../textures/skybox/right.jpg",
        "../textures/skybox/left.jpg",
        "../textures/skybox/top.jpg",
        "../textures/skybox/bottom.jpg",
        "../textures/skybox/front.jpg",
        "../textures/skybox/back.jpg"
    };
    unsigned int cubemapTexture = loadCubemap(skyboxFaces);

    stbi_set_flip_vertically_on_load(true);

    //for cubes
    unsigned int VBO, containerVAO;
    glGenVertexArrays(1, &containerVAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(containerVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //for floor
    unsigned int planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    //for windows
    unsigned int transparentVAO, transparentVBO;
    glGenVertexArrays(1, &transparentVAO);
    glGenBuffers(1, &transparentVBO);
    glBindVertexArray(transparentVAO);
    glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    //for lamps
    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    //for skybox
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    
    //for mirror cubes
    unsigned int mirrorVAO;
    glGenVertexArrays(1, &mirrorVAO);
    glBindVertexArray(mirrorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)(5 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //framebuffer for shadows
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    unsigned int shadowMapFBO;
    glGenFramebuffers(1, &shadowMapFBO);
    unsigned int shadowMap;
    glGenTextures(1, &shadowMap);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    unsigned int diffuseMap = loadTexture("../textures/container2.png");
    unsigned int specularMap = loadTexture("../textures/container2_specular.png");
    unsigned int emissionMap = loadTexture("../textures/matrix.jpg");
    unsigned int floorTexture = loadTexture("../textures/metal_floor.jpg");
    unsigned int windowTexture = loadTexture("../textures/window.png");

    //we need to set up proper texture unit
    myShader.Use();
    myShader.setInt("material.diffuse", 0);
    myShader.setInt("material.specular", 1);
    myShader.setInt("material.emission", 2);
    windowShader.Use();
    windowShader.setInt("windowTexture", 0);
    skyboxShader.Use();
    skyboxShader.setInt("skybox", 0);

    glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done to not F up

    while (!glfwWindowShouldClose(window))
    {
        // Calculate deltatime of current frame
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        do_movements();

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // Create transformation
        glm::mat4 projectionMat = glm::mat4(1.0f);
        glm::mat4 viewMat = glm::mat4(1.0f);
        glm::mat4 modelMat = glm::mat4(1.0f);
        projectionMat = glm::perspective(glm::radians(camera.Zoom), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
        viewMat = camera.GetViewMatrix();
        
        myShader.Use();
        //passing all sorts of values to the shader
        myShader.setVec3("viewPos", camera.Position.x, camera.Position.y, camera.Position.z);
        myShader.setFloat("time", 5.0 * currentFrame);
        //Material
        myShader.setFloat("material.shininess", 64.0f);
        //Lights
        glm::vec3 lightColor = glm::vec3(1.0f);
        glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f); // decrease the influence
        glm::vec3 ambientColor = lightColor * glm::vec3(0.2f); // low influence
        //direction light
        myShader.setVec3("directLight.direction", directLightPos);
        myShader.setVec3("directLight.ambient", glm::vec3(0.05f));
        myShader.setVec3("directLight.diffuse", glm::vec3(0.7f));
        myShader.setVec3("directLight.specular", glm::vec3(1.0f));
        // four point lights
        if (showLampsAndTheirLight)
        {
            for (unsigned int i = 0; i < numberOfPointLights; i++)
            {
                std::string curName = "pointLights[" + std::to_string(i) + std::string(1, ']');
                myShader.setVec3(curName + ".position", pointLightPositions[i]);
                myShader.setFloat(curName + ".constant", 1.0f);
                myShader.setFloat(curName + ".linear", 0.09f);
                myShader.setFloat(curName + ".quadratic", 0.032f);
                myShader.setVec3(curName + ".ambient", ambientColor);
                myShader.setVec3(curName + ".diffuse", diffuseColor);
                myShader.setVec3(curName + ".specular", glm::vec3(1.0f));
            }
        }
        myShader.setBool("lampsLightEnabled", showLampsAndTheirLight);

        //spotlight
        myShader.setBool("spotlight.enabled", globalSpotlightSwitch);
        myShader.setVec3("spotlight.position", camera.Position);
        myShader.setVec3("spotlight.direction", camera.Front);
        myShader.setFloat("spotlight.cutOff", glm::cos(glm::radians(12.5f)));
        myShader.setFloat("spotlight.outerCutOff", glm::cos(glm::radians(15.5f)));
        myShader.setFloat("spotlight.constant", 1.0f);          //chose constants for 50 units
        myShader.setFloat("spotlight.linear", 0.09f);
        myShader.setFloat("spotlight.quadratic", 0.032f);
        myShader.setVec3("spotlight.ambient", glm::vec3(0.0f));
        myShader.setVec3("spotlight.diffuse", glm::vec3(1.0f));
        myShader.setVec3("spotlight.specular", glm::vec3(1.0f));

        drawFloor(projectionMat, planeVAO, myShader, floorTexture);

        drawCubesAndOutline(projectionMat, containerVAO, myShader, outlineShader, cubePositions, diffuseMap, specularMap, emissionMap);

        if (showLampsAndTheirLight)
            drawLamps(projectionMat, lightVAO, lampShader, pointLightPositions, ambientColor, diffuseColor);

        drawSkyboxAndMirrorCube(projectionMat, skyboxVAO, mirrorVAO, skyboxShader, mirrorShader, cubemapTexture);

        drawWindows(projectionMat, transparentVAO, windowShader, windows, windowTexture);

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &containerVAO);
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteVertexArrays(1, &transparentVAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteVertexArrays(1, &mirrorVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &transparentVBO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteBuffers(1, &skyboxVBO);

    glfwTerminate();
    return 0;
}