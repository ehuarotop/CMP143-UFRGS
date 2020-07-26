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

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;
using namespace nanogui;


int WINDOW_WIDTH=600;
int WINDOW_HEIGHT=600;


///////////////////////////////// CANVAS DECLARATION OPENGL /////////////////////////////////
class MyGLCanvas : public GLCanvas{
public:
    //what model is being rendered at the moment (0:none, 1:cube, 2:cow)
    int model_used;
    //Camera camera = Camera();

    MyGLCanvas(Widget *parent) : nanogui::GLCanvas(parent), custom_shader("../src/shader_vertex.glsl", "../src/shader_fragment.glsl"){

        //Reading file with the information corresponding to the cube
        custom_shader.use();
    }

    virtual void drawGL() override {

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        //Loading the shader program
        custom_shader.use();

        //create transformations
        glm::mat4 model         = glm::mat4(1.0f);
        glm::mat4 view          = glm::mat4(1.0f);
        glm::mat4 projection    = glm::mat4(1.0f);
        
        glDisable(GL_DEPTH_TEST);

    }

private:
    unsigned int VAO;
    CustomShader custom_shader;
};

class App : public nanogui::Screen {
public:

    Window *window;
    Window *windowGUI;

    App() : nanogui::Screen(Eigen::Vector2i(1000, 700), "Final Project", false) {
        using namespace nanogui;

        // Printing to terminal opengl and glsl version
        const GLubyte *vendor      = glGetString(GL_VENDOR);
        const GLubyte *renderer    = glGetString(GL_RENDERER);
        const GLubyte *glversion   = glGetString(GL_VERSION);
        const GLubyte *glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);
        printf("GPU: %s, %s, OpenGL %s, GLSL %s\n", vendor, renderer, glversion, glslversion);

        //Creating window for render object with OpenGL
        window = new Window(this, "3D Visualization");
        window->setPosition(Vector2i(15, 15));
        window->setLayout(new GroupLayout());

        mCanvasObject = new MyGLCanvas(window);
        mCanvasObject->setBackgroundColor({255, 255, 255, 255});
        mCanvasObject->setSize({WINDOW_WIDTH, WINDOW_HEIGHT});

        //Creating window for GUI options
        /*windowGUI = new Window(this, "Options");
        windowGUI->setPosition(Vector2i(1010,15));
        windowGUI->setLayout(new GroupLayout());

        Widget *tools = new Widget(windowGUI);
        tools->setLayout(new BoxLayout(Orientation::Vertical,
                                       Alignment::Fill, 0, 6));

        new Label(tools, "Loading models", "sans-bold");

        Button *load_cube = new Button(tools, "Load cube object");
        load_cube->setCallback([this](){
            //mCanvasObject->setModel("../data/cube.in");
        });*/
    }

    virtual bool scrollEvent(const Vector2i &p, const Vector2f &rel){
        /*if (fov >= 1.0f && fov <= 45.0f)
            fov -= rel.y();
        if (fov <= 1.0f)
            fov = 1.0f;
        if (fov >= 45.0f)
            fov = 45.0f;

        mCanvasObject->camera.fov = fov;*/
    }

    virtual bool mouseMotionEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers){
        /*if(((p.x() >= 15.0f && p.x() <= 15.0f + WINDOW_WIDTH) || 
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
        }*/

        return true;

    }

    virtual bool keyboardEvent(int key, int scancode, int action, int modifiers) {

        if (Screen::keyboardEvent(key, scancode, action, modifiers))
            return true;
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            setVisible(false);
            return true;
        }

        /*const float camera_speed = 1.0f;

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

        /*if (key == GLFW_KEY_Q){
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
        }*/

        return false;
    }

private:
    MyGLCanvas *mCanvasObject;
    MyGLCanvas *mCanvasGUI;
};


int main(){
    try{
        nanogui::init();
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