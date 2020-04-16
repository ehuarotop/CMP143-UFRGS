#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

//Declaring shaders for vertex and fragments infile
const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec4 NDC_coefficients;\n"
    "layout (location = 1) in vec4 color_coefficients;\n"
    "out vec4 rasterizer_color;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = NDC_coefficients;\n"
    "   rasterizer_color = color_coefficients;\n"
    "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
    "in vec4 rasterizer_color;\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = rasterizer_color;\n"
    "}\n\0";

int main()
{   

    //Initiating glfw
    glfwInit();

    //Defining glfw properties for opengl Version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //Specify the size of the window to be created
    int window_width = 500;
    int window_height = 500;

    //Creating window with width and height specified before.
    GLFWwindow* window = glfwCreateWindow(window_width, window_height, "CMP143 - 00312086 - Felix Eduardo Huaroto Pachas", NULL, NULL);
	if (window == NULL)
	{
	    std::cout << "Failed to create GLFW window" << std::endl;
	    glfwTerminate();
	    return -1;
	}

    //Indicating that current context will be window recently created
	glfwMakeContextCurrent(window);


	//Initialize GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
	    std::cout << "Failed to initialize GLAD" << std::endl;
	    return -1;
	}

	/////////////////////////////////////////////////////////////////////////////////

	// SHADERS IMPLEMENTATION
    
    // Creating vertex shader, getting and compile the code saved in vertexShaderSource variable
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Checking if exists some compilation errors when compile vertex shader
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Creating fragment shader, getting and compile the code saved in fragmentShaderSource variable
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    
    // Check if exists some compilation errors when compile fragment shader
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    
    // Creating GPU program and linking shaders recently created to it 
    // Creating GPU program
    int shaderProgram = glCreateProgram();
    //Attaching/Linking shaders
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    //Linking program.
    glLinkProgram(shaderProgram);
    
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    
    //Removing shaders after used (not needed anymore)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    ///////////////////////////////////////////////////////////////////////////////////
    //Defining coefficients for triangle vertex and colours

    //Setting vertex ndc_coefficients (Normal device coordinates)
    GLfloat NDC_coefficients[] = {
    //    X      Y     Z     W
        -0.5f, -0.5f, 0.0f, 1.0f,
         0.5f, -0.5f, 0.0f, 1.0f,
         0.0f,  0.5f, 0.0f, 1.0f,
         0.5f,  0.5f, 0.0f, 1.0f
    };

    //Declaring Virtual Buffer Objects (VBO) and Vertex Array Object(VAO)
    unsigned int VBO, VAO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);
    
    // binding the Vertex Array Object.
    glBindVertexArray(VAO);
    // Binding buffer for vertex VBO 
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    //Indicating data size and passing vertex information to shader
    glBufferData(GL_ARRAY_BUFFER, sizeof(NDC_coefficients), NDC_coefficients, GL_STATIC_DRAW);
    //Setting 0 in position (according to the specification on the shader)
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    //Unbinding the VBO buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    //Setting variable vertices colors (Red, Green and Blue) A for opacity.
    GLfloat color_coefficients[] = {
    //  R     G     B     A
        1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f, 1.0f
    };

    //Registering a new VBO for color coefficients
    unsigned int VBO_color;
    glGenBuffers(1, &VBO_color);

    //Binding the new VBO to the VAO
    glBindBuffer(GL_ARRAY_BUFFER, VBO_color);

    //Indicating data size and passing vertices color information to shader
    glBufferData(GL_ARRAY_BUFFER, sizeof(color_coefficients), color_coefficients, GL_STATIC_DRAW);
    //Setting 1 in position (according to the specification on the shader)
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    //Unbinding VBO for color coefficiente
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //Unbinding VAO
    glBindVertexArray(0);

    /////////////////////////////////////////////////////////////////////////////////

	//Setting size of rendering window
	glViewport(0, 0, window_width, window_height);

	//Setting callback for buffer size change
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    //Rendering into the created window
	while(!glfwWindowShouldClose(window))
	{
		//Processing input to close window
		processInput(window);

		// Clearing the window background
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Using GPU Program created
        glUseProgram(shaderProgram);
        //Binding VAO, maybe no need to binding it every time because have only one VAO
        glBindVertexArray(VAO);

        //Drawing actual triangles
        glDrawArrays(GL_TRIANGLES, 0, 4);

		//Swapping buffers
	    glfwSwapBuffers(window);
        
	    //Keyboard and mouse events
	    glfwPollEvents();    
	}

	// optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

	glfwTerminate();
  
    return 0;
}

void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}