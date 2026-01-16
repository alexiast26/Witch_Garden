#if defined (__APPLE__)
    #define GLFW_INCLUDE_GLCOREARB
    #define GL_SILENCE_DEPRECATION
#else
    #define GLEW_STATIC
    #include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"
#include <iostream>
#include <irrKlang.h>
#include <string.h>
#include <stdio.h>
#pragma comment(lib, "irrKlang.lib") // link with irrKlang.dll

// window
gps::Window myWindow;
int glWindowWidth = 800;
int glWindowHeight = 600;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;


// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;
glm::mat4 lightRotation;


// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;


// camera
gps::Camera myCamera(
    glm::vec3(0.999f, 0.2f, -0.001681),
    glm::vec3(0.0f, 0.1392f, -1.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.05f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

GLboolean pressedKeys[1024];

// models
gps::Model3D screenQuad;
gps::Model3D cat;
gps::Model3D scene;
gps::Model3D ground;
gps::Model3D broom;
gps::Model3D teapot;
gps::Model3D spoon;
gps::Model3D big_grass;
gps::Model3D candle1;
gps::Model3D candle2;
gps::Model3D candle3;
gps::Model3D house_light;
gps::Model3D pole_light1;
gps::Model3D pole_light2;
gps::Model3D pole_light3;
GLfloat angle;

//cat roation
GLfloat catRotaition = 0.0f;

//teapot balancing
bool isBowing = false;
float bowTime = 0.0f;
float crtAngle = 0.0f;


// shaders
gps::Shader myBasicShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;
gps::Shader skyboxShader;

// skybox
gps::SkyBox mySkyBox;

// depthTexture
const unsigned int SHADOW_WIDTH = 4096;  //4096
const unsigned int SHADOW_HEIGHT = 4096;

GLuint shadowMapFBO;
GLuint depthMapTexture;
GLuint textureID;

bool showDepthMap;

// mouse data
//initial mouse positions
float lastX = 400, lastY = 300;
//camera rotation
float yaw = -90.0f, pitch = 0.0f;
bool firstMouse = true;
float mouseSensitivity = 0.05f;
bool mouseCaptured = true;

//sound
irrklang::ISoundEngine* SoundEngine = irrklang::createIrrKlangDevice();


GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
	//TODO
    if (width == 0 || height == 0) {
        return;
    }

    //update the wiewport
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, width, height);

    //update global retina
    retina_width = fbWidth;
    retina_height = fbHeight;

    //recompute the projection matrix with the new aspect ratio
    float aspect = (float)fbWidth / (float)fbHeight;
    projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 1000.0f);

    lightShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    //send the updated mtrix to the shader
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    skyboxShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key == GLFW_KEY_M && action == GLFW_PRESS)
        showDepthMap = !showDepthMap;

    if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS) {
        mouseCaptured = !mouseCaptured;

        if (mouseCaptured) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true;
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }

    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        if (!isBowing) {
            isBowing = true;
            bowTime = 0.0f;
        }
    }

    if (key == GLFW_KEY_0 && action == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    //wireframe
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    
    //points
    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }

	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }

}


void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    //TODO
    //if mouse not captured, don't process movement
    if (!mouseCaptured) {
        return;
    }

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; //reversed cuz y-coord go from bottom to top
    lastX = xpos;
    lastY = ypos;

    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    //Constrain pitch to prevent flipping
    if (pitch > 89.0f) {
        pitch = 89.0f;
    }
    if (pitch < -89.0f) {
        pitch = -89.0f;
    }

    myCamera.rotate(pitch, yaw);

    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

}


