#include "Entity.h"
Entity::Entity()
{
    position = glm::vec3(0);
    movement = glm::vec3(0);
    acceleration = glm::vec3(0);
    velocity = glm::vec3(0);

        
    speed = 0;

    modelMatrix = glm::mat4(1.0f);
}

bool Entity::CheckCollision(Entity *other)
{
    //if (other == this){return false;}
    float xdist = fabs(position.x - other->position.x) - ((width + other->width) / 2.0f);
    float ydist = fabs(position.y - other->position.y) - ((height + other->height)/ 2.0f);
    
    if (xdist < 0 && ydist < 0) return true;
    
    return false;
}

void Entity::CheckCollisionsY(Entity *objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity *object = &objects[i];

        if (CheckCollision(object))
        {
            float ydist = fabs(position.y - object->position.y);
            float penetrationY = fabs(ydist - (height / 2.0f) - (object->height / 2.0f));
            if (velocity.y > 0) {
                position.y -= penetrationY;
                velocity.y = 0;

            }
            else if (velocity.y < 0) {
                position.y += penetrationY;
                velocity.y = 0;
                collidedBottom = true;
                acceleration.x = 0;
                acceleration.y= 0;
                
                //if (position.x > -1  and position.x < 1){
                 //   collidedPlatform = true;
               // }
                
            }
        }
    }
}

void Entity::CheckCollisionsX(Entity *objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity *object = &objects[i];

        if (CheckCollision(object))
        {
            float xdist = fabs(position.x - object->position.x);
            float penetrationX = fabs(xdist - (width / 2.0f) - (object->width / 2.0f));
            if (velocity.x > 0) {
                position.x -= penetrationX;
                velocity.x = 0;
                collidedRight = true;
                acceleration.x = 0;
                acceleration.y= 0;
            }
            else if (velocity.x < 0) {
                position.x += penetrationX;
                velocity.x = 0;
                collidedLeft = true;
                acceleration.x = 0;
                acceleration.y= 0;
            }
        }
    }
}

void Entity::AIWalker(Entity *player){
    if (player->position.x > position.x)
    {movement = glm::vec3(1, 0, 0);}
    else {movement = glm::vec3(-1, 0, 0);}
}
void Entity::AIPatrol(Entity *player){
    movement = glm::vec3(1, 0, 0);
    if (position.x < -4.0){
        movement = glm::vec3(1, 0, 0);
    }
    else if (position.x > 1.0){
        movement = glm::vec3(-1, 0, 0);
    }
}
void Entity::AIWaitandgo(Entity *player){
    switch(aiState){
        case IDLE:
            if (glm::distance(position, player->position) < 4.0f){
                aiState  = FLYING;
            }
        case FLYING:
            movement = glm::vec3(0, 1, 0);
            if (player->position.y < -3.0){
                movement = glm::vec3(0, -1, 0);
            }
            else if (player->position.y > 0.0){
                movement = glm::vec3(0, 1, 0);
            }
        case ATTACKING:
            break;
    }
}
void Entity::AI(Entity *player){
    switch(aiType){
        case WALKER:
            AIWalker(player);
            break;
        case WAITANDGO:
            AIWaitandgo(player);
            break;
        case PATROL:
            AIPatrol(player);
            break;
    }
}

void Entity::Update(float deltaTime, Entity *player, Entity *platforms, int platformCount, Entity *obstacles, int obstacleCount){
    
    if (entityType == ENEMY){
        AI(player);
    }
    
    collidedBottom = false;
    collidedLeft = false;
    collidedRight = false;
    //collidedPlatform = false;
    
    velocity.x = movement.x * speed;
    velocity += acceleration * deltaTime;
    position += velocity * deltaTime;
    
    
    position.y += velocity.y * deltaTime; // Move on Y
    CheckCollisionsY(platforms, platformCount);
  //  if (platforms->collidedBottom == true){
  //      collidedPlatform = true;
 //   }
    
    CheckCollisionsY(obstacles, obstacleCount);// Fix if needed
    

    position.x += velocity.x * deltaTime; // Move on X
    CheckCollisionsX(platforms, platformCount);
    CheckCollisionsX(obstacles, obstacleCount);// Fix if needed

    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    }

void Entity::Render(ShaderProgram *program) {
     program->SetModelMatrix(modelMatrix);

     float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
     float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

     glBindTexture(GL_TEXTURE_2D, textureID);

     glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
     glEnableVertexAttribArray(program->positionAttribute);

     glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
     glEnableVertexAttribArray(program->texCoordAttribute);

     glDrawArrays(GL_TRIANGLES, 0, 6);

     glDisableVertexAttribArray(program->positionAttribute);
     glDisableVertexAttribArray(program->texCoordAttribute);
}
