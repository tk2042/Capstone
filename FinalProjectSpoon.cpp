/*
 * FinalProjectSpoon.cpp
 *
 *  Created on: Dec 13, 2020
 *      Author: Timothy Kelly
 *      CS-330: Final Milestone
 *              Kitchen Spoon
 */
// Header inclusions
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>

// GLM math header inclusions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// SOIL image loader inclusion
#include "SOIL2/SOIL2.h"

using namespace std; //standard namespace

#define WINDOW_TITLE "Kitchen Spoon" // Window Title Macro

// Shader program macro
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version "\n" #Source
#endif

// Variable declaration for shader, window size initialization, buffer and array objects
GLint shaderProgram, WindowWidth = 800, WindowHeight = 600;
GLuint VBO, VAO, texture;
GLfloat degrees = glm::radians(0.0f); // Converts float to degrees

GLfloat cameraSpeed = 0.0005f; // Movement speed per frame

GLchar currentKey; // will store key pressed
GLfloat lastMouseX = 400, lastMouseY = 300; // Locks mouse cursor at center of screen
GLfloat mouseXOffset, mouseYOffset, yaw = 0.0f, pitch = 0.0f; // Mouse offset variables
GLfloat sensitivity = 0.005f;  // Used for mouse/camera rotation sensitivity
bool mouseDetected = true; // Initially true when mouse movement is detected

// Cube and light color
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
glm::vec3 secondLightColor(1.0f, 1.0f, 1.0f);

// Light position and scale
glm::vec3 lightPosition(1.0f, 0.5f, -3.0f);
glm::vec3 lightScale(0.3f);

// Ambient, Specular, Highlight
glm::vec3 lightStrength(0.1f,     1.0f,       0.5f);

// Camera position
glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 0.0f); // Initial camera position
glm::vec3 CameraUpY = glm::vec3(0.0f, 1.0f, 0.0f); // Temp Y unit vector
glm::vec3 CameraForwardZ = glm::vec3(0.0f, 0.0f, -1.0f); // Temp Z unit vector
glm::vec3 front; // Temp Z unit vector for mouse

// Camera rotation
float cameraRotation = glm::radians(-45.0f);

// Function prototypes
void UResizeWindow(int, int);
void URenderGraphics(void);
void UCreateShader(void);
void UCreateBuffers(void);
void UGenerateTexture(void);
void UMouseMove(int x, int y);
void UKeyboard(unsigned char key, int x, int y);

// Vertex Shader source code
const GLchar * vertexShaderSource = GLSL(330,
	layout (location = 0) in vec3 position; // Vertex data from vertex attrib pointer0
	layout (location = 1) in vec3 color;  // Color data
	layout (location = 2) in vec2 textureCoordinate; // Texture data

	out vec3 mobileColor;
	out vec2 mobileTextureCoordinate;

	// Global variables for the transform matrices
	uniform mat4 model;
	uniform mat4 view;
	uniform mat4 projection;

	void main(){
		gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices to clip coordinates
		mobileColor = color;
		mobileTextureCoordinate = vec2(textureCoordinate.x, 1.0f - textureCoordinate.y); // Flips the texture horizontal
	}
);

// Fragment shader source code
const GLchar * fragmentShaderSource = GLSL(330,

	in vec3 mobileColor;
	in vec2 mobileTextureCoordinate;

	out vec4 gpuColor;
	out vec4 gpuTexture; // Variable to pass color data to the GPU

	uniform sampler2D uTexture; // Useful when working with multiple textures

	void main(){

		gpuColor = vec4(mobileColor, 1.0);
		gpuTexture = texture(uTexture, mobileTextureCoordinate);
	}
);

