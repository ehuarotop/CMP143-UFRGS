#ifndef CAMERA_H
#define CAMERA_H

//#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

enum camera_movement {
	UP,
	DOWN,
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

//Initial camera values
const float INITIAL_YAW         		= -90.0f;
const float INITIAL_PITCH       		=  0.0f;
const float SPEED_MODEL_1				=  1.0f;
const float SPEED_MODEL_1_FB			=  0.25f;
const float SPEED_MODEL_2				=  60.0f;
const float SPEED_MODEL_2_FB			=  15.0f;
const float INITIAL_SENSITIVITY_MODEL1 	=  0.25f;
const float INITIAL_SENSITIVITY_MODEL2 	=  0.1f;
const float INITIAL_FOV       			=  45.0f;


class Camera{
public:
	// Camera Attributes
	glm::vec3 cameraPos;
    glm::vec3 cameraFront;
    glm::vec3 cameraUp;
    glm::vec3 cameraRight;
    //glm::vec3 WorldUp;

    //Euler angles
    float yaw;
    float pitch;
    float movementSpeed;
    float movementSpeed_FB;
    float mouseSensitivity;
    float fov;
    int model_used;
    float distanceProjSphere = 0.0f;

    Camera(){
    	//Initializing general attributes 
    	yaw = INITIAL_YAW;
    	pitch = INITIAL_PITCH;
    	fov = INITIAL_FOV;
    }

    void setDistanceProjSphere(float distance){
    	distanceProjSphere = distance;
    }

    void setModel(int model){

    	cameraPos = glm::vec3(0.0f, 0.0f, distanceProjSphere);
        cameraFront = glm::vec3(0.0f,0.0f,-1.0f);
        cameraUp = glm::vec3(0.0f,1.0f,0.0f);
        cameraRight = glm::vec3(1.0f,0.0f,0.0f);

    	if(model == 1){
    		model_used = 1;
            movementSpeed = SPEED_MODEL_1;
            movementSpeed_FB = SPEED_MODEL_1_FB;
    	} else if (model == 2){
    		model_used = 2;
            movementSpeed = SPEED_MODEL_2;
            movementSpeed_FB = SPEED_MODEL_2_FB;
    	}
    }

    void processRotation(camera_movement movement_direction){
    	if (movement_direction == FORWARD){
    		cameraPos += movementSpeed_FB * cameraFront;

    		//Updating distance Projection Sphere
    		distanceProjSphere = glm::length(cameraPos);
    	}else if (movement_direction == BACKWARD){
    		cameraPos -= movementSpeed_FB * cameraFront;

    		//Updating distance Projection Sphere
    		distanceProjSphere = glm::length(cameraPos);
    	}else if (movement_direction == RIGHT){
    		//Calculating update for cameraRight
    		cameraRight = glm::normalize(-glm::cross(cameraFront, cameraUp));

    		//Calculating new Camera position
    		cameraPos +=  cameraRight * movementSpeed;

    		//Updating cameraFront
    		cameraFront = glm::normalize(-cameraPos);

    	}else if (movement_direction == LEFT){
    		//Calculating update for camera right
    		cameraRight = glm::normalize(-glm::cross(cameraFront, cameraUp));

    		//Calculating new Camera position
    		cameraPos -= cameraRight * movementSpeed;

    		//Updating cameraFront
    		cameraFront = glm::normalize(-cameraPos);

    	}/*else if (movement_direction == UP){
    		//Calculating update for cameraUp
    		cameraUp = glm::normalize(-glm::cross(cameraFront, cameraRight));

    		//Calculating new Camera Position
    		cameraPos += cameraUp * movementSpeed;

    		//Updating camera front
    		cameraFront = glm::normalize(-cameraPos);

    	}else if (movement_direction == DOWN){
    		//Calculating update for cameraUp
    		cameraUp = glm::normalize(-glm::cross(cameraFront, cameraRight));

    		//Calculating new Camera Position
    		cameraPos -= cameraUp * movementSpeed;

    		//Updating camera front
    		cameraFront = glm::normalize(-cameraPos);
    	}*/
    }

    // Returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 getLookAtMatrix()
    {
        return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    }

/*private:
	void updateCameraVectors(){
		cout<<"To implement";
	}
*/

};

#endif

