#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

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
#define MAX_BULLETS 250
enum GameMode {STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_GAME_OVER, STATE_WIN};
int bulletTotal = 0;
float timer = 0.0f;
#define SPRITE_COUNT_X 14
#define SPRITE_COUNT_Y 7
#define LEVEL_HEIGHT 17
#define LEVEL_WIDTH 22
#define TILE_SIZE .15f

int gameLevel = 0;

unsigned int levelData[LEVEL_HEIGHT][LEVEL_WIDTH] = {
    
    { 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 6, 7, 8, 9, 10, 11},
    { 129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,129,130,131,132,133,134,135 },
    { 190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,190,191,192,193,194,195,196 },
    { 190,191,192,193,194,195,196,197,198,199,200,201,202,190,191,192,193,194,195,196,197,198 },
    { 128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,128,129,130,131,132,133,134 },
    { 129,130,131,132,133,134,135,136,137,138,139,140,141,129,130,131,132,133,134,135,136,137 },
    { 190,191,192,193,194,195,196,197,198,199,200,201,202,190,191,192,193,194,195,196,197,198 },
    { 132,133,134,135,136,137,138,139,140,141,129,130,131,132,133,134,135,136,137,138,139,140 },
    { 193,194,195,196,197,198,199,200,201,202,190,191,192,193,194,195,196,197,198,199,200,201 },
    { 132,133,134,135,136,137,138,139,140,141,129,130,131,132,133,134,135,136,137,138,139,140 },
    { 193,194,195,196,197,198,199,200,201,202,190,191,192,193,194,195,196,197,198,199,200,201 },
    { 129,130,131,132,133,134,135,136,137,138,139,140,141,129,130,131,132,133,134,135,136,137 },
    { 190,191,192,193,194,195,196,197,198,199,200,201,202,190,191,192,193,194,195,196,197,198 },
    { 129,130,131,132,133,134,135,136,137,138,139,140,141,129,130,131,132,133,134,135,136,137 },
    { 129,130,131,132,133,134,135,136,137,138,139,140,141,129,130,131,132,133,134,135,136,137 },
    { 190,191,192,193,194,195,196,197,198,199,200,201,202,190,191,192,193,194,195,196,197,198 },
    { 129,130,131,132,133,134,135,136,137,138,139,140,141,129,130,131,132,133,134,135,136,137 }
    
};


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

bool win = false;
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

