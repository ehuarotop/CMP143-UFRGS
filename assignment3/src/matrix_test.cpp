//Includes for matrix transformations using glm
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp>
//to print vectors and matrices
#include <glm/gtx/string_cast.hpp>
#include <iostream>

#include <close2gl.h>

using namespace std;

int main(){
	glm::mat4 model         = glm::mat4(1.0f);
	glm::mat4 view          = glm::mat4(1.0f);

	float aaa[16] = {
	   1, 2, 3, 4,
	   5, 6, 7, 8,
	   9, 10, 11, 12,
	   13, 14, 15, 16
	};

	glm::mat4 bbb;

	memcpy( glm::value_ptr( bbb ), aaa, sizeof( aaa ) );

	Close2GL close2gl = Close2GL();

	//glm::mat4 result = matrix_4.multiply_matrix(bbb, bbb);

	//glm::vec4 result = matrix_4.multiply_matrix_vector(bbb, glm::vec4(3.0f, 4.0f,5.0f,1.0f));

	glm::mat4 translation = close2gl.translate(bbb, glm::vec3(3.0f,4.0f,5.0f));

	cout<<glm::to_string(translation)<<endl;
}