const GLchar * lightVertexShaderSource = GLSL(330,
	layout (location = 0) in vec3 position; // VAP position 0 for vertex position data
	layout (location = 1) in vec3 normal; // VAP position 1 for normals
	layout (location = 2) in vec2 textureCoordinate;

	out vec3 Normal; // For outgoing normals to fragment shader
	out vec3 FragmentPos; // For outgoing color / pixels to fragment shader
	out vec2 mobileTextureCoordinate;

	// Uniform / global variables for the transform matrices
	uniform mat4 model;
	uniform mat4 view;
	uniform mat4 projection;

    void main(){
        gl_Position = projection * view * model * vec4(position, 1.0f); //Transforms vertices into clip coordinates
        Normal = mat3(transpose(inverse(model))) * normal; // Get normal vectors in world space only and exclude normal translation properties
        FragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)
		mobileTextureCoordinate = vec2(textureCoordinate.x, 1.0f - textureCoordinate.y); // Flips the texture horizontal
	}
);

const GLchar * lightFragmentShaderSource = GLSL(330,
	in vec3 Normal; // For incoming normals
	in vec3 FragmentPos; // For incoming fragment position
	in vec2 mobileTextureCoordinate;

	out vec4 result; // For outgoing light color to the GPU

	// Uniform / Global variables for object color, light color, light position and camera/view position
	uniform vec3 lightColor;
	uniform vec3 secondLightColor;
	uniform vec3 lightPos;
	uniform vec3 viewPosition;
    uniform vec3 lightStrength;
	uniform sampler2D uTexture; // Useful when working with multiple textures

    void main(){
    	vec3 norm = normalize(Normal); // Normalize vectors to 1 unit
    	vec3 ambient = lightStrength.x * lightColor; // Generate ambient light color
    	vec3 ambientTwo = lightStrength.x * secondLightColor;// Generate second ambient light color
    	vec3 lightDirection = normalize(lightPos - FragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on

    	float impact = max(dot(norm, lightDirection), 0.0); // Calculate diffuse impact by generating dot product of normal and light

    	vec3 diffuse = impact * lightColor; // Generate diffuse light color
    	vec3 viewDir = normalize(viewPosition - FragmentPos); // Calculate view direction
    	vec3 reflectDir = reflect(-lightDirection, norm); // Calculate reflection vector

    	float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), lightStrength.z);

    	vec3 specular = lightStrength.y * specularComponent * lightColor;

    	// Calculate phong result
    	vec3 phongOne = (ambient + diffuse + specular) * vec3(texture(uTexture, mobileTextureCoordinate));

    	// Second light position
    	lightDirection = normalize(vec3(6.0f, 0.0f, -3.0f)- FragmentPos);
    	impact = max(dot(norm, lightDirection), 0.0); // Calculate diffuse impact by generating dot product of normal and light
    	diffuse = impact * secondLightColor; // Generate diffuse light color
    	viewDir = normalize(viewPosition - FragmentPos); // Calculate view direction
    	reflectDir = reflect(-lightDirection, norm); // Calculate reflection vector
    	specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), lightStrength.z);

    	// Second light spec
    	vec3 specularTwo = 0.1f * specularComponent * secondLightColor;

    	vec3 phongTwo = (ambientTwo + diffuse + specularTwo) * vec3(texture(uTexture, mobileTextureCoordinate));

    	result = vec4(phongOne + phongTwo, 1.0f); // Send lighting results to GPU
	}
);

// Main Program
int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(WindowWidth, WindowHeight);
	glutCreateWindow(WINDOW_TITLE);

	glutReshapeFunc(UResizeWindow);

	glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

	UCreateShader();

	UCreateBuffers();

	UGenerateTexture();

	// Use the Shader Program
	glUseProgram(shaderProgram);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color

	glutDisplayFunc(URenderGraphics);

	glutPassiveMotionFunc(UMouseMove); // Detects mouse movement

	glutKeyboardFunc(UKeyboard); // Detects key press

	glutMainLoop();

	// Destroy buffer objects once used
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	return 0;
}

// Resize the window
void UResizeWindow(int w, int h)
{
	WindowWidth = w;
	WindowHeight = h;
	glViewport(0, 0, WindowWidth, WindowHeight);
}

