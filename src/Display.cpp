#include <cassert>

#include "../include/Display.hpp"
#include "../include/MEM.hpp"

void mat4::ortho(float left, float right, float bottom, float top, float zNear, float zFar){
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            m[i][j] = 0.0f;
    m[0][0] = 2.0f / (right - left);
    m[1][1] = 2.0f / (top - bottom);
    m[2][2] = -2.0f / (zFar - zNear);
    m[3][0] = -(right + left) / (right - left);
    m[3][1] = -(top + bottom) / (top - bottom);
    m[3][2] = -(zFar + zNear) / (zFar - zNear);
    m[3][3] = 1.0f;
}
const float* mat4::ptr(){
    return &m[0][0];
}

unsigned int compileShader(GLenum shaderType, const char *shaderSource)
{
    unsigned int shader;
    shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderSource, NULL);
    glCompileShader(shader);
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) exit(-1);
    return shader;
}
Shader::Shader()
{
    const char* vertexCode =
    "#version 330 core\n"\
    "layout (location = 0) in vec2 aPos;"\
    "layout (location = 1) in vec2 tPos;"\
    "out vec2 TexturePos;"\
    "uniform mat4 projection;"\
    "void main()"\
    "{"\
    "   gl_Position = projection * vec4(aPos, 0.0, 1.0);"\
    "   TexturePos = tPos;"\
    "}";
    const char* fragmentCode =\
    "#version 330 core\n"\
    "out vec4 FragColor;"\
    "uniform sampler2D Texture;"\
    "in vec2 TexturePos;"\
    "void main()"\
    "{"\
    "   vec4 objectColor = texture(Texture, TexturePos);"\
    "   FragColor = vec4(objectColor.rgb, 1.0);"\
    "}";
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexCode);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentCode);
    
    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}
void Shader::setMat4(const char* name, const float* mat) const {
    glUniformMatrix4fv(glGetUniformLocation(program, name), 1, GL_FALSE, mat);
}

Texture::Texture(int width, int height) :
_width(width), _height(height)
{
    glGenTextures(1, &_texture);
    bind();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 _width, _height, 0, 
                 GL_RGB, GL_UNSIGNED_BYTE, nullptr);
}
void Texture::update(Color* data) {
    bind();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 _width, _height, 0, 
                 GL_RGB, GL_UNSIGNED_BYTE, data);
}
Texture::~Texture(){
    glDeleteTextures(1, &_texture);
}
void Texture::bind(){
    glBindTexture(GL_TEXTURE_2D, _texture);
}

const Vertex vertices[] = {
    {-1.f, -1.f, 0, 1},  // v0
    { 1.f, -1.f, 1, 1},  // v1
    {-1.f,  1.f, 0, 0},  // v2
    { 1.f, -1.f, 1, 1},  // v1
    { 1.f,  1.f, 1, 0},  // v3
    {-1.f,  1.f, 0, 0},  // v2
};

Mesh::Mesh()
{
    size = 6;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(Vertex), vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0); 

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

Mesh::~Mesh(){
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}
void Mesh::Show()
{
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, size);
}

