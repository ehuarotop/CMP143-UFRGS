#include <glad/glad.h>

#if defined(NANOGUI_GLAD)
    #if defined(NANOGUI_SHARED) && !defined(GLAD_GLAPI_EXPORT)
        #define GLAD_GLAPI_EXPORT
    #endif

    #include <glad/glad.h>
#else
    #if defined(__APPLE__)
        #define GLFW_INCLUDE_GLCOREARB
    #else
        #define GL_GLEXT_PROTOTYPES
    #endif
#endif


#include <nanogui/nanogui.h>

#include <GLFW/glfw3.h>

//Includes for matrix transformations using glm
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp>

#include <shader_s.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;
using namespace nanogui;
//using nanogui::Screen;

//Defining nanogui variables
Screen *screen = nullptr;
enum test_enum {
    Item1 = 0,
    Item2,
    Item3
};

bool bvar = true;
int ivar = 12345678;
double dvar = 3.1415926;
float fvar = (float)dvar;
std::string strval = "A string";
test_enum enumval = Item2;
Color colval(0.5f, 0.5f, 0.7f, 1.f);

//Defining constant for max number of materials in input file.
const int MAX_MATERIAL_COUNT = 1;
int g_num_triangles = 0;

float g_min_X=0.0f, g_max_X=0.0f, g_min_Y=0.0f, g_max_Y=0.0f, g_min_Z=0.0f, g_max_Z=0.0f;
float g_min_total=0.0f, g_max_total=0.0f;

//Defining struct for triangle
struct Triangle {
	glm::vec3 v0, v1, v2, face_normal;
	glm::vec3 normal[3];
	float Color[3];
};

//Function declaration
//calback for resizing window
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
//callback for processing input
void processInput(GLFWwindow *window);
//function to read files with extension .in (models)
unsigned int readFile(const char* filename);
//function to get camera working
glm::mat4 camera(float Translate, glm::vec2 const & Rotate);

