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
//function to read files with extension .in (models)
unsigned int readFile(const char* filename);

///////////////////////// GLOBAL VARIABLES /////////////////////////
//// DATA ////
float g_min_X=0.0f, g_max_X=0.0f, g_min_Y=0.0f, g_max_Y=0.0f, g_min_Z=0.0f, g_max_Z=0.0f;
float g_min_total=0.0f, g_max_total=0.0f;
const int MAX_MATERIAL_COUNT = 1; //Defining constant for max number of materials in input file.
int g_num_triangles = 0;

////////NANOGUI///////////
bool firstMouse = true;
float yaw   = -90.0f;   // yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch =  0.0f;
float lastX =  500.0f / 2.0;
float lastY =  500.0 / 2.0;
float translateCamZ = 0.0f;
float translateCamX = 0.0f;
float translateCamY = 0.0f;
float fov   =  45.0f;
std::string text_textBox = "";
float g_near_plane = 1.0f;
float g_far_plane = 3000.0f;

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
    glm::vec3 cameraRight;
    float deltaTime = 0.0f; // time between current frame and last frame
    float lastFrame = 0.0f;
    float distanceProjSphere = 0.0f;

    MyGLCanvas(Widget *parent) : nanogui::GLCanvas(parent), custom_shader("../src/shader_vertex.glsl", "../src/shader_fragment.glsl"){

        //Setting initial color to white
        this->color = glm::vec4(1.0f);

        //Setting initial drawing mode to 3 -> solid polygons
        this->drawing_mode = 3;

        //Reading file with the information corresponding to the cube
        custom_shader.use();
        if (model_filename != "../data/cube.in" && model_filename != "../data/cow_up.in")
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

        if (filename == "../data/cube.in"){
            model_used = 1;
            distanceProjSphere = 6.0f;
            this->cameraPos = glm::vec3(0.0f, 0.0f, distanceProjSphere);
            this->cameraFront = glm::vec3(0.0f,0.0f,-1.0f);
            this->cameraUp = glm::vec3(0.0f,1.0f,0.0f);
            this->cameraRight = glm::vec3(1.0f,0.0f,0.0f);
            translateCamZ = 0.0f;
            translateCamX = 0.0f;
            translateCamY = 0.0f;
        } else if (filename == "../data/cow_up.in") {
            model_used = 2;
            distanceProjSphere = g_max_total - g_min_total;
            this->cameraPos = glm::vec3(0.0f, 0.0f, distanceProjSphere);
            this->cameraFront = glm::vec3(0.0f,0.0f,-1.0f);
            this->cameraUp = glm::vec3(0.0f,1.0f,0.0f);
            this->cameraRight = glm::vec3(1.0f,0.0f,0.0f);
            translateCamZ = 0.0f;//g_max_total - g_min_total;
            translateCamX = 0.0f;
            translateCamY = 0.0f;
        }

        firstMouse = true;
    }

    void setColor(glm::vec4 choosen_color){
        this->color = choosen_color;
    }

    void setDrawingMode(unsigned int drawing_mode){
        this->drawing_mode = drawing_mode;
    }

    void setCullingOrientation(unsigned int culling_orientation){
        this->culling_orientation = culling_orientation;
    }

    virtual bool scrollEvent(const Vector2i &p, const Vector2f &rel){
        if (fov >= 1.0f && fov <= 45.0f)
            fov -= rel.y();
        if (fov <= 1.0f)
            fov = 1.0f;
        if (fov >= 45.0f)
            fov = 45.0f;
    }

    virtual bool mouseMotionEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers){
        //If the mouse is moved for the very first time
        if(firstMouse){
            lastX = p.x();
            lastY = p.y();
            firstMouse = false;
        }

        float xoffset = p.x() - lastX;
        float yoffset = lastY - p.y(); // reversed since y-coordinates go from bottom to top
        lastX = p.x();
        lastY = p.y();

        float sensitivity = 0.01f; // change this value to your liking
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw += xoffset;
        pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        /*glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));*/

        //cameraFront = glm::normalize(front);

        return true;

    }

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

            if(firstMouse){

                if(model_used == 1)
                    cameraPos = glm::normalize(cameraPos)*(distanceProjSphere);
                else
                    cameraPos = glm::normalize(cameraPos)*(distanceProjSphere);

                view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
                
            } else {

                if(model_used == 1)
                    cameraPos = glm::normalize(cameraPos)*(distanceProjSphere);
                else
                    cameraPos = glm::normalize(cameraPos)*(distanceProjSphere);

                view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
                
                //Performing rotation
                view = glm::rotate(view, pitch, glm::vec3(-1.0f, 0.0f, 0.0f));
                view = glm::rotate(view, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
            }

            projection = glm::perspective(glm::radians(fov),
                                            ((float)this->width())/this->height(),
                                            g_near_plane, g_far_plane);

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

            if(culling_orientation == 1){
                glEnable(GL_CULL_FACE);
                glCullFace(GL_FRONT);
                glFrontFace(GL_CW); 
            }else if (culling_orientation == 2){
                glEnable(GL_CULL_FACE);
                glCullFace(GL_FRONT);
                glFrontFace(GL_CCW);
            }

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
            
            glDisable(GL_DEPTH_TEST);
        }
    }

