#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, rocketMatrix, projectionMatrix;

GLuint playerTextureID;
GLuint rocketTextureID;

GLuint LoadTexture(const char* filePath) {
 int w, h, n;
 unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha);

 if (image == NULL) {
 std::cout << "Unable to load image. Make sure the path is correct\n";
 assert(false);
 }

 GLuint textureID;
 glGenTextures(1, &textureID);
 glBindTexture(GL_TEXTURE_2D, textureID);
 glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

 stbi_image_free(image);
 return textureID;
}

void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("P1", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 1280, 960);
    
    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");
    
    viewMatrix = glm::mat4(1.0f);
    modelMatrix = glm::mat4(1.0f);
    rocketMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-7.0f, 7.0f, -4.0f, 4.0f, -1.0f, 1.0f);
    
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    //program.SetColor(0.0f, 0.5f, 0.5f, 1.0f);
    
    glUseProgram(program.programID);
    
    glClearColor(0.0f, 0.5f, 0.5f, 1.0f);
    
    glEnable(GL_BLEND);
    // Good setting for transparency
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    playerTextureID = LoadTexture("ctg.png");
    rocketTextureID = LoadTexture("playerShip1_blue.png");
}

void ProcessInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            gameIsRunning = false;
        }
    }
}

void Update() {
    //modelMatrix = glm::mat4(1.0f);
    //modelMatrix = glm::scale(modelMatrix, glm::vec3(2.0f, 2.0f, 1.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(2.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    rocketMatrix = glm::translate(rocketMatrix, glm::vec3(0.03f, 0.03f, 0.0f));
}
void DrawMan() {
    program.SetModelMatrix(modelMatrix);
    glBindTexture(GL_TEXTURE_2D, playerTextureID);
     glDrawArrays(GL_TRIANGLES, 0, 6);
}
void DrawRocket() {
    program.SetModelMatrix(rocketMatrix);
    glBindTexture(GL_TEXTURE_2D, rocketTextureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
void Render() {
    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
    
    glClear(GL_COLOR_BUFFER_BIT);
    
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);
 
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program.texCoordAttribute);
    
    DrawMan();
    DrawRocket();
    
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
    
    SDL_GL_SwapWindow(displayWindow);
    
}

void Shutdown() {
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    Initialize();
    
    while (gameIsRunning) {
        ProcessInput();
        Update();
        Render();
    }
    
    Shutdown();
    return 0;
}
