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
#include "SatCollision.hpp"


#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif



SDL_Window* displayWindow;

enum GameMode {STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_GAME_OVER, STATE_WIN};

float timer = 0.0f;


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
    
    Vector3() : x(0.0f), y(0.0f), z(0.0f) {};
    
    Vector3 operator*(const Matrix& mat) {
        Vector3 newv(0.0f, 0.0f, 0.0f);
        
        newv.x = mat.m[0][0] * this->x + mat.m[1][0] * this->y + mat.m[2][0] * this->z + mat.m[3][0];
        newv.y = mat.m[0][1] * this->x + mat.m[1][1] * this->y + mat.m[2][1] * this->z + mat.m[3][1];
        newv.z = mat.m[0][2] * this->x + mat.m[1][2] * this->y + mat.m[2][2] * this->z + mat.m[3][2];
        
        return newv;
    }
    float x;
    float y;
    float z;
    
    
};

enum EntityType {ENTITY_PLAYER, ENTITY_ENEMY, ENTITY_STATIC};
class Entity{
public:
    Entity(float positionX, float positionY, float sizeX, float sizeY):position(positionX, positionY, 0.0f), size(sizeX, sizeY,0.0f){};
    
    Entity(float positionX, float positionY, float sizeX, float sizeY, EntityType type):position(positionX, positionY, 0.0f), size(sizeX, sizeY,0.0f), entityType(type){
        
        Vector3 topLeft;
        Vector3 topRight;
        Vector3 botLeft;
        Vector3 botRight;
        botRight.x = 0.5f * this->size.x;
        botLeft.x = -0.5f * this->size.y;
        botRight.y = -0.5f * this->size.y;
        botLeft.y = -0.5 * this->size.y;
        
        topRight.x = 0.5f * this->size.x;
        topLeft.x = -0.5f * this->size.x;
        topRight.y = -0.5f * this->size.y;
        topLeft.y = 0.5 * this->size.y;
        
        this->points.push_back(topLeft);
        this->points.push_back(topRight);
        this->points.push_back(botLeft);
        this->points.push_back(botRight);
        
        //points.push_back(
    };
    
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
        position.x -= elapsed * 0.3f;
        if(position.x >= 3.55 + size.x * 0.5 * 0.2){
            position.x = -3.55f - size.x * 0.4 * 0.2;
        }
    }
    

    
    
    bool SatCollision(Entity& entity){
        
        matrix.Identity();
        matrix.Translate(position.x, position.y, position.z);
        matrix.Scale(size.x, size.y, size.z);
        
        std::pair<float,float> penetration;
        std::vector<std::pair<float,float>> point1;
        std::vector<std::pair<float,float>> point2;
        
        
        for(int i=0; i < points.size(); i++) {
            Vector3 point = points[i]*matrix;
            point1.push_back(std::make_pair(point.x, point.y));
        }
        
        for(int i=0; i < points.size(); i++) {
            Vector3 point = points[i]*entity.matrix ;
            point2.push_back(std::make_pair(point.x, point.y));
        }
        
        bool collided = CheckSATCollision(point1, point2, penetration);
        
        if(collided){
            position.x += (penetration.first * 0.5f);
            position.y += (penetration.second * 0.5f);
            entity.position.x -= (penetration.first * 0.5f);
            entity.position.y -= (penetration.second * 0.5f);
            for (Vector3& v : points) {
                v.x +=(penetration.first * 0.5f);
                v.y += (penetration.second * 0.5f);
            }
            
        }

        return collided;
    
    }
    
    Vector3 position;
    Vector3 size;
    float yVelocity;
    float xVelocity;
    bool colTop;
    bool colBottom;
    bool colLeft;
    bool colRight;
                         
    EntityType entityType;
    Matrix matrix;
    std::vector<Vector3> points;
    
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
        glClearColor(0.00f/255.0f, 0.0f/255.0f, 0.0f/255.0f, 1.0f);
        
        program->Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ProcessGameInput(SDL_Event* event, bool& done, const Entity& player){
        while (SDL_PollEvent(event)){
                if(event->type == SDL_QUIT || event->type == SDL_WINDOWEVENT_CLOSE){
                        done = true;
                    }
                else if (event->type == SDL_KEYDOWN){
                    if(event->key.keysym.scancode == SDL_SCANCODE_UP){
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
                gameMode = STATE_MAIN_MENU;
            }
        }
    }
}

/*
void random(std::vector<Entity>& aliens, std::vector<Entity>& bullets) {
    srand (time(NULL));
    for (size_t i = 0; i < aliens.size(); i++){
        int x = rand() % 3 + 1;
        if (x % 2 == 0) {
            aliens[i].shootBullets(bullets);
        }
    }
}
 */

