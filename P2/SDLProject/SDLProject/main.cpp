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
glm::mat4 viewMatrix, leftPaddle, rightPaddle, ball, projectionMatrix;

// Start at -4.0, 0, 0
glm::vec3 leftPlayer_position = glm::vec3(-4, 0, 0);
glm::vec3 rightPlayer_position = glm::vec3(4, 0, 0);
glm::vec3 ball_position = glm::vec3(0, 0, 0);
// Don’t go anywhere (yet).
glm::vec3 leftPlayer_movement = glm::vec3(0, 0, 0);
glm::vec3 rightPlayer_movement = glm::vec3(0, 0, 0);
glm::vec3 ball_movement = glm::vec3(0, 0, 0);

float player_speed = 1.0f;

GLuint paddleTextureID;
GLuint ballTextureID;

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
    displayWindow = SDL_CreateWindow("Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 1280, 960);
    
    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");
    
    viewMatrix = glm::mat4(1.0f);
    leftPaddle = glm::mat4(1.0f);
    rightPaddle = glm::mat4(1.0f);
    ball = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -4.25f, 4.25f, -1.0f, 1.0f);
    
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    //program.SetColor(0.7f, 0.7f, 0.7f, 0.5f);
    
    glUseProgram(program.programID);
    
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    
    glEnable(GL_BLEND);
    // Good setting for transparency
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    paddleTextureID = LoadTexture("paddle.png");
    ballTextureID = LoadTexture("ball.png");
    
    
}

void ProcessInput() {
 
    leftPlayer_movement = glm::vec3(0);
    rightPlayer_movement = glm::vec3(0);
    
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
            switch (event.type) {
                    case SDL_QUIT:
                    case SDL_WINDOWEVENT_CLOSE:
                        gameIsRunning = false;
                        break;

                    case SDL_KEYDOWN:
                        switch (event.key.keysym.sym) {
                            case SDLK_LEFT:
                                // Move the player left
                                break;

                            case SDLK_RIGHT:
                                // Move the player right
                                break;

                            case SDLK_SPACE:
                                ball_movement.x = 1.0f;
                                ball_movement.y = 1.0f;
                                break;
                        }
                    break; // SDL_KEYDOWN
            }
    }
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    if (keys[SDL_SCANCODE_UP]) {
        rightPlayer_movement.y = 1.0f;
    }
    
    else if (keys[SDL_SCANCODE_DOWN]) {
        rightPlayer_movement.y = -1.0f;
    }
    
    if (glm::length(rightPlayer_movement) > 1.0f) {
        rightPlayer_movement = glm::normalize(rightPlayer_movement);
    }
    
    if (keys[SDL_SCANCODE_W]) {
            leftPlayer_movement.y = 1.0f;
    }
    
    else if (keys[SDL_SCANCODE_S]) {
        leftPlayer_movement.y = -1.0f;
    }
    
    if (glm::length(leftPlayer_movement) > 1.0f) {
        leftPlayer_movement = glm::normalize(leftPlayer_movement);
    }

}

float lastTicks = 0.0f;

void Update() {
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;
    // Add (direction * units per second * elapsed time)
    leftPlayer_position += leftPlayer_movement * 2.0f * player_speed * deltaTime;
    rightPlayer_position += rightPlayer_movement * 2.0f * player_speed * deltaTime;
    ball_position += ball_movement * player_speed * deltaTime;
    
    leftPaddle = glm::mat4(1.0f);
    if (leftPlayer_position.y > 3.75){
        leftPlayer_position.y = 3.75;
    }
    else if (leftPlayer_position.y <  -3.75) {
        leftPlayer_position.y = -3.75;
    }
    //leftPaddle = glm::translate(leftPaddle, glm::vec3(-4.5f, -3.2f, 1.0f));}
    leftPaddle = glm::translate(leftPaddle, leftPlayer_position);
    
    rightPaddle = glm::mat4(1.0f);
    if (rightPlayer_position.y > 3.75){
        rightPlayer_position.y = 3.75;
    }
    else if (rightPlayer_position.y <  -3.75) {
        rightPlayer_position.y = -3.75;
    }
    //rightPaddle = glm::translate(rightPaddle, glm::vec3(4.5f, 3.2f, 1.0f));
    //leftPaddle = glm::translate(leftPaddle, glm::vec3(-4.5f, -3.2f, 1.0f));
    rightPaddle = glm::translate(rightPaddle, rightPlayer_position);
    
    ball = glm::mat4(1.0f);
    if (ball_position.y > 3.75){
        ball_position.y = 3.75;
    }
    
    if (ball_position.y <  -3.75) {
        ball_position.y = -3.75;
    }
    
    if (ball_position.x > 4){
        ball_position.x = 4;
    }
    
    if (ball_position.x <  -4) {
        ball_position.x = -4;
    }
    
    if (ball_position.y == 3.75 or ball_position.y == -3.75) {
        ball_movement.y *= -1;
        ball = glm::translate(ball, ball_position);
    }
    
    else {
        ball = glm::translate(ball, ball_position);
    }
    
    if (ball_position.x == 4 or ball_position.x == -4) {
        ball = glm::translate(ball, ball_position);
    }
    
    else {
        ball = glm::translate(ball, glm::vec3(0.0f, 0.0f, 0.0f));
        
    }
}
void DrawPlayer_left(){
    program.SetModelMatrix(leftPaddle);
    glBindTexture(GL_TEXTURE_2D, paddleTextureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void DrawPlayer_right(){
    program.SetModelMatrix(rightPaddle);
    glBindTexture(GL_TEXTURE_2D, paddleTextureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void DrawBall(){
    program.SetModelMatrix(ball);
    glBindTexture(GL_TEXTURE_2D, ballTextureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Render() {
 glClear(GL_COLOR_BUFFER_BIT);

 float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
 float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
 
 glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
 glEnableVertexAttribArray(program.positionAttribute);
 
 glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
 glEnableVertexAttribArray(program.texCoordAttribute);
 
 DrawPlayer_left();
 DrawPlayer_right();
 DrawBall();

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

