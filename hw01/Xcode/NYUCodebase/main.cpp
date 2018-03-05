//Stanley Zeng

//hw01


#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "Matrix.h"
#include "ShaderProgram.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

GLuint LoadTexture(const char *image_path)
{
    SDL_Surface *surface = IMG_Load(image_path);
    
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    SDL_FreeSurface(surface);
    return textureID;
}


int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    ///setup
    //ShaderProgram program = ShaderProgram(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    ShaderProgram program = ShaderProgram(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    Matrix projectionMatrix;
    Matrix modelMatrix;
    Matrix viewMatrix;
    
    projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0, 2.0, -1.0, 1.0);
    
    GLuint red = LoadTexture("red.png");
    GLuint blue = LoadTexture("blue.png");
    GLuint yellow = LoadTexture("yellow.png");
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    float lastFrameTicks = 0.0f;
    float angle = 0.0f;
    
    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }
        program.setModelMatrix(modelMatrix);
        program.setProjectionMatrix(projectionMatrix);
        program.setViewMatrix(viewMatrix);
        
        glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glUseProgram(program.programID);
        
        float vertices[] = { -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f };
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        
        float texCoords[] = { 0.0, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0 };
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glBindTexture(GL_TEXTURE_2D, red);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        float vertices2[] = { -1.5f, -0.5f, -0.5f, 0.5f, -1.5f, 0.5f, -0.5f, 0.5f, -1.5f, -0.5f, -0.5f, -0.5f };
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices2);
        glEnableVertexAttribArray(program.positionAttribute);
        
        float texCoords2[] = { 0.0, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0 };
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords2);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glBindTexture(GL_TEXTURE_2D, yellow);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        float vertices3[] = { 1.5f, -0.5f, 0.5f, 0.5f, 1.5f, 0.5f, 0.5f, 0.5f, 1.5f, -0.5f, 0.5f, -0.5f };
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices3);
        glEnableVertexAttribArray(program.positionAttribute);
        
        float texCoords3[] = { 0.0, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0 };
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords3);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glBindTexture(GL_TEXTURE_2D, blue);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        float ticks = (float)SDL_GetTicks() / 1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        angle += elapsed;
        glMatrixMode(blue);
        modelMatrix.identity();
        modelMatrix.Rotate(angle);
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}
