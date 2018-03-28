#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

#include <vector>
#include "Matrix.h"
#include "ShaderProgram.h"
#include "FlareMap.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
using namespace std;

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

// 60 FPS (1.0f/60.0f) (update sixty times a second)
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6
#define spriteCountX 16
#define spriteCountY 8
#define TILE_SIZE 0.5f
FlareMap map;

SDL_Window* displayWindow;

enum GameMode { STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_GAME_OVER, STATE_WIN};


GLuint LoadTexture(const char* filepath){
    int w,h,comp;
    unsigned char* image = stbi_load(filepath, &w, &h, &comp, STBI_rgb_alpha);
    
    if(image == NULL){
        std::cout << "Unable to load image. Make sure the path is corret\n";
        assert(false);
    }
    
    GLuint retTexture;
    glGenTextures(1, &retTexture);
    glBindTexture(GL_TEXTURE_2D, retTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    stbi_image_free(image);
    return retTexture;
}

class SheetSprite{
public:
    SheetSprite(){};
    SheetSprite(unsigned int textureID, float u, float v, float width, float height, float
                size):textureID(textureID), u(u), v(v), width(width), height(height), size(size){ }
    
    SheetSprite(unsigned int textureID, float x, float y, float spritewidth, float spriteheight, float size, float sheetwidth, float sheetheight) :
    textureID(textureID),
    size(size)
    {
        u = x / sheetwidth;
        v = y / sheetheight;
        width = spritewidth / sheetwidth;
        height = spriteheight / sheetheight;
    }
    
    
    void Draw(ShaderProgram *program) const {
        
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        GLfloat texCoords[] = {
            u, v+height,
            u+width, v,
            u, v,
            u+width, v,
            u, v+height,
            u+width, v+height
        };
        
        
        float vertices[] = {
            -0.5f , -0.5f,
            0.5f, 0.5f ,
            -0.5f , 0.5f ,
            0.5f, 0.5f ,
            -0.5f, -0.5f  ,
            0.5f , -0.5f };
        
        glUseProgram(program->programID);
        
        glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program->positionAttribute);
        glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program->texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program->positionAttribute);
        glDisableVertexAttribArray(program->texCoordAttribute);
    }
    
    
    
    unsigned int textureID;
    float u;
    float v;
    float width;
    float height;
    float size;
};


class Vector3{
public:
    Vector3() : x(0.0f), y(0.0f), z(0.0f) {};
    Vector3(float x, float y, float z):x(x), y(y), z(z){};
    
    Vector3(float x, float y): x(x), y(y), z(0.0f) {};
    
    float x;
    float y;
    float z;
};

float lerp(float v0, float v1, float t) {
    return (1.0-t)*v0 + t*v1;
}

enum EntityType {ENTITY_PLAYER, ENTITY_ENEMY, ENTITY_COIN, ENTITY_STATIC};

void DrawSprite(ShaderProgram* program, int index){
    
    float u = (float)(((int)index) % spriteCountX) / (float) spriteCountX;
    float v = (float)(((int)index) / spriteCountX) / (float) spriteCountY;
    float spriteWidth = 1.0f / (float)spriteCountX;
    float spriteHeight = 1.0f / (float)spriteCountY;
    
    GLfloat texCoords[] = {
        u, v + spriteHeight,
        u + spriteWidth, v,
        u, v,
        u + spriteWidth, v,
        u, v + spriteHeight,
        u + spriteWidth, v + spriteHeight
    };
    
    float vertices[] = { -TILE_SIZE, -TILE_SIZE,
        TILE_SIZE,  TILE_SIZE,
        -TILE_SIZE,  TILE_SIZE,
        
        TILE_SIZE, TILE_SIZE,
        -TILE_SIZE, -TILE_SIZE,
        TILE_SIZE, -TILE_SIZE
    };
    
    glUseProgram(program->programID);
    
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);
    
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
    
}

