#include "SnapLib2D.h"
#include <windows.h>
#include <GL/gl.h>
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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
        if (!f.is_open()) return "";
        return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    }

    std::vector<unsigned char> File::LoadPNG(const std::string& path, int& w, int& h) {
        int channels;
        unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 4);
        if (!data) return {};
        std::vector<unsigned char> result(data, data + (w * h * 4));
        stbi_image_free(data);
        return result;
    }

    std::vector<unsigned char> File::LoadWAV(const std::string& path, int& freq) { return {}; }

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

    void Window::Update(float& fps) {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT || msg.message == WM_CLOSE) m_running = false;
            
            // Key States
            if (msg.message == WM_KEYDOWN) Key::s_states[msg.wParam] = true;
            if (msg.message == WM_KEYUP)   Key::s_states[msg.wParam] = false;

            // Mouse States
            if (msg.message == WM_LBUTTONDOWN) Mouse::s_btns[0] = true;
            if (msg.message == WM_LBUTTONUP)   Mouse::s_btns[0] = false;
            if (msg.message == WM_MOUSEMOVE) {
                Mouse::s_x = (float)LOWORD(msg.lParam);
                Mouse::s_y = (float)HIWORD(msg.lParam);
            }
            
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // FPS Calculation Logic
        static auto lastTime = std::chrono::steady_clock::now();
        static int frames = 0;
        frames++;

        auto currentTime = std::chrono::steady_clock::now();
        std::chrono::duration<float> elapsed = currentTime - lastTime;
        if (elapsed.count() >= 1.0f) {
            fps = (float)frames;
            frames = 0;
            lastTime = currentTime;
        }
    }

    void Window::Display() {
        SwapBuffers((HDC)m_hdc);
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
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
    }

} // namespace sl2
