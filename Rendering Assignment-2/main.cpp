/*
Refrences:
Cube Map: https://learnopengl.com/Advanced-OpenGL/Cubemaps
Cube Map texture: https://learnopengl.com/Advanced-OpenGL/Cubemaps
Cube Map: http://antongerdelan.net/opengl/cubemaps.html
Reflection and Refraction: http://antongerdelan.net/opengl/cubemaps.html
Fresnel and Chromatic Dispersion: https://uce-cg.di.uminho.pt/wp-content/uploads/Cube-Mapping-and-GLSL.pdf
*/


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "skybox.h"
#include "maths_funcs.h"
#include "object_loader.h"

#include <windows.h> //not sure i need
#include <mmsystem.h> //not sure i need

int width = 800;
int height = 600;

GLuint SkyBoxShaderProgramID;
GLuint ReflectionShaderProgramID;
GLuint RefractionShaderProgramID;
GLuint FresnelShaderProgramID;
GLuint ChromaticShaderProgramID;
GLuint ShaderProgramID;
GLuint textureID;

GLuint skyboxVAO;
GLuint skyboxVBO;
GLuint objectVAO = 0;
GLuint objectVBO = 0;
GLuint objectNormalVBO = 0;
GLuint objectloc1;
GLuint objectloc2;
GLuint skyboxloc1;


GLfloat cameraAnglex = 4.7f;
GLfloat cameraAngley = 0.15f;
GLfloat cameraAnglez = 1.4f;

GLfloat cameraTargetPosx = -20.4f;
GLfloat cameraTargetPosy = 42.1f;
GLfloat cameraTargetPosz = 1.2f;

/*
GLfloat cameraAnglex = 0.0f;
GLfloat cameraAngley = 0.0f;
GLfloat cameraAnglez = 0.0f;

GLfloat cameraTargetPosx = 0.1f;
GLfloat cameraTargetPosy = 0.0f;
GLfloat cameraTargetPosz = 0.1f;
*/
GLfloat rotatez = 0.0f;
GLfloat objSize = -266.0f;

char OutputFlag = 'm';

LoadObj Object1("C:/Users/lenovo/source/repos/Object/bunny.obj");

std::string readShaderSource(const std::string& fileName)
{
	std::ifstream file(fileName.c_str());
	if (file.fail()) {
		std::cerr << "Error loading shader called " << fileName << std::endl;
		exit(EXIT_FAILURE);
	}

	std::stringstream stream;
	stream << file.rdbuf();
	file.close();

	return stream.str();
}

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType) {
	GLuint ShaderObj = glCreateShader(ShaderType);
	if (ShaderObj == 0) {
		std::cerr << "Error creating shader type " << ShaderType << std::endl;
		exit(EXIT_FAILURE);
	}

	/* bind shader source code to shader object */
	std::string outShader = readShaderSource(pShaderText);
	const char* pShaderSource = outShader.c_str();
	glShaderSource(ShaderObj, 1, (const GLchar * *)& pShaderSource, NULL);

	/* compile the shader and check for errors */
	glCompileShader(ShaderObj);
	GLint success;
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		std::cerr << "Error compiling shader type " << ShaderType << ": " << InfoLog << std::endl;
		exit(EXIT_FAILURE);
	}
	glAttachShader(ShaderProgram, ShaderObj); /* attach compiled shader to shader programme */
}

