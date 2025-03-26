#include "gl_renderer.h"
#include "alloc_lib.h"
#include "file_lib.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "render_interface.h"

#include "input.h"

const char* SPRITE_PATH = "assets/sprites/test_atlas.png";

struct GLContext
{
    GLuint programID;
    GLuint textureID;
    GLuint transformSBOID;
    GLuint screenSizeID;
    GLuint orthoProjID;
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
        int width, height, channels;

        char* data = (char*)stbi_load(SPRITE_PATH, &width, &height, &channels, 4);

        _ASSERT(data, "Failed to load sprite");

        if(!data)
        {
            return false;
        }

        glGenTextures(1, &glContext.textureID);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, glContext.textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);
    }

    {
        glGenBuffers(1, &glContext.transformSBOID);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, glContext.transformSBOID);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Transform) * MAX_TRANSFORMS, renderData->transforms, GL_DYNAMIC_DRAW);
    }

    {
        glContext.screenSizeID = glGetUniformLocation(glContext.programID, "screenSize");
        glContext.orthoProjID = glGetUniformLocation(glContext.programID, "orthoProj");
    }

    glEnable(GL_FRAMEBUFFER_SRGB);
    glDisable(0x809D);

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
    glViewport(0, 0, input->screenSize.x, input->screenSize.y);

    Vec2 screenSize = {(float)input->screenSize.x, (float)input->screenSize.y};
    
    glUniform2fv(glContext.screenSizeID, 1, &screenSize.x);

    OrthoCamera2D camera = renderData->mainCamera;
    Matrix4x4 orthoProj = orthographic_projection(camera.position.x - camera.dimensions.x / 2.0f,
                                                  camera.position.x + camera.dimensions.x / 2.0f,
                                                  camera.position.y - camera.dimensions.y / 2.0f,
                                                  camera.position.y + camera.dimensions.y / 2.0f);
    
    glUniformMatrix4fv(glContext.orthoProjID, 1, GL_FALSE, &orthoProj.ax);

    {
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Transform) * renderData->transformCount, renderData->transforms);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, renderData->transformCount);

        renderData->transformCount = 0;
    }
}