#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

#include <vector>
#include "Matrix.h"
#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif



SDL_Window* displayWindow;
enum GameMode {STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_GAME_OVER, STATE_WIN};
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6


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
    
        SheetSprite();
        SheetSprite(unsigned int textureID, float u, float v, float width, float height, float size) : textureID(textureID), u(u), v(v), width(width), height(height), size(size){}
        
        void Draw(ShaderProgram *program) const;
       
};

void SheetSprite::Draw(ShaderProgram *program) const {
            glBindTexture(GL_TEXTURE_2D, textureID);
            
            GLfloat texCoords[] = {
                    u, v+height,
                    u+width, v,
                    u, v,
                    u+width, v,
                    u, v+height,
                    u+width, v+height
                };
            
            
            float aspect = width / height;
            float vertices[] = {
                    -0.5f * size * aspect, -0.5f * size,
                    0.5f * size * aspect, 0.5f * size,
                    -0.5f * size * aspect, 0.5f * size,
                    0.5f * size * aspect, 0.5f * size,
                    -0.5f * size * aspect, -0.5f * size ,
                    0.5f * size * aspect, -0.5f * size};
            
            glUseProgram(program->programID);
            
            glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
            glEnableVertexAttribArray(program->positionAttribute);
            glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
            glEnableVertexAttribArray(program->texCoordAttribute);
            
            glDrawArrays(GL_TRIANGLES, 0, 6);
            
            glDisableVertexAttribArray(program->positionAttribute);
            glDisableVertexAttribArray(program->texCoordAttribute);
        }


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
        Vector3(float x, float y, float z):x(x), y(y), z(z){};
        
        float x;
        float y;
        float z;
};

float lerp(float v0, float v1, float t) {  //CHANGE THISS
    return (1.0-t)*v0 + t*v1;
}

