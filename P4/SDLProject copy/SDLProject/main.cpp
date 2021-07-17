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

#include <string>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"
#include "Entity.h"
#include <SDL_mixer.h>

#define PLATFORM_COUNT 0
#define OBSTACLE_COUNT 13
#define ENEMY_COUNT 3
//using namespace std;

struct GameState {
    Entity *player;
    Entity *platforms;
    Entity *obstacles;
    Entity *enemies;
};

GameState state;

SDL_Window* displayWindow;
bool gameIsRunning = true;

Mix_Music *music;
Mix_Chunk *bounce;

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

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

void DrawText(ShaderProgram *program, GLuint fontTextureID, std::string text, float size, float spacing, glm::vec3 position) {
    float width = 1.0f / 16.0f;
    float height = 1.0f / 16.0f;

    std::vector<float> vertices;
    std::vector<float> texCoords;

    for(int i = 0; i < text.size(); i++) {

        int index = (int)text[i];
        float offset = (size + spacing) * i;
        float u = (float)(index % 16) / 16.0f;
        float v = (float)(index / 16) / 16.0f;
     
        vertices.insert(vertices.end(), {
            offset + (-0.5f * size), 0.5f * size,
            offset + (-0.5f * size), -0.5f * size,
            offset + (0.5f * size), 0.5f * size,
            offset + (0.5f * size), -0.5f * size,
            offset + (0.5f * size), 0.5f * size,
            offset + (-0.5f * size), -0.5f * size,
        });
        texCoords.insert(texCoords.end(), {
            u, v,
            u, v + height,
            u + width, v,
            u + width, v + height,
            u + width, v,
            u, v + height,
        });

    } // end of for loop
    
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    program->SetModelMatrix(modelMatrix);

    glUseProgram(program->programID);

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->positionAttribute);

    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords.data());
    glEnableVertexAttribArray(program->texCoordAttribute);

    glBindTexture(GL_TEXTURE_2D, fontTextureID);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}


