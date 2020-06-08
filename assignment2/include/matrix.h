#ifndef MATRIX_H
#define MATRIX_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

using namespace std;

#define PI 3.14159265

class Matrix{
public:

	Matrix(){

	}

	glm::mat4 multiply_matrix(glm::mat4 matrix1, glm::mat4 matrix2){
		glm::mat4 result_matrix = glm::mat4(0.0f);

		for(int i=0; i<4; i++){
			for(int j=0; j<4; j++){
				for(int k=0; k<4; k++){
					result_matrix[i][j] += matrix1[i][k] * matrix2[k][j];
				}
			}
		}

		return result_matrix;
	}

	glm::vec4 multiply_matrix_vector(glm::mat4 matrix, glm::vec4 vector){
		glm::vec4 result_vec = glm::vec4(0.0f);

		for(int i=0; i<4; i++){
			for(int j=0; j<4; j++){
				result_vec[i] += matrix[i][j] * vector[i];
			}
		}

		return result_vec;
	}

	glm::mat4 translate(glm::vec3 vector){
		glm::mat4 translation = glm::mat4(1.0f);
		translation[3][0] = vector.x;
		translation[3][1] = vector.y;
		translation[3][2] = vector.z;

		return translation;

	}

	glm::mat4 pitchRotate(glm::mat4 matrix, float angle){
		float cosa = cos(angle * PI/180.0f);
		float sina = sin(angle * PI/180.0f);

		glm::mat4 m; //rotation matrix around x axis

		m[0][0] = 1.0f;		m[1][0] = 0.0f;		m[2][0] = 0.0f;		m[3][0] = 0;
		m[0][1] = 0.0f;		m[1][1] = cosa;		m[2][1] = sina;		m[3][1] = 0;
		m[0][2] = 0.0f;		m[1][2] = -sina;	m[2][2] = cosa;		m[3][2] = 0;
		m[0][3] = 0.0f;		m[1][3] = 0.0f;		m[2][3] = 0.0f;		m[3][3] = 1.0f;

		return multiply_matrix(m, matrix);
	}

	glm::mat4 rotate(glm::mat4 matrix, float angle, glm::vec3 axis){

		float sina = sin(angle);
		float cosa = cos(angle);

		float x = axis.x;
		float y = axis.y;
		float z = axis.z;

		glm::mat4 rotate;

		rotate[0][0] = cosa + (1.0f-cosa)*pow(x,2);		rotate[1][0] = (1.0f-cosa)*x*y - sina*z;		rotate[2][0] = (1.0f-cosa)*x*z + sina*y;		rotate[3][0] = 0.0f;
		rotate[0][1] = (1.0f-cosa)*y*x + sina*z;		rotate[1][1] = cosa + (1.0f-cosa)*pow(y, 2);	rotate[2][1] = (1.0f-cosa)*y*z - sina*x;		rotate[3][1] = 0.0f;
		rotate[0][2] = (1.0f-cosa)*z*x - sina*y;		rotate[1][2] = (1.0f-cosa)*z*y + sina*x;		rotate[2][2] = cosa + (1.0f-cosa)*pow(z, 2);	rotate[3][2] = 0.0f;
		rotate[0][3] = 0.0f;							rotate[1][3] = 0.0f;							rotate[2][3] = 0.0f;							rotate[3][3] = 1.0f;

		return multiply_matrix(rotate, matrix);
	}

	float dotProduct(glm::vec3 vec1, glm::vec3 vec2){
		//return(vec1.x*vec2.x + vec1.y*vec2.y + vec1.z*vec2.z + vec1.w*vec2.w);
		return(vec1.x*vec2.x + vec1.y*vec2.y + vec1.z*vec2.z);
	}

	glm::vec3 crossProduct(glm::vec3 vec1, glm::vec3 vec2){
		glm::vec4 crossProduct;

		crossProduct.x =  vec1.y * vec2.z - vec1.z * vec2.y;
	    crossProduct.y = -vec1.x * vec2.z + vec1.z * vec2.x;
	    crossProduct.z =  vec1.x * vec2.y - vec1.y * vec2.x;

	    return crossProduct;
	}

	float lengthv3(glm::vec3 vector){
		return((float)sqrt(vector.x*vector.x + vector.y*vector.y + vector.z*vector.z));
	}

	glm::vec3 normalizev3(glm::vec3 vector){
		float length = lengthv3(vector);

		vector.x = vector.x / length;
		vector.y = vector.y / length;
		vector.z = vector.z / length;

		return vector;
	}

};


#endif