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

	glm::mat4 rotate(glm::mat4 matrix, float angle, glm::vec3 axis){

		float sina = sin(angle * PI/180.0f);
		float cosa = cos(angle * PI/180.0f);

		matrix[0][0] = cosa + (1-cosa)*axis.x;					matrix[1][0] = (1-cosa)*axis.x*axis.y + sina*axis.z;	matrix[2][0] = (1-cosa)*axis.x*axis.z - sina*axis.y;	matrix[3][0] = 0;
		matrix[0][1] = (1-cosa)*axis.y*axis.x - sina*axis.z;	matrix[1][1] = cosa + (1-cosa)*pow(axis.y, 2);			matrix[2][1] = (1-cosa)*axis.y*axis.z - sina*axis.x;	matrix[3][1] = 0;
		matrix[0][2] = (1-cosa)*axis.z*axis.x + sina*axis.y;	matrix[1][2] = (1-cosa)*axis.z*axis.z - sina*axis.x;	matrix[2][2] = cosa + (1-cosa)*pow(axis.z, 2);			matrix[3][2] = 0;
		matrix[0][3] = 0.0f;									matrix[1][3] = 0.0f;									matrix[2][3] = 0.0f;									matrix[3][3] = 1.0f;

		/*matrix[0][0] = cosa + (1-cosa)*axis.x;					matrix[1][0] = (1-cosa)*axis.x*axis.y + sina*axis.z;	matrix[2][0] = (1-cosa)*axis.x*axis.z - sina*axis.y;	matrix[3][0] = 0;
		matrix[0][1] = (1-cosa)*axis.y*axis.x - sina*axis.z;	matrix[1][1] = cosa + (1-cosa)*pow(axis.y, 2);			matrix[2][1] = (1-cosa)*axis.y*axis.z - sina*axis.x;	matrix[3][1] = 0;
		matrix[0][2] = (1-cosa)*axis.z*axis.x + sina*axis.y;	matrix[1][2] = (1-cosa)*axis.z*axis.z - sina*axis.x;	matrix[2][2] = cosa + (1-cosa)*pow(axis.z, 2);			matrix[3][2] = 0;
		matrix[0][3] = 0.0f;									matrix[1][3] = 0.0f;									matrix[2][3] = 0.0f;									matrix[3][3] = 1.0f;*/

		return matrix;
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