class Entity{
public:
    
    SheetSprite sprite;
    
    Vector3 position;
    Vector3 size;
    Vector3 velocity;
    Vector3 acceleration;
    
    //    bool isStatic;
    
    EntityType entityType;
    
    bool collideTop;
    bool collideBottom;
    bool collideLeft;
    bool collideRight;
    int spriteint;
    float x;
    float y;
    float width;
    float height;
    
    Entity(const SheetSprite& sprite, float positionX, float positionY, float sizeX, float sizeY, float velocityX, float velocityY, float accelerationX, float accelerationY, EntityType entityType):sprite(sprite), position(positionX, positionY, 0.0f), size(sizeX*sprite.size*sprite.width/sprite.height, sizeY*sprite.size, 0.0f), velocity(velocityX, velocityY, 0.0f), acceleration(accelerationX, accelerationY, 0.0f), entityType(entityType){
    };
    
    Entity(float x, float y, float width, float height, int sprite, EntityType entityType)
    : position(x, y), size(width, height), spriteint(sprite), entityType(entityType) {};
    
    void Draw(ShaderProgram* program) {
        Matrix modelMatrix;
        modelMatrix.Translate(position.x, position.y, 0.0);
        program->SetModelMatrix(modelMatrix);
        DrawSprite(program, spriteint);
    }
    
    
    void UpdateX(float elapsed){
        
        if(entityType == ENTITY_PLAYER){
            const Uint8 *keys = SDL_GetKeyboardState(NULL);
            
            if(keys[SDL_SCANCODE_RIGHT]){
                acceleration.x = 2.5f;
            }else if(keys[SDL_SCANCODE_LEFT]){
                acceleration.x = -2.5f;
            }else{
                acceleration.x = 0.0f;
            }
            
            velocity.x = lerp(velocity.x, 0.0f, elapsed * 1.5f);
            velocity.x += acceleration.x * elapsed;
            position.x += velocity.x * elapsed;
            
        }
    }
    
    void UpdateY(float elapsed){
        if(entityType == ENTITY_PLAYER){
            acceleration.y = -2.0f;
            
            velocity.y += acceleration.y * elapsed;
            position.y += velocity.y * elapsed;
            
            if(position.y <= -2.0f+size.y*0.5){
                position.y = -2.0f+size.y*0.5;
                collideBottom = true;
            }
            
        }
    }
    
    void Render(ShaderProgram* program, Entity* player){
        
        Matrix projectionMatrix;
        projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
        Matrix modelMatrix;
        modelMatrix.Translate(position.x, position.y, position.z);
        modelMatrix.Scale(size.x, size.y, size.z);
        
        Matrix viewMatrix;
        
        viewMatrix.Translate(-1.0f*player->position.x, -1.0f*player->position.y, 0.0f);
        
        glUseProgram(program->programID);
        
        program->SetProjectionMatrix(projectionMatrix);
        program->SetModelMatrix(modelMatrix);
        program->SetViewMatrix(viewMatrix);
        
        sprite.Draw(program);
    }
    
    bool CollidesWithX(Entity* entity){
        
        if(position.x+size.x*0.5 < entity->position.x-entity->size.x*0.5 || position.x-size.x*0.5 > entity->position.x+entity->size.x*0.5|| position.y+size.y*0.5 < entity->position.y-entity->size.y*0.5 || position.y-size.y*0.5 > entity->position.y+entity->size.y*0.5){
            return false;
        }else{
            if(entity->entityType == ENTITY_COIN){
                entity->position.x = -2000.0f;
            }else if(entity->entityType == ENTITY_STATIC){
                double Xpenetration = 0.0f;
                
                Xpenetration = fabs(fabs(position.x-entity->position.x) - size.x*0.5 - entity->size.x*0.5);
                
                if(position.x>entity->position.x){
                    position.x = position.x + Xpenetration + 0.00001f;
                    collideLeft = true;
                }else{
                    position.x = position.x - Xpenetration - 0.000001f;
                    collideRight = true;
                }
                
                velocity.x = 0.0f;
            }
            return true;
        }
    }
    