// Renders Graphics
void URenderGraphics(void)
{
	glEnable(GL_DEPTH_TEST); // Enable z-depth

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clears the screen

	glBindVertexArray(VAO); // Activate the vertex array object before rendering and transforming them

	CameraForwardZ = front;

	// Transforms the object
	glm::mat4 model;
	model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f)); // Increase the object size by a scale of 2
	model = glm::rotate(model, degrees, glm::vec3(0.0, 1.0f, 0.0f)); // Rotate the object y -45 degrees
	model = glm::translate(model, glm::vec3(0.0, 0.0f, 0.0f)); // Place the object at the center of the viewport

	// Transforms the camera
	glm::mat4 view;
	view = glm::lookAt(CameraForwardZ, cameraPosition, CameraUpY);

	// Creates a perspective projection
	glm::mat4 projection;
	projection = glm::perspective(45.0f, (GLfloat)WindowWidth / (GLfloat)WindowHeight, 0.1f, 100.0f);

	// Set camera to 3D
	if(currentKey != '3'){
		projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);
	}

	// Set the camera projection to 2D
	if(currentKey != '2'){
		projection = glm::perspective(45.0f, (GLfloat)WindowWidth / (GLfloat)WindowHeight, 0.1f, 100.0f);
	}

	// Retrieves and passes transform matrices to the shader program
	GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
	GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
	GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    GLint secondLightColorLoc, lightColorLoc, lightPositionLoc, lightStrengthLoc, viewPositionLoc;

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
	lightPositionLoc = glGetUniformLocation(shaderProgram, "lightPos");
    lightStrengthLoc = glGetUniformLocation(shaderProgram, "lightStrength");
    secondLightColorLoc = glGetUniformLocation(shaderProgram, "secondLightColor");
	viewPositionLoc = glGetUniformLocation(shaderProgram, "viewPosition");

	// Pass color, light, and camera data to the cube shader programs corresponding uniforms
	glUniform3f(lightColorLoc, lightColor.r, lightColor.g, lightColor.b);
	glUniform3f(secondLightColorLoc, secondLightColor.r, secondLightColor.g, secondLightColor.b);
	glUniform3f(lightPositionLoc, lightPosition.x, lightPosition.y, lightPosition.z);
	glUniform3f(lightStrengthLoc, lightStrength.x, lightStrength.y, lightStrength.z);
	glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

	glutPostRedisplay();

	glBindTexture(GL_TEXTURE_2D, texture);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, 102);

	glBindVertexArray(0); // Deactivates the vertex array object

	glutSwapBuffers(); // Flips the back buffer with the front buffer every frame
}

// Creates the shader program
void UCreateShader()
{
	// Vertex shader
	GLint vertexShader = glCreateShader(GL_VERTEX_SHADER); // creates the vertex shader
	glShaderSource(vertexShader, 1, &lightVertexShaderSource, NULL); //Attaches the vertex shader to the source code
	glCompileShader(vertexShader); //compiles the vertex shader

	// Fragment shader
	GLint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); //Creates the fragment shader
	glShaderSource(fragmentShader, 1, &lightFragmentShaderSource, NULL); //Attaches the fragment shader to the source code
	glCompileShader(fragmentShader); //compiles the fragment shader

	// Shader Program
	shaderProgram = glCreateProgram(); // Creates the shader program and returns an id
	glAttachShader(shaderProgram, vertexShader); // Attach vertex shader to the shader program
	glAttachShader(shaderProgram, fragmentShader); // Attach fragment shader to the shader program
	glLinkProgram(shaderProgram); // Link vertex and fragment shader to shader program

	// Delete the vertex and fragment shaders once linked
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

