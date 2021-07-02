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

enum EntityType {PLAYER, PLATFORM, OBSTACLE};

class Entity {
public:
    EntityType entityType;
    glm::vec3 position;
    glm::vec3 movement;
    glm::vec3 acceleration;
    glm::vec3 velocity;
    
    float width = 1;
    float height = 1;
    
    
    float speed;

    GLuint textureID;

    glm::mat4 modelMatrix;
    
    Entity();
    
    bool collidedBottom = false;
    bool collidedLeft= false;
    bool collidedRight = false;
    bool collidedPlatform = false;
    
    
    bool CheckCollision(Entity *other);
    void CheckCollisionsY(Entity *objects, int objectCount);
    void CheckCollisionsX(Entity *objects, int objectCount);
    void Update(float deltaTime, Entity *platforms, int platformCount, Entity *obstacles, int obstacleCount);
    void Render(ShaderProgram *program);
};