void processMovement() {
    float actualSpeed = cameraSpeed * deltaTime;
    glm::vec3 crtPos = myCamera.getCameraPosition();
    glm::vec3 frontDir = myCamera.getCameraFront();
    glm::vec3 rightDir = myCamera.getCameraRight();

    glm::vec3 frontHorizontal = glm::normalize(glm::vec3(frontDir.x, 0.0f, frontDir.z));
    float y = 2;

    if (pressedKeys[GLFW_KEY_W]) {
        glm::vec3 nextPos = crtPos + actualSpeed * (-frontHorizontal);
        nextPos.y = y;
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_S]) {
        glm::vec3 nextPos = crtPos + actualSpeed * (frontHorizontal);
        nextPos.y = y;
        myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_A]) {
        glm::vec3 nextPos = crtPos + actualSpeed * (rightDir);
        nextPos.y = y;
        myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_D]) {
        glm::vec3 nextPos = crtPos + actualSpeed * (rightDir);
        nextPos.y = y;
        myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    //if (pressedKeys[GLFW_KEY_Q]) {
    //    angle -= 1.0f;
    //    // update model matrix for teapot
    //    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
    //    // update normal matrix for teapot
    //    normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    //}

    if (pressedKeys[GLFW_KEY_R]) {
        catRotaition -= 180.0f * deltaTime;
        if (catRotaition < -360.0f) {
            catRotaition += 360.0f;
        }
    }
    if (isBowing) {
        bowTime += deltaTime * 0.8f;
        crtAngle = sin(bowTime) / 2;
        if (bowTime >= 3.14159f) {
            isBowing = false;
            bowTime = 0.0f;
            crtAngle = 0.0f; 
        }
    }
}

bool initOpenGLWindow() {
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not start GLFW3\n");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);


    //window scaling for HiDPI displays
    myWindow.Create(glWindowWidth, glWindowHeight, "OpenGL Shader Example");
    glWindow = myWindow.getWindow();

    glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
    glfwSetKeyCallback(glWindow, keyboardCallback);
    glfwSetCursorPosCallback(glWindow, mouseCallback);
    glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwMakeContextCurrent(glWindow);

    glfwSwapInterval(1);

#if not defined (__APPLE__)
    // start GLEW extension handler
    glewExperimental = GL_TRUE;
    glewInit();
#endif

    glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

    return true;
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
    glfwSetKeyCallback(glWindow, keyboardCallback);
    glfwSetCursorPosCallback(glWindow, mouseCallback);
}