void Initialize() {
    // Initialize audio
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    int Mix_OpenAudio(int frequency, Uint16 format, int channels, int chunksize);
    // Start Audio
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    music =  Mix_LoadMUS("background.wav");
    Mix_PlayMusic(music, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 4);
    
    bounce = Mix_LoadWAV("boop.wav");
    
    displayWindow = SDL_CreateWindow("Lunar Lander", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 1280, 960);
    
    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");
    
    viewMatrix = glm::mat4(1.0f);
    modelMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    
    glUseProgram(program.programID);
    
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    
    // Initialize Player
    state.player = new Entity();
    state.player->entityType = PLAYER;
    state.player->position = glm::vec3(0, 1.8, 0);
    state.player->movement = glm::vec3(0);
    state.player->acceleration = glm::vec3(0, -.1f, 0);
    state.player->collidedBottom =  false;
    //state.player->collidedRight =  false;
    //state.player->collidedLeft =  false;

   // state.player->collidedPlatform =  false;
    state.player->speed = 1.0f;
    state.player->textureID = LoadTexture("rocket.png");
    
    state.platforms = new Entity[PLATFORM_COUNT];
    //GLuint platformTextureID = LoadTexture("cloud5.png");
   
/*
    for (int i = 0; i < 10; i++)
    {
        state.platforms[i].entityType = PLATFORM;
        state.platforms[i].textureID = platformTextureID;
        //bottom
        state.platforms[i].position = glm::vec3(-5 + i, -3.2f, 0);
    }
    
    for (int i = 0; i < 7; i++)
    {
        state.platforms[i].entityType = PLATFORM;
        state.platforms[i].textureID = platformTextureID;
        //left
        state.platforms[i].position = glm::vec3(-4.0f, 3.2 + i, 0);
    }
    
    for (int i = 0; i < 7; i++)
    {
        state.platforms[i].entityType = PLATFORM;
        state.platforms[i].textureID = platformTextureID;
        //right
        state.platforms[i].position = glm::vec3(4.0f, -3.2 + i, 0);
    }
    
    for (int i = 0; i < 10; i++)
    {
        state.platforms[i].entityType = PLATFORM;
        state.platforms[i].textureID = platformTextureID;
        //top
        state.platforms[i].position = glm::vec3(-5 + i, 3.2f, 0);
    }
*/
    // Initialize Game Objects
    
    state.obstacles = new Entity[OBSTACLE_COUNT];
    GLuint obstacleTextureID = LoadTexture("cloud5.png");
    
    state.enemies = new Entity[ENEMY_COUNT];
    state.enemies->collidedBottom =  false;
    state.enemies->collidedRight =  false;
    state.enemies->collidedLeft =  false;
    GLuint enemyTextureID = LoadTexture("alien.png");
    
    
    
    state.enemies[0].entityType = ENEMY;
    state.enemies[0].textureID = enemyTextureID;
    state.enemies[0].position = glm::vec3(-3, -2.2, 0);
    state.enemies[0].speed = .5;
    state.enemies[0].aiType = WALKER;
    
    state.enemies[1].entityType = ENEMY;
    state.enemies[1].textureID = enemyTextureID;
    state.enemies[1].position = glm::vec3(3, -2.2, 0);
    state.enemies[1].speed = .5;
    state.enemies[1].aiType = WAITANDGO;
    state.enemies[1].aiState = FLYING;
    
    state.enemies[2].entityType = ENEMY;
    state.enemies[2].textureID = enemyTextureID;
    state.enemies[2].position = glm::vec3(-3, 2.2, 0);
    state.enemies[2].speed = .5;
    state.enemies[2].aiType = PATROL;
    
    

 // landing area
    
    //state.platforms[0].entityType = PLATFORM;
    //state.platforms[0].textureID = platformTextureID;
    //state.platforms[0].position = glm::vec3(-1, -3.2, 0);
    
    //state.platforms[1].entityType = PLATFORM;
   // state.platforms[1].textureID = platformTextureID;
   // state.platforms[1].position = glm::vec3(0, -3.2, 0);
    // base
    state.obstacles[0].entityType = OBSTACLE;
    state.obstacles[0].textureID = obstacleTextureID;
    state.obstacles[0].position = glm::vec3(-4, -3.2, 0);
    
    state.obstacles[1].entityType = OBSTACLE;
    state.obstacles[1].textureID = obstacleTextureID;
    state.obstacles[1].position = glm::vec3(-3, -3.2, 0);
    
    state.obstacles[2].entityType = OBSTACLE;
    state.obstacles[2].textureID = obstacleTextureID;
    state.obstacles[2].position = glm::vec3(-2, -3.2, 0);
    
    state.obstacles[3].entityType = OBSTACLE;
    state.obstacles[3].textureID = obstacleTextureID;
    state.obstacles[3].position = glm::vec3(-1, -3.2, 0);
    
    state.obstacles[4].entityType = OBSTACLE;
    state.obstacles[4].textureID = obstacleTextureID;
    state.obstacles[4].position = glm::vec3(0, -3.2, 0);
    
    state.obstacles[5].entityType = OBSTACLE;
    state.obstacles[5].textureID = obstacleTextureID;
    state.obstacles[5].position = glm::vec3(1, -3.2, 0);
    
    state.obstacles[6].entityType = OBSTACLE;
    state.obstacles[6].textureID = obstacleTextureID;
    state.obstacles[6].position = glm::vec3(2, -3.2, 0);
    
    state.obstacles[7].entityType = OBSTACLE;
    state.obstacles[7].textureID = obstacleTextureID;
    state.obstacles[7].position = glm::vec3(3, -3.2, 0);
    
    state.obstacles[8].entityType = OBSTACLE;
    state.obstacles[8].textureID = obstacleTextureID;
    state.obstacles[8].position = glm::vec3(4, -3.2, 0);
    
    
    //level
    state.obstacles[9].entityType = OBSTACLE;
    state.obstacles[9].textureID = obstacleTextureID;
    state.obstacles[9].position = glm::vec3(-4, 1.2, 0);
    
    state.obstacles[10].entityType = OBSTACLE;
    state.obstacles[10].textureID = obstacleTextureID;
    state.obstacles[10].position = glm::vec3(-3, 1.2, 0);
    
    state.obstacles[11].entityType = OBSTACLE;
    state.obstacles[11].textureID = obstacleTextureID;
    state.obstacles[11].position = glm::vec3(-2, 1.2, 0);
    
    state.obstacles[12].entityType = OBSTACLE;
    state.obstacles[12].textureID = obstacleTextureID;
    state.obstacles[12].position = glm::vec3(-1, 1.2, 0);
    
    //block
    state.obstacles[13].entityType = OBSTACLE;
    state.obstacles[13].textureID = obstacleTextureID;
    state.obstacles[13].position = glm::vec3(3, 2.2, 0);


  //  for (int i = 0; i < PLATFORM_COUNT; i++){
  //      state.platforms[i].Update(0, NULL, NULL, 0, NULL, 0);
  //  }
    
    for (int i = 0; i < OBSTACLE_COUNT; i++){
        state.obstacles[i].Update(0, NULL, NULL, 0, NULL, 0);
    }
    
 
}

