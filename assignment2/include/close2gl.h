#ifndef CLOSE2GL_H
#define CLOSE2GL_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <matrix.h>

//Initial camera values
/*const float INITIAL_YAW         		= -90.0f;
const float INITIAL_PITCH       		=  0.0f;
const float SPEED_MODEL_1				=  1.0f;
const float SPEED_MODEL_1_FB			=  0.25f;
const float SPEED_MODEL_2				=  60.0f;
const float SPEED_MODEL_2_FB			=  15.0f;
const float INITIAL_SENSITIVITY_MODEL1 	=  0.25f;
const float INITIAL_SENSITIVITY_MODEL2 	=  0.1f;
const float INITIAL_FOV       			=  45.0f;*/

class Close2GL{
public:

	glm::vec3 position;
	glm::vec3 front;  //n
	glm::vec3 up; 		//v
	glm::vec3 right;   //u

	glm::mat4 view = glm::mat4(0.0f); //view matrix

	float yaw;
    float pitch;
    float movementSpeed;
    float movementSpeed_FB;
    float mouseSensitivity;
    float fov;
    int model_used;
	float distanceProjSphere = 0.0f;
	float translateCamX;
    float translateCamY;
    float translateCamZ;

	Matrix matrix = Matrix();

	Close2GL(){
		//Initializing general attributes 
    	yaw = INITIAL_YAW;
    	pitch = INITIAL_PITCH;
    	fov = INITIAL_FOV;
	}

	void setDistanceProjSphere(float distance){
    	distanceProjSphere = distance;
    }

    void setModel(int model){

    	position = glm::vec3(0.0f, 0.0f, distanceProjSphere);
        front = glm::vec3(0.0f,0.0f,-1.0f);
        up = glm::vec3(0.0f,1.0f,0.0f);
        right = glm::vec3(1.0f,0.0f,0.0f);
        translateCamZ = 0.0f;
        translateCamX = 0.0f;
        translateCamY = 0.0f;

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

	void lookAt(glm::vec3 position, glm::vec3 lookat, glm::vec3 up){

		front = glm::vec3(position.x - lookat.x, position.y - lookat.y, position.z - lookat.z);
		
		front = matrix.normalizev3(front);

		right = glm::cross(up, front);
		up = glm::cross(front, right);
		
		view = getModelViewMatrix();
	}

	glm::mat4 getLookAtMatrix(){
		lookAt(position, position + front, up);
		return view;
	}

	glm::mat4 getModelViewMatrix(){

		glm::mat4 m = glm::mat4(0.0f);

		//Setting model matrix

		m[0][0] = right.x;		m[1][0] = right.y;		m[2][0] = right.z;		m[3][0] = -matrix.dotProduct(position, right);
		m[0][1] = up.x;			m[1][1] = up.y;			m[2][1] = up.z;			m[3][1] = -matrix.dotProduct(position, up);
		m[0][2] = front.x;		m[1][2] = front.y;		m[2][2] = front.z;		m[3][2] = -matrix.dotProduct(position, front);
		m[0][3] = 0.0f;			m[1][3] = 0.0f;			m[2][3] = 0.0f;			m[3][3] = 1.0f;

		return m;
	}


	void processRotation(camera_movement movement_direction){
    	if (movement_direction == FORWARD){
    		position += movementSpeed_FB * front;

    		//Updating distance Projection Sphere
    		distanceProjSphere = glm::length(position);
    	}else if (movement_direction == BACKWARD){
    		position -= movementSpeed_FB * front;

    		//Updating distance Projection Sphere
    		distanceProjSphere = glm::length(position);
    	}else if (movement_direction == RIGHT){
    		//Calculating update for right
    		right = glm::normalize(-glm::cross(front, up));

    		//Calculating new Camera position
    		position +=  right * movementSpeed;

    		//Updating front
    		front = glm::normalize(-position);

    	}else if (movement_direction == LEFT){
    		//Calculating update for camera right
    		right = glm::normalize(-glm::cross(front, up));

    		//Calculating new Camera position
    		position -= right * movementSpeed;

    		//Updating front
    		front = glm::normalize(-position);

    	}else if (movement_direction == UP){
    		//Calculating update for up
    		up = glm::normalize(-glm::cross(front, right));

    		//Calculating new Camera Position
    		position += up * movementSpeed;

    		//Updating camera front
    		front = glm::normalize(-position);

    	}else if (movement_direction == DOWN){
    		//Calculating update for up
    		up = glm::normalize(-glm::cross(front, right));

    		//Calculating new Camera Position
    		position -= up * movementSpeed;

    		//Updating camera front
    		front = glm::normalize(-position);
    	}
	}

};


#endif