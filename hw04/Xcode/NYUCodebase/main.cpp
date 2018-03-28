#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
using namespace std;

#include "Matrix.h"
#include "ShaderProgram.h"
#include "FlareMap.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif



SDL_Window* displayWindow;
const Uint8 *keys = SDL_GetKeyboardState(NULL);
enum GameMode {STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_GAME_OVER, STATE_WIN};
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6
#define TILE_SIZE 0.5f
#define LEVEL_WIDTH 128
#define LEVEL_HEIGHT 32
#define spriteCountX 16
#define spriteCountY 8
FlareMap map;
float gravity = 3.0f;
GLuint tiles;

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
    unsigned int textureID;
    float u;
    float v;
    float width;
    float height;
    float size;
    int id;
    
    SheetSprite();
    SheetSprite(unsigned int textureID, int id, float size) : textureID(textureID), id(id), size(size) {
        u = (float)(id % 30) / (float)30;
        v = (float)(id / 30) / (float)30;
        width = 1.0f / 30.0f;
        height = 1.0f / 30.0f;
    }
    void Draw(ShaderProgram *program, float x, float y) {
        glBindTexture(GL_TEXTURE_2D, textureID);
        float vertices[] = {
            size * x, size * y,
            size * x, (size * y) - size,
            (size * x) + size, (size * y) - size,
            size * x, size * y,
            (size * x) + size, (size * y) - size,
            (size * x) + size, size * y
        };
        GLfloat texCoords[] = {
            u, v,
            u, v + (height),
            u + width, v + (height),
            u, v,
            u + width, v + (height),
            u + width, v
        };
        glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program->positionAttribute);
        glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program->texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program->positionAttribute);
        glDisableVertexAttribArray(program->texCoordAttribute);
    }
};


void DrawText(ShaderProgram *program, GLuint fontTexture, std::string text, float size, float spacing) {
    
    float texture_size=1.0/16.0f;
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    
    for (int i=0;i<text.size();i++){
        int spriteIndex = (int)text[i];
        float texture_x=(float)(spriteIndex%16)/16.0f;
        float texture_y=(float)(spriteIndex/16)/16.0f;
        
        vertexData.insert(vertexData.end(), {
            ((size+spacing)*i)+(-0.5f*size),0.5f*size,
            ((size+spacing)*i)+(-0.5f*size),-0.5f*size,
            ((size+spacing)*i)+(0.5f*size),0.5f*size,
            ((size+spacing)*i)+(0.5f*size),-0.5f*size,
            ((size+spacing)*i)+(0.5f*size),0.5f*size,
            ((size+spacing)*i)+(-0.5f*size),-0.5f*size,
        });
        texCoordData.insert(texCoordData.end(), {
            texture_x,texture_y,
            texture_x,texture_y+texture_size,
            texture_x+texture_size,texture_y,
            texture_x+texture_size,texture_y+texture_size,
            texture_x+texture_size,texture_y,
            texture_x,texture_y+texture_size,
        });
    }
    
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    
    
    for (int i=0;i<text.size()*10;i+=12)
    {
        float texCoord[]={
            texCoordData[i],texCoordData[i+1],texCoordData[i+2],
            texCoordData[i+3],texCoordData[i+4],texCoordData[i+5],
            texCoordData[i+6],texCoordData[i+7],texCoordData[i+8]
        };
        glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoord);
        glEnableVertexAttribArray(program->texCoordAttribute);
        glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
        glEnableVertexAttribArray(program->positionAttribute);
        glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
        glEnableVertexAttribArray(program->texCoordAttribute);
        glBindTexture(GL_TEXTURE_2D, fontTexture);
        glDrawArrays(GL_TRIANGLES, 0 , text.size()*6);
        glDisableVertexAttribArray(program->positionAttribute);
        glDisableVertexAttribArray(program->texCoordAttribute);
        
    }
    
}

class Vector3{
public:
    Vector3(float x=0.0f, float y=0.0f, float z = 0.0f):x(x), y(y), z(z){};
    float x;
    float y;
    float z;
};



enum EntityType {ENTITY_PLAYER, ENTITY_ENEMY, ENTITY_STATIC};

class Entity{
public:
    
    Vector3 position;
    Vector3 size;
    Vector3 acceleration;
    Vector3 velocity;
    EntityType entityType;
    bool colLeft;
    bool colRight;
    bool colTop;
    bool colBot;
    bool isStatic;
    bool isJump;
    float limit = 0.0f;
    SheetSprite sprite;
    
    Entity() {};
    
    Entity(float x, float y, float width, float height, SheetSprite sprite, EntityType type, bool isStatic) : position(x,y), size(width, height), sprite(sprite), entityType(type), isStatic(isStatic) {}
    