GLuint CompileShaders(const char* pVShaderText, const char* pFShaderText)
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
	GLuint ShaderProgramID = glCreateProgram();
	if (ShaderProgramID == 0) {
		std::cerr << "Error creating shader program" << std::endl;
		exit(EXIT_FAILURE);
	}

	// Create two shader objects, one for the vertex, and one for the fragment shader
	AddShader(ShaderProgramID, pVShaderText, GL_VERTEX_SHADER);
	AddShader(ShaderProgramID, pFShaderText, GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };


	// After compiling all shader objects and attaching them to the program, we can finally link it
	glLinkProgram(ShaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(ShaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(ShaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Error linking shader program: " << ErrorLog << std::endl;
		exit(EXIT_FAILURE);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(ShaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(ShaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Invalid shader program: " << ErrorLog << std::endl;
		exit(EXIT_FAILURE);
	}
	return ShaderProgramID;
}


bool LoadCubeMapSide(GLuint texture, GLenum sideTarget, const char* fileName) {

	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

	int x, y, n;
	int forceChannels = 4;
	unsigned char* image_data = stbi_load(fileName, &x, &y, &n, 0); //stbi_load(fileName, &x, &y, &n, forceChannels);

	if (!image_data) {
		std::cerr << "Error: could not load " << fileName << std::endl;
		return false;
	}

	glTexImage2D(sideTarget, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data); // glTexImage2D(sideTarget, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
	free(image_data);
	return true;
}



void CreateCubeMap(const char* front, const char* back, const char* top, const char* bottom, const char* left, const char* right, GLuint* texCube) {

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, texCube);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, *texCube);


	LoadCubeMapSide(*texCube, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, front);
	LoadCubeMapSide(*texCube, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, back);
	LoadCubeMapSide(*texCube, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, top);
	LoadCubeMapSide(*texCube, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, bottom);
	LoadCubeMapSide(*texCube, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, left);
	LoadCubeMapSide(*texCube, GL_TEXTURE_CUBE_MAP_POSITIVE_X, right);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}


void generateObjectBuffer(GLuint temp)
{
	GLuint vp_vbo = 0;

	objectloc1 = glGetAttribLocation(temp, "vertex_position");
	objectloc2 = glGetAttribLocation(temp, "vertex_normals");

	glGenBuffers(1, &vp_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glBufferData(GL_ARRAY_BUFFER, 3 * Object1.getNumVertices() * sizeof(float), Object1.getVertices(), GL_STATIC_DRAW);
	GLuint vn_vbo = 0;
	glGenBuffers(1, &vn_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glBufferData(GL_ARRAY_BUFFER, 3 * Object1.getNumVertices()* sizeof(float), Object1.getNormals(), GL_STATIC_DRAW);

	glGenVertexArrays(1, &objectVAO);
	glBindVertexArray(objectVAO);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
}



void init(void) {


	// Cube Map
	glGenBuffers(1, &skyboxVBO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

	glGenVertexArrays(1, &skyboxVAO);
	glBindVertexArray(skyboxVAO);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL); //3 * sizeof(float)

	// Cube Map
	SkyBoxShaderProgramID = CompileShaders("C:/Users/lenovo/source/repos/Rendering Assignment-2/Shaders/sbVS.txt",
		"C:/Users/lenovo/source/repos/Rendering Assignment-2/Shaders/sbFS.txt");
	generateObjectBuffer(SkyBoxShaderProgramID);

	// Normal Phong Shader
	ShaderProgramID = CompileShaders("C:/Users/lenovo/source/repos/Rendering Assignment-2/Shaders/objectVS - Phong.txt",
		"C:/Users/lenovo/source/repos/Rendering Assignment-2/Shaders/objectFS - Phong.txt");
	generateObjectBuffer(SkyBoxShaderProgramID);

	// Reflect
	ReflectionShaderProgramID = CompileShaders("C:/Users/lenovo/source/repos/Rendering Assignment-2/Shaders/objectVS.txt",
		"C:/Users/lenovo/source/repos/Rendering Assignment-2/Shaders/objectFS - Reflect.txt");
	generateObjectBuffer(ReflectionShaderProgramID);

	// Refraction
	RefractionShaderProgramID = CompileShaders("C:/Users/lenovo/source/repos/Rendering Assignment-2/Shaders/objectVS.txt",
		"C:/Users/lenovo/source/repos/Rendering Assignment-2/Shaders/objectFS - Refraction.txt");
	generateObjectBuffer(RefractionShaderProgramID);

	// Fresnel
	FresnelShaderProgramID = CompileShaders("C:/Users/lenovo/source/repos/Rendering Assignment-2/Shaders/objectVS.txt",
		"C:/Users/lenovo/source/repos/Rendering Assignment-2/Shaders/objectFS - Fresnel.txt");
	generateObjectBuffer(FresnelShaderProgramID);

	// Chromatic
	ChromaticShaderProgramID = CompileShaders("C:/Users/lenovo/source/repos/Rendering Assignment-2/Shaders/objectVS.txt",
		"C:/Users/lenovo/source/repos/Rendering Assignment-2/Shaders/objectFS - Chromatic Dispersion.txt");
	generateObjectBuffer(ChromaticShaderProgramID);

	CreateCubeMap("C:/Users/lenovo/source/repos/Rendering Assignment-2/Skybox/learnopengl/negz.jpg",
		"C:/Users/lenovo/source/repos/Rendering Assignment-2/Skybox/learnopengl/posz.jpg",
		"C:/Users/lenovo/source/repos/Rendering Assignment-2/Skybox/learnopengl/posy.jpg",
		"C:/Users/lenovo/source/repos/Rendering Assignment-2/Skybox/learnopengl/negy.jpg",
		"C:/Users/lenovo/source/repos/Rendering Assignment-2/Skybox/learnopengl/negx.jpg",
		"C:/Users/lenovo/source/repos/Rendering Assignment-2/Skybox/learnopengl/posx.jpg", &textureID);
}

void display(void) {

	// Cube Map
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glClearColor(0.5f, 0.5f, 0.5f, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDepthFunc(GL_LEQUAL);
	glUseProgram(SkyBoxShaderProgramID);
	glActiveTexture(GL_TEXTURE0);

	int matrix_location = glGetUniformLocation(SkyBoxShaderProgramID, "model");
	int view_mat_location = glGetUniformLocation(SkyBoxShaderProgramID, "view");
	int proj_mat_location = glGetUniformLocation(SkyBoxShaderProgramID, "proj");

//	mat4 view = look_at(vec3(cameraAnglex, cameraAngley, cameraAnglez), vec3(cameraTargetPosx, cameraTargetPosy, cameraTargetPosz), vec3(0.0, 1.0, 0.0));
	mat4 view = look_at(vec3(0.0f, 0.0f, 0.0f), vec3(0.1, 0.0, 0.1), vec3(0.0, 1.0, 0.0));
	mat4 persp_proj = perspective(90.0, width / height, 0.01, 10.0);

	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	glBindVertexArray(skyboxVAO);
	glDrawArrays(GL_TRIANGLES, 0, skyboxVertexCount);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS);

	// Phong Shader
	if (OutputFlag == 'm') {
		glUseProgram(ShaderProgramID);
		int shader_matrix_location = glGetUniformLocation(ShaderProgramID, "model");
		int shader_view_mat_location = glGetUniformLocation(ShaderProgramID, "view");
		int shader_proj_mat_location = glGetUniformLocation(ShaderProgramID, "proj");

		mat4 shader_persp_proj = perspective(objSize, (float)width / (float)height, 0.5, 50.0);
		mat4 shader_model = identity_mat4();
		shader_model = rotate_x_deg(shader_model, cameraTargetPosx + rotatez);
		shader_model = rotate_y_deg(shader_model, cameraTargetPosy);
		shader_model = rotate_z_deg(shader_model, cameraTargetPosz);
		shader_model = translate(shader_model, vec3(cameraAnglex, cameraAngley, cameraAnglez));

		glUniformMatrix4fv(shader_proj_mat_location, 1, GL_FALSE, shader_persp_proj.m);
		glUniformMatrix4fv(shader_view_mat_location, 1, GL_FALSE, view.m);
		glUniformMatrix4fv(shader_matrix_location, 1, GL_FALSE, shader_model.m);
		glBindVertexArray(objectVAO);
		glDrawArrays(GL_TRIANGLES, 0, Object1.getNumVertices());
		glBindVertexArray(0);
	}

	// Reflect
	if (OutputFlag == 'z') {
		glUseProgram(ReflectionShaderProgramID);
		int reflection_matrix_location = glGetUniformLocation(ReflectionShaderProgramID, "model");
		int reflection_view_mat_location = glGetUniformLocation(ReflectionShaderProgramID, "view");
		int reflection_proj_mat_location = glGetUniformLocation(ReflectionShaderProgramID, "proj");

		mat4 reflection_persp_proj = perspective(objSize, (float)width / (float)height, 0.5, 50.0);
		mat4 reflection_model = identity_mat4();
		reflection_model = rotate_x_deg(reflection_model, cameraTargetPosx + rotatez);
		reflection_model = rotate_y_deg(reflection_model, cameraTargetPosy);
		reflection_model = rotate_z_deg(reflection_model, cameraTargetPosz);
		reflection_model = translate(reflection_model, vec3(cameraAnglex, cameraAngley, cameraAnglez));

		glUniformMatrix4fv(reflection_proj_mat_location, 1, GL_FALSE, reflection_persp_proj.m);
		glUniformMatrix4fv(reflection_view_mat_location, 1, GL_FALSE, view.m);
		glUniformMatrix4fv(reflection_matrix_location, 1, GL_FALSE, reflection_model.m);
		glBindVertexArray(objectVAO);
		glDrawArrays(GL_TRIANGLES, 0, Object1.getNumVertices());
		glBindVertexArray(0);
	}

	// Refraction
	if (OutputFlag == 'x') {
		glUseProgram(RefractionShaderProgramID);
		int refracted_matrix_location = glGetUniformLocation(RefractionShaderProgramID, "model");
		int refracted_view_mat_location = glGetUniformLocation(RefractionShaderProgramID, "view");
		int refracted_proj_mat_location = glGetUniformLocation(RefractionShaderProgramID, "proj");

		mat4 refracted_persp_proj = perspective(objSize, (float)width / (float)height, 0.5, 50.0);
		mat4 refracted_model = identity_mat4();
		refracted_model = rotate_x_deg(refracted_model, cameraTargetPosx + rotatez);
		refracted_model = rotate_y_deg(refracted_model, cameraTargetPosy);
		refracted_model = rotate_z_deg(refracted_model, cameraTargetPosz);
		refracted_model = translate(refracted_model, vec3(cameraAnglex, cameraAngley, cameraAnglez));

		glUniformMatrix4fv(refracted_proj_mat_location, 1, GL_FALSE, refracted_persp_proj.m);
		glUniformMatrix4fv(refracted_view_mat_location, 1, GL_FALSE, view.m);
		glUniformMatrix4fv(refracted_matrix_location, 1, GL_FALSE, refracted_model.m);
		glBindVertexArray(objectVAO);
		glDrawArrays(GL_TRIANGLES, 0, Object1.getNumVertices());
		glBindVertexArray(0);
	}

	// Fresnel
	if (OutputFlag == 'c') {
		glUseProgram(FresnelShaderProgramID);
		int fresnel_matrix_location = glGetUniformLocation(FresnelShaderProgramID, "model");
		int fresnel_view_mat_location = glGetUniformLocation(FresnelShaderProgramID, "view");
		int fresnel_proj_mat_location = glGetUniformLocation(FresnelShaderProgramID, "proj");

		mat4 fresnel_persp_proj = perspective(objSize, (float)width / (float)height, 0.5, 50.0);
		mat4 fresnel_model = identity_mat4();
		fresnel_model = rotate_x_deg(fresnel_model, cameraTargetPosx + rotatez);
		fresnel_model = rotate_y_deg(fresnel_model, cameraTargetPosy);
		fresnel_model = rotate_z_deg(fresnel_model, cameraTargetPosz);
		fresnel_model = translate(fresnel_model, vec3(cameraAnglex, cameraAngley, cameraAnglez));

		glUniformMatrix4fv(fresnel_proj_mat_location, 1, GL_FALSE, fresnel_persp_proj.m);
		glUniformMatrix4fv(fresnel_view_mat_location, 1, GL_FALSE, view.m);
		glUniformMatrix4fv(fresnel_matrix_location, 1, GL_FALSE, fresnel_model.m);
		glBindVertexArray(objectVAO);
		glDrawArrays(GL_TRIANGLES, 0, Object1.getNumVertices());
		glBindVertexArray(0);
	}

	// Chromatic Dispersion 
	if (OutputFlag == 'v') {
		glUseProgram(ChromaticShaderProgramID);
		int chromatic_matrix_location = glGetUniformLocation(ChromaticShaderProgramID, "model");
		int chromatic_view_mat_location = glGetUniformLocation(ChromaticShaderProgramID, "view");
		int chromatic_proj_mat_location = glGetUniformLocation(ChromaticShaderProgramID, "proj");

		mat4 chromatic_persp_proj = perspective(objSize, (float)width / (float)height, 0.5, 50.0);
		mat4 chromatic_model = identity_mat4();
		chromatic_model = rotate_x_deg(chromatic_model, cameraTargetPosx + rotatez);
		chromatic_model = rotate_y_deg(chromatic_model, cameraTargetPosy);
		chromatic_model = rotate_z_deg(chromatic_model, cameraTargetPosz);
		chromatic_model = translate(chromatic_model, vec3(cameraAnglex, cameraAngley, cameraAnglez));

		glUniformMatrix4fv(chromatic_proj_mat_location, 1, GL_FALSE, chromatic_persp_proj.m);
		glUniformMatrix4fv(chromatic_view_mat_location, 1, GL_FALSE, view.m);
		glUniformMatrix4fv(chromatic_matrix_location, 1, GL_FALSE, chromatic_model.m);
		glBindVertexArray(objectVAO);
		glDrawArrays(GL_TRIANGLES, 0, Object1.getNumVertices());
		glBindVertexArray(0);
	}
	
	glutSwapBuffers();
}



void keyPress(unsigned char key, int xmouse, int ymouse) {
	switch (key) {
	case('q'):
		objSize += 2;
		std::cout << objSize << "objSize" << std::endl;
		break;
	case('a'):
		objSize -= 2;
		std::cout << objSize << "objSize" << std::endl;
		break;
	case ('w'):
		cameraAnglex += 0.05;
		std::cout << cameraAnglex << "Keypress: " << key << std::endl;
		break;
	case ('s'):
		cameraAnglex -= 0.05;
		std::cout << cameraAnglex << "Keypress: " << key << std::endl;
		break;
	case ('e'):
		cameraAngley += 0.05;
		std::cout << cameraAngley << "Keypress: " << key << std::endl;
		break;
	case ('d'):
		cameraAngley -= 0.05;
		std::cout << cameraAngley << "Keypress: " << key << std::endl;
		break;
	case ('r'):
		cameraAnglez += 0.05;
		std::cout << cameraAnglez << "Keypress: " << key << std::endl;
		break;
	case ('f'):
		cameraAnglez -= 0.05;
		std::cout << cameraAnglez << "Keypress: " << key << std::endl;
		break;
	case ('7'):
		cameraTargetPosx += 0.1;
		std::cout << cameraTargetPosx << "Keypress: " << key << std::endl;
		break;
	case ('4'):
		cameraTargetPosx -= 0.1;
		std::cout << cameraTargetPosx << "Keypress: " << key << std::endl;
		break;
	case ('8'):
		cameraTargetPosy += 0.1;
		std::cout << cameraTargetPosy << "Keypress: " << key << std::endl;
		break;
	case ('5'):
		cameraTargetPosy -= 0.1;
		std::cout << cameraTargetPosy << "Keypress: " << key << std::endl;
		break;
	case ('9'):
		cameraTargetPosz += 0.1;
		std::cout << cameraTargetPosz << "Keypress: " << key << std::endl;
		break;
	case ('6'):
		cameraTargetPosz -= 0.1;
		std::cout << cameraTargetPosz << "Keypress: " << key << std::endl;
		break;
	case ('z'):
		OutputFlag = 'z';
		break;
	case('x'):
		OutputFlag = 'x';
		break;
	case('c'):
		OutputFlag = 'c';
		break;
	case('v'):
		OutputFlag = 'v';
		break;
	case('m'):
		OutputFlag = 'm';
		break;
	}
};




//not sure i need this
void updateScene() {

	// Wait until at least 16ms passed since start of last frame (Effectively caps framerate at ~60fps)
	static DWORD  last_time = 0;
	DWORD  curr_time = timeGetTime();
	float  delta = (curr_time - last_time) * 0.001f;
	if (delta > 0.03f)
		delta = 0.03f;
	last_time = curr_time;

	// For Rotation
	rotatez += 0.1f;
	//std::cout << rotatez << std::endl;
	// Draw the next frame
	glutPostRedisplay();
}


int main(int argc, char** argv) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(width, height);
	glutCreateWindow(argv[1]);

	/* register call back functions */
	glutDisplayFunc(display);

	glutIdleFunc(updateScene);
	glutKeyboardFunc(keyPress);

	glewExperimental = GL_TRUE; //not sure point of this?
	GLenum res = glewInit();
	if (res != GLEW_OK) {
		std::cerr << "Error: " << glewGetErrorString(res) << std::endl;
		return EXIT_FAILURE;
	}

	init();
	glutMainLoop();
	return 0;
}