void initOpenGLState() {
    glClearColor(0.01f, 0.01f, 0.05f, 1.0f);
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {
    screenQuad.LoadModel("models/quad/quad.obj");
    cat.LoadModel("models/main_scene/cat.obj");
    scene.LoadModel("models/main_scene/main_scene.obj");
    ground.LoadModel("models/ground/ground.obj");
    broom.LoadModel("models/main_scene/broom.obj");
    teapot.LoadModel("models/main_scene/teapot.obj");
    spoon.LoadModel("models/main_scene/spoon.obj");
    big_grass.LoadModel("models/main_scene/big_grass.obj");
    candle1.LoadModel("models/main_scene/candle1.obj");
    candle2.LoadModel("models/main_scene/candle2.obj");
    candle3.LoadModel("models/main_scene/candle3.obj");
    house_light.LoadModel("models/main_scene/house_light.obj");
    house_light.LoadModel("models/main_scene/light_pole1.obj");
    house_light.LoadModel("models/main_scene/light_pole2.obj");
    house_light.LoadModel("models/main_scene/light_pole3.obj");
   
}

void initShaders() {
    myBasicShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
    myBasicShader.useShaderProgram();
    lightShader.loadShader("shaders/lightSource.vert", "shaders/lightSource.frag");
    lightShader.useShaderProgram();
    screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
    screenQuadShader.useShaderProgram();
    depthMapShader.loadShader("shaders/depthMapShader.vert", "shaders/depthMapShader.frag");
    depthMapShader.useShaderProgram();
    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyboxShader.useShaderProgram();

}

void initUniforms() {
	myBasicShader.useShaderProgram();

    // get dimensions from the now-initialized myWindow object
    float width = (float)myWindow.getWindowDimensions().width;
    float height = (float)myWindow.getWindowDimensions().height;

    // safety guard
    if (height <= 0) {
        height = 1.0f;
    }
    float aspectRatio = width / height;
    // create model matrix for teapot
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

	// get view matrix for current camera
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	// send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

	// create projection matrix
	projection = glm::perspective(glm::radians(45.0f),
                               aspectRatio,
                               0.1f, 100.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	// send light dir to shader
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 0.15f, 0.25f); //white light
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	// send light color to shader
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    lightShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

}

void initFBO() {
    //TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
    glGenFramebuffers(1, &shadowMapFBO);

    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

glm::mat4 computeLightSpaceTrMatrix() {
    //TODO - Return the light-space transformation matrix
    glm::mat4 lightView = glm::lookAt(lightDir * 50.0f, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    const GLfloat near_plane = 1.0f, far_plane = 100;
    glm::mat4 lightProjection = glm::ortho(-4.5f, 4.5f, -4.5f, 4.5f, near_plane, far_plane);
    glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;

    return lightSpaceTrMatrix;
}


void renderCat(gps::Shader shader, bool depthPass) {
    shader.useShaderProgram();

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    
    glm::vec3 pivot = glm::vec3(1.00869f, 0.06932f, -2.18939f);

    // PASUL 3: put it back in the scene
    modelMatrix = glm::translate(modelMatrix, pivot);

    // PASUL 2: rotate
    modelMatrix = glm::rotate(modelMatrix, glm::radians(catRotaition), glm::vec3(0.0f, 1.0f, 0.0f));

    // PASUL 1: bring it to origin (0,0,0)            
    modelMatrix = glm::translate(modelMatrix, -pivot);

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * modelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    cat.Draw(shader);
}

void renderMScene(gps::Shader shader, bool depthPass) {
    shader.useShaderProgram();

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
 
    scene.Draw(shader);
}

void renderGround(gps::Shader shader, bool depthPass) {
    shader.useShaderProgram();

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    ground.Draw(shader);
}

void renderBroom(gps::Shader shader, bool depthPass) {
    shader.useShaderProgram();

    //the broom will move up and down to simulate levitation effect

    float time = (float)glfwGetTime();
    float deltaY = sin(time * 0.8f) / 22;  //(sin(time * 0.8f) * 0.7) / 20;
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, deltaY, 0.0f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * modelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    broom.Draw(shader);
}

void renderSpoon(gps::Shader shader, bool depthPass) {
    shader.useShaderProgram();

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::vec3 pivot = glm::vec3(0.59f, 0.08f, -2.67f);

    modelMatrix = glm::translate(modelMatrix, pivot);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(100 * (float)glfwGetTime()), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::translate(modelMatrix, -pivot);

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * modelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    spoon.Draw(shader);
}

void renderTeapot(gps::Shader shader, bool depthPass) {
    shader.useShaderProgram();

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::vec3 pivot = glm::vec3(0.118652f, 0.128433f, -1.67852f);

    modelMatrix = glm::translate(modelMatrix, pivot);
    modelMatrix = glm::rotate(modelMatrix, crtAngle, glm::vec3(0.0f, 0.0f, 1.0f));
    modelMatrix = glm::translate(modelMatrix, -pivot);

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    teapot.Draw(shader);
}

void renderBigGrass(gps::Shader shader, bool depthPass) {
    shader.useShaderProgram();

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    big_grass.Draw(shader);
}

void renderLights(gps::Shader shader) {
    shader.useShaderProgram();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);

    GLint loc = glGetUniformLocation(shader.shaderProgram, "model");
    if (loc != -1) {
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
    }
    GLint colorLoc = glGetUniformLocation(shader.shaderProgram, "lightColor");
    if (colorLoc != -1) {
        glm::vec3 glowColor = glm::vec3(2.55f, 1.99f, 0.61f);
        glUniform3fv(colorLoc, 1, glm::value_ptr(glowColor));
    }
    glDisable(GL_CULL_FACE);

    house_light.Draw(shader);
    pole_light1.Draw(shader);
    pole_light2.Draw(shader);
    pole_light3.Draw(shader);
    candle1.Draw(shader);
    candle2.Draw(shader);
    candle3.Draw(shader);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
}


void renderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//render the scene

    // shadow pass (depth map)
     depthMapShader.useShaderProgram();
     glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));
     glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
     glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
     glClear(GL_DEPTH_BUFFER_BIT);

	// render the teapot
	 //renderTeapot(myBasicShader, true);
     renderCat(depthMapShader, true);
     renderMScene(depthMapShader, true);
     renderGround(depthMapShader, true);
     renderBroom(depthMapShader, true);
     renderTeapot(depthMapShader, true);
     renderSpoon(depthMapShader, true);
     renderBigGrass(depthMapShader, true);
     glBindFramebuffer(GL_FRAMEBUFFER, 0);

     // main pass
     if (showDepthMap) {
		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT);

		screenQuadShader.useShaderProgram();

		//bind the depth map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

		glDisable(GL_DEPTH_TEST);
		screenQuad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);
	 }
	 else {

		// final scene rendering pass (with shadows)
		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myBasicShader.useShaderProgram();

        glm::vec3 pointLightPositions[7] = {
        glm::vec3(0.633715f, 0.178798f, -3.02519f),  // candle1
        glm::vec3(0.636248f, 0.113232f, -3.00641f),  // candle2
        glm::vec3(0.044475f, 0.391092f, -1.6515f),  // candle3
        glm::vec3(1.45561f, 0.340656f, -1.99858f),  // house_light
        glm::vec3(0.620615f, 0.297354f,         0.01794f),  // pole_light1
        glm::vec3(1.08481f, 0.230449f, 1.26476), // pole_light2
        glm::vec3(-2.15379f, 0.259114f, 0.928646f)   // pole_light3
        };

        glm::vec3 pointLightColors[7] = {
            glm::vec3(0.5f, 0.4f, 0.1f),  // warm candle glow
            glm::vec3(0.5f, 0.4f, 0.1f),  // warm candle glow
            glm::vec3(0.5f, 0.4f, 0.1f),  // warm candle glow
            glm::vec3(0.1f, 0.1f, 0.5f),  // house light
            glm::vec3(0.5f, 0.4f, 0.1f),  // pole light
            glm::vec3(0.5f, 0.4f, 0.1f),   // pole light
            glm::vec3(0.5f, 0.4f, 0.1f)   // pole light
        };

        for (int i = 0; i < 7; i++)
        {
            std::string posName = "pointLightPositions[" + std::to_string(i) + "]";
            std::string colName = "pointLightColors[" + std::to_string(i) + "]";

            GLint posLoc = glGetUniformLocation(myBasicShader.shaderProgram, posName.c_str());
            GLint colLoc = glGetUniformLocation(myBasicShader.shaderProgram, colName.c_str());

            glUniform3fv(posLoc, 1, glm::value_ptr(pointLightPositions[i]));
            glUniform3fv(colLoc, 1, glm::value_ptr(pointLightColors[i]));
        }

		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

        glm::vec3 moonColor = glm::vec3(0.1f, 0.15f, 0.25f);
        glUniform3fv(lightColorLoc, 1, glm::value_ptr(moonColor));

		//lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

		//bind the shadow map
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap"), 3);

		glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceTrMatrix"),
			1,
			GL_FALSE,
			glm::value_ptr(computeLightSpaceTrMatrix()));

        //render objects
		//renderTeapot(myBasicShader, false);
        renderMScene(myBasicShader, false);
        renderGround(myBasicShader, false);
        renderBroom(myBasicShader, false);
        renderTeapot(myBasicShader, false);
        renderSpoon(myBasicShader, false);
        renderCat(myBasicShader, false);
        renderBigGrass(myBasicShader, false);

        lightShader.useShaderProgram();
        glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

      
        renderLights(lightShader);

		mySkyBox.Draw(skyboxShader, view, projection);
	}
}


void initSkybox() {
    std::vector<const GLchar*> faces;
    faces.push_back("skybox/right.png");
    faces.push_back("skybox/left.png");
    faces.push_back("skybox/top.png");
    faces.push_back("skybox/bottom.png ");
    faces.push_back("skybox/back.png");
    faces.push_back("skybox/front.png");

    mySkyBox.Load(faces);
}


void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}

int main(int argc, const char * argv[]) {
    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }


    initOpenGLState();
	initModels();
	initShaders();
	initUniforms();
    initFBO();
    initSkybox();
    setWindowCallbacks();

    if (!SoundEngine) {
        return 0;
    }
    SoundEngine->play2D("audio/background.wav", true);

	glCheckError();
	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processMovement();
	    renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}

	cleanup();
    SoundEngine->drop();
    return EXIT_SUCCESS;
}
