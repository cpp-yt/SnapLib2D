#include "SnapLib2D.h"
#include <windows.h>
#include <GL/gl.h>
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" // Requires stbi_image.h in your project folder

namespace sl2 {
    // Static Initialization
    bool Key::s_states[512] = { false };
    bool Mouse::s_btns[5] = { false };
    float Mouse::s_x = 0; float Mouse::s_y = 0;

    // --- CLOCK & TIME ---
    Clock::Clock() { m_last = std::chrono::steady_clock::now().time_since_epoch().count(); }
    Time Clock::Restart() {
        long long now = std::chrono::steady_clock::now().time_since_epoch().count();
        float d = (float)(now - m_last) / 1000000000.0f; 
        m_last = now; 
        return Time{d};
    }

    // --- FILE SYSTEM ---
    std::string File::Include(const std::string& path) {
        std::ifstream f(path, std::ios::binary);
        if (!f.is_open()) { std::cerr << "sl2::File Error: File not found -> " << path << std::endl; return ""; }
        return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    }

    std::vector<unsigned char> File::LoadPNG(const std::string& path, int& w, int& h) {
        int channels;
        unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 4);
        if (!data) { std::cerr << "sl2::File Error: PNG not found -> " << path << std::endl; return {}; }
        std::vector<unsigned char> result(data, data + (w * h * 4));
        stbi_image_free(data);
        return result;
    }

    std::vector<unsigned char> File::LoadWAV(const std::string& path, int& freq) {
        std::ifstream f(path, std::ios::binary);
        if (!f.is_open()) { std::cerr << "sl2::File Error: WAV not found -> " << path << std::endl; return {}; }
        // Simple WAV Header parser logic would go here
        return {}; 
    }

    // --- INPUT ---
    bool Key::IsDown(Code code) { return s_states[(int)code]; }
    bool Mouse::IsBtnDown(int btn) { return s_btns[btn]; }
    void Mouse::GetPos(float& x, float& y) { x = s_x; y = s_y; }

    // --- WINDOW ---
    Window::Window(int w, int h, std::string title) : m_running(true) {
        HINSTANCE inst = GetModuleHandle(NULL);
        WNDCLASS wc = {0}; wc.lpfnWndProc = DefWindowProc; wc.hInstance = inst;
        wc.style = CS_OWNDC; wc.lpszClassName = "SL2"; RegisterClass(&wc);

        HWND native = CreateWindowA("SL2", title.c_str(), WS_OVERLAPPEDWINDOW | WS_VISIBLE, 
                                     100, 100, w, h, NULL, NULL, inst, NULL);
        m_hwnd = (void*)native;
        m_hdc = (void*)GetDC(native);

        PIXELFORMATDESCRIPTOR pfd = { sizeof(pfd), 1, PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, PFD_TYPE_RGBA, 32 };
        int format = ChoosePixelFormat((HDC)m_hdc, &pfd);
        SetPixelFormat((HDC)m_hdc, format, &pfd);
        m_hrc = (void*)wglCreateContext((HDC)m_hdc);
        wglMakeCurrent((HDC)m_hdc, (HGLRC)m_hrc);
    }

    Window::~Window() { wglDeleteContext((HGLRC)m_hrc); m_running = false; }
    bool Window::IsOpen() const { return m_running; }
    void Window::Clear(Color c) { glClearColor(c.r, c.g, c.b, c.a); glClear(GL_COLOR_BUFFER_BIT); }

    void Window::Display() {
        SwapBuffers((HDC)m_hdc);
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_KEYDOWN) Key::s_states[msg.wParam] = true;
            if (msg.message == WM_KEYUP)   Key::s_states[msg.wParam] = false;
            if (msg.message == WM_QUIT)    m_running = false;
            DispatchMessage(&msg);
        }
    }

    // --- ENTITY ---
    void Entity::Load(const std::string& path) {
        int w, h;
        auto pixels = File::LoadPNG(path, w, h);
        if (!pixels.empty()) {
            glGenTextures(1, &m_texID);
            glBindTexture(GL_TEXTURE_2D, m_texID);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        }
    }

} // namespace sl2