    float lerp(float v0, float v1, float t) {
        return (1.0-t)*v0 + t*v1;
    }
    
    void Update(float elapsed, float grav) {
        if (!isStatic){
            if (isJump) {
                colBot = false;
                velocity.y = 3.0f;
            }
            velocity.x = lerp(velocity.x, 0.0f, elapsed);
            velocity.y = lerp(velocity.y, 0.0f, elapsed);
            velocity.x += acceleration.x * elapsed;
            velocity.y += acceleration.y * elapsed;
            if (colBot) {
                isJump = false;
                velocity.y = 0.0f;
            }
            position.x += velocity.x * elapsed;
            if (colTop || (isJump && position.y >= limit)){
                isJump = false;
                velocity.y *= -1;
            }
            if (colLeft || colRight){
                velocity.x = 0.0f;
            }
            position.y += velocity.y * elapsed;
        }
    }
    
    void jump() {
        if (colBot){
            isJump = true;
            limit = position.y + velocity.y * 1.5f;
        }
    }
    
    bool collidesWith(const Entity& ent) {
        return !(position.y - size.y / 2 > ent.position.y + ent.size.y / 2 ||
                 position.y + size.y / 2 < ent.position.y - ent.size.y / 2 ||
                 position.x - size.x / 2 > ent.position.x + ent.size.x / 2 ||
                 position.x + size.x / 2 < ent.position.x - ent.size.x / 2);
    }
    
    void Draw(ShaderProgram& program) {
        if (entityType == ENTITY_PLAYER) {
            Matrix modelMatrix;
            Matrix viewMatrix;
            viewMatrix.Translate(position.x, position.y, 0.0);
            modelMatrix.Translate(position.x, position.y, 0.0f);
            program.SetModelMatrix(modelMatrix);
            program.SetViewMatrix(viewMatrix);
        }
        sprite.Draw(&program, position.x, position.y);
    }
    
    bool CollisionX(Entity* entity) {
        if (position.x + size.x * 0.5 < (entity->position.x - entity->size.x * 0.5) || position.x - size.x * 0.5 > entity->position.x + entity->size.x * 0.5 || position.y + size.y * 0.5 < entity->position.y - entity->size.y * 0.5 || position.y - entity->size.y * 0.5 > entity->position.y + entity->size.y * 0.5) {
            return false;
        }
        else {
            if (entity->entityType == ENTITY_ENEMY) {
                entity->position.x = -2000.f;
            } else if (entity->entityType == ENTITY_STATIC) {
                double Xpen = 0.0f;
                Xpen = fabs(fabs(position.x-entity->position.x) - size.x * 0.5 - entity->size.x * 0.5);
                
                if (position.x > entity->position.x){
                    position.x = position.x + Xpen + 0.00001f;
                    colLeft = true;
                }
                else {
                    position.x = position.x - Xpen - 0.00001f;
                    colRight = true;
                }
                
                velocity.x = 0.0f;
            }
            return true;
        }
    }
    
    bool CollisionY(Entity* entity) {
        if (position.x + size.x * 0.5 < (entity->position.x - entity->size.x * 0.5) || position.x - size.x * 0.5 > entity->position.x + entity->size.x * 0.5 || position.y + size.y * 0.5 < entity->position.y - entity->size.y * 0.5 || position.y - entity->size.y * 0.5 > entity->position.y + entity->size.y * 0.5) {
            return false;
        }
        else {
            if (entity->entityType == ENTITY_ENEMY) {
                entity->position.x = -2000.f;
            } else if (entity->entityType == ENTITY_STATIC) {
                double Ypen = 0.0f;
                Ypen = fabs(fabs(position.y-entity->position.y) - size.y * 0.5 - entity->size.y * 0.5);
                
                if (position.y > entity->position.y){
                    position.y = position.y + Ypen + 0.00001f;
                    colBot = true;
                }
                else {
                    position.y = position.y - Ypen - 0.00001f;
                    colTop = true;
                }
                
                velocity.y = 0.0f;
            }
            return true;
        }
    }
    
    void updateX(float elapsed) {
        if (entityType == ENTITY_PLAYER) {
            const Uint8 *keys = SDL_GetKeyboardState(NULL);
            
            if (keys[SDL_SCANCODE_LEFT]) {
                acceleration.x = -2.5f;
            }
            else if (keys[SDL_SCANCODE_RIGHT]) {
                acceleration.x = 2.5f;
            }
            else {
                acceleration.x = 0.0f;
            }
            
            velocity.x = lerp(velocity.x, 0.0f, elapsed * 1.5f);
            velocity.x += acceleration.x * elapsed;
            position.x += velocity.x * elapsed;
        }
        
    }
    