// Creates the buffer and array objects(Kitchen Spoon)
void UCreateBuffers()
{
	// Position and texture coordinate data for 102 triangles
	GLfloat vertices[] ={
					// Positions			// Variables
				 0.2f, -2.0f, -0.5f, 	1.0f, 0.0f, 0.0f, // Front handle of spoon
				-0.2f, -2.0f, -0.5f, 	1.0f, 0.0f, 0.0f,
				 0.1f,  0.5f, -0.5f, 	1.0f, 0.0f, 0.0f,
			    -0.1f,  0.5f, -0.5f, 	1.0f, 0.0f, 0.0f,
				 0.1f,  0.5f, -0.5f, 	1.0f, 0.0f, 0.0f,
				-0.2f, -2.0f, -0.5f, 	1.0f, 0.0f, 0.0f,

				-0.2f, -2.0f, -0.4f, 	0.0f, 1.0f, 0.0f, // Back handle of spoon
				 0.2f, -2.0f, -0.4f, 	0.0f, 1.0f, 0.0f,
				 0.1f,  0.5f, -0.4f, 	0.0f, 1.0f, 0.0f,
			    -0.1f,  0.5f, -0.4f, 	0.0f, 1.0f, 0.0f,
				 0.1f,  0.5f, -0.4f, 	0.0f, 1.0f, 0.0f,
				-0.2f, -2.0f, -0.4f, 	0.0f, 1.0f, 0.0f,

				-0.1f,  0.5f, -0.4f, 	0.0f, 0.0f, 1.0f, // Right handle side
				-0.1f,  0.5f, -0.5f, 	0.0f, 0.0f, 1.0f,
				-0.2f, -2.0f, -0.5f, 	0.0f, 0.0f, 1.0f,
				-0.2f, -2.0f, -0.5f, 	0.0f, 0.0f, 1.0f,
				-0.2f, -2.0f, -0.4f, 	0.0f, 0.0f, 1.0f,
				-0.1f,  0.5f, -0.4f, 	0.0f, 0.0f, 1.0f,

				0.1f,  0.5f, -0.4f, 	1.0f, 1.0f, 0.0f, // Left handle side
				0.1f,  0.5f, -0.5f, 	1.0f, 1.0f, 0.0f,
				0.2f, -2.0f, -0.5f, 	1.0f, 1.0f, 0.0f,
				0.2f, -2.0f, -0.5f, 	1.0f, 1.0f, 0.0f,
				0.2f, -2.0f, -0.4f, 	1.0f, 1.0f, 0.0f,
				0.1f,  0.5f, -0.4f, 	1.0f, 1.0f, 0.0f,

				0.5f,  1.0f, -0.5f, 	1.0f, 0.0f, 1.0f, // Head of spoon(bottom)
			    0.0f,  1.0f, -0.47f, 	1.0f, 0.0f, 1.0f,
			    0.35f, 0.7f, -0.5f, 	1.0f, 0.0f, 1.0f,
			   -0.35f, 0.7f, -0.5f, 	1.0f, 0.0f, 1.0f,
			    0.0f,  1.0f, -0.47f, 	1.0f, 0.0f, 1.0f,
			   -0.5f,  1.0f, -0.5f, 	1.0f, 0.0f, 1.0f,

				0.5f,  1.0f, -0.5f, 	1.0f, 0.0f, 1.0f, // Back Head of spoon(bottom)
				0.0f,  1.0f, -0.4f, 	1.0f, 0.0f, 1.0f,
				0.35f, 0.7f, -0.5f, 	1.0f, 0.0f, 1.0f,
			   -0.35f, 0.7f, -0.5f, 	1.0f, 0.0f, 1.0f,
			    0.0f,  1.0f, -0.4f, 	1.0f, 0.0f, 1.0f,
			   -0.5f,  1.0f, -0.5f, 	1.0f, 0.0f, 1.0f,

			    0.0f,  1.0f, -0.47f, 	0.0f, 0.5f, 0.0f, // Back Head of spoon(bottom)
				0.35f, 0.7f, -0.5f, 	0.0f, 0.5f, 0.0f,
			    0.1f,  0.5f, -0.5f, 	0.0f, 0.5f, 0.0f,
			   -0.1f,  0.5f, -0.5f, 	0.0f, 0.5f, 0.0f,
			    0.0f,  1.0f, -0.47f, 	0.0f, 0.5f, 0.0f,
			   -0.35f, 0.7f, -0.5f, 	0.0f, 0.5f, 0.0f,

				0.0f,  1.0f, -0.4f,     0.0f, 0.5f, 0.0f, // Back Head of spoon(bottom)
			    0.35f, 0.7f, -0.5f, 	0.0f, 0.5f, 0.0f,
				0.1f,  0.5f, -0.5f,     0.0f, 0.5f, 0.0f,
			   -0.1f,  0.5f, -0.5f,    	0.0f, 0.5f, 0.0f,
				0.0f,  1.0f, -0.4f,	    0.0f, 0.5f, 0.0f,
			   -0.35f, 0.7f, -0.5f, 	0.0f, 0.5f, 0.0f,

			    0.5f, 1.0f,  -0.5f,		0.5f, 0.5f, 0.5f, // Head of spoon(middle)
			    0.5f, 1.35f, -0.5f, 	0.5f, 0.5f, 0.5f,
				0.0f, 1.0f,  -0.47f, 	0.5f, 0.5f, 0.5f,
			   -0.5f, 1.0f,  -0.5f,		0.5f, 0.5f, 0.5f,
			   -0.5f, 1.35f, -0.5f, 	0.5f, 0.5f, 0.5f,
			    0.0f, 1.0f,  -0.47f, 	0.5f, 0.5f, 0.5f,

			    0.5f, 1.0f,  -0.5f,		0.8f, 0.8f, 0.8f, // Back Head of spoon(middle)
			    0.5f, 1.35f, -0.5f, 	0.8f, 0.8f, 0.8f,
			    0.0f, 1.0f,  -0.4f,     0.8f, 0.8f, 0.8f,
			   -0.5f, 1.0f,  -0.5f,		0.8f, 0.8f, 0.8f,
			   -0.5f, 1.35f, -0.5f, 	0.8f, 0.8f, 0.8f,
			    0.0f, 1.0f,  -0.4f,     0.8f, 0.8f, 0.8f,

				0.5f,  1.35f, -0.5f,	1.0f, 0.5f, 0.0f, // Head of spoon(top)
				0.35f, 1.7f,  -0.5f, 	1.0f, 0.5f, 0.0f,
			    0.0f,  1.0f,  -0.47f, 	1.0f, 0.5f, 0.0f,
			   -0.5f,  1.35f, -0.5f,	1.0f, 0.5f, 0.0f,
			   -0.35f, 1.7f,  -0.5f, 	1.0f, 0.5f, 0.0f,
				0.0f,  1.0f,  -0.47f, 	1.0f, 0.5f, 0.0f,

				0.5f,  1.35f, -0.5f,	1.0f, 0.0f, 0.5f, // Back. Head of spoon(top)
				0.35f, 1.7f,  -0.5f, 	1.0f, 0.0f, 0.5f,
				0.0f,  1.0f,  -0.4f, 	1.0f, 0.0f, 0.5f,
			   -0.5f,  1.35f, -0.5f,	1.0f, 0.0f, 0.5f,
			   -0.35f, 1.7f,  -0.5f, 	1.0f, 0.0f, 0.5f,
				0.0f,  1.0f,  -0.4f, 	1.0f, 0.0f, 0.5f,

				0.1f,  1.9f, -0.5f, 	1.0f, 1.0f, 1.0f, // Back. Head of spoon(top)
				0.35f, 1.7f, -0.5f,		1.0f, 1.0f, 1.0f,
				0.0f,  1.0f, -0.47f,	1.0f, 1.0f, 1.0f,
			   -0.1f,  1.9f, -0.5f,		1.0f, 1.0f, 1.0f,
			   -0.35f, 1.7f, -0.5f,		1.0f, 1.0f, 1.0f,
				0.0f,  1.0f, -0.47f, 	1.0f, 1.0f, 1.0f,

				0.1f,  1.9f, -0.5f, 	0.0f, 1.0f, 1.0f, // Front. Head of spoon(top)
				0.35f, 1.7f, -0.5f,		0.0f, 1.0f, 1.0f,
				0.0f,  1.0f, -0.4f,	    0.0f, 1.0f, 1.0f,
			   -0.1f,  1.9f, -0.5f,		0.0f, 1.0f, 1.0f,
			   -0.35f, 1.7f, -0.5f,		0.0f, 1.0f, 1.0f,
				0.0f,  1.0f, -0.4f, 	0.0f, 1.0f, 1.0f,

			   -0.1f, 0.5f, -0.5f, 	    0.0f, 0.0f, 1.0f, // Base of spoon head bottom
				0.0f, 1.0f, -0.47f,		0.0f, 0.0f, 1.0f,
				0.1f, 0.5f, -0.5f, 	    0.0f, 0.0f, 1.0f,
				0.1f, 1.9f, -0.5f, 	    0.0f, 0.0f, 1.0f, // Spoon head top triangle
				0.0f, 1.0f, -0.47f,		0.0f, 0.0f, 1.0f,
			   -0.1f, 1.9f, -0.5f, 	    0.0f, 0.0f, 1.0f,

				0.1f, 1.9f, -0.5f,		1.0f, 0.0f, 0.5f, // Top & bottom base back
				0.0f, 1.0f, -0.4f, 		1.0f, 0.0f, 0.5f,
			   -0.1f, 1.9f, -0.5f,		1.0f, 0.0f, 0.5f,
			   -0.1f, 0.5f, -0.4f,		1.0f, 0.0f, 0.5f,
				0.0f, 1.0f, -0.4f,		1.0f, 0.0f, 0.5f,
				0.1f, 0.5f, -0.4f,		1.0f, 0.0f, 0.5f,

				0.0f,  1.0f, -0.4f, 	1.0f, 1.0f, 0.0f, // Left back base
				0.1f,  0.5f, -0.4f, 	1.0f, 1.0f, 0.0f,
				0.1f,  0.5f, -0.5f, 	1.0f, 1.0f, 0.0f,
			   -0.1f, 0.5f, -0.5f, 		1.0f, 1.0f, 1.0f, // Right back base
			   -0.1f, 0.5f, -0.4f, 		1.0f, 1.0f, 1.0f,
			   -0.0f,  1.0f, -0.4f, 	1.0f, 1.0f, 1.0f

	};

	// Generate buffer ids
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	// Activate the VAO before binding and setting VBOs and VAPs
	glBindVertexArray(VAO);

	// Activate the VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); //Copy vertices to VBO

	// Set attribute pointer 0 to hold position data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0); //Enables vertex attribute

	// Set attribute pointer 1 to hold Normal data
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// Set attribute pointer 2 to hold Texture coordinate data
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0); // Unbind the VAO
}

