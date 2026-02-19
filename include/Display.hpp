#pragma once

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <SDL2/SDL_video.h>
#include "types.hpp"
#include "Joypad.hpp"

class mat4 {
    float m[4][4]; // m[col][row]
public:
    void ortho(float left, float right, float bottom, float top, float zNear, float zFar);
    const float* ptr();
};


class Shader{
    unsigned int program;
public:
    Shader();
    void setMat4(const char* name, const float* mat) const;
    operator unsigned int() const {
        return program;
    }
};

class Texture{
    unsigned int _texture;
    int _width;
    int _height;
public:
    Texture(int width, int height);
    ~Texture();
    void update(Color* data);
    void bind();
    operator unsigned int(){
        return _texture;
    }
};

class Mesh{
    unsigned int VAO, VBO;
    unsigned int size;
public:
    Mesh();
    ~Mesh();
    void Show();
};

class MemoryMaster;
class GLFWwindow;
class Window{
    SDL_Window* window;
    SDL_GLContext glContext;
    mat4 _projection;
    Color* display;
    Shader* shader;
    Texture* screen;
    Mesh* mesh;
    bool shouldClose;

    int _width;
    int _height;
public:
    Joypad joypad;
    
    Window(unsigned int width, unsigned int height, const char* name);
    ~Window();
    void clear();
    void resize(int width, int height);
    void poolEvents();
    bool poolFile(MemoryMaster& master);

    void drawLine(uint8_t* lines, uint8_t y);
    void setPixel(uint8_t x, int dy, Color& color);
    
    void show();
    bool isOpen();
};