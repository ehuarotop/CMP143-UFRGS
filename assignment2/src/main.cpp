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
//to print vectors and matrices
#include <glm/gtx/string_cast.hpp>

#include <shader_s.h>
#include <camera.h>
#include <close2gl.h>
#include <matrix.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;
using namespace nanogui;

//Defining struct for triangle
struct Triangle {
    glm::vec3 v0, v1, v2, face_normal;
    glm::vec3 normal[3];
    float Color[3];
};

//Defining struct for triangle (close2gl)
struct Triangle_c2gl {
    glm::vec4 v0, v1, v2;
    glm::vec3 face_normal, normal[3];
    float Color[3];  
};

///////////////////////// FUNCTION DECLARATION /////////////////////////
//function to read files with extension .in (models)
unsigned int readFile(const char* filename);
vector<Triangle_c2gl> readFile_close2gl(const char* filename);

///////////////////////// GLOBAL VARIABLES /////////////////////////
//// DATA ////
float g_min_X=0.0f, g_max_X=0.0f, g_min_Y=0.0f, g_max_Y=0.0f, g_min_Z=0.0f, g_max_Z=0.0f;
float g_min_total=0.0f, g_max_total=0.0f;
const int MAX_MATERIAL_COUNT = 1; //Defining constant for max number of materials in input file.
int g_num_triangles = 0;

////////NANOGUI///////////
unsigned int WINDOW_WIDTH = 450;
unsigned int WINDOW_HEIGHT = 450;
bool firstMouse = true;
float yaw   = -90.0f;   // yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch =  0.0f;
float lastX =  (float)(WINDOW_WIDTH) / 2.0;
float lastY =  (float)(WINDOW_HEIGHT) / 2.0;
float translateCamZ = 0.0f;
float translateCamX = 0.0f;
float translateCamY = 0.0f;
float fov   =  60.0f;
std::string text_textBox = "";
float g_near_plane = 1.0f;
float g_far_plane = 3000.0f;

int fpsrate_opengl = 0.0f;
int fpsrate_close2gl = 0.0f;


//Debugging
int count_print;

///////////////////////////////// CANVAS DECLARATION OPENGL /////////////////////////////////
class MyGLCanvas : public GLCanvas{
public:
    //what model is being rendered at the moment (0:none, 1:cube, 2:cow)
    int model_used;
    Camera camera = Camera();
    float deltaTime = 0.0f; // time between current frame and last frame
    float lastFrame = 0.0f;
    float distanceProjSphere = 0.0f;

    //variables for fps rate
    int framesPerSecond = 0;
    float lastTime = 0.0f;

    MyGLCanvas(Widget *parent) : nanogui::GLCanvas(parent), custom_shader("../src/shader_vertex.glsl", "../src/shader_fragment.glsl"){

        //Setting initial color to white
        this->color = glm::vec4(1.0f);

        //Setting initial drawing mode to 3 -> solid polygons
        this->drawing_mode = 3;

        //Setting initial culling orientation to clockwise
        //this->culling_orientation = 1;

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
            camera.setDistanceProjSphere(6.0f);
        } else if (filename == "../data/cow_up.in") {
            model_used = 2;
            camera.setDistanceProjSphere(g_max_total - g_min_total);
        }

        camera.setModel(model_used);

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