// Generate and load the texture
void UGenerateTexture(){
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	int width, height;

	unsigned char* image = SOIL_load_image("grey070.jpg", &width, &height, 0, SOIL_LOAD_RGB);// Loads texture file

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture
}

/* Implements UMouseMove function */
void UMouseMove(int x, int y)
{
	// Immediately replaces center locked coords with new mouse coords
	if(mouseDetected)
		{
			lastMouseX = x;
			lastMouseY = y;
			mouseDetected = false;
		}

	// Gets the direction the mouse was moved in x and y
	mouseXOffset = x - lastMouseX;
	mouseYOffset = lastMouseY - y;

	// Updates with new mouse coords
	lastMouseX = x;
	lastMouseY = y;

	// Applies sensitivity to mouse direction
	mouseXOffset *= sensitivity;
	mouseYOffset *= sensitivity;

	// Accumulates the yaw and pitch variables
	yaw += mouseXOffset;
	pitch += mouseYOffset;

	// Converts mouse coords
	front.x = 10.0f * cos(yaw);
	front.y = 10.0f * sin(pitch);
	front.z = sin(yaw) * cos(pitch) * 10.0f;
}
// UKeyboard
void UKeyboard(unsigned char key, int x, int y) {
	switch(key){

	// Switches to 2D
	case '2':
		currentKey = key;
		cout << "Switching to 2D!" << endl;
		break;

	// Switches to 3D
	case '3':
		currentKey = key;
		cout << "Switching to 3D!" << endl;
		break;

	default:
		cout << "Press a key!" << endl;
	}
}

