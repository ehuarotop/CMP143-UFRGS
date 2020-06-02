#ifndef MATRIX_H
#define MATRIX_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

using namespace std;

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
		translation[0][3] = vector.x;
		translation[1][3] = vector.y;
		translation[2][3] = vector.z;

		return translation;

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