    virtual void drawGL() override {

        double currentTime = glfwGetTime();
        ++framesPerSecond;

        if(currentTime - lastTime > 1.0f){
            lastTime = currentTime;
            fpsrate_opengl = framesPerSecond;
            framesPerSecond = 0;
        }

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

            //Normalizing and scaling cameraPos by distanceProjSphere
            camera.cameraPos = glm::normalize(camera.cameraPos)*(camera.distanceProjSphere);

            if(firstMouse){
                view = camera.getLookAtMatrix();
            } else {
                view = camera.getLookAtMatrix();

                //Performing rotation
                view = glm::rotate(view, camera.pitch, glm::vec3(-1.0f, 0.0f, 0.0f));
                view = glm::rotate(view, camera.yaw, glm::vec3(0.0f, 1.0f, 0.0f));
            }

            projection = glm::perspective(glm::radians(camera.fov),
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
                glCullFace(GL_BACK);
                glFrontFace(GL_CW); 
            }else if (culling_orientation == 2){
                glEnable(GL_CULL_FACE);
                glCullFace(GL_BACK);
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


///////////////////////////////// CANVAS DECLARATION CLOSE2GL /////////////////////////////////
class MyGLCanvasC2GL : public GLCanvas{
public:
    //what model is being rendered at the moment (0:none, 1:cube, 2:cow)
    int model_used;
    //Camera camera = Camera();
    Close2GL close2gl = Close2GL();
    Matrix matrix = Matrix();
    vector<Triangle_c2gl> read_triangles;
    vector<Triangle_c2gl> triangles;
    vector<Triangle_c2gl> clipped_triangles;
    vector<Triangle_c2gl> temp;
    float distanceProjSphere = 0.0f;

    //variables for fps rate
    int framesPerSecond = 0;
    float lastTime = 0.0f;

    MyGLCanvasC2GL(Widget *parent) : nanogui::GLCanvas(parent), custom_shader("../src/shader_vertex_c2gl.glsl", "../src/shader_fragment_c2gl.glsl"){

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
        read_triangles = readFile_close2gl(model_filename);

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
            close2gl.setDistanceProjSphere(6.0f);
        } else if (filename == "../data/cow_up.in") {
            model_used = 2;
            close2gl.setDistanceProjSphere(g_max_total - g_min_total);
        }

        //camera.setModel(model_used);
        close2gl.setModel(model_used);

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

    virtual void drawGL() override {

        double currentTime = glfwGetTime();
        ++framesPerSecond;

        if(currentTime - lastTime > 1.0f){
            lastTime = currentTime;
            fpsrate_close2gl = framesPerSecond;
            framesPerSecond = 0;
        }

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

            //Translating the model to the origin (0,0,0) using manually implemented matrix class.
            glm::mat4 trans = matrix.translate(glm::vec3(-center_x, -center_y, -center_z));
            model = matrix.multiply_matrix(trans, model);

            //Normalizing and scaling cameraPos by distanceProjSphere
            close2gl.position = matrix.normalizev3(close2gl.position)*(close2gl.distanceProjSphere);

            if(firstMouse){
                view = close2gl.getLookAtMatrix();
            } else {
                view = close2gl.getLookAtMatrix();

                //Performing rotation
                view = matrix.rotate(view, close2gl.pitch, glm::vec3(-1.0f, 0.0f, 0.0f));
                view = matrix.rotate(view, close2gl.yaw, glm::vec3(0.0f, 1.0f, 0.0f));
                
            }

            projection = close2gl.getProjectionMatrix(glm::radians(close2gl.fov), 
                                                        ((float)this->width())/this->height(),
                                                        g_near_plane, g_far_plane);


            //Calculating the model view projection matrix
            glm::mat4 modelViewProj = projection * view * model;

            //assigning read_triangles to triangles vector in order to iterate over it.
            triangles = read_triangles;

            for (int i=0; i<triangles.size(); i++){

                triangles[i].v0 = matrix.transform_vector(triangles[i].v0, modelViewProj);
                triangles[i].v1 = matrix.transform_vector(triangles[i].v1, modelViewProj);
                triangles[i].v2 = matrix.transform_vector(triangles[i].v2, modelViewProj);

            }

            //Clearing clipped_triangles
            clipped_triangles.clear();

            //Clipping (considering) only triangles inside the perspective volume
            for(int i=0; i<triangles.size(); i++){
                //Getting w position of each vertex
                float w0 = abs(triangles[i].v0.w);
                float w1 = abs(triangles[i].v1.w);
                float w2 = abs(triangles[i].v2.w);

                if ( abs(triangles[i].v0.z) <= w0 && abs(triangles[i].v1.z) <= w1 && abs(triangles[i].v2.z <= w2)){
                    clipped_triangles.push_back(triangles[i]);
                }

            }

            //glm::mat4 viewportMatrix = close2gl.getViewPortMatrix(0.0f, float(WINDOW_WIDTH), float(WINDOW_HEIGHT), 0.0f);

            //Performing perspective division over the clipped triangles and transforming them with viewport matrix
            for(int i=0; i<clipped_triangles.size(); i++){
                clipped_triangles[i].v0 = clipped_triangles[i].v0 / clipped_triangles[i].v0.w;
                clipped_triangles[i].v1 = clipped_triangles[i].v1 / clipped_triangles[i].v1.w;
                clipped_triangles[i].v2 = clipped_triangles[i].v2 / clipped_triangles[i].v2.w;

            }


            //Performing backface culling
            temp.clear();
            for(int i=0; i<clipped_triangles.size(); i++){
                glm::vec3 vecBA = clipped_triangles[i].v1 - clipped_triangles[i].v0;
                glm::vec3 vecCA = clipped_triangles[i].v2 - clipped_triangles[i].v0;

                glm::vec3 bfvec = matrix.crossProduct(vecBA, vecCA);

                if(culling_orientation == 1){
                    //clockwise
                    if(bfvec.z < 0){
                        temp.push_back(clipped_triangles[i]);
                    }
                } else if (culling_orientation == 2){
                    if(bfvec.z > 0) {
                        //counter clockwise
                        temp.push_back(clipped_triangles[i]);
                    }
                } else {
                    temp.push_back(clipped_triangles[i]);
                }

            }

            clipped_triangles = temp;

            //Getting vertices in a 1d array to pass it to the shader
            float vert[6*clipped_triangles.size()];

            for(int i=0; i < clipped_triangles.size(); i++){
                vert[6*i]   = clipped_triangles[i].v0.x;
                vert[6*i+1] = clipped_triangles[i].v0.y;
                vert[6*i+2] = clipped_triangles[i].v1.x;
                vert[6*i+3] = clipped_triangles[i].v1.y;
                vert[6*i+4] = clipped_triangles[i].v2.x;
                vert[6*i+5] = clipped_triangles[i].v2.y;
            }

            // Dealing with VAOs and VBOs
            unsigned int VBO, VAO;
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            // binding the Vertex Array Object.
            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            //Putting buffer data
            glBufferData(GL_ARRAY_BUFFER, sizeof(vert), vert, GL_STATIC_DRAW);
            //Position, # dimensions, data type, ##, 0, 0
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
            //Setting 0 in position (according to the specification on the shader)
            glEnableVertexAttribArray(0);

            // ---------------- Actual Drawing ---------------- //
            //glEnable(GL_DEPTH_TEST);

            if(drawing_mode == 1){
                glDrawArrays(GL_POINTS, 0, clipped_triangles.size()*3);
            } else if (drawing_mode == 2){
                glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
                glDrawArrays(GL_TRIANGLES, 0, clipped_triangles.size()*3);
                glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
            }
            else if (drawing_mode == 3)
                glDrawArrays(GL_TRIANGLES, 0, clipped_triangles.size()*3);

            //glDisable(GL_DEPTH_TEST);

            // ---------------- Actual Drawing ---------------- //


            //Unbinding the VBO buffer
            glBindBuffer(GL_ARRAY_BUFFER, 0);


            // Passing only color as uniform to the fragment shader. Retrieving the matrix uniform locations
            unsigned int colorLoc = glGetUniformLocation(custom_shader.ID, "rasterizer_color");

            // pass matrix uniform locations to the shaders
            glUniform4fv(colorLoc, 1, glm::value_ptr(this->color));

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

    MyTextBox(Widget *parent) : nanogui::TextBox(parent){

    }

    virtual bool focusEvent(bool focused){
        if (focused){
            this->setValue("");
            this->setFocused(true);
        }else{
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
    Window *windowC2GL;
    Window *windowGUI;
    Window *windowGUI2;
    MyTextBox *textBox_np;
    MyTextBox *textBox_fp;
    MyTextBox *textBox_fpsrate;

    App() : nanogui::Screen(Eigen::Vector2i(1250, 900), "Programming Assignment 1", false) {
        using namespace nanogui;

        // Printing to terminal opengl and glsl version
        const GLubyte *vendor      = glGetString(GL_VENDOR);
        const GLubyte *renderer    = glGetString(GL_RENDERER);
        const GLubyte *glversion   = glGetString(GL_VERSION);
        const GLubyte *glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);
        printf("GPU: %s, %s, OpenGL %s, GLSL %s\n", vendor, renderer, glversion, glslversion);

        //Creating window for render object with OpenGL
        window = new Window(this, "OpenGL Implementation");
        window->setPosition(Vector2i(15, 15));
        window->setLayout(new GroupLayout());

        mCanvasObject = new MyGLCanvas(window);
        mCanvasObject->setBackgroundColor({100, 100, 100, 255});
        mCanvasObject->setSize({WINDOW_WIDTH, WINDOW_HEIGHT});


        //Creating window for render object with Close2GL
        windowC2GL = new Window(this, "Close2GL Implementation");
        windowC2GL->setPosition(Vector2i(515, 15));
        windowC2GL->setLayout(new GroupLayout());

        mCanvasObjectC2GL = new MyGLCanvasC2GL(windowC2GL);
        mCanvasObjectC2GL->setBackgroundColor({100, 100, 100, 255});
        mCanvasObjectC2GL->setSize({WINDOW_WIDTH, WINDOW_HEIGHT});

        //Creating window for GUI options
        windowGUI = new Window(this, "Options");
        windowGUI->setPosition(Vector2i(1010,15));
        windowGUI->setLayout(new GroupLayout());

        Widget *tools = new Widget(windowGUI);
        tools->setLayout(new BoxLayout(Orientation::Vertical,
                                       Alignment::Fill, 0, 6));

        new Label(tools, "Loading models", "sans-bold");

        Button *load_cube = new Button(tools, "Load cube object");
        load_cube->setCallback([this](){
            mCanvasObject->setModel("../data/cube.in");
            mCanvasObjectC2GL->setModel("../data/cube.in");
        });

        Button *load_cow = new Button(tools, "Load Cow object");
        load_cow->setCallback([this](){
            mCanvasObject->setModel("../data/cow_up.in");
            mCanvasObjectC2GL->setModel("../data/cow_up.in");
        });

        new Label(tools, "Choose a color", "sans-bold");

        ColorWheel *colorwheel = new ColorWheel(tools);
        colorwheel->setCallback([this](const Color &c){
            //Getting chosen color
            glm::vec4 choosen_color = glm::vec4(c.r(), c.g(), c.b(), c.w());

            //Setting color in canvas objects.
            mCanvasObject->setColor(choosen_color);
            mCanvasObjectC2GL->setColor(choosen_color);
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
            //Setting drawing mode to points
            mCanvasObject->setDrawingMode(1);
            mCanvasObjectC2GL->setDrawingMode(1);
        });
        Button *draw_wireframe = new Button(tools, "Wireframes");
        draw_wireframe->setCallback([this](){
            //Setting drawing mode to wireframes
            mCanvasObject->setDrawingMode(2);
            mCanvasObjectC2GL->setDrawingMode(2);
        });
        Button *draw_polygon = new Button(tools, "Solid Polygons");
        draw_polygon->setCallback([this](){
            //Setting drawing mode to polygon
            mCanvasObject->setDrawingMode(3);
            mCanvasObjectC2GL->setDrawingMode(3);
        });


        new Label(tools, "Culling Orientation", "sans-bold");
        Button *normal = new Button(tools, "Without Culling");
        normal->setCallback([this](){
            //Setting culling orientation
            mCanvasObject->setCullingOrientation(0);
            mCanvasObjectC2GL->setCullingOrientation(0);
        });
        Button *clockwise = new Button(tools, "Clockwise");
        clockwise->setCallback([this](){
            //Setting culling orientation
            mCanvasObject->setCullingOrientation(1);
            mCanvasObjectC2GL->setCullingOrientation(1);
        });
        Button *counter_clockwise = new Button(tools, "Counter Clockwise");
        counter_clockwise->setCallback([this](){
            //Setting culling orientation
            mCanvasObject->setCullingOrientation(2);
            mCanvasObjectC2GL->setCullingOrientation(2);
        });

        new Label(tools, "FPS Rate");
        textBox_fpsrate = new MyTextBox(tools);
        textBox_fpsrate->setFixedSize(Vector2i(200, 25));
        textBox_fpsrate->setValue("0.0f");
        textBox_fpsrate->setUnits("fps");
        textBox_fpsrate->setFontSize(16);


        ////////////////////// Second window GUI for missing options in the first one /////////////////////////////
        
        //Creating window for GUI options
        windowGUI2 = new Window(this, "Shading");
        windowGUI2->setPosition(Vector2i(850,525));
        windowGUI2->setLayout(new GroupLayout());

        Widget *tools2 = new Widget(windowGUI2);
        tools2->setLayout(new BoxLayout(Orientation::Vertical,
                                       Alignment::Fill, 0, 6));

        new Label(tools2, "Shading type", "sans-bold");

        Button *gouraud_ad = new Button(tools2, "Gouraud AD");
        gouraud_ad->setCallback([this](){
            mCanvasObject->setModel("../data/cube.in");
            mCanvasObjectC2GL->setModel("../data/cube.in");
        });

        Button *gouraud_ads = new Button(tools2, "Gouraud ADS");
        gouraud_ads->setCallback([this](){
            mCanvasObject->setModel("../data/cow_up.in");
            mCanvasObjectC2GL->setModel("../data/cow_up.in");
        });

        Button *phong = new Button(tools2, "Phong");
        phong->setCallback([this](){
            mCanvasObject->setModel("../data/cow_up.in");
            mCanvasObjectC2GL->setModel("../data/cow_up.in");
        });

        performLayout();
    }

    virtual bool scrollEvent(const Vector2i &p, const Vector2f &rel){
        if (fov >= 1.0f && fov <= 45.0f)
            fov -= rel.y();
        if (fov <= 1.0f)
            fov = 1.0f;
        if (fov >= 45.0f)
            fov = 45.0f;

        mCanvasObject->camera.fov = fov;
        mCanvasObjectC2GL->close2gl.fov = fov;
    }

    virtual bool mouseMotionEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers){
        if(((p.x() >= 15.0f && p.x() <= 15.0f + WINDOW_WIDTH) || 
            (p.x() >= 515.0f && p.x() <= 515.0f + WINDOW_WIDTH)) && (p.y() >= 15.0f && p.y() <= 15.0f + WINDOW_HEIGHT) ){
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

            //Setting new yaw and pitch values for opengl canvas
            mCanvasObject->camera.yaw = yaw;
            mCanvasObject->camera.pitch = pitch;

            //Setting new yaw and pitch values for close2gl canvas
            mCanvasObjectC2GL->close2gl.yaw = yaw;
            mCanvasObjectC2GL->close2gl.pitch = pitch;
        }

        return true;

    }

    virtual bool keyboardEvent(int key, int scancode, int action, int modifiers) {

        if (Screen::keyboardEvent(key, scancode, action, modifiers))
            return true;
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            setVisible(false);
            return true;
        }

        const float camera_speed = 1.0f;

        //Translation along the z axis (towards negative z axis, close up effect)
        if (key == GLFW_KEY_W){
            while(action == GLFW_REPEAT || action == GLFW_PRESS){

                mCanvasObject->camera.processRotation(FORWARD);
                mCanvasObjectC2GL->close2gl.processRotation(FORWARD);

                return true;
            }
        }

        //Translation along the z axis (towards positive z axis, zoom out effect)
        if (key == GLFW_KEY_S){
            while(action == GLFW_REPEAT || action == GLFW_PRESS){

                mCanvasObject->camera.processRotation(BACKWARD);
                mCanvasObjectC2GL->close2gl.processRotation(BACKWARD);

                return true;
            }
        
        }

        //Translation along the X axis (towards positive x axis)
        if (key == GLFW_KEY_D){
            while(action == GLFW_REPEAT || action == GLFW_PRESS){

                mCanvasObject->camera.processRotation(RIGHT);
                mCanvasObjectC2GL->close2gl.processRotation(RIGHT);

                return true;
            }
        }

        if (key == GLFW_KEY_A){
            while(action == GLFW_REPEAT || action == GLFW_PRESS){

                mCanvasObject->camera.processRotation(LEFT);
                mCanvasObjectC2GL->close2gl.processRotation(LEFT);

                return true;
            }
        }

        if (key == GLFW_KEY_Q){
            while(action == GLFW_REPEAT || action == GLFW_PRESS){

                mCanvasObject->camera.processRotation(UP);
                mCanvasObjectC2GL->close2gl.processRotation(UP);

                return true;
            }
        }

        if (key == GLFW_KEY_Z){
            while(action == GLFW_REPEAT || action == GLFW_PRESS){

                mCanvasObject->camera.processRotation(DOWN);
                mCanvasObjectC2GL->close2gl.processRotation(DOWN);

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
        // FPS Rate
        textBox_fpsrate->setValue("OpenGL:" + std::to_string(fpsrate_opengl) + "/Close2GL:" + std::to_string(fpsrate_close2gl));
    }

private:
    MyGLCanvas *mCanvasObject;
    MyGLCanvasC2GL *mCanvasObjectC2GL;
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

vector<Triangle_c2gl> readFile_close2gl(const char* filename){
    vector<Triangle_c2gl> triangles;
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
    struct Triangle_c2gl Tris[num_triangles];

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

        //Adding the homegeneus component
        Tris[i].v0.w = 1.0f;
        Tris[i].v1.w = 1.0f;
        Tris[i].v2.w = 1.0f;
        
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

        triangles.push_back(Tris[i]);

    }

    fclose(fp);

    return triangles;
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