int main(){

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
    GLFWwindow* window = glfwCreateWindow(window_width, window_height, 
        "CMP143 - 00312086 - Felix Eduardo Huaroto Pachas", NULL, NULL);
	if (window == NULL)
	{
	    std::cout << "Failed to create GLFW window" << std::endl;
	    glfwTerminate();
	    return -1;
	}

    /*GLFWwindow* window = glfwCreateWindow(window_width, window_height, 
        "OpenGL control", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }*/

    //Indicating that current context will be window recently created
	glfwMakeContextCurrent(window);

	//Initialize GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
	    std::cout << "Failed to initialize GLAD" << std::endl;
	    return -1;
	}

    // Printing to terminal opengl and glsl version
    const GLubyte *vendor      = glGetString(GL_VENDOR);
    const GLubyte *renderer    = glGetString(GL_RENDERER);
    const GLubyte *glversion   = glGetString(GL_VERSION);
    const GLubyte *glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);
    printf("GPU: %s, %s, OpenGL %s, GLSL %s\n", vendor, renderer, glversion, glslversion);

    //////////////////// NANOGUI ////////////////////
    // Create a nanogui screen and pass the glfw pointer to initialize
    screen = new Screen();
    screen->initialize(window, true);

    bool enabled = true;
    FormHelper *gui = new FormHelper(screen);
    nanogui::ref<Window> nanoguiWindow = gui->addWindow(Eigen::Vector2i(10, 10), "Form helper example");
    gui->addGroup("Basic types");
    gui->addVariable("bool", bvar)->setTooltip("Test tooltip.");
    gui->addVariable("string", strval);

    gui->addGroup("Validating fields");
    gui->addVariable("int", ivar)->setSpinnable(true);
    gui->addVariable("float", fvar)->setTooltip("Test.");
    gui->addVariable("double", dvar)->setSpinnable(true);

    gui->addGroup("Complex types");
    gui->addVariable("Enumeration", enumval, enabled)->setItems({ "Item 1", "Item 2", "Item 3" });
    gui->addVariable("Color", colval)
       ->setFinalCallback([](const Color &c) {
             std::cout << "ColorPicker Final Callback: ["
                       << c.r() << ", "
                       << c.g() << ", "
                       << c.b() << ", "
                       << c.w() << "]" << std::endl;
         });

    gui->addGroup("Other widgets");
    gui->addButton("A button", []() { std::cout << "Button pressed." << std::endl; })->setTooltip("Testing a much longer tooltip, that will wrap around to new lines multiple times.");;

    screen->setVisible(true);
    screen->performLayout();
    nanoguiWindow->center();

    glfwSetCursorPosCallback(window,
            [](GLFWwindow *, double x, double y) {
            screen->cursorPosCallbackEvent(x, y);
        }
    );

    glfwSetMouseButtonCallback(window,
        [](GLFWwindow *, int button, int action, int modifiers) {
            screen->mouseButtonCallbackEvent(button, action, modifiers);
        }
    );

    glfwSetKeyCallback(window,
        [](GLFWwindow *, int key, int scancode, int action, int mods) {
            screen->keyCallbackEvent(key, scancode, action, mods);
        }
    );

    glfwSetCharCallback(window,
        [](GLFWwindow *, unsigned int codepoint) {
            screen->charCallbackEvent(codepoint);
        }
    );

    glfwSetDropCallback(window,
        [](GLFWwindow *, int count, const char **filenames) {
            screen->dropCallbackEvent(count, filenames);
        }
    );

    glfwSetScrollCallback(window,
        [](GLFWwindow *, double x, double y) {
            screen->scrollCallbackEvent(x, y);
       }
    );

    glfwSetFramebufferSizeCallback(window,
        [](GLFWwindow *, int width, int height) {
            screen->resizeCallbackEvent(width, height);
        }
    );

    //////////////////// END NANOGUI ////////////////////

    //Loading shaders
    CustomShader custom_shader("src/shader_vertex.glsl", "src/shader_fragment.glsl");

    /////////////////// Reading file ///////////////////
    unsigned int VAO = readFile("data/cow_up.in");

    printf("min_X: %f\nmax_X: %f\nmin_Y: %f\nmax_Y: %f\nmin_Z: %f\nmax_Z: %f\n", 
        g_min_X,g_max_X,g_min_Y,g_max_Y,g_min_Z,g_max_Z);

    //Setting size of rendering window
    glViewport(0, 0, window_width, window_height);

    //Setting callback for buffer size change
    //glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    //Getting min and max total
    ////// MIN /////
    if(g_min_X < g_min_Y){
        if(g_min_X < g_min_Z){
            g_min_total = g_min_X;
        }else{
            g_min_total = g_min_Z;
        }
    }else{
        if(g_min_Y < g_min_Z){
            g_min_total = g_min_Y;
        }else{
            g_min_total = g_min_Z;
        }
    }

    ///// MAX /////
    if (g_max_X > g_max_Y){
        if(g_max_X > g_max_Z){
            g_max_total = g_max_X;
        }else{
            g_max_total = g_max_Z;
        }
    }else{
        if(g_max_Y > g_max_Z){
            g_max_total = g_max_Y;
        }else{
            g_max_total = g_max_Z;
        }
    }


    //Rendering into the created window
	while(!glfwWindowShouldClose(window))
	{
		//Processing input to close window
		//processInput(window);

		// Clearing the window background
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

        // Draw nanogui
        screen->drawContents();
        screen->drawWidgets();

		// Using GPU Program created
        //glUseProgram(shaderProgram);
        custom_shader.use();

		//create transformations
		glm::mat4 model         = glm::mat4(1.0f);
        glm::mat4 view          = glm::mat4(1.0f);
        glm::mat4 projection    = glm::mat4(1.0f);

        float range_x = (g_min_X+g_max_X)/2.0f;
        float range_y = (g_min_Y+g_max_Y)/2.0f;
        float range_z = (g_min_Z+g_max_Z)/2.0f;

        //Translating the model to the origin (0,0,0)
        glm::mat4 trans = glm::translate(model, glm::vec3(-range_x, -range_y, -range_z));
        model = trans * model;

        //View matrix
        view = glm::lookAt(glm::vec3(0.0f, 0.0f, g_max_total - g_min_total),
                            glm::vec3(0.0f,0.0f,0.0f),
                            glm::vec3(0.0f,1.0f,0.0f));

        projection = glm::perspective(glm::radians(45.0f),
                                        float(window_width)/float(window_height),
                                        0.1f, g_max_total - g_min_total);

        // retrieve the matrix uniform locations
        unsigned int modelLoc = glGetUniformLocation(custom_shader.ID, "model");
        unsigned int viewLoc  = glGetUniformLocation(custom_shader.ID, "view");
        unsigned int projectionLoc  = glGetUniformLocation(custom_shader.ID, "projection");

        // pass them to the shaders (3 different ways)
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

		//Binding VAO, maybe no need to binding it every time because have only one VAO
        glBindVertexArray(VAO);

        //Drawing actual triangles
        glDrawArrays(GL_TRIANGLES, 0, g_num_triangles*3);

		//Swapping buffers
	    glfwSwapBuffers(window);
        
	    //Keyboard and mouse events
	    glfwPollEvents();    
	}
}