    void updateY(float elapsed) {
        if (entityType == ENTITY_PLAYER) {
            acceleration.y = -2.0f;
            velocity.y += acceleration.y * elapsed;
            position.y += velocity.y * elapsed;
            
            if (position.y <= -2.0f + size.y * 0.5){
                position.y = -2.0f + size.y * 0.5;
                colBot = true;
            }
        }
    }
    /*
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
        
        sprites.Draw(program);
    }
     */
};

Entity player;
Entity enemy;


void worldToTileCoords(float worldX, float worldY, int* gridX, int* gridY) {
    *gridX = (int)(worldX / TILE_SIZE * 0.5);
    *gridY = (int)(-worldY / TILE_SIZE * 0.5);
}

    
void setup(ShaderProgram* program){
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Space Invaders", SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
        glewInit();
#endif
    glViewport(0, 0, 1280, 720);
    //glClearColor(0.00f/255.0f, 0.0f/255.0f, 0.0f/255.0f, 1.0f);
    program->Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.455f, 0.0f, 0.416f, 1.0f);

}
    
void ProcessGameInput(SDL_Event* event, bool& done){
    while (SDL_PollEvent(event)){
        if(event->type == SDL_QUIT || event->type == SDL_WINDOWEVENT_CLOSE){
            done = true;
        }
        /*
        else if (event->type == SDL_KEYDOWN){
            if(event->key.keysym.scancode == SDL_SCANCODE_UP && player.colBot){
                player.velocity.y = 2.0f;
            }
         
        }
         */
    }
}


void updateGame(float elapsed) {
    if (keys[SDL_SCANCODE_RIGHT] && !(player.colRight)){
        player.velocity.x = 3.0f;
    }
    else if (keys[SDL_SCANCODE_LEFT] && !(player.colLeft)){
        player.velocity.x = -3.0f;
    }
    
    player.colRight = false;
    player.colTop = false;
    player.colLeft = false;
    player.colBot = false;
}

Matrix projectionMatrix, modelMatrix, viewMatrix;

void renderLevel(ShaderProgram* program) {
    modelMatrix.Identity();
    program->SetModelMatrix(modelMatrix);
    program->SetProjectionMatrix(projectionMatrix);
    program->SetViewMatrix(viewMatrix);
    
    glBindTexture(GL_TEXTURE_2D, tiles);
    std::vector<float> vertexData;
    std::vector<float> texCoorData;
    for (int y = 0; y < map.mapHeight; y++) {
        for (int x = 0; x < map.mapWidth; x++) {
            if (map.mapData[y][x] != 0) {
                float u = (float)((((int)map.mapData[y][x])) % spriteCountX) / (float)spriteCountX;
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
                texCoorData.insert(texCoorData.end(), {
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
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoorData.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    
    glBindTexture(GL_TEXTURE_2D, tiles);
    glDrawArrays(GL_TRIANGLES, 0, vertexData.size()/2);
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}




int main(int argc, char *argv[]) {

    ShaderProgram program;
    float lastFrameTicks = 0.0f;
    float accumulator = 0.0f;
    setup(&program);
    float lfticks;
    bool done = false;
    SDL_Event event;
    
    tiles = LoadTexture("arne_sprites.png");
    projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
    glUseProgram(program.programID);
    
    map.Load("TileMap.txt");
    SheetSprite playerSprite;
    SheetSprite enemySprite;
    
    playerSprite = SheetSprite(tiles, 79, TILE_SIZE);
    player = Entity(13.0f, -14.0f, 0.5f, 0.5f, playerSprite, ENTITY_PLAYER, false);
    enemySprite = SheetSprite(tiles, 445, TILE_SIZE);
    enemy = Entity(28.0f, -8.0f, 0.5f, 0.5f, enemySprite, ENTITY_ENEMY, false);
    while (!done) {
        float ticks = (float)SDL_GetTicks() /1000.f;
        float elapsed = ticks - lfticks;
        lfticks = ticks;
        ProcessGameInput(&event, done);
        updateGame(elapsed);
        glClear(GL_COLOR_BUFFER_BIT);
        modelMatrix.Identity();
        program.SetModelMatrix(modelMatrix);
        program.SetViewMatrix(viewMatrix);
        
        renderLevel(&program);
        viewMatrix.Identity();
        player.Draw(program);
        viewMatrix.Identity();
        enemy.Draw(program);
        viewMatrix.Translate(-player.position.x * 0.5f, -player.position.y * 0.5f, 0.0f);
        program.SetViewMatrix(viewMatrix);
        
        SDL_GL_SwapWindow(displayWindow);
    }
    
    
    SDL_Quit();
    return 0;
}

