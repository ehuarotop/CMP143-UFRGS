#ifndef CLOSE2GL_H
#define CLOSE2GL_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <matrix.h>

#define PI 3.14159265

class Close2GL{
public:

	glm::vec3 position;
	glm::vec3 right;   	//u
	glm::vec3 up; 		//v
	glm::vec3 front;  	//n

	glm::mat4 view = glm::mat4(0.0f); //view matrix

	float yaw;
    float pitch;
    float movementSpeed;
    float movementSpeed_FB;
    float mouseSensitivity;
    float fov;
    int model_used;
	float distanceProjSphere = 0.0f;

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
        front = glm::vec3(0.0f,0.0f,-1.0f); //n
        up = glm::vec3(0.0f,1.0f,0.0f); //v
        right = glm::vec3(1.0f,0.0f,0.0f); //u

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

		//front = matrix.normalizev3(glm::vec3(position.x - lookat.x, position.y - lookat.y, position.z - lookat.z));
		front = matrix.normalizev3(glm::vec3(lookat.x, lookat.y, lookat.z));

		right = matrix.normalizev3(matrix.crossProduct(up, front));
		up = matrix.normalizev3(matrix.crossProduct(front, right));
		
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

	glm::mat4 getProjectionMatrix(float fov, float aspect_ratio, float near_plane, float far_plane){
		glm::mat4 p = glm::mat4(0.0f);

		float n = near_plane;
		float f = far_plane;

		float t = tan(fov/2.0f) * n;
		float b = -t;
		float r = t*aspect_ratio;
		float l = -t*aspect_ratio;

		//Setting model matrix

		p[0][0] = 2*n/(r-l);	p[1][0] = 0.0f;			p[2][0] = (r+l)/(r-l);		p[3][0] = 0.0f;
		p[0][1] = 0.0f;			p[1][1] = 2*n/(t-b);	p[2][1] = (t+b)/(t-b);		p[3][1] = 0.0f;
		p[0][2] = 0.0f;			p[1][2] = 0.0f;			p[2][2] = -(f+n)/(f-n);		p[3][2] = -(2*f*n)/(f-n);
		p[0][3] = 0.0f;			p[1][3] = 0.0f;			p[2][3] = -1.0f;			p[3][3] = 0.0f;

		return p;	
	}

	glm::mat4 getViewPortMatrix(float lv, float rv, float bv, float tv){
		glm::mat4 v = glm::mat4(0.0f);

		//Setting model matrix

		v[0][0] = (rv-lv)/2;	v[1][0] = 0.0f;			v[2][0] = 0.0f;		v[3][0] = (rv+lv)/2;
		v[0][1] = 0.0f;			v[1][1] = (tv-bv)/2;	v[2][1] = 0.0f;		v[3][1] = (tv+bv)/2;
		v[0][2] = 0.0f;			v[1][2] = 0.0f;			v[2][2] = 1.0f;		v[3][2] = 0.0f;
		v[0][3] = 0.0f;			v[1][3] = 0.0f;			v[2][3] = 0.0f;		v[3][3] = 1.0f;

		return v;
	}

	void processRotation(camera_movement movement_direction){
    	if (movement_direction == FORWARD){
    		position -= movementSpeed_FB * front;

    		//Updating distance Projection Sphere
    		distanceProjSphere = matrix.lengthv3(position);
    	}else if (movement_direction == BACKWARD){
    		position += movementSpeed_FB * front;

    		//Updating distance Projection Sphere
    		distanceProjSphere = matrix.lengthv3(position);
    	}else if (movement_direction == RIGHT){
    		//Calculating update for right
    		right = matrix.normalizev3(matrix.crossProduct(up, front));

    		//Calculating new Camera position
    		position -=  right * movementSpeed;

    		//Updating front
    		front = matrix.normalizev3(position);

    	}else if (movement_direction == LEFT){
    		//Calculating update for camera right
    		right = matrix.normalizev3(matrix.crossProduct(up, front));

    		//Calculating new Camera position
    		position += right * movementSpeed;

    		//Updating front
    		front = matrix.normalizev3(position);

    	} /*else if (movement_direction == UP){
    		//Calculating update for up
    		up = matrix.normalizev3(matrix.crossProduct(right, front));

    		//Calculating new Camera Position
    		position -= up * movementSpeed;

    		//Updating camera front
    		front = matrix.normalizev3(position);

    	}else if (movement_direction == DOWN){
    		//Calculating update for up
    		up = matrix.normalizev3(matrix.crossProduct(right, front));

    		//Calculating new Camera Position
    		position -= up * movementSpeed;

    		//Updating camera front
    		front = matrix.normalizev3(position);
    	}*/
	}

};


#endif