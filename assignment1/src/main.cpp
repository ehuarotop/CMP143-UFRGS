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

///////////////////////// FUNCTION DECLARATION /////////////////////////
//calback for resizing window
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
//callback for processing input
//void processInput(GLFWwindow *window);
//function to read files with extension .in (models)
unsigned int readFile(const char* filename);

///////////////////////// GLOBAL VARIABLES /////////////////////////
//// DATA ////
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
    //what model is being rendered at the moment (0:none, 1:cube, 2:cow)
    int model_used;
    glm::vec3 cameraPos;
    glm::vec3 cameraFront;
    glm::vec3 cameraUp;
    float deltaTime = 0.0f; // time between current frame and last frame
    float lastFrame = 0.0f;

    MyGLCanvas(Widget *parent) : nanogui::GLCanvas(parent), custom_shader("src/shader_vertex.glsl", "src/shader_fragment.glsl"){

        //Setting initial color to white
        this->color = glm::vec4(1.0f);

        //Setting initial drawing mode to 3 -> solid polygons
        this->drawing_mode = 3;

        //Reading file with the information corresponding to the cube
        custom_shader.use();
        if (model_filename != "data/cube.in" && model_filename != "data/cow_up.in")
            model_used = 0;

    }

    void setModel(const char* filename){
        this->model_filename = filename;
        VAO = readFile(model_filename);

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

        if (filename == "data/cube.in"){
            model_used = 1;
            this->cameraPos = glm::vec3(0.0f, 0.0f, 4.0f);
            this->cameraFront = glm::vec3(0.0f,0.0f,0.0f);
            this->cameraUp = glm::vec3(0.0f,1.0f,0.0f);
        } else if (filename == "data/cow_up.in") {
            model_used = 2;
            this->cameraPos = glm::vec3(0.0f, 0.0f, g_max_total - g_min_total);
            this->cameraFront = glm::vec3(0.0f,0.0f,0.0f);
            this->cameraUp = glm::vec3(0.0f,1.0f,0.0f);
        }
    }

    void setColor(glm::vec4 choosen_color){
        this->color = choosen_color;
    }

    void setDrawingMode(unsigned int drawing_mode){
        this->drawing_mode = drawing_mode;
    }

    /*~MyGLCanvas() {
        mShader.free();
    }*/

    virtual void drawGL() override {

        float currentFrame = glfwGetTime();
        this->deltaTime = currentFrame - this->lastFrame;
        this->lastFrame = currentFrame;

        if (model_used == 0){
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
        }else{
            //Loading the shader program
            custom_shader.use();

            //create transformations
            glm::mat4 model         = glm::mat4(1.0f);
            glm::mat4 view          = glm::mat4(1.0f);
            glm::mat4 projection    = glm::mat4(1.0f);

            float center_x = (g_min_X+g_max_X)/2.0f;
            float center_y = (g_min_Y+g_max_Y)/2.0f;
            float center_z = (g_min_Z+g_max_Z)/2.0f;

            //Translating the model to the origin (0,0,0)
            glm::mat4 trans = glm::translate(glm::mat4(1.0f), glm::vec3(-center_x, -center_y, -center_z));
            model = trans * model;

            view = glm::lookAt(this->cameraPos,
                               this->cameraFront,
                               this->cameraUp);

            projection = glm::perspective(glm::radians(45.0f),
                                            ((float)this->width())/this->height(),
                                            0.1f, g_max_total - g_min_total);

            // retrieve the matrix uniform locations
            unsigned int modelLoc = glGetUniformLocation(custom_shader.ID, "model");
            unsigned int viewLoc  = glGetUniformLocation(custom_shader.ID, "view");
            unsigned int projectionLoc  = glGetUniformLocation(custom_shader.ID, "projection");
            unsigned int colorLoc = glGetUniformLocation(custom_shader.ID, "rasterizer_color");

            // pass them to the shaders (3 different ways)
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
            glUniform4fv(colorLoc, 1, glm::value_ptr(this->color));

            glBindVertexArray(VAO);

            glEnable(GL_DEPTH_TEST);

            if(drawing_mode == 1)
                glDrawArrays(GL_POINTS, 0, g_num_triangles*3);
            else if (drawing_mode == 2){
                glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
                glDrawArrays(GL_TRIANGLES, 0, g_num_triangles*3);
                glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
            }
            else if (drawing_mode == 3)
                glDrawArrays(GL_TRIANGLES, 0, g_num_triangles*3);
            
            //Keyboard and mouse events
            glfwPollEvents();
            
            glDisable(GL_DEPTH_TEST);
        }
    }

