#include "gl_renderer.h"
#include "alloc_lib.h"
#include "file_lib.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct GLContext
{
    GLuint programID;
};

static GLContext glContext;

static void APIENTRY gl_debug_callback(GLenum source, GLenum type, GLuint id, 
    GLenum severity, GLsizei length, const GLchar* message, const void* user)
{
    if(severity == GL_DEBUG_SEVERITY_LOW || 
        severity == GL_DEBUG_SEVERITY_MEDIUM || 
        severity == GL_DEBUG_SEVERITY_HIGH)
    {
        _ASSERT(0, "OpenGL error: %s", message);
    }
    else
    {
        _TRACE("OpenGL msg: %s", message);
    }
}

void check_shader_compile_status(GLuint shaderID)
{
    int success = 0;
    char shaderLog[2048] = {};

    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);

    if(!success)
    {
        glGetShaderInfoLog(shaderID, 2048, 0, shaderLog);

        _ASSERT(0, "Failed to compile shader: %s", shaderLog);
    }
}

bool gl_init(BumpAllocator* transientStorage)
{
    gl_load_functions();
    glDebugMessageCallback(&gl_debug_callback, nullptr);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glEnable(GL_DEBUG_OUTPUT);

    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    int fileSize = 0;
    char* vertexShader = read_file("assets/shaders/quad.vert", &fileSize, transientStorage);
    char* fragmentShader = read_file("assets/shaders/quad.frag", &fileSize, transientStorage);

    _ASSERT(vertexShader && fragmentShader, "Failed to load shaders!");

    if(!vertexShader || !fragmentShader)
    {
        return false;
    }

    glShaderSource(vertexShaderID, 1, &vertexShader, 0);
    glShaderSource(fragmentShaderID, 1, &fragmentShader, 0);

    glCompileShader(vertexShaderID);
    glCompileShader(fragmentShaderID);

    check_shader_compile_status(vertexShaderID);
    check_shader_compile_status(fragmentShaderID);

    glContext.programID = glCreateProgram();
    
    glAttachShader(glContext.programID, vertexShaderID);
    glAttachShader(glContext.programID, fragmentShaderID);
    
    glLinkProgram(glContext.programID);

    glDetachShader(glContext.programID, vertexShaderID);
    glDetachShader(glContext.programID, fragmentShaderID);

    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);

    GLuint VAO;
    
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    {
        
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER);

    glUseProgram(glContext.programID);

    return true;
}

void gl_render()
{
    glClearColor(119.0f / 255.0f, 33.0f / 255.0f, 111.0f / 255.0f, 1.0f);
    glClearDepth(0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, input.screenWidth, input.screenHeight);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}