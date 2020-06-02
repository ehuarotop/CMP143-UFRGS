#ifndef CLOSE2GL_H
#define CLOSE2GL_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <matrix.h>

using namespace std;

class Close2GL{
public:

	glm::vec3 position;
	glm::vec3 front;  //n
	glm::vec3 up; 		//v
	glm::vec3 right;   //u

	glm::mat4 m = glm::mat4(0.0f); //Model view matrix

	Matrix matrix = Matrix();

	Close2GL(){

	}

	void getLookAtMatrix(glm::vec3 position, glm::vec3 lookat, glm::vec3 up){

		front = glm::vec3(position.x - lookat.x, position.y - lookat.y, position.z - lookat.z);

		front = matrix.normalizev3(front);
		right = matrix.normalizev3(right);

		right = matrix.crossProduct(up, front);
		up = matrix.crossProduct(front, right); // make v = n X u
		
		setModelViewMatrix();

	}

	glm::mat4 setModelViewMatrix(){ 

		m[0][0] = right.x;		m[0][1] = right.y;		m[0][2] = right.z;		m[0][3] = -matrix.dotProduct(position, right);
		m[1][0] = up.x;			m[1][1] = up.y;			m[1][2] = up.z;			m[1][3] = -matrix.dotProduct(position, up);
		m[2][0] = front.x;		m[2][1] = front.y;		m[2][2] = front.z;		m[2][3] = -matrix.dotProduct(position, front);
		m[3][0] = 0.0f;			m[3][1] = 0.0f;			m[3][2] = 0.0f;			m[3][3] = 1.0f;
	}
};


#endif