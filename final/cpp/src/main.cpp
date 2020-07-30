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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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
#include <stdlib.h>

using namespace std;
using namespace nanogui;


int WINDOW_WIDTH=950;
int WINDOW_HEIGHT=650;

float positionScale = 10.0f;

//Defining struct for triangle (close2gl)
struct image {
    string path;
    glm::vec3 position;
};

//Function declaration
vector<image> readCSV(const char* filename);
bool replace(std::string& str, const std::string& from, const std::string& to);

//global variables used to control camera
Camera camera = Camera(glm::vec3(0.0f, 0.0f, 20.0f));
float lastX = WINDOW_WIDTH / 2.0f;
float lastY = WINDOW_HEIGHT / 2.0f;
bool firstMouse = true;
bool mouseButtonPressed = false;

float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;

///////////////////////////////// CANVAS DECLARATION OPENGL /////////////////////////////////
class MyGLCanvas : public GLCanvas{
public:

    MyGLCanvas(Widget *parent) : nanogui::GLCanvas(parent), custom_shader("../src/shader_vertex.glsl", "../src/shader_fragment.glsl"){
        //Reading file with the information corresponding to the cube
        images = readCSV("../dataset/Features/position_features.csv");

        //Generating VAOs for each image
        for(int i=0; i<images.size(); i++){
            drawImage(images[i]);
        }

        //Loading the shader program
        custom_shader.use();
    }

    void drawImage(image img){
        unsigned int VAO, VBO, EBO;

        //Loading image at this point to control aspect ratio
        int width, height, nrChannels;
        stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
        unsigned char *data = stbi_load((string("./../") + string(img.path)).c_str(), &width, &height, &nrChannels, 3);

        //Calculating y_size in order to preserve aspect aspect ratio
        //y_size = (float(height) * x_size)/float(width);
        y_size= x_size * (float(height)/float(width));

        //Appending y_size to y_sizes vector
        y_sizes.push_back(y_size);

        //cout<<height<<" "<<width<<" "<<float(height)/float(width)<<" "<<y_size/x_size<<endl;

        float vertices[] = {
            img.position.x + x_size, img.position.y + y_size, img.position.z, 1.0f, 1.0f, // top right
            img.position.x + x_size, img.position.y - y_size, img.position.z, 1.0f, 0.0f, // bottom right
            img.position.x - x_size, img.position.y - y_size, img.position.z, 0.0f, 0.0f, // bottom left
            img.position.x - x_size, img.position.y + y_size, img.position.z, 0.0f, 1.0f// top left 
        };
        
        unsigned int indices[] = {
            0, 1, 3,  // first Triangle
            1, 2, 3   // second Triangle
        };
    
        //unsigned int VBO, VAO, EBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        //Vertexs
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        //Texture coordinates
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
        glBindBuffer(GL_ARRAY_BUFFER, 0); 
        //Unbinding VAO
        glBindVertexArray(0);

        vaos.push_back(VAO);

        unsigned int texture1;
        // texture 1
        // ---------
        glGenTextures(1, &texture1);
        glBindTexture(GL_TEXTURE_2D, texture1);
        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        //create texture and generate mipmaps from loaded image at the begining of this function
        if (data)
        {   
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }   
        else
        {
            std::cout << "Failed to load texture" << std::endl;
            cout<<stbi_failure_reason()<<endl;
        }
        stbi_image_free(data);

        //Storing textures
        textures.push_back(texture1);

    }

    virtual void drawGL() override {

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        //Loading the shader program
        custom_shader.use();

        //Enabling depth test
        glEnable(GL_DEPTH_TEST);

        // pass projection matrix to shader (note that in this case it could change every frame)
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);

        // camera/view transformation
        glm::mat4 view = camera.GetViewMatrix();

        //Billboarding: Getting camera right and Up vector in world space
        glm::vec3 cameraRightWorldSpace = glm::vec3(view[0][0], view[1][0], view[2][0]);
        glm::vec3 cameraUpWorldSpace = glm::vec3(view[0][1], view[1][1], view[2][1]);

