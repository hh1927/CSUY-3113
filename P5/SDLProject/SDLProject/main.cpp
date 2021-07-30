#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#include <string>
#include <vector>

#include "Util.h"
#include "Entity.h"
#include "Map.h"
#include "Scene.h"
#include "Menu.h"
#include "Level1.h"
#include "Level2.h"
#include "Level3.h"

SDL_Window* displayWindow;
bool gameIsRunning = true;
int lives;

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

Scene *currentScene;
Scene *sceneList[4];

void SwitchToScene(Scene *scene) {
    currentScene = scene;
    currentScene->Initialize();
}

Mix_Music *music;
Mix_Chunk *bounce;
Mix_Chunk *boing;
Mix_Chunk *levelcomplete;
Mix_Chunk *gameover;

void Initialize() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    displayWindow = SDL_CreateWindow("RUNAWAY GAME", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 1280, 960);
    
    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");
    
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    music = Mix_LoadMUS("background.wav");
    Mix_PlayMusic(music, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 4);
    
    bounce = Mix_LoadWAV("invalid.mp3");
    boing = Mix_LoadWAV("boop.wav");
    levelcomplete = Mix_LoadWAV("levelend.wav");
    gameover = Mix_LoadWAV("gameover.flac");
    
    lives = 3;
    
    viewMatrix = glm::mat4(1.0f);
    modelMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    
    glUseProgram(program.programID);
    
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    sceneList[0] = new Menu();
    sceneList[1] = new Level1();
    sceneList[2] = new Level2();
    sceneList[3] = new Level3();
    SwitchToScene(sceneList[0]);
    
}

void ProcessInput() {
    
    currentScene->state.player->movement = glm::vec3(0);
    
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
                        // Some sort of action
                        if (currentScene->state.player->collidedBottom){
                            currentScene->state.player->jump = true;
                            Mix_PlayChannel(-1, boing, 0);
                        }
                        break;
                }
                break; // SDL_KEYDOWN
        }
    }
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    if (keys[SDL_SCANCODE_LEFT]) {
        currentScene->state.player->movement.x = -1.0f;
        currentScene->state.player->animIndices = currentScene->state.player->animLeft;
    }
    else if (keys[SDL_SCANCODE_RIGHT]) {
        currentScene->state.player->movement.x = 1.0f;
        currentScene->state.player->animIndices = currentScene->state.player->animRight;
    }
    

    if (glm::length(currentScene->state.player->movement) > 1.0f) {
        currentScene->state.player->movement = glm::normalize(currentScene->state.player->movement);
    }

}

#define FIXED_TIMESTEP 0.0166666f
float lastTicks = 0;
float accumulator = 0.0f;
void Update() {
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;

    deltaTime += accumulator;
    if (deltaTime < FIXED_TIMESTEP) {
        accumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP) {
        // Update. Notice it's FIXED_TIMESTEP. Not deltaTime
        currentScene->Update(FIXED_TIMESTEP);
        deltaTime -= FIXED_TIMESTEP;
    }
    
    //boop noise when collide right or left.
    if (currentScene->state.player->collidedLeft || currentScene->state.player->collidedRight){
        Mix_PlayChannel(-1, bounce, 0);
    }
    if (currentScene->state.player->Entity::CheckCollision(currentScene->state.enemies) && lives != 0){
        lives -= 1;
    }
    //kills player
    if (lives == 0){
        currentScene->state.player->isActive = false;
    }

    accumulator = deltaTime;
    
    viewMatrix = glm::mat4(1.0f);
    if (currentScene->state.player->position.x > 5) {
        viewMatrix = glm::translate(viewMatrix, glm::vec3(-currentScene->state.player->position.x, 3.75, 0));
    } else {
        viewMatrix = glm::translate(viewMatrix, glm::vec3(-5, 3.75, 0));
    }
}

void Render() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    program.SetViewMatrix(viewMatrix);
    
    currentScene->Render(&program);
    
    GLuint fontTextureID = Util::LoadTexture("font1.png");
    Util::DrawText(&program, fontTextureID, "Lives: " + std::to_string(lives), .75f, -0.4f, glm::vec3(.5f, -.5f, 0));
    if (lives == 0){
        //GLuint fontTextureID = Util::LoadTexture("font1.png");
        Mix_PlayChannel(-1, gameover, 0);
        Util::DrawText(&program, fontTextureID, "You Lose!", 1, -0.5f, glm::vec3(3, -2.5f, 0));
    }
    
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
        
        if (currentScene->state.nextScene >= 0){
            Mix_PlayChannel(-1, levelcomplete, 0);
            SwitchToScene(sceneList[currentScene->state.nextScene]);
        }
            
        
        Render();
    }
    
    Shutdown();
    return 0;
}
