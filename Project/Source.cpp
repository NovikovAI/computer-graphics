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
#include "stb_image.h"

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;

/*stupididty here*/
float globalTransparancyValue = 0.2f;
/*stupididty here*/

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    printf("%d\n",key);
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    /*stupidity here*/
    if (key == GLFW_KEY_UP && action == GLFW_PRESS)
    {
        globalTransparancyValue += 0.1f;
        if (globalTransparancyValue >= 1.0f)
            globalTransparancyValue = 1.0f;
    }
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
    {
        globalTransparancyValue -= 0.1f;
        if (globalTransparancyValue <= 0.0f)
            globalTransparancyValue = 0.0f;
    }
    /*stupidity here*/
}

int main()
{
    //Init GLFW
    if (!glfwInit())
        return -1;
    // Set all the required options for GLFW
    //��������� GLFW
    //�������� ����������� ��������� ������ OpenGL. 
    //�������� 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    //��������
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //��������� �������� ��� �������� ��������� ��������
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //���������� ����������� ��������� ������� ����
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Graphics", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Set the required callback functions
    glfwSetKeyCallback(window, key_callback);

    // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
    glewExperimental = GL_TRUE;
    // Initialize GLEW to setup the OpenGL Function pointers
    glewInit();

    // Define the viewport dimensions
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    //Build and compile our shader program
    Shader myShader("../shaders/default.ver", "../shaders/default.frag");

    // Set up vertex data (and buffer(s)) and attribute pointers
    GLfloat vertices[] = {
        //Position          //Color            //Texture Coordinates
         0.5f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  2.0f, 2.0f,  // Top Right
         0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  2.0f, 0.0f,  // Bottom Right
        -0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,  // Bottom Left
        -0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 2.0f   // Top Left
    };
    GLuint indices[] = {  // Note that we start from 0!
        0, 1, 3,  // First Triangle
        1, 2, 3   // Second Triangle
    };
    GLuint VBO, VAO, EBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    // Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
    glBindVertexArray(VAO);

    //VBO, EBO in VAO-start
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    //Texture coordinates attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0); // Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind
    //VBO, EBO in VAO-end

    glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs)
    
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
    myShader.Use(); // �� ������ ������������ ������ ����� ���������� uniform-���������� 
    glUniform1i(glGetUniformLocation(myShader.Program, "texture1"), 0); //it only needs to be done once
    glUniform1i(glGetUniformLocation(myShader.Program, "texture2"), 1);

    glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done to not F up

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //2nd arg: use GL_LINE for outline only, GL_FILL for filled primitives 

    //Game loop
    while (!glfwWindowShouldClose(window))
    {
        // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
        glfwPollEvents();

        //render
        // Clear the colorbuffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Bind Texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        /*stupidity here*/
        glUniform1f(glGetUniformLocation(myShader.Program, "stupidity"), globalTransparancyValue);
        /*stupidity here*/

        // Create transformation
        /*�� ����� ��������� ������ ������ ������� ���������� �� ������,
        ������� ��� ���� ������ ��������� ������ ������.
        ������������� � ������ ��������������, ����� ������� � � ����� ��������, �� ����� ����������� ������,
        � ���� ������ ��� ����� �������� ����-�����.*/
        GLfloat curTime = (sin((GLfloat)glfwGetTime()) / 2) + 0.5f;   //not really time, but a "normalized" one
        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, glm::vec3(curTime * 0.5f, curTime * (-0.5f), 0.0f));      //�����
        transform = glm::rotate(transform, (GLfloat)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));        //�������
        transform = glm::scale(transform, glm::vec3(curTime * 0.5f, 0.5f, 0.5f));                       //���������������

        // Get matrix's uniform location and set matrix
        GLint transformLocation = glGetUniformLocation(myShader.Program, "transform");
        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(transform));

        //Draw second triangle
        myShader.Use();
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);    //last arg: (void*) (x*sizeof(GLuint))
        glBindVertexArray(0);

        // Swap the screen buffers
        glfwSwapBuffers(window);
    }

    // Properly de-allocate all resources once they've outlived their purpose
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
    return 0;
}