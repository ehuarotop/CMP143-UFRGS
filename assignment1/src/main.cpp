//#include <glad/glad.h>

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

//NANOGUI includes
#include <nanogui/opengl.h>
#include <nanogui/glutil.h>
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/checkbox.h>
#include <nanogui/button.h>
#include <nanogui/toolbutton.h>
#include <nanogui/popupbutton.h>
#include <nanogui/combobox.h>
#include <nanogui/progressbar.h>
#include <nanogui/entypo.h>
#include <nanogui/messagedialog.h>
#include <nanogui/textbox.h>
#include <nanogui/slider.h>
#include <nanogui/imagepanel.h>
#include <nanogui/imageview.h>
#include <nanogui/vscrollpanel.h>
#include <nanogui/colorwheel.h>
#include <nanogui/graph.h>
#include <nanogui/tabwidget.h>
#include <nanogui/glcanvas.h>
#include <nanogui/nanogui.h>

//#include <glad/glad.h>
//#include <GLFW/glfw3.h>

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

//Loading shaders
CustomShader custom_shader("src/shader_vertex.glsl", "src/shader_fragment.glsl");

///////////////////////// FUNCTION DECLARATION /////////////////////////
//calback for resizing window
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
//callback for processing input
void processInput(GLFWwindow *window);
//function to read files with extension .in (models)
unsigned int readFile(const char* filename);

///////////////////////// GLOBAL VARIABLES /////////////////////////
//// NANOGUI ////
//Screen *screen = nullptr;
MatrixXu indices;
MatrixXf positions;

//// DATA ////
//MatrixXf positions;
float g_min_X=0.0f, g_max_X=0.0f, g_min_Y=0.0f, g_max_Y=0.0f, g_min_Z=0.0f, g_max_Z=0.0f;
float g_min_total=0.0f, g_max_total=0.0f;
const int MAX_MATERIAL_COUNT = 1; //Defining constant for max number of materials in input file.
int g_num_triangles = 0;


//Defining struct for triangle
struct Triangle {
	glm::vec3 v0, v1, v2, face_normal;
	glm::vec3 normal[3];
	float Color[3];
};

//Defining a canvas class where objects will be rendered
class MyGLCanvas : public GLCanvas{
public:

    MyGLCanvas(Widget *parent) : nanogui::GLCanvas(parent){
        //using namespace nanogui;

        //Setting viewport
        glViewport(0, 0, 500, 500);

        //Reading file with the information corresponding to the cube
        VAO = readFile("data/cube.in");

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

        /*mShader.init(
            /* An identifying name */
            //"assigment_shader",

            /* Vertex shader */
            /*"#version 330\n"
            "layout (location = 0) in vec3 vert;\n"
            "layout (location = 1) in vec3 color;\n"
            "out vec4 rasterizer_color;\n"
            "uniform mat4 model;\n"
            "uniform mat4 view;\n"
            "uniform mat4 projection;\n"
            "void main() {\n"
            "    gl_Position = projection * view * model * vec4(vert, 1.0);\n"
            "    rasterizer_color = vec4(color, 1.0f);\n"
            "}",

            /* Fragment shader */
            /*"#version 330\n"
            "uniform vec4 rasterizer_color;\n"
            "out vec4 FragColor;\n"
            "void main() {\n"
            "    FragColor = rasterizer_color;\n"
            "}"
        );*/

        //Binding the shader
        //mShader.bind();

    }

    /*~MyGLCanvas() {
        mShader.free();
    }*/

    virtual void drawGL() override {
        //Loading the shader program
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
                                        float(500)/float(500),
                                        0.1f, g_max_total - g_min_total);

        // retrieve the matrix uniform locations
        unsigned int modelLoc = glGetUniformLocation(custom_shader.ID, "model");
        unsigned int viewLoc  = glGetUniformLocation(custom_shader.ID, "view");
        unsigned int projectionLoc  = glGetUniformLocation(custom_shader.ID, "projection");