void ProcessInput() {
    
    state.player->movement = glm::vec3(0);
    
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
                        break;
                }
                break; // SDL_KEYDOWN
        }
    }
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    if (keys[SDL_SCANCODE_LEFT]) {
        state.player->acceleration.x = -20.0f;
    }
    else if (keys[SDL_SCANCODE_RIGHT]) {
        state.player->acceleration.x = 20.0f;
    }
    else if (keys[SDL_SCANCODE_UP]) {
        state.player->acceleration.y = 1.3f;
    }
    else if (keys[SDL_SCANCODE_DOWN]) {
        state.player->acceleration.y = -1.3f;
    }
    if (glm::length(state.player->movement) > 1.0f) {
        state.player->movement = glm::normalize(state.player->movement);
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
         state.player->Update(FIXED_TIMESTEP, state.player, state.platforms, PLATFORM_COUNT, state.obstacles, OBSTACLE_COUNT);
         for (int i = 0; i < ENEMY_COUNT; i++){
             state.enemies[i].Update(FIXED_TIMESTEP, state.player, state.platforms, PLATFORM_COUNT, state.obstacles, OBSTACLE_COUNT);
         }
         deltaTime -= FIXED_TIMESTEP;
     }

    accumulator = deltaTime;
    
    if (state.player->collidedLeft or state.player->collidedRight or state.player->collidedBottom){
        Mix_PlayChannel(-1, bounce, 0);
    }
}


void Render() {
    glClear(GL_COLOR_BUFFER_BIT);

  //  for (int i = 0; i < PLATFORM_COUNT; i++){
   //     state.platforms[i].Render(&program);
   // }
    
    for (int i = 0; i < OBSTACLE_COUNT; i++){
        state.obstacles[i].Render(&program);
    }
    
    for (int i = 0; i < ENEMY_COUNT; i++){
        if (not (state.enemies[i].collidedLeft or state.enemies[i].collidedRight or state.enemies[i].collidedBottom)){
            state.enemies[i].Render(&program);
        }
       ;
    }
    
    GLuint fontTextureID = LoadTexture("font1.png");
    if (state.enemies->collidedLeft or state.enemies->collidedRight or state.enemies->collidedBottom){
        DrawText(&program, fontTextureID, "You Lose", 1, -0.5f, glm::vec3(-3.25f, 0, 0));
    }
   // else if (state.player->collidedPlatform){
    //    DrawText(&program, fontTextureID, "Mission Successful", 1, -0.5f, glm::vec3(-4.15f, 0, 0));
    //}
    else if (state.player->collidedBottom){
        DrawText(&program, fontTextureID, "You Win!", 1, -0.5f, glm::vec3(-3.25f, 0, 0));
    }
    
    state.player->Render(&program);
    
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