void UpdateGame(std::vector<Entity>& aliens, Entity& player, float elapsed, GameMode& gameMode){
    
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
    
    else if (keys[SDL_SCANCODE_UP]) {
        player.position.y += elapsed * 1.6f;
        if (player.position.y >= 1.9f){
            player.position.y = player.position.y -= 0.07f;
        }
    }
    
    else if (keys[SDL_SCANCODE_DOWN]) {
        player.position.y -= elapsed * 1.6f;
        if (player.position.y <= -1.9f) {
            player.position.y = player.position.y + 0.07f;
        }
    }
        
        
    win = true;
    for (Entity& alien : aliens) {
        player.SatCollision(alien);
    }
    for(GLsizei i=0; i<aliens.size(); i++){
        if(aliens[i].position.x > -3.55f && aliens[i].position.x < 3.55f){
            win = false;
            
        }
    }
    if (win){
        gameMode = STATE_WIN;
    }
        
}

void RenderGame(ShaderProgram* program, const std::vector<Entity>& aliens, const SheetSprite& PlayerSprite, const SheetSprite& EnemySprite, const Entity& player, const SheetSprite& bulletSprite, const SheetSprite& EnemySprite2){
        
        
    for(size_t i=0; i<aliens.size(); i++){
        if ( i < 20){
            aliens[i].Draw(program, EnemySprite);
        }
        else {
            aliens[i].Draw(program, EnemySprite2);
        }
        
    }
    
    player.Draw(program, PlayerSprite);
        
}

void RenderWin(ShaderProgram* program, Entity& font, Entity& font2, GLuint fontTexture){
    font.drawWords(program, fontTexture, "You Win!");
    font2.drawInstructions(program, fontTexture, "Press Space to Start");
}

void RenderGameOver(ShaderProgram* program, Entity& font, Entity& font2, GLuint fontTexture){
    font.drawWords(program, fontTexture, "Game Over!");
    //font2.drawInstructions(program, fontTexture, "SPACE to retry!"); not working currently
    
}

    
void RenderMainMenu(ShaderProgram* program, Entity& font, Entity& font2, Entity& font3, GLuint fontTexture, const SheetSprite& titleImage, Entity& titleImg, Entity& titleImg2){
    font.drawWords(program, fontTexture, "Not Really");
    font2.drawInstructions(program, fontTexture, "Press Space to Start");
    font3.drawTitle(program, fontTexture, "SPACE INVADERS");
    titleImg.Draw(program, titleImage);
    titleImg2.Draw(program, titleImage);
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
        Entity player(01.0f, -1.3f, 1.5f, 1.5f, ENTITY_PLAYER);
        Entity font2(-1.08f, 0.0f, 0.4f, 0.5f);
        Entity font3(-1.08f, 0.0f, 0.5f, 0.3f);
        Entity titleim(-1.0f, -1.3f, 1.5f, 1.5f);
        Entity titleim2(1.0f, -1.3f, 1.5f, 1.5f);
        Entity newEnemy(-3.55f*0.2*1.5f+1.0f, 0.3f, 1.5f, 1.5f, ENTITY_STATIC);
        Entity newEnemy2(-3.55f*0.2*1.5f+1.0f, 0.1f, 1.5f, 1.5f, ENTITY_STATIC);
        std::vector<Entity> aliens;
    
        aliens.push_back(newEnemy);
    aliens.push_back(newEnemy2);
        
        float initialX = -3.55f*0.2*1.5f;
        float initialY = 1.8f;
    
    /*
        
        for(size_t j=0; j<1; j++){
                for(size_t i=0; i<4; i++){
                        Entity newEnemy(initialX, initialY, 1.5f, 1.5f, ENTITY_STATIC);
                        initialX+=1.3f;
                        aliens.push_back(newEnemy);
                    }
                initialX = -3.55f-0.5*0.2*1.5f;
                initialY-=0.4f;
            }
     */

        

        
        while (!done) {
                float ticks = (float)SDL_GetTicks()/1000.0f;
                float elapsed = ticks - lastFrameTicks;
                lastFrameTicks = ticks;
                
                if(mode == STATE_MAIN_MENU){
                        ProcessMainMenuInput(&event, done, mode);
                    }else if(mode == STATE_GAME_LEVEL){
                            ProcessGameInput(&event, done, player);
                        }else if(mode == STATE_GAME_OVER){
                                ProcessGameOverInput(&event, done, mode);
                            }else if(mode == STATE_WIN){
                                    ProcessGameOverInput(&event, done, mode);
                                }
                
                glClear(GL_COLOR_BUFFER_BIT);
                
                
                if(mode == STATE_MAIN_MENU){
                        RenderMainMenu(&program, font, font2, font3, fontTexture, titleSprite, titleim, titleim2);
                    }else if(mode == STATE_GAME_LEVEL){
                            UpdateGame(aliens, player, elapsed, mode);
                            RenderGame(&program, aliens, playerSprite, EnemySprite, player, BulletSprite, EnemySprite2);
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