    bool CollidesWithY(Entity* entity){
        
        if(position.x+size.x*0.5 < entity->position.x-entity->size.x*0.5 || position.x-size.x*0.5 > entity->position.x+entity->size.x*0.5|| position.y+size.y*0.5 < entity->position.y-entity->size.y*0.5 || position.y-size.y*0.5 > entity->position.y+entity->size.y*0.5){
            return false;
        }else{
            if(entity->entityType == ENTITY_COIN){
                entity->position.x = -2000.0f;
            }else if(entity->entityType == ENTITY_STATIC){
                double Ypenetration = 0.0f;
                
                
                Ypenetration = fabs(fabs(position.y-entity->position.y) - size.y*0.5 - entity->size.y*0.5);
                
                if(position.y>entity->position.y){
                    position.y = position.y + Ypenetration + 0.00001f;
                    collideBottom = true;
                }else{
                    position.y = position.y - Ypenetration - 0.00001f;
                    collideTop = true;
                }
                
                velocity.y = 0.0f;
                
            }
            return true;
        }
    }
    
    
};

void worldToTileCoordinates(float worldX, float worldY, int * gridX, int *gridY) {
    *gridX = (int)(worldX / TILE_SIZE);
    *gridY = (int)(-worldY / TILE_SIZE);
}

/*
 void parseMap(string levelFile) {
 map.Load(levelFile);
 for (int i = 0; i < map.entities.size(); i++) {
 PlaceEntity(map.entities[i].type, map.entities[i].x * TILE_SIZE, map.entities[i].y * -TILE_SIZE);
 }
 
 }
 vector<float> vertexData;
 vector<float> texCoordData;
 void drawMap() {
 parseMap("TileMap.txt");
 
 for (int y = 0; y < map.mapHeight; y++) {
 for (int x = 0; x < map.mapWidth; x++) {
 if (map.mapData[y][x] != 0) {
 float u = (float)(((int)map.mapData[y][x]) % spriteCountX) / (float)spriteCountX;
 float v = (float)(((int)map.mapData[y][x]) / spriteCountX) / (float)spriteCountY;
 
 float spriteWidth = 1.0f / (float)spriteCountX;
 float spriteHeight = 1.0f / (float)spriteCountY;
 
 vertexData.insert(vertexData.end(), {
 TILE_SIZE * x, -TILE_SIZE * y,
 TILE_SIZE * x, (-TILE_SIZE * y) - TILE_SIZE,
 (TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
 TILE_SIZE * x, -TILE_SIZE * y,
 (TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
 (TILE_SIZE * x) + TILE_SIZE, -TILE_SIZE * y
 });
 
 texCoordData.insert(texCoordData.end(), {
 u, v,
 u, v + (spriteHeight),
 u + spriteWidth, v + (spriteHeight),
 u, v,
 u + spriteWidth, v + (spriteHeight),
 u + spriteWidth, v
 });
 
 }
 }
 }
 
 }
 */
void setup(ShaderProgram* program){
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 1280, 720);
    //    glClearColor(254.0f/255.0f, 223.0f/255.0f, 225.0f/255.0f, 1.0f);
    
    program->Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void processGameInput(SDL_Event* event, bool& done, Entity* player){
    while (SDL_PollEvent(event)) {
        if (event->type == SDL_QUIT || event->type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
        }else if(event->type == SDL_KEYDOWN){
            if(event->key.keysym.scancode == SDL_SCANCODE_SPACE && player->collideBottom == true){
                player->velocity.y = 2.0f;
            }
        }
    }
}