enum EntityType {ENTITY_PLAYER, ENTITY_ENEMY, ENTITY_COIN, ENTITY_STATIC};

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
    SheetSprite sprite;
    
    Entity(const SheetSprite& sprite, float positionX, float positionY, float sizeX, float sizeY, float velocityX, float velocityY, float accelerationX, float accelerationY, EntityType entityType):sprite(sprite), position(positionX, positionY, 0.0f), size(sizeX*sprite.size*sprite.width/sprite.height, sizeY*sprite.size, 0.0f), velocity(velocityX, velocityY, 0.0f), acceleration(accelerationX, accelerationY, 0.0f), entityType(entityType){};
    
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
};
    
    
void setup(ShaderProgram* program){
        SDL_Init(SDL_INIT_VIDEO);
        displayWindow = SDL_CreateWindow("Space Invaders", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
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
}
    
void ProcessGameInput(SDL_Event* event, bool& done, std::vector<Entity>& bullets, Entity& player){
        while (SDL_PollEvent(event)){
                if(event->type == SDL_QUIT || event->type == SDL_WINDOWEVENT_CLOSE){
                        done = true;
                    }
        else if (event->type == SDL_KEYDOWN){
            if(event->key.keysym.scancode == SDL_SCANCODE_UP && player.colBot){
                player.velocity.y = 2.0f;
            }
        }
    }
}

void renderGame(ShaderProgram* program, Entity* player, Entity* enemy) {
    player->Render(program, player);
    enemy->Render(program, player);
}

void updateGame(float elapsed, Entity* player, Entity* enemy) {
    player->colRight = false;
    player->colLeft = false;
    player->colTop = false;
    player->colBot = false;
    
    player->updateY(elapsed);
    player->CollisionY(enemy);
    
    player->updateX(elapsed);
    player->CollisionX(enemy);
}



int main(int argc, char *argv[])
{
        ShaderProgram program;
        setup(&program);
        
        GLuint fontTexture = LoadTexture(RESOURCE_FOLDER"font1.png");
        GLuint spriteSheetTexture = LoadTexture(RESOURCE_FOLDER"sheet.png");
    
    
        SheetSprite EnemySprite = SheetSprite(spriteSheetTexture, 425.0f/1024.0f, 468.0f/1024.0f, 93.0f/1024.0f, 84.0f/1024.0f, 0.2);
        SheetSprite playerSprite = SheetSprite(spriteSheetTexture, 247.0f/1024.0f, 84.0f/1024.0f, 99.0f/1024.0f, 75.0f/1024.0f, 0.2);
        SheetSprite BulletSprite = SheetSprite(spriteSheetTexture, 842.0f/1024.0f, 230.0f/1024.0f, 9.0f/1024.0f, 54.0f/1024.0f, 0.2);
    SheetSprite titleSprite = SheetSprite(spriteSheetTexture, 247.0/1024.0f, 84.0f/1024.0f, 99.0f/1024.0f, 75.0f/1024.0f, 0.2);
    SheetSprite EnemySprite2 = SheetSprite(spriteSheetTexture, 230.0f/1024.0f, 530.0f/1024.0f, 100.0f/1024.0f, 140.0f/1024.0f, 0.2);
        
        float lastFrameTicks = 0.0f;
        
        
        SDL_Event event;
        bool done = false;
        
        GameMode mode = STATE_MAIN_MENU;
        Entity font(-1.08f, 0.0f, 0.3f, 0.3f);
        Entity player(01.0f, -1.3f, 1.5f, 1.5f);
    Entity font2(-1.08f, 0.0f, 0.4f, 0.5f);
        Entity font3(-1.08f, 0.0f, 0.5f, 0.3f);
    Entity titleim(-1.0f, -1.3f, 1.5f, 1.5f);
        Entity titleim2(1.0f, -1.3f, 1.5f, 1.5f);
        
        std::vector<Entity> aliens;
        
        float initialX = -3.55f-0.5*0.2*1.5f;
        float initialY = 1.8f;
        
        for(GLsizei j=0; j<5; j++){
                for(GLsizei i=0; i<11; i++){
                        Entity newEnemy(initialX, initialY, 1.5f, 1.5f);
                        initialX+=0.5*1.3f;
                        aliens.push_back(newEnemy);
                    }
                initialX = -3.55f-0.5*0.2*1.5f;
                initialY-=0.4f;
            }
        
        
        std::vector<Entity> bullets;
        for(GLsizei i=0; i<MAX_BULLETS; i++){
                Entity newBullet(-2000.0f, 0.0f, 1.5f, 1.5f);
                bullets.push_back(newBullet);
            }
        
        while (!done) {
                float ticks = (float)SDL_GetTicks()/1000.0f;
                float elapsed = ticks - lastFrameTicks;
                lastFrameTicks = ticks;
                
                if(mode == STATE_MAIN_MENU){
                        ProcessMainMenuInput(&event, done, mode);
                    }else if(mode == STATE_GAME_LEVEL){
                            ProcessGameInput(&event, done, bullets, player);
                        }else if(mode == STATE_GAME_OVER){
                                ProcessGameOverInput(&event, done, mode);
                            }else if(mode == STATE_WIN){
                                    ProcessGameOverInput(&event, done, mode);
                                }
                
                glClear(GL_COLOR_BUFFER_BIT);
                
                
                if(mode == STATE_MAIN_MENU){
                        RenderMainMenu(&program, font, font2, font3, fontTexture, titleSprite, titleim, titleim2);
                    }else if(mode == STATE_GAME_LEVEL){
                            UpdateGame(aliens, player, bullets, elapsed, mode);
                            RenderGame(&program, aliens, playerSprite, EnemySprite, player, bullets, BulletSprite, EnemySprite2);
                        }else if(mode == STATE_GAME_OVER){
                                RenderGameOver(&program, font, font2, fontTexture);
                            }else if(mode == STATE_WIN){
                                    RenderWin(&program, font, font2, fontTexture);
                                }
                
                
                SDL_GL_SwapWindow(displayWindow);
                
            }
        
        SDL_Quit();
        return 0;
}

