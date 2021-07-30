#include "Menu.h"

#define Menu_WIDTH 14
#define Menu_HEIGHT 8
#define Menu_ENEMY_COUNT 1


unsigned int Menu_data[] =
{
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
     2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0
};

void Menu::Initialize() {
    // change background color
    glClearColor(0.7f, 0.4f, 0.6f, 1.0f);
    
    state.nextScene = -1;
    GLuint mapTextureID = Util::LoadTexture("sheet.png");
    state.map = new Map(Menu_WIDTH, Menu_HEIGHT, Menu_data, mapTextureID, 1.0f, 4, 1);
    
    // Initialize Game Objects
    
    // Initialize Player
    state.player = new Entity();
    state.player->entityType = PLAYER;
    state.player->position = glm::vec3(5, 0, 0);
    state.player->movement = glm::vec3(0);
    state.player->acceleration = glm::vec3(0, -9.81f, 0);
    state.player->speed = 2.0f;
    state.player->textureID = Util::LoadTexture("player_stand.png");
    
    state.player->animRight = new int[4] {3, 7, 11, 15};
    state.player->animLeft = new int[4] {1, 5, 9, 13};
    state.player->animUp = new int[4] {2, 6, 10, 14};
    state.player->animDown = new int[4] {0, 4, 8, 12};

    state.player->animIndices = state.player->animRight;
    state.player->animFrames = 4;
    state.player->animIndex = 0;
    state.player->animTime = 0;
    state.player->animCols = 4;
    state.player->animRows = 4;
    
    state.player->height = 0.8f;
    state.player->width = 0.8f;
    
    state.player->jumpPower = 6.0f;
    
    
    state.enemies = new Entity[Menu_ENEMY_COUNT];
    GLuint enemyTextureID = Util::LoadTexture("lollipopGreen.png");
    
    state.enemies->entityType = ENEMY;
    state.enemies[0].textureID = enemyTextureID;
    state.enemies[0].position = glm::vec3(4, -2.25, 0);
    state.enemies[0].speed = 1;
    state.enemies[0].aiType =  WAITANDGO;
    state.enemies[0].aiState = IDLE;
    state.enemies[0].isActive = false;
}

void Menu::Update(float deltaTime) {
    state.player->Update(deltaTime, state.player, state.enemies, Menu_ENEMY_COUNT, state.map);
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    
    if (keys[SDL_SCANCODE_RETURN]) {
        state.nextScene = 1;
    }
}
void Menu::Render(ShaderProgram *program) {
    GLuint fontTextureID = Util::LoadTexture("font1.png");
    Util::DrawText(program, fontTextureID, "RUNAWAY", 1, -0.5f, glm::vec3(1, -2, 0));
    Util::DrawText(program, fontTextureID, "Press Return", 1, -0.5f, glm::vec3(1, -5, 0));
    state.map->Render(program);
    state.player->Render(program);
}
