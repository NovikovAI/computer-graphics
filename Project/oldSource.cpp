#include <iostream>
#include <cmath>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other includes
#include "Shader.h"
#include "Camera.h"
#include "stb_image.h"

//Global parameters
//====================================================
//texture related
float globalTransparancyValue = 0.2f;
// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;
//keyboard related
bool keys[1024];
//camera related
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
GLfloat lastX = (GLfloat)WIDTH / 2.0;
GLfloat lastY = (GLfloat)HEIGHT / 2.0;
bool firstMouse = true;
//lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
// Deltatime-time between current frame and last frame
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
//====================================================


// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    std::cout<<key<<std::endl;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
}

void do_movements(){
    if (keys[GLFW_KEY_UP])
    {
        globalTransparancyValue += 0.01f;
        if (globalTransparancyValue >= 1.0f)
            globalTransparancyValue = 1.0f;
    }
    if (keys[GLFW_KEY_DOWN])
    {
        globalTransparancyValue -= 0.01f;
        if (globalTransparancyValue <= 0.0f)
            globalTransparancyValue = 0.0f;
    }
    if (keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (keys[GLFW_KEY_R]) {
        camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f));//this is the vector from the first camera constructor so this is VERY bad thing to do(also known as THE CRUTCH)
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

int main()
{
    //Init GLFW
    if (!glfwInit())
        return -1;
    // Set all the required options for GLFW
    //Настройка GLFW
    //Задается минимальная требуемая версия OpenGL. 
    //Мажорная 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    //Минорная
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //Установка профайла для которого создается контекст
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //Выключение возможности изменения размера окна
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Graphics", NULL, NULL);
    if (window == NULL)
    {
        std::cout<<"Failed to create GLFW window"<<std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Set the required callback functions
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    //GLFW options
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
    glewExperimental = GL_TRUE;
    // Initialize GLEW to setup the OpenGL Function pointers
    glewInit();

    // Define the viewport dimensions
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    //OpenGL options
    glEnable(GL_DEPTH_TEST);

    //Build and compile our shader programs
    Shader myShader("../shaders/default.ver", "../shaders/default.frag");
    Shader lampShader("../shaders/lamp.ver", "../shaders/lamp.frag");

    // Set up vertex data (and buffer(s)) and attribute pointers
    GLfloat vertices[] = {
    //Position            //TextureCoord    //Normals
    -0.5f, -0.5f, -0.5f,    0.0f, 0.0f,    0.0f, 0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,    1.0f, 0.0f,    0.0f, 0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,    1.0f, 1.0f,    0.0f, 0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,    1.0f, 1.0f,    0.0f, 0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f,    0.0f, 1.0f,    0.0f, 0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,    0.0f, 0.0f,    0.0f, 0.0f, -1.0f,

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
     0.5f,  0.5f, -0.5f,    1.0f, 1.0f,     1.0f, 0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,    0.0f, 1.0f,     1.0f, 0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,    0.0f, 1.0f,     1.0f, 0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,    0.0f, 0.0f,     1.0f, 0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,    1.0f, 0.0f,     1.0f, 0.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,    0.0f, 1.0f,    0.0f, -1.0f, 0.0f,
     0.5f, -0.5f, -0.5f,    1.0f, 1.0f,    0.0f, -1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,    1.0f, 0.0f,    0.0f, -1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,    1.0f, 0.0f,    0.0f, -1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,    0.0f, 0.0f,    0.0f, -1.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,    0.0f, 1.0f,    0.0f, -1.0f, 0.0f,

    -0.5f,  0.5f, -0.5f,    0.0f, 1.0f,     0.0f, 1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,    1.0f, 1.0f,     0.0f, 1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,    1.0f, 0.0f,     0.0f, 1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,    1.0f, 0.0f,     0.0f, 1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,    0.0f, 0.0f,     0.0f, 1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,    0.0f, 1.0f,     0.0f, 1.0f, 0.0f
    };
    //different WORLD posistions for cubes
    glm::vec3 cubePositions[] = {
        glm::vec3(0.0f,  0.0f,  0.0f),
        glm::vec3(2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3(1.3f, -2.0f, -2.5f),
        glm::vec3(1.5f,  2.0f, -2.5f),
        glm::vec3(1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };
    GLuint VBO, containerVAO;

    glGenVertexArrays(1, &containerVAO);
    glGenBuffers(1, &VBO);
    // Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
    glBindVertexArray(containerVAO);

    //VBO in containerVAO-start
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    //Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    //Texture coordinates attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    //Normals attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0); // Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind
    //VBO in containerVAO-end

    glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs)

    GLuint lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    
    //Create and load textures
    unsigned int texture1, texture2;
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    // Set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	// Set texture wrapping to GL_REPEAT (usually basic wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // Set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //load image
    int texWidth, texHeight, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load("../textures/wooden_container.jpg", &texWidth, &texHeight, &nrChannels, 0);
    //check it
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    //generate another texture
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    // Same parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// Set texture wrapping to GL_REPEAT (usually basic wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //load image
    data = stbi_load("../textures/awesomeface.png", &texWidth, &texHeight, &nrChannels, 0);
    //check it
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    //and don't forget to free it
    stbi_image_free(data);

    //we need to set up proper texture unit
    myShader.Use(); // не забыть активировать шейдер перед настройкой uniform-переменных 
    myShader.setInt("texture1", 0); //it only needs to be done once
    myShader.setInt("texture2", 1);

    glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done to not F up

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //2nd arg: use GL_LINE for outline only, GL_FILL for filled primitives 

    //Game loop
    while (!glfwWindowShouldClose(window))
    {
        // Calculate deltatime of current frame
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
        glfwPollEvents();
        do_movements();

        //render
        // Clear the colorbuffer
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Bind Texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        //Activate shader
        myShader.Use();

        //passing transparency value to the shader
        myShader.setFloat("transparency", globalTransparancyValue);

        //let's spin the lamp too
        float lampRadius = 3.0f;
        lightPos.x = sin(glfwGetTime()) * lampRadius;
        lightPos.z = cos(glfwGetTime()) * lampRadius;

        //passing all sorts of values to the shader
        myShader.setVec3("viewPos", camera.Position.x, camera.Position.y, camera.Position.z);
        //Material
        myShader.setVec3("material.ambient", 1.0f, 0.5f, 0.31f);
        myShader.setVec3("material.diffuse", 1.0f, 0.5f, 0.31f);
        myShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
        myShader.setFloat("material.shininess", 64.0f);
        //Light
        glm::vec3 lightColor;
        lightColor.x = sin(glfwGetTime() * 0.3f);
        lightColor.y = sin(glfwGetTime() * 0.5f);
        lightColor.z = sin(glfwGetTime() * 0.7f);
        glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f); // decrease the influence
        glm::vec3 ambientColor = lightColor * glm::vec3(0.2f); // low influence
        myShader.setVec3("light.position", lightPos.x, lightPos.y, lightPos.z);
        myShader.setVec3("light.ambient", ambientColor);
        myShader.setVec3("light.diffuse", diffuseColor);
        myShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

        // Create transformation
        //still need to think about efficiency of making matrices in the game loop in the future
        glm::mat4 viewMat = glm::mat4(1.0f);
        glm::mat4 projectionMat = glm::mat4(1.0f);
        viewMat = camera.GetViewMatrix();
        projectionMat = glm::perspective(glm::radians(camera.Zoom), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);

        // Get matrices' uniform location and set matrices
        myShader.setMat4("viewMat", viewMat);
        myShader.setMat4("projectionMat", projectionMat);

        //Draw figures
        glBindVertexArray(containerVAO);
        for (GLint i = 0; i < 10; i++)
        {
            glm::mat4 modelMat = glm::mat4(1.0f);
            modelMat = glm::translate(modelMat, cubePositions[i]);
            GLfloat cubeAngle = 20.0f * i;
            modelMat = glm::rotate(modelMat, (GLfloat)glfwGetTime() * glm::radians(-55.0f) + cubeAngle, glm::vec3(0.5f * (GLfloat)i, 1.0f, 0.0f));
            myShader.setMat4("modelMat", modelMat);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glBindVertexArray(0);

        //and lighting
        lampShader.Use();
        lampShader.setMat4("viewMat", viewMat);
        lampShader.setMat4("projectionMat", projectionMat);
        glm::mat4 modelMat = glm::mat4(1.0f);
        modelMat = glm::translate(modelMat, lightPos);
        modelMat = glm::scale(modelMat, glm::vec3(0.1f));
        lampShader.setMat4("modelMat", modelMat);
        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        // Swap the screen buffers
        glfwSwapBuffers(window);
    }

    // Properly de-allocate all resources once they've outlived their purpose
    glDeleteVertexArrays(1, &containerVAO);
    glDeleteBuffers(1, &VBO);
    //glDeleteBuffers(1, &EBO);

    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
    return 0;
}