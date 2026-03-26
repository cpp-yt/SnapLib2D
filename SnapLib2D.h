#ifndef SNAPLIB2D_H
#define SNAPLIB2D_H

#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <iostream>

namespace sl2 {

    struct Color {
        float r, g, b, a;
        Color(float r = 0, float g = 0, float b = 0, float a = 1) : r(r), g(g), b(b), a(a) {}
    };

    class Time { public: float s; float AsSeconds() const { return s; } };
    class Clock {
    public:
        Clock();
        Time Restart();
    private:
        long long m_last;
    };

    class File {
    public:
        static std::string Include(const std::string& path);
        static std::vector<unsigned char> LoadPNG(const std::string& path, int& w, int& h);
        static std::vector<unsigned char> LoadWAV(const std::string& path, int& freq);
    };

    class Key {
    public:
        enum Code {
            None = 0, Backspace = 8, Tab = 9, Enter = 13, Escape = 27, Space = 32,
            Left = 37, Up = 38, Right = 39, Down = 40,
            Num0 = 48, Num1 = 49, Num2 = 50, Num3 = 51, Num4 = 52,
            Num5 = 53, Num6 = 54, Num7 = 55, Num8 = 56, Num9 = 57,
            A = 65, B = 66, C = 67, D = 68, E = 69, F = 70, G = 71, H = 72,
            I = 73, J = 74, K = 75, L = 76, M = 77, N = 78, O = 79, P = 80,
            Q = 81, R = 82, S = 83, T = 84, U = 85, V = 86, W = 87, X = 88,
            Y = 89, Z = 90
        };
        static bool IsDown(Code code);
    private:
        static bool s_states[512];
        friend class Window;
    };

    class Mouse {
    public:
        static bool IsBtnDown(int btn);
        static void GetPos(float& x, float& y);
    private:
        static bool s_btns[5];
        static float s_x, s_y;
        friend class Window;
    };

    class Window {
    public:
        Window(int w, int h, std::string title);
        ~Window();
        bool IsOpen() const;
        void Clear(Color c);
        void Display();
        void Update(float& fps); // Added Update logic
    private:
        void* m_hwnd; void* m_hdc; void* m_hrc; bool m_running;
    };

    class Entity {
    public:
        Entity(float x, float y) : x(x), y(y), m_texID(0) {}
        virtual ~Entity() = default;
        void Load(const std::string& path);
        virtual void Update(Time dt) = 0;
        virtual void Draw() = 0;
    protected:
        float x, y; unsigned int m_texID;
    };

    class Background { 
    public: 
        void SetImage(std::string path); 
        void Draw(); 
    private: 
        unsigned int m_texID = 0; 
    };

    class Sound {
    public:
        Sound(std::string path);
        void Play();
    private:
        unsigned int m_bufID = 0;
    };

} // namespace sl2

#endif