private:
    unsigned int VAO;
    CustomShader custom_shader;
    const char* model_filename;
    glm::vec4 color;
    unsigned int drawing_mode;
    unsigned int culling_orientation;
};

class MyTextBox : public nanogui::TextBox{
public:

    //bool isFocused = false;

    MyTextBox(Widget *parent) : nanogui::TextBox(parent){

    }

    virtual bool focusEvent(bool focused){
        if (focused){
            this->setValue("");
            //isFocused = true;
            this->setFocused(true);
        }else{
            //isFocused = false;
            this->setFocused(false);
            text_textBox = "";
        }
    }

private:
    int type;
};

class App : public nanogui::Screen {
public:

    Window *window;
    Window *windowGUI;
    MyTextBox *textBox_np;
    MyTextBox *textBox_fp;

    App() : nanogui::Screen(Eigen::Vector2i(800, 650), "Programming Assignment 1", false) {
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
            mCanvasObject->setModel("../data/cube.in");
        });

        Button *load_cow = new Button(tools, "Load Cow object");
        load_cow->setCallback([this](){
            mCanvasObject->setModel("../data/cow_up.in");
        });

        new Label(tools, "Choose a color", "sans-bold");

        ColorWheel *colorwheel = new ColorWheel(tools);
        colorwheel->setCallback([this](const Color &c){
            glm::vec4 choosen_color = glm::vec4(c.r(), c.g(), c.b(), c.w());
            mCanvasObject->setColor(choosen_color);
        });

        new Label(tools, "Near Plane", "sans-bold");

        textBox_np = new MyTextBox(tools);
        textBox_np->setFixedSize(Vector2i(200, 25));
        textBox_np->setValue("1.0");
        textBox_np->setUnits("f");
        textBox_np->setFontSize(16);

        new Label(tools, "Far Plane", "sans-bold");

        textBox_fp = new MyTextBox(tools);
        textBox_fp->setFixedSize(Vector2i(200, 25));
        textBox_fp->setValue("3000.0");
        textBox_fp->setUnits("f");
        textBox_fp->setFontSize(16);
        textBox_fp->setFormat("[-]?[0-9]*\\.?[0-9]+");

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


        new Label(tools, "Culling Orientation", "sans-bold");
        Button *clockwise = new Button(tools, "Clockwise");
        clockwise->setCallback([this](){
            mCanvasObject->setCullingOrientation(1);
        });
        Button *counter_clockwise = new Button(tools, "Counter Clockwise");
        counter_clockwise->setCallback([this](){
            mCanvasObject->setCullingOrientation(2);
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

        const float camera_speed = 1.0f;

        if (key == GLFW_KEY_W){
            while(action == GLFW_REPEAT || action == GLFW_PRESS){
                if(this->mCanvasObject->model_used == 1)
                    this->mCanvasObject->cameraPos += 0.25f*camera_speed * this->mCanvasObject->cameraFront;
                else
                    this->mCanvasObject->cameraPos += 15.0f*camera_speed * this->mCanvasObject->cameraFront;

                //Updating distance Projection Sphere
                this->mCanvasObject->distanceProjSphere = glm::length(this->mCanvasObject->cameraPos);

                return true;
            }
        }

        if (key == GLFW_KEY_S){
            while(action == GLFW_REPEAT || action == GLFW_PRESS){
                if(this->mCanvasObject->model_used == 1)
                    this->mCanvasObject->cameraPos -= 0.25f*camera_speed * this->mCanvasObject->cameraFront;
                else
                    this->mCanvasObject->cameraPos -= 15.0f*camera_speed * this->mCanvasObject->cameraFront;

                //Updating distance Projection Sphere
                this->mCanvasObject->distanceProjSphere = glm::length(this->mCanvasObject->cameraPos);

                return true;
            }
        
        }

        if (key == GLFW_KEY_D){
            while(action == GLFW_REPEAT || action == GLFW_PRESS){

                this->mCanvasObject->cameraRight = glm::normalize(-glm::cross(this->mCanvasObject->cameraFront, 
                                                            this->mCanvasObject->cameraUp));

                if(this->mCanvasObject->model_used == 1)
                    this->mCanvasObject->cameraPos +=  this->mCanvasObject->cameraRight * camera_speed;
                else
                    this->mCanvasObject->cameraPos += this->mCanvasObject->cameraRight * 60.0f*camera_speed;

                this->mCanvasObject->cameraFront = glm::normalize(-this->mCanvasObject->cameraPos);
                /*this->mCanvasObject->cameraRight = glm::normalize(-glm::cross(this->mCanvasObject->cameraFront, 
                                                            this->mCanvasObject->cameraUp));*/

                return true;
            }
        }

        if (key == GLFW_KEY_A){
            while(action == GLFW_REPEAT || action == GLFW_PRESS){

                this->mCanvasObject->cameraRight = glm::normalize(-glm::cross(this->mCanvasObject->cameraFront, 
                                                            this->mCanvasObject->cameraUp));

                if(this->mCanvasObject->model_used == 1)
                    this->mCanvasObject->cameraPos -= this->mCanvasObject->cameraRight * camera_speed;
                else
                    this->mCanvasObject->cameraPos -= this->mCanvasObject->cameraRight * 60.0f*camera_speed;

                this->mCanvasObject->cameraFront = glm::normalize(-this->mCanvasObject->cameraPos);
                /*this->mCanvasObject->cameraRight = glm::normalize(-glm::cross(this->mCanvasObject->cameraFront, 
                                                            this->mCanvasObject->cameraUp));*/

                return true;
            }
        }

        if (key == GLFW_KEY_Q){
            while(action == GLFW_REPEAT || action == GLFW_PRESS){

                this->mCanvasObject->cameraUp = glm::normalize(-glm::cross(this->mCanvasObject->cameraFront, 
                                                            this->mCanvasObject->cameraRight));

                if(this->mCanvasObject->model_used == 1)
                    this->mCanvasObject->cameraPos += this->mCanvasObject->cameraUp * camera_speed;
                else
                    this->mCanvasObject->cameraPos += this->mCanvasObject->cameraUp * 60.0f*camera_speed;


                this->mCanvasObject->cameraFront = glm::normalize(-this->mCanvasObject->cameraPos);

                return true;
            }
        }

        if (key == GLFW_KEY_Z){
            while(action == GLFW_REPEAT || action == GLFW_PRESS){

                this->mCanvasObject->cameraUp = glm::normalize(-glm::cross(this->mCanvasObject->cameraFront, 
                                                            this->mCanvasObject->cameraRight));

                if(this->mCanvasObject->model_used == 1)
                    this->mCanvasObject->cameraPos -= this->mCanvasObject->cameraUp * camera_speed;
                else
                    this->mCanvasObject->cameraPos -= this->mCanvasObject->cameraUp * 60.0f*camera_speed;

                this->mCanvasObject->cameraFront = glm::normalize(-this->mCanvasObject->cameraPos);

                return true;
            }
        }

        //Implementing delete of string with backspace (set with scancode because glfw 
        //does not recognize GLFW_KEY_BACKSPACE)
        if(key == -1 && scancode == 22){
            while(action == GLFW_REPEAT || action == GLFW_PRESS){
                text_textBox = text_textBox.substr(0, text_textBox.size()-1);

                if(textBox_np->focused())
                    textBox_np->setValue(text_textBox);
                else if (textBox_fp->focused())
                    textBox_fp->setValue(text_textBox);

                return true;
            }
        }

        if (key >= 46 && key <= 57 && action == GLFW_PRESS){

            if(textBox_np->focused() || textBox_fp->focused()){
                if (key == 48)
                    text_textBox += "0";
                else if (key == 49)
                    text_textBox += "1";
                else if (key == 50)
                    text_textBox += "2";
                else if (key == 51)
                    text_textBox += "3";
                else if (key == 52)
                    text_textBox += "4";
                else if (key == 53)
                    text_textBox += "5";
                else if (key == 54)
                    text_textBox += "6";
                else if (key == 55)
                    text_textBox += "7";
                else if (key == 56)
                    text_textBox += "8";
                else if (key == 57)
                    text_textBox += "9";
                else if (key == 46)
                    text_textBox += ".";

                if(textBox_np->focused())
                    textBox_np->setValue(text_textBox);
                else if (textBox_fp->focused())
                    textBox_fp->setValue(text_textBox);

                g_near_plane = std::stof(textBox_np->value());
                g_far_plane = std::stof(textBox_fp->value());

            }

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