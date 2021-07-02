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

#define PLATFORM_COUNT 2
#define OBSTACLE_COUNT 19
//using namespace std;

struct GameState {
    Entity *player;
    Entity *platforms;
    Entity *obstacles;
};

GameState state;

SDL_Window* displayWindow;
bool gameIsRunning = true;


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
    SDL_Init(SDL_INIT_VIDEO);
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
    state.player->position = glm::vec3(0, 3.2, 0);
    state.player->movement = glm::vec3(0);
    state.player->acceleration = glm::vec3(0, -.1f, 0);
    state.player->collidedBottom =  false;
    state.player->collidedRight =  false;
    state.player->collidedLeft =  false;
    state.player->collidedPlatform =  false;
    state.player->speed = 1.0f;
    state.player->textureID = LoadTexture("ufoGreen.png");
    
    state.platforms = new Entity[PLATFORM_COUNT];
    GLuint platformTextureID = LoadTexture("landingplatform.png");
    
    // Initialize Game Objects
    
    state.obstacles = new Entity[OBSTACLE_COUNT];
    GLuint obstacleTextureID = LoadTexture("obstacle.png");
    

   // landing area
    state.platforms[0].entityType = PLATFORM;
    state.platforms[0].textureID = platformTextureID;
    state.platforms[0].position = glm::vec3(-1, -3.2, 0);
    
    state.platforms[1].entityType = PLATFORM;
    state.platforms[1].textureID = platformTextureID;
    state.platforms[1].position = glm::vec3(0, -3.2, 0);
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
    state.obstacles[3].position = glm::vec3(1, -3.2, 0);
    
    state.obstacles[4].entityType = OBSTACLE;
    state.obstacles[4].textureID = obstacleTextureID;
    state.obstacles[4].position = glm::vec3(2, -3.2, 0);
    
    state.obstacles[5].entityType = OBSTACLE;
    state.obstacles[5].textureID = obstacleTextureID;
    state.obstacles[5].position = glm::vec3(3, -3.2, 0);
    
    state.obstacles[6].entityType = OBSTACLE;
    state.obstacles[6].textureID = obstacleTextureID;
    state.obstacles[6].position = glm::vec3(4, -3.2, 0);
     
    //right wall
    state.obstacles[7].entityType = OBSTACLE;
    state.obstacles[7].textureID = obstacleTextureID;
    state.obstacles[7].position = glm::vec3(4, -2.2, 0);
    
    state.obstacles[8].entityType = OBSTACLE;
    state.obstacles[8].textureID = obstacleTextureID;
    state.obstacles[8].position = glm::vec3(4, -1.2, 0);
    
    state.obstacles[9].entityType = OBSTACLE;
    state.obstacles[9].textureID = obstacleTextureID;
    state.obstacles[9].position = glm::vec3(4, -0.2, 0);
    
    state.obstacles[10].entityType = OBSTACLE;
    state.obstacles[10].textureID = obstacleTextureID;
    state.obstacles[10].position = glm::vec3(4, 0.8, 0);
    
    state.obstacles[11].entityType = OBSTACLE;
    state.obstacles[11].textureID = obstacleTextureID;
    state.obstacles[11].position = glm::vec3(4, 1.8, 0);
    
    state.obstacles[12].entityType = OBSTACLE;
    state.obstacles[12].textureID = obstacleTextureID;
    state.obstacles[12].position = glm::vec3(4, 2.8, 0);
    
    //left wall
    state.obstacles[13].entityType = OBSTACLE;
    state.obstacles[13].textureID = obstacleTextureID;
    state.obstacles[13].position = glm::vec3(-4, -2.2, 0);
    
    state.obstacles[14].entityType = OBSTACLE;
    state.obstacles[14].textureID = obstacleTextureID;
    state.obstacles[14].position = glm::vec3(-4, -1.2, 0);
    
    state.obstacles[15].entityType = OBSTACLE;
    state.obstacles[15].textureID = obstacleTextureID;
    state.obstacles[15].position = glm::vec3(-4, -0.2, 0);
    
    state.obstacles[16].entityType = OBSTACLE;
    state.obstacles[16].textureID = obstacleTextureID;
    state.obstacles[16].position = glm::vec3(-4, .8, 0);
    
    state.obstacles[17].entityType = OBSTACLE;
    state.obstacles[17].textureID = obstacleTextureID;
    state.obstacles[17].position = glm::vec3(-4, 1.8, 0);
    
    state.obstacles[18].entityType = OBSTACLE;
    state.obstacles[18].textureID = obstacleTextureID;
    state.obstacles[18].position = glm::vec3(-4, 2.8, 0);
    
    for (int i = 0; i < PLATFORM_COUNT; i++){
        state.platforms[i].Update(0, NULL, 0, NULL, 0);
    }
    
    for (int i = 0; i < OBSTACLE_COUNT; i++){
        state.obstacles[i].Update(0, NULL, 0, NULL, 0);
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
         state.player->Update(FIXED_TIMESTEP, state.platforms, PLATFORM_COUNT, state.obstacles, OBSTACLE_COUNT);
         deltaTime -= FIXED_TIMESTEP;
     }

    accumulator = deltaTime;
}


void Render() {
    glClear(GL_COLOR_BUFFER_BIT);

    for (int i = 0; i < PLATFORM_COUNT; i++){
        state.platforms[i].Render(&program);
    }
    
    for (int i = 0; i < OBSTACLE_COUNT; i++){
        state.obstacles[i].Render(&program);
    }
    
    GLuint fontTextureID = LoadTexture("font1.png");
    if (state.player->collidedLeft or state.player->collidedRight){
        DrawText(&program, fontTextureID, "Mission Failed", 1, -0.5f, glm::vec3(-3.25f, 0, 0));
    }
    else if (state.player->collidedPlatform){
        DrawText(&program, fontTextureID, "Mission Successful", 1, -0.5f, glm::vec3(-4.15f, 0, 0));
    }
    else if (state.player->collidedBottom){
        DrawText(&program, fontTextureID, "Mission Failed", 1, -0.5f, glm::vec3(-3.25f, 0, 0));
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