        // pass them to the shaders (3 different ways)
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);

        glEnable(GL_DEPTH_TEST);
        /* Draw 12 triangles starting at index 0 */
        //mShader.drawIndexed(GL_TRIANGLES, 0, 12);

        glDrawArrays(GL_TRIANGLES, 0, g_num_triangles*3);

        //Swapping buffers
        //glfwSwapBuffers(this);
        
        //Keyboard and mouse events
        glfwPollEvents();
        
        glDisable(GL_DEPTH_TEST);
    }

private:
    nanogui::GLShader mShader;
    unsigned int VAO;
    //CustomShader custom_shader;
};


class App : public nanogui::Screen {
public:

    Window *window;
    Window *windowGUI;

    App() : nanogui::Screen(Eigen::Vector2i(800, 600), "Programming Assignment 1", false) {
        using namespace nanogui;

        // Printing to terminal opengl and glsl version
        const GLubyte *vendor      = glGetString(GL_VENDOR);
        const GLubyte *renderer    = glGetString(GL_RENDERER);
        const GLubyte *glversion   = glGetString(GL_VERSION);
        const GLubyte *glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);
        printf("GPU: %s, %s, OpenGL %s, GLSL %s\n", vendor, renderer, glversion, glslversion);

        //Creating window for render the object and perform operations
        window = new Window(this, "Rendering Polygonal Meshes with OpenGL and GLSL shaders");
        window->setPosition(Vector2i(15, 15));
        window->setLayout(new GroupLayout());

        mCanvasObject = new MyGLCanvas(window);
        mCanvasObject->setBackgroundColor({100, 100, 100, 255});
        mCanvasObject->setSize({500, 500});

        windowGUI = new Window(this, "Options");
        windowGUI->setPosition(Vector2i(570,15));
        windowGUI->setLayout(new GroupLayout());

        Widget *tools = new Widget(windowGUI);
        tools->setLayout(new BoxLayout(Orientation::Vertical,
                                       Alignment::Fill, 0, 6));

        Button *load_cube = new Button(tools, "Load cube object");
        load_cube->setCallback([this](){
            cout<<"load cube button pressed\n";
            cout<<custom_shader.ID<<"\n";
        });

        Button *load_cow = new Button(tools, "Load Cow object");
        load_cow->setCallback([this](){
            cout<<"load cube button pressed\n";
        });

        /*Button *b0 = new Button(tools, "Random Color");
        b0->setCallback([this]() { mCanvas->setBackgroundColor(Vector4i(rand() % 256, rand() % 256, rand() % 256, 255)); });

        Button *b1 = new Button(tools, "Random Rotation");
        b1->setCallback([this]() { mCanvas->setRotation(nanogui::Vector3f((rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f)); });*/

        performLayout();
    }

    virtual bool keyboardEvent(int key, int scancode, int action, int modifiers) {
        if (Screen::keyboardEvent(key, scancode, action, modifiers))
            return true;
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            setVisible(false);
            return true;
        }
        return false;
    }

    virtual void draw(NVGcontext *ctx) {
        /* Draw the user interface */
        Screen::draw(ctx);
    }

private:
    MyGLCanvas *mCanvasObject;
    MyGLCanvas *mCanvasGUI;
};

int main(){

    try{
        nanogui::init();
        
        // scoped variables
        {
            nanogui::ref<App> app = new App();
            //Indicating that current context will be window recently created
            app->drawAll();
            app->setVisible(true);
            nanogui::mainloop();
        }

        nanogui::shutdown();
    }catch (const std::runtime_error &e) {
        std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
        #if defined(_WIN32)
            MessageBoxA(nullptr, error_msg.c_str(), NULL, MB_ICONERROR | MB_OK);
        #else
            std::cerr << error_msg << endl;
        #endif
        return -1;
    }

    //return 0;

    //Initiating glfw
    /*glfwInit();

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
	}*/
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
    //return 0;
}