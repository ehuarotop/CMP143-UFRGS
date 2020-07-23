import pygame as pg
from pygame.locals import *

from OpenGL.GL import *
import OpenGL.GL.shaders
from OpenGL.GLU import *

import glm
from PIL import Image
import numpy as np

import utils

#Size of the planes that will be drawn
y_size = 0.15
x_size = 0.05

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

def drawRectangle(image, shader):
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
    image.thumbnail((100,100))
    img_data = np.array(list(image.getdata()), np.uint8)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.width, image.height, 0, GL_RGB, GL_UNSIGNED_BYTE, img_data)
    glEnable(GL_TEXTURE_2D)


def main():
    #Getting image positions from pre-calculated CSV
    img_positions = utils.get_img_positions_from_csv("dataset/Features/positions_all_features.csv")

    img_positions = img_positions[:100]

    pg.init()
    display = (900, 600)
    pg.display.set_mode(display, DOUBLEBUF|OPENGL)

    # Compiling shaders and program
    shader = OpenGL.GL.shaders.compileProgram(OpenGL.GL.shaders.compileShader(VERTEX_SHADER, GL_VERTEX_SHADER),
                                              OpenGL.GL.shaders.compileShader(FRAGMENT_SHADER, GL_FRAGMENT_SHADER))

    glUseProgram(shader)

    glEnable(GL_DEPTH_TEST)

    #gluPerspective(45, (display[0]/display[1]), 0.1, 50.0)

    glTranslatef(0.0, 0.0, -5)

    while True:
        for event in pg.event.get():
            if event.type == pg.QUIT:
                pg.quit()
                quit()

            if event.type == pg.KEYDOWN:
                if event.key == pg.K_LEFT:
                    glTranslatef(-0.1,0,0)
                if event.key == pg.K_RIGHT:
                    glTranslatef(0.1,0,0)

                if event.key == pg.K_UP:
                    glTranslatef(0,1,0)
                if event.key == pg.K_DOWN:
                    glTranslatef(0,-1,0)

            if event.type == pg.MOUSEBUTTONDOWN:
                if event.button == 4:
                    glTranslatef(0,0,1.0)

                if event.button == 5:
                    glTranslatef(0,0,-1.0)

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

        transformLoc = glGetUniformLocation(shader, "transform")
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, np.identity(4))

        #Draw image planes
        for image in img_positions:
            drawRectangle(image, shader)
            #Actual Drawing elements
            #glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_INT, None)
            glDrawArrays(GL_TRIANGLES, 0, 6)

        pg.display.flip()
        pg.time.wait(10)

if __name__ == "__main__":
    main()
























#################### useful URL ####################
#http://pyopengl.sourceforge.net/context/tutorials/shader_1.html
#https://codeloop.org/python-modern-opengl-texturing-rotating-cube/