Window::Window(unsigned int width, unsigned int height, const char* name)
: _width(width), _height(height)
{
    assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == 0 && "Failed to initialize SDL");
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    #ifdef __APPLE__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    #endif
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_GL_SetSwapInterval(1);

    window = SDL_CreateWindow(name, SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, width, height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
    assert(window != nullptr && "Failed to create SDL window");

    glContext = SDL_GL_CreateContext(window);
    assert(glContext != 0 && "Failed to create OpenGL context");

    glewExperimental = GL_TRUE;
    assert(glewInit() == GLEW_OK && "Failed to initialize GLEW");

    screen = new Texture(SCW, SCH);
    shader = new Shader;
    mesh = new Mesh();
    display = new Color[SCW*SCH];
    for (unsigned int i = 0; i < SCW*SCH; i++)
        display[i] = {255, 255, 255};
    screen->update(display);
    glUseProgram(*shader);
    glActiveTexture(GL_TEXTURE0);
    screen->bind();
    glUniform1i(glGetUniformLocation(*shader, "Texture"), 0);
    resize(width, height);
};
Window::~Window() {
    delete[] display;
    delete screen;
    delete shader;
    delete mesh;
    if (glContext) {
        SDL_GL_DeleteContext(glContext);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }
    SDL_Quit();
}
void Window::clear(){
    glClearColor(0.f, 0.f, 0.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}
void Window::resize(int width, int height){
    glViewport(0, 0, width, height);
    _width = width;
    _height = height;
    float aspect = (float)_width / _height;
    float gbAspect = 160.0f / 144.0f;

    if (aspect > gbAspect) {
        float scale = gbAspect / aspect;
        _projection.ortho(-1.0f/scale, 1.0f/scale, -1.0f, 1.0f, -1.0f, 1.0f);
    } else {
        float scale = aspect / gbAspect;
        _projection.ortho(-1.0f, 1.0f, -1.0f/scale, 1.0f/scale, -1.0f, 1.0f);
    }
    shader->setMat4("projection", _projection.ptr());
}
void Window::poolEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                shouldClose = true;
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    resize(event.window.data1, event.window.data2);
                }
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case(SDLK_s):
                        joypad.directions &= ~0x8; break;
                    case(SDLK_w):
                        joypad.directions &= ~0x4; break;
                    case(SDLK_a):
                        joypad.directions &= ~0x2; break;
                    case(SDLK_d):
                        joypad.directions &= ~0x1; break;
                    case(SDLK_z):
                        joypad.buttons &= ~0x8; break;
                    case(SDLK_x):
                        joypad.buttons &= ~0x4; break;
                    case(SDLK_q):
                        joypad.buttons &= ~0x2; break;
                    case(SDLK_e):
                        joypad.buttons &= ~0x1; break;
                    case(SDLK_c):
                        joypad.buttons &= ~0xF; break;
                    default:
                        return;
                }
                joypad.update();
                break;
            case SDL_KEYUP:
                switch (event.key.keysym.sym) {
                    case(SDLK_s):
                        joypad.directions |= 0x8; break;
                    case(SDLK_w):
                        joypad.directions |= 0x4; break;
                    case(SDLK_a):
                        joypad.directions |= 0x2; break;
                    case(SDLK_d):
                        joypad.directions |= 0x1; break;
                    case(SDLK_z):
                        joypad.buttons |= 0x8; break;
                    case(SDLK_x):
                        joypad.buttons |= 0x4; break;
                    case(SDLK_q):
                        joypad.buttons |= 0x2; break;
                    case(SDLK_e):
                        joypad.buttons |= 0x1; break;
                    case(SDLK_c):
                        joypad.buttons |= 0xF; break;
                    default:
                        return;
                }
                joypad.update();
                break;
        }
    }
}
bool Window::poolFile(MemoryMaster& master) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                shouldClose = true;
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    resize(event.window.data1, event.window.data2);
                }
                break;
            case SDL_DROPFILE: {
                
                std::string filepath = event.drop.file;
                SDL_free(event.drop.file);
                if (master.readFromFile(filepath.c_str())) return true;
                
                break;
            }
        }
    }
    return false;
}
static constexpr int FRAME_DELAY = 1000 / 60;
void Window::show(){
    clear();
    screen->update(display);
    mesh->Show();
    SDL_GL_SwapWindow(window);

    static Uint32 lastTime = SDL_GetTicks();

    Uint32 currentTime = SDL_GetTicks();
    Uint32 frameTime = currentTime - lastTime;
    
    if (frameTime < FRAME_DELAY) {
        SDL_Delay(FRAME_DELAY - frameTime);
    }
    
    lastTime = SDL_GetTicks(); 
}
const uint8_t col[4] = {0xFF, 0xAA, 0x55, 0x00};
void Window::drawLine(uint8_t* lines, uint8_t y){
    int dy = y * SCW;
    for (int x = 0; x < 160; x++) {
        int adr = dy + x;
        uint8_t color = col[lines[x]];
        display[adr] = {color, color, color};
    }
}
void Window::setPixel(uint8_t x, int dy, Color& color){
    int idx = dy + x;
    display[idx] = color;
}
bool Window::isOpen() { return !shouldClose; }