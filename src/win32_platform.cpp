#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include "wglext.h"

#include "platform.h"
#include "debug_lib.h"

static HWND window = NULL;
static HDC deviceContext = NULL;

LRESULT CALLBACK windows_window_callback(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    switch(msg)
    {
        case WM_CLOSE:
        {
            isRunning = false;
            
            break;
        }
        
        case WM_SIZE:
        {
            RECT rect = {};
            
            GetClientRect(window, &rect);

            input.screenWidth = rect.right - rect.left;
            input.screenHeight = rect.bottom - rect.top;

            break;
        }

        default:
        {
            result = DefWindowProcA(window, msg, wParam, lParam);
        }
    }

    return result;
}

bool platform_create_window(int width, int height, char* title)
{
    HINSTANCE instance = GetModuleHandleA(0);

    WNDCLASSA wc = {};
    wc.hInstance = instance;
    wc.hIcon = LoadIcon(instance, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = title;
    wc.lpfnWndProc = windows_window_callback;
    wc.hbrBackground = NULL;

    if(!RegisterClassA(&wc))
    {
        return false;
    }

    int dwStyle = WS_OVERLAPPEDWINDOW;

    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = nullptr;
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;

    {
        window = CreateWindowExA(0,
                                    title,
                                    title,
                                    dwStyle,
                                    100,
                                    100,
                                    width,
                                    height,
                                    NULL,
                                    NULL,
                                    instance,
                                    NULL);

        _ASSERT(window, "Failed to create window");

        if(window == NULL)
        {
            return false;
        }

        HDC fakeDC = GetDC(window);

        _ASSERT(fakeDC, "Failed to get fake device context!");

        if(!fakeDC)
        {            
            return false;
        }

        PIXELFORMATDESCRIPTOR pfd = {0};
        pfd.nSize = sizeof(pfd);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;
        pfd.cAlphaBits = 8;
        pfd.cDepthBits = 24;

        int pixelFormat = ChoosePixelFormat(fakeDC, &pfd);

        _ASSERT(pixelFormat, "Failed to choose pixel format!");

        if(!pixelFormat)
        {
            return false;
        }

        bool didSetPixelFormat = SetPixelFormat(fakeDC, pixelFormat, &pfd);

        _ASSERT(didSetPixelFormat, "Failed to set pixel format!");

        if(!didSetPixelFormat)
        {
            return false;
        }

        HGLRC fakeRC = wglCreateContext(fakeDC);

        _ASSERT(fakeRC, "Failed to create fake rendering context handle!");

        if(!fakeRC)
        {
            return false;
        }

        bool didMakeCurrent = wglMakeCurrent(fakeDC, fakeRC);

        _ASSERT(didMakeCurrent, "Faile to make window current!");

        if(!didMakeCurrent)
        {
            return false;
        }

        wglChoosePixelFormatARB = 
            (PFNWGLCHOOSEPIXELFORMATARBPROC)platform_load_gl_function("wglChoosePixelFormatARB");

        _ASSERT(wglChoosePixelFormatARB, "Failed to load OpenGL function: wglChoosePixelFormatARB");

        wglCreateContextAttribsARB = 
            (PFNWGLCREATECONTEXTATTRIBSARBPROC)platform_load_gl_function("wglCreateContextAttribsARB");
                
        _ASSERT(wglCreateContextAttribsARB, "Failed to load OpenGL function: wglCreateContextAttribsARB");
        
        if(!wglChoosePixelFormatARB || !wglCreateContextAttribsARB)
        {
            return false;
        }

        wglMakeCurrent(fakeDC, 0);
        wglDeleteContext(fakeRC);
        ReleaseDC(window, fakeDC);

        DestroyWindow(window);
    }

    {
        RECT borderSize = {};
        AdjustWindowRectEx(&borderSize, dwStyle, 0, 0);

        width += borderSize.right - borderSize.left;
        height += borderSize.bottom - borderSize.top;

        window = CreateWindowExA(0,
            title,
            title,
            dwStyle,
            100,
            100,
            width,
            height,
            NULL,
            NULL,
            instance,
            NULL);

        _ASSERT(window, "Failed to create window");

        if(window == NULL)
        {
        return false;
        }

        deviceContext = GetDC(window);

        _ASSERT(deviceContext, "Failed to get device context!");

        if(!deviceContext)
        {            
            return false;
        }

        const int pixelAttribs[] =
        {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
            WGL_SWAP_METHOD_ARB,    WGL_SWAP_COPY_ARB,
            WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
            WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
            WGL_COLOR_BITS_ARB,     32,
            WGL_ALPHA_BITS_ARB,     8,
            WGL_DEPTH_BITS_ARB,     24,
            0
        };

        UINT numPixelFormats;
        int pixelFormat = 0;

        bool didChoosePixelFormat = wglChoosePixelFormatARB(
            deviceContext, 
            pixelAttribs,
            0,
            1,
            &pixelFormat,
            &numPixelFormats
        );

        _ASSERT(didChoosePixelFormat, "Failed to choose pixel format!");

        if(!didChoosePixelFormat)
        {
            return false;
        }

        PIXELFORMATDESCRIPTOR pfd = {0};
        
        DescribePixelFormat(deviceContext, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
        bool didSetPixelFormat = SetPixelFormat(deviceContext, pixelFormat, &pfd);

        _ASSERT(didSetPixelFormat, "Failed to set pixel format!");

        if(!didSetPixelFormat)
        {
            return false;
        }

        const int contextAttribs[] =
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB,  4,
            WGL_CONTEXT_MINOR_VERSION_ARB,  3,
            WGL_CONTEXT_PROFILE_MASK_ARB,   WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            WGL_CONTEXT_FLAGS_ARB,          WGL_CONTEXT_DEBUG_BIT_ARB,
            0
        };

        HGLRC rc = wglCreateContextAttribsARB(deviceContext, 0, contextAttribs);

        _ASSERT(rc, "Failed to create render context!");

        if(!rc)
        {
            return false;
        }

        bool didMakeCurrent = wglMakeCurrent(deviceContext, rc);

        _ASSERT(didMakeCurrent, "Failed to make window current!");

        if(!didMakeCurrent)
        {
            return false;
        }
    }

    ShowWindow(window, SW_SHOW);
    
    return true;
}

void platform_update_window()
{
    MSG msg;

    while(PeekMessageA(&msg, window, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
}

void* platform_load_gl_function(char* functionName)
{
    PROC proc = wglGetProcAddress(functionName);

    if(!proc)
    {
        static HMODULE glDLL = LoadLibraryA("opengl32.dll");

        proc = GetProcAddress(glDLL, functionName);
    }

    if(!proc)
    {
        _ASSERT(false, "Failed to load GL function %s", functionName);

        return nullptr;
    }
    
    return (void*)proc;
}

void platform_swap_buffers()
{
    SwapBuffers(deviceContext);
}