void updateGame(float elapsed, Entity* player, std::vector<Entity*> woods, Entity* coin ){
    player->collideBottom = false;
    player->collideTop = false;
    player->collideRight = false;
    player->collideLeft = false;
    
    player->UpdateY(elapsed);
    for (Entity* woodPtr : woods){
        player->CollidesWithY(woodPtr);
    }
    player->CollidesWithY(coin);
    
    
    player->UpdateX(elapsed);
    for (Entity* woodPtr : woods){
        player->CollidesWithX(woodPtr);
    }
    player->CollidesWithX(coin);
    
}

void renderGame(ShaderProgram* program, Entity* player, std::vector<Entity*> woods, Entity* coin){
    player->Render(program, player);
    for (Entity* woodPtr : woods){
        woodPtr->Render(program, player);
    }
    
    coin->Render(program, player);
}
/*
 void renderMap(ShaderProgram *program, int mapSheet) {
 Matrix modelMatrix;
 program->SetModelMatrix(modelMatrix);
 
 glBindTexture(GL_TEXTURE_2D, mapSheet);
 glUseProgram(program->programID);
 
 glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
 glEnableVertexAttribArray(program->positionAttribute);
 
 glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
 glEnableVertexAttribArray(program->texCoordAttribute);
 
 glDrawArrays(GL_TRIANGLES, 0, vertexData.size() / 2);
 
 
 glDisableVertexAttribArray(program->positionAttribute);
 glDisableVertexAttribArray(program->texCoordAttribute);
 
 }
 */

int main(int argc, char *argv[])
{
    ShaderProgram program;
    float lastFrameTicks = 0.0f;
    float accumulator = 0.0f;
    
    setup(&program);
    
    GLuint itemSpriteSheet = LoadTexture(RESOURCE_FOLDER"items_spritesheet.png");
    GLuint woodSpriteSheet = LoadTexture(RESOURCE_FOLDER"spritesheet_wood.png");
    GLuint newSpriteSheet = LoadTexture(RESOURCE_FOLDER"arne_sprites.png");
    GLuint playertextureID = LoadTexture(RESOURCE_FOLDER"aliens.png");
    
    SheetSprite itemSheet = SheetSprite(itemSpriteSheet, 288.0f/1024.0f, 432.0f/1024.0f, 70.0f/1024.0f, 70.0f/1024.0f, 0.2);
    SheetSprite woodSheet = SheetSprite(woodSpriteSheet, 0.0f/1024.0f, 630.0f/1024.0f, 220.0f/1024.0f, 140.0f/1024.0f, 0.2);
    SheetSprite playersprite(playertextureID, 67, 196, 66, 92, 0.75, 512, 512);
    
    Entity player(playersprite, -3.35f, -1.0f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, -2.0f,ENTITY_PLAYER);
    player.Draw(&program);
    //Entity player(playerSheet, -3.35f, -1.0f, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, -2.0f, ENTITY_PLAYER);
    Entity coin(itemSheet, 2.5f, 1.5f, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_COIN);
    
    float posX = -1.5f;
    float posY = -1.8f;
    
    std::vector<Entity*> woods;
    
    for (size_t i=0; i<5; i++){
        Entity* newWoodPtr = new Entity(woodSheet, posX, posY, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC);
        woods.push_back(newWoodPtr);
        posX += 0.8f;
        posY += 0.7f;
    }
    
    SDL_Event event;
    bool done = false;
    while (!done) {
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        elapsed += accumulator;
        if(elapsed < FIXED_TIMESTEP) {
            accumulator = elapsed;
            continue; }
        
        processGameInput(&event, done, &player);
        
        glClear(GL_COLOR_BUFFER_BIT);
        
        while(elapsed >= FIXED_TIMESTEP) {
            updateGame(FIXED_TIMESTEP, &player, woods, &coin);
            elapsed -= FIXED_TIMESTEP;
        }
        
        accumulator = elapsed;
        renderGame(&program, &player, woods, &coin);
        
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}

