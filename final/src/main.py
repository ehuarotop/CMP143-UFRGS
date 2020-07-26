import pygame as pg
from pygame.locals import *

from OpenGL.GL import *
import OpenGL.GL.shaders
from OpenGL.GLU import *

#import glm
from PIL import Image
import numpy as np
import utils
from camera import Camera
from pyrr import Vector3,Vector4,Matrix33,Matrix44, Quaternion, matrix44

#Size of the planes that will be drawn
y_size = 0.30
x_size = 0.15

img_thumb_size = 128,128

VERTEX_SHADER = """
 
                #version 330

                in vec3 position;
                in vec3 color;
                in vec2 InTexCoords;

                out vec3 newColor;
                out vec2 OutTexCoords;

                uniform mat4 transform; 

                void main() {
                    gl_Position = transform * vec4(position, 1.0f);
                    newColor = color;
                    OutTexCoords = InTexCoords;
                }
 
 
          """
 
FRAGMENT_SHADER = """
                #version 330

                in vec3 newColor;
                in vec2 OutTexCoords;

                out vec4 outColor;
                uniform sampler2D samplerTex;

                void main() {
                    outColor = texture(samplerTex, OutTexCoords);
                }

   """

def drawImage(image, shader):
    filename = image[0]
    position = image[1]

                            ########### position ############## ,          #######color#####    #texture#
                    #first triangle
    vertex_info = [ position[0]-x_size, position[1]-y_size, position[2],    1.0, 0.0, 0.0,      0.0, 0.0,
                    position[0]-x_size, position[1]+y_size, position[2],    1.0, 0.0, 1.0,      0.0, 1.0,
                    position[0]+x_size, position[1]+y_size, position[2],    1.0, 0.0, 1.0,      1.0, 1.0,
                    #second triangle
                    position[0]-x_size, position[1]-y_size, position[2],    1.0, 0.0, 0.0,      0.0, 0.0,
                    position[0]+x_size, position[1]+y_size, position[2],    1.0, 0.0, 1.0,      1.0, 1.0,
                    position[0]+x_size, position[1]-y_size, position[2],    1.0, 0.0, 1.0,      1.0, 0.0]

    #Converting vertex info into a numpy array
    vertex_info = np.array(vertex_info, dtype=np.float32)

    #Creating buffer in GPU
    VBO = glGenBuffers(1)
    #Binding the buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO)
    glBufferData(GL_ARRAY_BUFFER, vertex_info.itemsize * len(vertex_info), vertex_info, GL_STATIC_DRAW)

    # get the position from  shader
    position = glGetAttribLocation(shader, 'position')
    glVertexAttribPointer(position, 3, GL_FLOAT, GL_FALSE, vertex_info.itemsize * 8, ctypes.c_void_p(0))
    glEnableVertexAttribArray(position)

    # get the color from  shader
    color = glGetAttribLocation(shader, 'color')
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vertex_info.itemsize * 8, ctypes.c_void_p(12))
    glEnableVertexAttribArray(1)

    texCoords = glGetAttribLocation(shader, "InTexCoords")
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, vertex_info.itemsize * 8, ctypes.c_void_p(24))
    glEnableVertexAttribArray(2)

    #Texture Creation
    texture = glGenTextures(1)
    glBindTexture(GL_TEXTURE_2D, texture)
    # Set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT)
    # Set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
    # load image
    image = Image.open(filename)
    #resizing image to 
    image.thumbnail(img_thumb_size, Image.ANTIALIAS)
    img_data = np.array(list(image.getdata()), np.uint8)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.width, image.height, 0, GL_RGB, GL_UNSIGNED_BYTE, img_data)
    glEnable(GL_TEXTURE_2D)

    glDrawArrays(GL_TRIANGLES, 0, 6)


def main():
    #Getting image positions from pre-calculated CSV
    img_positions = utils.get_img_positions_from_csv("dataset/Features/positions_all_features.csv")

    #Initializing pygame
    pg.init()
    display = (900, 600)
    pg.display.set_mode(display, DOUBLEBUF|OPENGL)

    # Compiling shaders and program
    shader = OpenGL.GL.shaders.compileProgram(OpenGL.GL.shaders.compileShader(VERTEX_SHADER, GL_VERTEX_SHADER),
                                              OpenGL.GL.shaders.compileShader(FRAGMENT_SHADER, GL_FRAGMENT_SHADER))

    #Using program based on loaded shaders
    glUseProgram(shader)

    #Enabling DEPTH_TEST
    glEnable(GL_DEPTH_TEST)

    #Initializing variables to control movement
    tx = 0
    ty = 0
    tz = 0
    ry = 0
    lastX = 0
    lastY = 0
    firstMouse = True
    movement = 0.01
    yaw = 0.0
    pitch = 0.0

    #Initialize camera
    #camera = Camera()

    while True:
        for event in pg.event.get():
            if event.type == pg.QUIT:
                pg.quit()
                quit()

            """Controlling movements (LEFT, RIGHT, UP, DOWN)"""
            if event.type == pg.KEYDOWN:
                if event.key == pg.K_LEFT:
                    tx -= movement
                elif event.key == pg.K_RIGHT:
                    tx += movement
                elif event.key == pg.K_UP:
                    ty += movement
                elif event.key == pg.K_DOWN:
                    ty -= movement

            """Controlling movements (scrollup 4, scrolldown 5)"""
            if event.type == pg.MOUSEBUTTONDOWN:
                if event.button == 4:
                    tz += movement
                elif event.button == 5:
                    tz -= movement

            """Controlling rotation"""
            if event.type == pg.MOUSEMOTION:
                if firstMouse:
                    lastX = event.pos[0]
                    lastY = event.pos[1]
                    firstMouse = False

                xoffset = event.pos[0] - lastX
                yoffset = event.pos[1] - lastY
                lastX = event.pos[0]
                lastY = event.pos[1]

                sensitivity = 0.2
                xoffset *= sensitivity
                yoffset *= sensitivity

                yaw += xoffset
                pitch += yoffset

                if pitch > 89.0:
                    pitch = 89.0
                if pitch < -89.0:
                    pitch = -89.0

        print(tx, ty, tz)

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

        #Applying translation
        view = Matrix44.identity()
        #view = Matrix44.from_translation(Vector3([0.0,0.0, 0.5]))
        view = view * Matrix44.from_translation(Vector3([tx, ty, tz]))
        view = view * Quaternion.from_x_rotation(pitch * (np.pi)/180.0) #We can multiply matrices and Quaternions directly.
        view = view * Quaternion.from_y_rotation(yaw * (np.pi)/180.0)
        #print(view)
        #print("---------------------------------")

        proj = matrix44.create_perspective_projection_matrix(45.0, float(display[0])/float(display[1]), 0.1, 100.0)

        #print(proj)
        #print("*********************************")

        transformLoc = glGetUniformLocation(shader, "transform")
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, view)

        #Getting only the top 50 images near to the current z
        img_positions.sort(key=lambda x: np.abs(x[1][2]-tz))
        current_imgs = img_positions[:5]

        #Draw image planes
        for image in current_imgs:
            drawImage(image, shader)

        pg.display.flip()
        pg.time.wait(10)

if __name__ == "__main__":
    main()
























#################### useful URL ####################
#http://pyopengl.sourceforge.net/context/tutorials/shader_1.html
#https://codeloop.org/python-modern-opengl-texturing-rotating-cube/