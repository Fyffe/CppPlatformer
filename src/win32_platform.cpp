#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include "wglext.h"

#include "platform.h"
#include "debug_lib.h"

#include "input.h"

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

            input->screenSize.x = rect.right - rect.left;
            input->screenSize.y = rect.bottom - rect.top;

            break;
        }

        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        {
            bool isDown = msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN || msg == WM_LBUTTONDOWN;

            KeyCodeID keyCode = keyCodeLUT[wParam];
            Key* key = &input->keys[keyCode];
            
            key->justPressed = !key->justPressed && !key->isDown && isDown;
            key->justReleased = !key->justReleased && key->isDown && !isDown;
            key->isDown = isDown;
            key->halfTransitionCount++;

            break;
        }

        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        {
            bool isDown = msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN;
            int mouseCode = (msg == WM_LBUTTONDOWN || WM_LBUTTONUP) ? VK_LBUTTON :
                            (msg == WM_RBUTTONDOWN || WM_RBUTTONUP) ? VK_RBUTTON :
                            (msg == WM_MBUTTONDOWN || WM_MBUTTONUP) ? VK_MBUTTON : -1;

            KeyCodeID keyCode = keyCodeLUT[mouseCode];
            Key* key = &input->keys[keyCode];
            
            key->justPressed = !key->justPressed && !key->isDown && isDown;
            key->justReleased = !key->justReleased && key->isDown && !isDown;
            key->isDown = isDown;
            key->halfTransitionCount++;

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
    for(int keyCode = 0; keyCode < KEY_COUNT; keyCode++)
    {
        input->keys[keyCode].justPressed = false;
        input->keys[keyCode].justReleased = false;
        input->keys[keyCode].halfTransitionCount = 0;
    }

    MSG msg;

    while(PeekMessageA(&msg, window, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    POINT point = {};
    
    GetCursorPos(&point);
    ScreenToClient(window, &point);

    input->previousMouseScreenPosition = input->mouseScreenPosition;
    input->mouseScreenPosition = {point.x, point.y};
    input->relativeMouseScreenPosition = input->mouseScreenPosition - input->previousMouseScreenPosition;

    input->mouseWorldPosition = screen_to_world_pp(input->mouseScreenPosition);
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

void* platform_load_dynamic_library(char* dll)
{
    HMODULE result = LoadLibraryA(dll);

    _ASSERT(result, "Failed to load dll: %s", dll);

    return result;
}

void* platform_load_dynamic_function(void* dll, char* functionName)
{
    FARPROC proc = GetProcAddress((HMODULE)dll, functionName);

    _ASSERT(proc, "Failed to load function: %s from DLL", functionName);

    return (void*)proc;
}

bool platform_free_dynamic_library(void* dll)
{
    BOOL result = FreeLibrary((HMODULE)dll);

    _ASSERT(result, "Failed to free DLL");

    return (bool)result;
}

void platform_create_keycode_lut()
{
    keyCodeLUT[VK_LBUTTON] = KEY_MOUSE_LEFT;
    keyCodeLUT[VK_MBUTTON] = KEY_MOUSE_MIDDLE;  
    keyCodeLUT[VK_RBUTTON] = KEY_MOUSE_RIGHT;

    keyCodeLUT['A'] = KEY_A;
    keyCodeLUT['B'] = KEY_B;
    keyCodeLUT['C'] = KEY_C;
    keyCodeLUT['D'] = KEY_D;
    keyCodeLUT['E'] = KEY_E;
    keyCodeLUT['F'] = KEY_F;
    keyCodeLUT['G'] = KEY_G;
    keyCodeLUT['H'] = KEY_H;
    keyCodeLUT['I'] = KEY_I;
    keyCodeLUT['J'] = KEY_J;
    keyCodeLUT['K'] = KEY_K;
    keyCodeLUT['L'] = KEY_L;
    keyCodeLUT['M'] = KEY_M;
    keyCodeLUT['N'] = KEY_N;
    keyCodeLUT['O'] = KEY_O;
    keyCodeLUT['P'] = KEY_P;
    keyCodeLUT['Q'] = KEY_Q;
    keyCodeLUT['R'] = KEY_R;
    keyCodeLUT['S'] = KEY_S;
    keyCodeLUT['T'] = KEY_T;
    keyCodeLUT['U'] = KEY_U;
    keyCodeLUT['V'] = KEY_V;
    keyCodeLUT['W'] = KEY_W;
    keyCodeLUT['X'] = KEY_X;
    keyCodeLUT['Y'] = KEY_Y;
    keyCodeLUT['Z'] = KEY_Z;
    keyCodeLUT['0'] = KEY_0;
    keyCodeLUT['1'] = KEY_1;
    keyCodeLUT['2'] = KEY_2;
    keyCodeLUT['3'] = KEY_3;
    keyCodeLUT['4'] = KEY_4;
    keyCodeLUT['5'] = KEY_5;
    keyCodeLUT['6'] = KEY_6;
    keyCodeLUT['7'] = KEY_7;
    keyCodeLUT['8'] = KEY_8;
    keyCodeLUT['9'] = KEY_9;
    keyCodeLUT[VK_SPACE] = KEY_SPACE,
    keyCodeLUT[VK_OEM_3] = KEY_TICK,
    keyCodeLUT[VK_OEM_MINUS] = KEY_MINUS,
    keyCodeLUT[VK_OEM_PLUS] = KEY_EQUAL,
    keyCodeLUT[VK_OEM_4] = KEY_LEFT_BRACKET,
    keyCodeLUT[VK_OEM_6] = KEY_RIGHT_BRACKET,
    keyCodeLUT[VK_OEM_1] = KEY_SEMICOLON,
    keyCodeLUT[VK_OEM_7] = KEY_QUOTE,
    keyCodeLUT[VK_OEM_COMMA] = KEY_COMMA,
    keyCodeLUT[VK_OEM_PERIOD] = KEY_PERIOD,
    keyCodeLUT[VK_OEM_2] = KEY_FORWARD_SLASH,
    keyCodeLUT[VK_OEM_5] = KEY_BACKWARD_SLASH,
    keyCodeLUT[VK_TAB] = KEY_TAB,
    keyCodeLUT[VK_ESCAPE] = KEY_ESCAPE,
    keyCodeLUT[VK_PAUSE] = KEY_PAUSE,
    keyCodeLUT[VK_UP] = KEY_UP,
    keyCodeLUT[VK_DOWN] = KEY_DOWN,
    keyCodeLUT[VK_LEFT] = KEY_LEFT,
    keyCodeLUT[VK_RIGHT] = KEY_RIGHT,
    keyCodeLUT[VK_BACK] = KEY_BACKSPACE,
    keyCodeLUT[VK_RETURN] = KEY_RETURN,
    keyCodeLUT[VK_DELETE] = KEY_DELETE,
    keyCodeLUT[VK_INSERT] = KEY_INSERT,
    keyCodeLUT[VK_HOME] = KEY_HOME,
    keyCodeLUT[VK_END] = KEY_END,
    keyCodeLUT[VK_PRIOR] = KEY_PAGE_UP,
    keyCodeLUT[VK_NEXT] = KEY_PAGE_DOWN,
    keyCodeLUT[VK_CAPITAL] = KEY_CAPS_LOCK,
    keyCodeLUT[VK_NUMLOCK] = KEY_NUM_LOCK,
    keyCodeLUT[VK_SCROLL] = KEY_SCROLL_LOCK,
    keyCodeLUT[VK_APPS] = KEY_MENU,
    keyCodeLUT[VK_SHIFT] = KEY_SHIFT,
    keyCodeLUT[VK_LSHIFT] = KEY_SHIFT,
    keyCodeLUT[VK_RSHIFT] = KEY_SHIFT,
    keyCodeLUT[VK_CONTROL] = KEY_CONTROL,
    keyCodeLUT[VK_LCONTROL] = KEY_CONTROL,
    keyCodeLUT[VK_RCONTROL] = KEY_CONTROL,
    keyCodeLUT[VK_MENU] = KEY_ALT,
    keyCodeLUT[VK_LMENU] = KEY_ALT,
    keyCodeLUT[VK_RMENU] = KEY_ALT,
    keyCodeLUT[VK_F1] = KEY_F1;
    keyCodeLUT[VK_F2] = KEY_F2;
    keyCodeLUT[VK_F3] = KEY_F3;
    keyCodeLUT[VK_F4] = KEY_F4;
    keyCodeLUT[VK_F5] = KEY_F5;
    keyCodeLUT[VK_F6] = KEY_F6;
    keyCodeLUT[VK_F7] = KEY_F7;
    keyCodeLUT[VK_F8] = KEY_F8;
    keyCodeLUT[VK_F9] = KEY_F9;
    keyCodeLUT[VK_F10] = KEY_F10;
    keyCodeLUT[VK_F11] = KEY_F11;
    keyCodeLUT[VK_F12] = KEY_F12;
    keyCodeLUT[VK_NUMPAD0] = KEY_NUMPAD_0;
    keyCodeLUT[VK_NUMPAD1] = KEY_NUMPAD_1;
    keyCodeLUT[VK_NUMPAD2] = KEY_NUMPAD_2;
    keyCodeLUT[VK_NUMPAD3] = KEY_NUMPAD_3;
    keyCodeLUT[VK_NUMPAD4] = KEY_NUMPAD_4;
    keyCodeLUT[VK_NUMPAD5] = KEY_NUMPAD_5;
    keyCodeLUT[VK_NUMPAD6] = KEY_NUMPAD_6;
    keyCodeLUT[VK_NUMPAD7] = KEY_NUMPAD_7;
    keyCodeLUT[VK_NUMPAD8] = KEY_NUMPAD_8;
    keyCodeLUT[VK_NUMPAD9] = KEY_NUMPAD_9;
}