unsigned int readFile(const char* filename){
	char ch;
	int num_triangles, material_count, i, color_index[3];

	glm::vec3 ambient[MAX_MATERIAL_COUNT], 
				diffuse[MAX_MATERIAL_COUNT], 
				specular[MAX_MATERIAL_COUNT];

	float shine[MAX_MATERIAL_COUNT];

	FILE* fp = fopen(filename,"r");
	if (fp==NULL) { printf("ERROR: unable to open TriObj [%s]!\n",filename); exit(1); }

	// skiping the first line – object’s name
	fscanf(fp, "%c", &ch);
	while(ch!= '\n')
		fscanf(fp, "%c", &ch);

	// read # of triangles
	fscanf(fp,"# triangles = %d\n", &num_triangles);
	g_num_triangles = num_triangles; //Setting global variable for number of triangles
	// read material count
	fscanf(fp,"Material count = %d\n", &material_count);

	for (i=0; i<material_count; i++) {
		fscanf(fp, "ambient color %f %f %f\n", &(ambient[i].x), &(ambient[i].y), &(ambient[i].z));
		fscanf(fp, "diffuse color %f %f %f\n", &(diffuse[i].x), &(diffuse[i].y), &(diffuse[i].z));
		fscanf(fp, "specular color %f %f %f\n", &(specular[i].x), &(specular[i].y), &(specular[i].z));
		fscanf(fp, "material shine %f\n", &(shine[i]));
	}

	// skiping documentation line
	fscanf(fp, "%c", &ch);
	while(ch!= '\n')
		fscanf(fp, "%c", &ch);

	// allocate triangles for tri model
	printf ("Reading in %s (%d triangles). . .\n", filename, num_triangles);
	struct Triangle Tris[num_triangles];
	//
	for (i=0; i<num_triangles; i++) { // read triangles

		fscanf(fp, "v0 %f %f %f %f %f %f %d\n",
				&(Tris[i].v0.x), &(Tris[i].v0.y), &(Tris[i].v0.z),
				&(Tris[i].normal[0].x), &(Tris[i]. normal [0].y), &(Tris[i]. normal [0].z),
				&(color_index[0]));
		
		fscanf(fp, "v1 %f %f %f %f %f %f %d\n",
				&(Tris[i].v1.x), &(Tris[i].v1.y), &(Tris[i].v1.z),
				&(Tris[i].normal[1].x), &(Tris[i].normal[1].y), &(Tris[i].normal[1].z),
				&(color_index[1]));
		
		fscanf(fp, "v2 %f %f %f %f %f %f %d\n",
				&(Tris[i].v2.x), &(Tris[i].v2.y), &(Tris[i].v2.z),
				&(Tris[i].normal[2].x), &(Tris[i].normal[2].y), &(Tris[i].normal[2].z),&(color_index[2]));
		
		fscanf(fp, "face normal %f %f %f\n", &(Tris[i].face_normal.x), &(Tris[i].face_normal.y),
				&(Tris[i].face_normal.z));
		
		//
		Tris[i].Color[0] = (unsigned char)(int)(255*(diffuse[color_index[0]].x));
		Tris[i].Color[1] = (unsigned char)(int)(255*(diffuse[color_index[1]].y));
		Tris[i].Color[2] = (unsigned char)(int)(255*(diffuse[color_index[2]].z));

        //Getting min and max for X axis
        if(Tris[i].v0.x < g_min_X){
            g_min_X = Tris[i].v0.x;
        }
        if(Tris[i].v1.x < g_min_X){
            g_min_X = Tris[i].v1.x;
        }
        if(Tris[i].v2.x < g_min_X){
            g_min_X = Tris[i].v2.x;
        }

        if(Tris[i].v0.x > g_max_X){
            g_max_X = Tris[i].v0.x;
        }
        if(Tris[i].v1.x > g_max_X){
            g_max_X = Tris[i].v1.x;
        }
        if(Tris[i].v2.x > g_max_X){
            g_max_X = Tris[i].v2.x;
        }

        //Getting min and max for Y axis
        if(Tris[i].v0.y < g_min_Y){
            g_min_Y = Tris[i].v0.y;
        }
        if(Tris[i].v1.y < g_min_Y){
            g_min_Y = Tris[i].v1.y;
        }
        if(Tris[i].v2.y < g_min_Y){
            g_min_Y = Tris[i].v2.y;
        }

        if(Tris[i].v0.y > g_max_Y){
            g_max_Y = Tris[i].v0.y;
        }
        if(Tris[i].v1.y > g_max_Y){
            g_max_Y = Tris[i].v1.y;
        }
        if(Tris[i].v2.y > g_max_Y){
            g_max_Y = Tris[i].v2.y;
        }

        //Getting min and max for Z axis
        if(Tris[i].v0.z < g_min_Z){
            g_min_Z = Tris[i].v0.z;
        }
        if(Tris[i].v1.z < g_min_Z){
            g_min_Z = Tris[i].v1.z;
        }
        if(Tris[i].v2.z < g_min_Z){
            g_min_Z = Tris[i].v2.z;
        }

        if(Tris[i].v0.z > g_max_Z){
            g_max_Z = Tris[i].v0.z;
        }
        if(Tris[i].v1.z > g_max_Z){
            g_max_Z = Tris[i].v1.z;
        }
        if(Tris[i].v2.z > g_max_Z){
            g_max_Z = Tris[i].v2.z;
        }
	}

	fclose(fp);

	//Saving triangles information into vertex buffer objects
	float vert[9*num_triangles];
	float vert_normal[9*num_triangles];
    float color_triangle[3*num_triangles];

	for(i=0; i < num_triangles; i++){
		vert[9*i] = Tris[i].v0.x;
		vert[9*i+1] = Tris[i].v0.y;
		vert[9*i+2] = Tris[i].v0.z;
		vert[9*i+3] = Tris[i].v1.x;
		vert[9*i+4] = Tris[i].v1.y;
		vert[9*i+5] = Tris[i].v1.z;
		vert[9*i+6] = Tris[i].v2.x;
		vert[9*i+7] = Tris[i].v2.y;
		vert[9*i+8] = Tris[i].v2.z;

        //Color coordinates
        color_triangle[3*i] = Tris[i].Color[0];
        color_triangle[3*i+1] = Tris[i].Color[1];
        color_triangle[3*i+2] = Tris[i].Color[2];

		//Vertex normal coordinates
		vert_normal[9*i] = Tris[i].normal[0].x;
		vert_normal[9*i+1] = Tris[i].normal[0].y;
		vert_normal[9*i+2] = Tris[i].normal[0].z;
		vert_normal[9*i+3] = Tris[i].normal[1].x;
		vert_normal[9*i+4] = Tris[i].normal[1].y;
		vert_normal[9*i+5] = Tris[i].normal[1].z;
		vert_normal[9*i+6] = Tris[i].normal[2].x;
		vert_normal[9*i+7] = Tris[i].normal[2].y;
		vert_normal[9*i+8] = Tris[i].normal[2].z;
	}

	//Defining VAO
	unsigned int VBO, VBO_color, VAO;
    glGenVertexArrays(1, &VAO);
    
    // binding the Vertex Array Object.
    glBindVertexArray(VAO);

    /////////////  Binding buffer for vertex VBO //////////////////////
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //Putting buffer data
    glBufferData(GL_ARRAY_BUFFER, num_triangles*9*sizeof(GL_FLOAT), vert, GL_STATIC_DRAW);
    //Position, # dimensions, data type, ##, 0, 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    //Setting 0 in position (according to the specification on the shader)
    glEnableVertexAttribArray(0);

    //Position, # dimensions, data type, ##, 0, 0
    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3* sizeof(float)));
    //Setting 0 in position (according to the specification on the shader)
    //glEnableVertexAttribArray(1);

    //Unbinding the VBO buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    /////////////  Binding buffer for vertex VBO_color //////////////////////
    glGenBuffers(1, &VBO_color);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_color);
    //Putting buffer data
    glBufferData(GL_ARRAY_BUFFER, num_triangles*3*sizeof(GL_FLOAT), color_triangle, GL_STATIC_DRAW);
    //Position, # dimensions, data type, ##, 0, 0
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    //Setting 0 in position (according to the specification on the shader)
    glEnableVertexAttribArray(1);
    //Unbinding the VBO buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //Unbinding VAO
    glBindVertexArray(0);

    return VAO;
}