private:
    unsigned int VAO;
    CustomShader custom_shader;
    const char* model_filename;
    glm::vec4 color;
    unsigned int drawing_mode;
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

        new Label(tools, "Loading models", "sans-bold");

        Button *load_cube = new Button(tools, "Load cube object");
        load_cube->setCallback([this](){
            mCanvasObject->setModel("data/cube.in");
        });

        Button *load_cow = new Button(tools, "Load Cow object");
        load_cow->setCallback([this](){
            mCanvasObject->setModel("data/cow_up.in");
        });

        new Label(tools, "Choose a color", "sans-bold");

        ColorWheel *colorwheel = new ColorWheel(tools);
        colorwheel->setCallback([this](const Color &c){
            glm::vec4 choosen_color = glm::vec4(c.r(), c.g(), c.b(), c.w());
            mCanvasObject->setColor(choosen_color);
        });

        new Label(tools, "Drawing mode", "sans-bold");
        Button *draw_points = new Button(tools, "Points");
        draw_points->setCallback([this](){
            mCanvasObject->setDrawingMode(1);
        });
        Button *draw_wireframe = new Button(tools, "Wireframes");
        draw_wireframe->setCallback([this](){
            mCanvasObject->setDrawingMode(2);
        });
        Button *draw_polygon = new Button(tools, "Solid Polygons");
        draw_polygon->setCallback([this](){
            mCanvasObject->setDrawingMode(3);
        });

        performLayout();
    }

    virtual bool keyboardEvent(int key, int scancode, int action, int modifiers) {
        if (Screen::keyboardEvent(key, scancode, action, modifiers))
            return true;
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            setVisible(false);
            return true;
        }

        float cameraSpeed = 0.5 * mCanvasObject->deltaTime;
        if (key == GLFW_KEY_W && action == GLFW_PRESS)
            mCanvasObject->cameraPos += cameraSpeed * mCanvasObject->cameraFront;
        if (key == GLFW_KEY_S && action == GLFW_PRESS)
            mCanvasObject->cameraPos -= cameraSpeed * mCanvasObject->cameraFront;
        if (key == GLFW_KEY_A && action == GLFW_PRESS)
            mCanvasObject->cameraPos -= glm::normalize(glm::cross(mCanvasObject->cameraFront, 
                                                            mCanvasObject->cameraUp)) * cameraSpeed;
        if (key == GLFW_KEY_D && action == GLFW_PRESS)
            mCanvasObject->cameraPos += glm::normalize(glm::cross(mCanvasObject->cameraFront, 
                                                            mCanvasObject->cameraUp)) * cameraSpeed;

        return false;
    }

    virtual void draw(NVGcontext *ctx) {
        /* Draw the user interface */
        Screen::draw(ctx);
    }

private:
    MyGLCanvas *mCanvasObject;
    //MyGLCanvas *mCanvasGUI;
};

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

    //Setting all mins and maxs to zero in order to recalculate them
    g_min_X=0.0f, g_max_X=0.0f, g_min_Y=0.0f, g_max_Y=0.0f, g_min_Z=0.0f, g_max_Z=0.0f;

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
    /*glGenBuffers(1, &VBO_color);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_color);
    //Putting buffer data
    glBufferData(GL_ARRAY_BUFFER, num_triangles*3*sizeof(GL_FLOAT), color_triangle, GL_STATIC_DRAW);
    //Position, # dimensions, data type, ##, 0, 0
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    //Setting 0 in position (according to the specification on the shader)
    glEnableVertexAttribArray(1);
    //Unbinding the VBO buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0);*/

    //Unbinding VAO
    glBindVertexArray(0);

    return VAO;
}

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
}