void DrawText2(ShaderProgram *program, GLuint fontTexture, std::string text, float size, float spacing) {
    
    
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

class Entity{
public:
        
        Entity(float positionX, float positionY, float sizeX, float sizeY):position(positionX, positionY, 0.0f), size(sizeX, sizeY,0.0f){health = 1; entityid = 1;};
    
        Entity(float positionX, float positionY, float sizeX, float sizeY, int health, int entityid):position(positionX, positionY, 0.0f), size(sizeX, sizeY,0.0f), health(health), entityid(entityid){};
        
        void drawWords(ShaderProgram* program, GLuint fontTexture, const std::string& text){
                Matrix projectionMatrix;
                Matrix modelMatrix;
                projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
                modelMatrix.Translate(position.x, position.y, position.z);
                Matrix viewMatrix;
                
                program->SetProjectionMatrix(projectionMatrix);
                program->SetModelMatrix(modelMatrix);
                program->SetViewMatrix(viewMatrix);
                
                DrawText(program, fontTexture, text, size.x, -size.x/5);
            }
    
    
        void drawInstructions(ShaderProgram* program, GLuint fontTexture, const std::string& text) {
        Matrix projectionMatrix;
                Matrix modelMatrix;
                projectionMatrix.SetOrthoProjection(-4.0f, 8.00f, -0.5f, 8.00f, -1.0f, 2.0f);
                modelMatrix.Translate(position.x, position.y, position.z);
                Matrix viewMatrix;
                
                program->SetProjectionMatrix(projectionMatrix);
                program->SetModelMatrix(modelMatrix);
                program->SetViewMatrix(viewMatrix);
                
                DrawText(program, fontTexture, text, size.x, -size.x/5);
    }
    
    void drawTitle(ShaderProgram* program, GLuint fontTexture, const std::string& text) {
        Matrix projectionMatrix;
                Matrix modelMatrix;
                projectionMatrix.SetOrthoProjection(-5.65f, 8.90f, -4.0f, 2.0f, -2.0f, 1.0f);
                modelMatrix.Translate(position.x, position.y, position.z);
                Matrix viewMatrix;
                
                program->SetProjectionMatrix(projectionMatrix);
                program->SetModelMatrix(modelMatrix);
                program->SetViewMatrix(viewMatrix);
                
                DrawText(program, fontTexture, text, size.x, -size.x/5);
    }
    
    void drawTimg(ShaderProgram* program, const SheetSprite& sprite) const {
                Matrix projectionMatrix;
                projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
                Matrix modelMatrix;
                modelMatrix.Translate(position.x, position.y, position.z);
                modelMatrix.Scale(size.x, size.y, size.z);
                
                Matrix viewMatrix;
                
                glUseProgram(program->programID);
                
                program->SetProjectionMatrix(projectionMatrix);
                program->SetModelMatrix(modelMatrix);
                program->SetViewMatrix(viewMatrix);
                
                sprite.Draw(program);
    }
    void Draw(ShaderProgram* program, const SheetSprite& sprite) const {
        Matrix projectionMatrix;
        projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
        Matrix modelMatrix;
        modelMatrix.Translate(position.x, position.y, position.z);
        modelMatrix.Scale(size.x, size.y, size.z);
        Matrix viewMatrix;
        glUseProgram(program->programID);
        program->SetProjectionMatrix(projectionMatrix);
        program->SetModelMatrix(modelMatrix);
        program->SetViewMatrix(viewMatrix);
        sprite.Draw(program);
        
    }
    
        
     void update(float elapsed){
        position.x += elapsed * 0.4f;
        if(position.x >= 3.55+size.x*0.5*0.2){
            position.x = -3.55f-size.x*0.4*0.2;
        }
        
    }
    
    void update2(float elapsed){ //not used... yet
        position.x -= elapsed * 0.4f;
        if(position.x >= 3.55 + size.x * 0.5 * 0.2){
            position.x = -3.55f - size.x * 0.4 * 0.2;
        }
    }
    
    void bossupdate1(float elapsed) {
        position.x += elapsed * 1.5f;
        position.y += elapsed * 0.5f;
        if(position.x >= 3.55+size.x*0.5*0.2){
            position.x = -3.55f-size.x*0.4*0.2;
        }
    }
        
    void bossupdate2(float elapsed) {
        position.x -= elapsed * 1.5f;
        position.y -= elapsed * 0.5f;
        if(position.x >= 3.55+size.x*0.5*0.2){
            position.x = -3.55f-size.x*0.4*0.2;
        }
    }
    
    void bossupdate3(float elapsed) {
        position.x += elapsed * 1.5f;
        position.y -= elapsed * 0.15f;
        if(position.x >= 3.55+size.x*0.5*0.2){
            position.x = -3.55f-size.x*0.4*0.2;
        }
        if (position.y < -1.0f) {
            position.y += elapsed * 0.3f;
        }
    }
    
    void shootBullets(std::vector<Entity>& bullets){
        bullets[bulletTotal].position.x = position.x;
        bullets[bulletTotal].position.y = position.y - 0.5;
        bullets[bulletTotal].yVelocity = -1.0f;
        bulletTotal++;
        if(bulletTotal > MAX_BULLETS-1) {
            bulletTotal = 0;
            
        }
        
    }
    
    void shootBossBullets(std::vector<Entity>& bullets) {
        bullets[bulletTotal].position.x = position.x - 0.5;
        bullets[bulletTotal].position.y = position.y - 0.3;
        bullets[bulletTotal].yVelocity = -1.0f;
        bulletTotal++;
        if(bulletTotal > MAX_BULLETS-1) {
            bulletTotal = 0;
        }
    }
        
    Vector3 position;
    Vector3 size;
    float yVelocity;
    int health;
    int entityid;
};


void setup(ShaderProgram* program){
        SDL_Init(SDL_INIT_VIDEO);
        displayWindow = SDL_CreateWindow("SNOW Invaders", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
        SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
        SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
        glewInit();
#endif
        
        glViewport(0, 0, 1280, 720);
        glClearColor(0.00f/255.0f, 0.0f/255.0f, 0.0f/255.0f, 1.0f);
        
        program->Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 4096 );
    Mix_Music *music;
    music = Mix_LoadMUS("winter.mp3");
    
    Mix_VolumeMusic(30);
    Mix_PlayMusic(music, -1);
}

std::vector<Entity> aliens;
std::vector<Entity> bullets;

void ProcessGameInput(SDL_Event* event, bool& done, std::vector<Entity>& bullets, const Entity& player, Mix_Chunk *shot){
        while (SDL_PollEvent(event)){
                if(event->type == SDL_QUIT || event->type == SDL_WINDOWEVENT_CLOSE){
                        done = true;
                    }
        else if (event->type == SDL_KEYDOWN){
            if(event->key.keysym.scancode == SDL_SCANCODE_UP){
                
                
                
                bullets[bulletTotal].position.y = player.position.y + 0.1;
                    bullets[bulletTotal].position.x = player.position.x;
                   bullets[bulletTotal].yVelocity = 4.0f;
                    bulletTotal++;
                //Mix_PlayMusic(shot, 1);
                Mix_PlayChannel(1, shot, 0);
                    if(bulletTotal > MAX_BULLETS-1) {
                     bulletTotal = 0;
                }
            }
        }
    }
}


void ProcessMainMenuInput(SDL_Event* event, bool& done, GameMode& gameMode){
        while (SDL_PollEvent(event)) {
                if (event->type == SDL_QUIT || event->type == SDL_WINDOWEVENT_CLOSE) {
                        done = true;
                    }
        else if(event->type == SDL_KEYDOWN){
            if(event->key.keysym.scancode == SDL_SCANCODE_SPACE) {
                gameMode = STATE_GAME_LEVEL;
            }
        }
    }
}

void ProcessGameOverInput(SDL_Event* event, bool& done, GameMode& gameMode){
    while(SDL_PollEvent(event)){
        if(event->type == SDL_QUIT || event->type == SDL_WINDOWEVENT_CLOSE){
            done = true;
        }
        else if(event->type == SDL_KEYDOWN) {
            if(event->key.keysym.scancode == SDL_SCANCODE_SPACE) {
                gameLevel = 0;
                int esize = aliens.size();
                for (int j = 0; j < esize; j++) {
                    aliens.pop_back();
                }
                int size = bullets.size();
                for (int i = 0; i < size; i++) {
                    bullets.pop_back();
                }
                float initialX = -3.55f-0.5*0.2*1.5f+1.0f;
                float initialY = 1.8f;
                for(GLsizei j=0; j<5; j++){
                            for(GLsizei i=0; i<8; i++){
                                    Entity newEnemy(initialX, initialY, 1.5f, 1.5f);
                                    initialX+=0.5*1.3f;
                                    aliens.push_back(newEnemy);
                                }
                            initialX = -3.55f-0.5*0.2*1.5f+1.0f;
                            initialY-=0.4f;
                        }
                    
                    
                    
                    for(GLsizei i=0; i<MAX_BULLETS; i++){
                            Entity newBullet(-2000.0f, 0.0f, 1.5f, 1.5f);
                            bullets.push_back(newBullet);
                        }
                gameMode = STATE_GAME_LEVEL;
                
            }
        }
    }
}
void setupLevel2(){
    int esize = aliens.size();
    for (int j = 0; j < esize; j++) {
        aliens.pop_back();
    }
    int size = bullets.size();
    for (int i = 0; i < size; i++) {
        bullets.pop_back();
    }
    for(GLsizei i=0; i<MAX_BULLETS; i++){
                Entity newBullet(-2000.0f, 0.0f, 1.5f, 1.5f);
                bullets.push_back(newBullet);
        
    }
    
    float initialX = 0.0f;
    float initialY = 1.6f;
    
    for(GLsizei i=0; i<1; i++){
        Entity newEnemy(initialX, initialY, 1.5f, 1.5f, 30, 2);
        aliens.push_back(newEnemy);
    }
    
    
}

void random(std::vector<Entity>& aliens, std::vector<Entity>& bullets) {
    srand (time(NULL));
    for (size_t i = 0; i < aliens.size(); i++){
        int x = rand() % 3 + 1;
        if (x % 2 == 0) {
            aliens[i].shootBullets(bullets);
        }
    }
}
void UpdateGame(std::vector<Entity>& aliens, Entity& player, std::vector<Entity>& bullets, float elapsed, GameMode& gameMode, Mix_Chunk *explosionSound, Mix_Chunk *enemyShot){
    
    timer += elapsed;
        
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
        
    if(keys[SDL_SCANCODE_LEFT]){
        player.position.x -= elapsed * 1.6f;
        if(player.position.x <= -3.55+player.size.x*0.5*0.2){
            player.position.x = -3.55+player.size.x*0.5*0.2;
            
        }
        
    } else if(keys[SDL_SCANCODE_RIGHT]){
        player.position.x += elapsed * 1.6f;
        if(player.position.x >= 3.55-player.size.x*0.5*0.2){
            player.position.x = 3.55-player.size.x*0.5*0.2;
            
        }
        
    }
        
     for (GLsizei i=0; i<bullets.size(); i++){
            bullets[i].position.y += bullets[i].yVelocity*elapsed;
          }
    
    if (gameLevel == 0) {
        if (timer > 1 && timer < 2) {
            for (size_t i = 0; i < aliens.size(); i++){
                aliens[i].update2(elapsed);
            }
        }
        else if (timer < 1) {
            for (size_t i = 0; i < aliens.size(); i++){
                aliens[i].update(elapsed);
            }
        }
    }
    if (gameLevel == 1) {
        if (timer > 0.7 && timer < 1.1) {
            for (size_t i = 0; i < aliens.size(); i++){
                aliens[i].bossupdate1(elapsed);
            }
        }
        else if (timer < 0.7) {
            for (size_t i = 0; i < aliens.size(); i++){
                aliens[i].bossupdate2(elapsed);
            }
        }
        else if (timer > 1.1) {
            for (size_t i = 0; i < aliens.size(); i++){
                aliens[i].bossupdate3(elapsed);
            }
        }
    }
    if (gameLevel == 0 ) {
        
     if(timer >= 2.1f){
        random(aliens, bullets);
        Mix_PlayChannel(-1, enemyShot, 0);
        timer -= 2.1f;
            }
            }
    
    if (gameLevel == 1) {
        if (timer >= 1.3f) {
            aliens[0].shootBullets(bullets);
            aliens[0].shootBossBullets(bullets);
            Mix_PlayChannel(-1, enemyShot, 0);
            timer -= 1.3f;
        }
    }
        
    for(size_t i=0; i<bullets.size(); i++){
                
        Entity bullet = bullets[i];
        for(GLsizei j=0; j<aliens.size(); j++){
            Entity enemy = aliens[j];
            if(bullet.position.x+bullet.size.x*0.2*0.4 < enemy.position.x-enemy.size.x*0.2*0.4 || bullet.position.x-bullet.size.x*0.2*0.4 > enemy.position.x+enemy.size.x*0.2*0.4 || bullet.position.y+bullet.size.y*0.2*0.4 < enemy.position.y-enemy.size.y*0.2*0.4 || bullet.position.y-bullet.size.y*0.2*0.4 > enemy.position.y+enemy.size.y*0.2*0.4){}
            else{
                if(bullets[i].yVelocity >= 0){
                    aliens[j].health -= 1;
                    bullets[i].position.x = -200.0f;
                    if (aliens[j].health == 0) {
                        aliens[j]= aliens.back();
                        aliens.pop_back();
                    }
                    Mix_PlayChannel(-1, explosionSound, 0);
                }
                
            }
            
            if(bullet.position.x+bullet.size.x*0.2*0.4 < player.position.x-player.size.x*0.2*0.4 || bullet.position.x-bullet.size.x*0.2*0.4 > player.position.x+player.size.x*0.2*0.4 || bullet.position.y+bullet.size.y*0.2*0.4 < player.position.y-player.size.y*0.2*0.4 || bullet.position.y-bullet.size.y*0.2*0.4 > player.position.y+player.size.y*0.2*0.4){}
            else{
                if(bullets[i].yVelocity <= 0){ //player hits bullet
                    Mix_PlayChannel(-1, explosionSound, 0);
                    
                    
                    gameMode = STATE_GAME_OVER;
                    
                }
                else if(aliens[0].position.y < -1.35f) {
                    Mix_PlayChannel(-1, explosionSound, 0);
                    gameMode = STATE_GAME_OVER;
                }
                
            }
            
        }
                
    }
        
    win = true;
    if (gameLevel == 0) {
        if (aliens.size() != 0){
            win = false;
        }
        else {
        setupLevel2();
        gameLevel = 1;
        }

    }
    
    if (gameLevel == 1) {
        if (aliens.size() != 0) {
            win = false;
        }
    }
        
    if (win){
        gameMode = STATE_WIN;
    }
        
}


void RenderMap(ShaderProgram &program) {
    
    GLuint mapTexture = LoadTexture("gtiles.png");
    glBindTexture(GL_TEXTURE_2D, mapTexture);
    
    
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    
    int count = 0;
    for (int y = 0; y < LEVEL_HEIGHT; y++) {
        for (int x = 0; x < LEVEL_WIDTH; x++) {
            if (levelData[y][x] != 0) {
                count++;
                float u = (float)(((int)levelData[y][x]) % SPRITE_COUNT_X / (float)SPRITE_COUNT_X);
                float v = (float)(((int)levelData[y][x]) / SPRITE_COUNT_X / (float)SPRITE_COUNT_Y);
                
                float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
                float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;
                
                vertexData.insert(vertexData.end(), {
                    
                    TILE_SIZE * x, -TILE_SIZE * y + .001f,
                    TILE_SIZE * x, (-TILE_SIZE * y) - TILE_SIZE - .001f,
                    (TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE - .001f,
                    TILE_SIZE * x, -TILE_SIZE * y + .001f,
                    (TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE - .001f,
                    (TILE_SIZE * x) + TILE_SIZE, -TILE_SIZE * y + .001f
                });
                
                texCoordData.insert(texCoordData.end(), {
                    
                    u, v,
                    u, v + spriteHeight,
                    u + spriteWidth, v + spriteHeight,
                    u, v,
                    u + spriteWidth, v + spriteHeight,
                    u + spriteWidth, v
                });
            }
        }
    }
    
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program.positionAttribute);
    
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program.texCoordAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0, 6 * count);
}
bool flag = true;

void RenderGame(ShaderProgram* program, const std::vector<Entity>& aliens, const SheetSprite& PlayerSprite, const SheetSprite& EnemySprite, const Entity& player,  const std::vector<Entity>& bullets, const SheetSprite& bulletSprite, const SheetSprite& EnemySprite2){
    
    //RenderMap(*program);
    
    
    
    for(size_t i=0; i<bullets.size(); i++){
        bullets[i].Draw(program, bulletSprite);
        
    }
    for(size_t i=0; i<aliens.size(); i++){
        if ( i < 20){
            aliens[i].Draw(program, EnemySprite2);
        }
        else {
            aliens[i].Draw(program, EnemySprite2);
        }
    
    }
    
    
    player.Draw(program, PlayerSprite);
        
}

void RenderGameLevel2(ShaderProgram* program, const std::vector<Entity>& aliens, const SheetSprite& PlayerSprite, const SheetSprite& EnemySprite, const Entity& player,  const std::vector<Entity>& bullets, const SheetSprite& bulletSprite){
    
    //RenderMap(*program);
    
    
    for(size_t i=0; i<bullets.size(); i++){
        bullets[i].Draw(program, bulletSprite);
        
    }
    for(size_t i=0; i<aliens.size(); i++){
        aliens[i].Draw(program, EnemySprite);
    }
    
    
    player.Draw(program, PlayerSprite);
        
}



void RenderWin(ShaderProgram* program, Entity& font, Entity& font2, GLuint fontTexture){
    font.drawWords(program, fontTexture, "You Win!");
    font2.drawInstructions(program, fontTexture, "Press Space to play!");
}

void RenderGameOver(ShaderProgram* program, Entity& font, Entity& font2, GLuint fontTexture){
    font.drawWords(program, fontTexture, "Game Over!");
    font2.drawInstructions(program, fontTexture, "press SPACE to retry!");
    
}


void RenderMainMenu(ShaderProgram* program, Entity& font, Entity& font2, Entity& font3, GLuint fontTexture, const SheetSprite& titleImage, Entity& titleImg, Entity& titleImg2){
    font.drawWords(program, fontTexture, "Not Really");
    font2.drawInstructions(program, fontTexture, "Press Space to Start");
    font3.drawTitle(program, fontTexture, "SNOW INVADERS");
    titleImg.Draw(program, titleImage);
    titleImg2.Draw(program, titleImage);
}




int main(int argc, char *argv[]){
    ShaderProgram program;
    setup(&program);
    
    
    GLuint fontTexture = LoadTexture(RESOURCE_FOLDER"font1.png");
    GLuint spriteSheetTexture = LoadTexture(RESOURCE_FOLDER"sheet.png");
    GLuint snowman = LoadTexture(RESOURCE_FOLDER"SnowMan.png");
    GLuint yeti = LoadTexture(RESOURCE_FOLDER"yeti.png");
    
    Mix_Chunk *shotSound;
    shotSound = Mix_LoadWAV("shot.wav");
    
    Mix_Chunk *deathSound;
    deathSound = Mix_LoadWAV("explode.wav");
    
    Mix_Chunk *enemyShotSound;
    enemyShotSound = Mix_LoadWAV("enemyShot.wav");
    
    SheetSprite yetiSprite = SheetSprite(yeti, 0, 0, 1, 1, 0.7);
        SheetSprite playerSprite = SheetSprite(spriteSheetTexture, 247.0f/1024.0f, 84.0f/1024.0f, 99.0f/1024.0f, 75.0f/1024.0f, 0.2);
        SheetSprite BulletSprite = SheetSprite(spriteSheetTexture, 842.0f/1024.0f, 230.0f/1024.0f, 9.0f/1024.0f, 54.0f/1024.0f, 0.2);
    SheetSprite titleSprite = SheetSprite(spriteSheetTexture, 247.0/1024.0f, 84.0f/1024.0f, 99.0f/1024.0f, 75.0f/1024.0f, 0.2);
    SheetSprite EnemySprite2 = SheetSprite(spriteSheetTexture, 230.0f/1024.0f, 530.0f/1024.0f, 100.0f/1024.0f, 140.0f/1024.0f, 0.2);
    SheetSprite snowmanSprite = SheetSprite(snowman, 0, 0, 1, 1, 0.3);
        
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
        
    
    float initialX = -3.55f-0.5*0.2*1.5f+1.0f;
    float initialY = 1.8f;
        
    for(GLsizei j=0; j<5; j++){
        for(GLsizei i=0; i<7; i++){
            Entity newEnemy(initialX, initialY, 1.5f, 1.5f);
            initialX+=0.5*1.3f;
            aliens.push_back(newEnemy);
            
        }
        initialX = -3.55f-0.5*0.2*1.5f+1.0f;
        initialY-=0.4f;
            }
        
    
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
                            ProcessGameInput(&event, done, bullets, player, shotSound);
                        }else if(mode == STATE_GAME_OVER){
                                ProcessGameOverInput(&event, done, mode);
                            }else if(mode == STATE_WIN){
                                    ProcessGameOverInput(&event, done, mode);
                                }
                
                glClear(GL_COLOR_BUFFER_BIT);
                
                std::cout << gameLevel << std::endl;
                if(mode == STATE_MAIN_MENU){
                        RenderMainMenu(&program, font, font2, font3, fontTexture, titleSprite, titleim, titleim2);
                    }else if(mode == STATE_GAME_LEVEL){
                            UpdateGame(aliens, player, bullets, elapsed, mode, deathSound, enemyShotSound);
                if (gameLevel == 0) {
                            RenderGame(&program, aliens, playerSprite, snowmanSprite, player, bullets, BulletSprite, snowmanSprite);
                }
                else {
                    RenderGameLevel2(&program, aliens, playerSprite, yetiSprite, player, bullets, BulletSprite);
                }
                        }else if(mode == STATE_GAME_OVER){
                                RenderGameOver(&program, font, font2, fontTexture);
                            }else if(mode == STATE_WIN){
                                    RenderWin(&program, font, font2, fontTexture);
                                }
                
                
                SDL_GL_SwapWindow(displayWindow);
                
            }
        Mix_FreeChunk(shotSound);
        SDL_Quit();
        return 0;
}