        for(int i=0; i<vaos.size(); i++){
            glm::mat4 model = glm::mat4(1.0f);
            //Passing model, view and projection matrix to the vertex shader
            unsigned int modelLoc = glGetUniformLocation(custom_shader.ID, "model");
            unsigned int viewLoc  = glGetUniformLocation(custom_shader.ID, "view");
            unsigned int projectionLoc  = glGetUniformLocation(custom_shader.ID, "projection");
            unsigned int textureLoc = glGetUniformLocation(custom_shader.ID, "texture1");
            unsigned int cameraRightWorldSpaceLoc = glGetUniformLocation(custom_shader.ID, "cameraRightWorldSpace");
            unsigned int cameraUpWorldSpaceLoc = glGetUniformLocation(custom_shader.ID, "cameraUpWorldSpace");
            unsigned int billboardPositionLoc = glGetUniformLocation(custom_shader.ID, "billboardPosition");
            unsigned int billboardSizeLoc = glGetUniformLocation(custom_shader.ID, "billboardSize");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
            glUniform1i(textureLoc, 0);

            //Setting needed elements for billboarding
            glm::vec3 billboardPosition = images[i].position;
            //glm::vec2 billboardSize = glm::vec2(0.4f, y_sizes[i]*2.0f);
            glm::vec2 billboardSize = glm::vec2(1.0f, 1.0f); //why??

            glUniform3fv(cameraRightWorldSpaceLoc, 1, glm::value_ptr(cameraRightWorldSpace));
            glUniform3fv(cameraUpWorldSpaceLoc, 1, glm::value_ptr(cameraUpWorldSpace));
            glUniform3fv(billboardPositionLoc, 1, glm::value_ptr(billboardPosition));
            glUniform2fv(billboardSizeLoc, 1, glm::value_ptr(billboardSize));

            //Activating textures
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textures[i]);

            //Binding VAOs
            glBindVertexArray(vaos[i]);
            //Actual drawing
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            //Unbinding VAOs
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0); 
        }

        glDisable(GL_DEPTH_TEST);

    }

private:
    unsigned int VAO;
    CustomShader custom_shader;
    vector<image> images;
    vector<int> vaos;
    vector<int> textures;
    vector<float> y_sizes;
    float x_size = 0.2f;
    float y_size;
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
        camera.ProcessMouseScroll(rel.y());
    }

    virtual bool mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers){
        if (button == GLFW_MOUSE_BUTTON_LEFT && down) {
            //glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
            mouseButtonPressed = true;
        }
        if (button == GLFW_MOUSE_BUTTON_LEFT && !down)
        {
            // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
            // variável abaixo para false.
            mouseButtonPressed = false;
        }
    }

    virtual bool mouseMotionEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers){

        if(mouseButtonPressed){
            if (firstMouse){
                lastX = p.x();
                lastY = p.y();
                firstMouse = false;
            }

            float xoffset = p.x() - lastX;
            float yoffset = lastY - p.y(); // reversed since y-coordinates go from bottom to top

            lastX = p.x();
            lastY = p.y();

            camera.ProcessMouseMovement(xoffset, yoffset);
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

        if(key == GLFW_KEY_W){
            while(action == GLFW_REPEAT || action == GLFW_PRESS){
                camera.ProcessKeyboard(FORWARD, deltaTime);
                return true;
            }
        }

        if (key == GLFW_KEY_S){
            while(action == GLFW_REPEAT || action == GLFW_PRESS){
                camera.ProcessKeyboard(BACKWARD, deltaTime);
                return true;
            }
        
        }

        if (key == GLFW_KEY_A){
            while(action == GLFW_REPEAT || action == GLFW_PRESS){
                camera.ProcessKeyboard(LEFT, deltaTime);
                return true;
            }
        }

        //Translation along the X axis (towards positive x axis)
        if (key == GLFW_KEY_D){
            while(action == GLFW_REPEAT || action == GLFW_PRESS){
                camera.ProcessKeyboard(RIGHT, deltaTime);
                return true;
            }
        }

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

vector<image> readCSV(const char* filename){
    vector<image> images;
    string line;
    ifstream csvfile(filename);

    int num_line = 0;

    while(getline(csvfile, line)){
        stringstream ss(line);
        string line_value;
        image current_image;
        int num_value = 0;
        while(getline(ss, line_value, ',')){
            if(num_line != 0){
                switch(num_value){
                    case 0:
                        //cout<<line_value.substr(line_value.find_last_of("."))<<endl;
                        if(line_value.substr(line_value.find_last_of(".")) == ".png"){
                            replace(line_value, ".png", ".jpg");
                        }
                        current_image.path = line_value;
                        break;
                    case 1:
                        current_image.position.x = stof(line_value)*positionScale;
                        break;
                    case 2:
                        current_image.position.y = stof(line_value)*positionScale;
                        break;
                    case 3:
                        current_image.position.z = stof(line_value)*positionScale;
                        break;
                }    
            }

            num_value += 1;
        }

        images.push_back(current_image);

        /*if(num_line >= 10){
            break;
        }*/

        num_line += 1;
    }

    return images;
}

bool replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}
