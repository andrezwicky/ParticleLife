#pragma once

#include <memory>
#include <string>

#include "Game.h"

namespace PLife
{
    struct ApplicationSpecification
    {
        // Initial window size in logical pixels (scaled by the monitor's DPI at startup).
        int Width = 1280;
        int Height = 800;
        std::string Title = "Particle Life";
    };

    class Application
    {
    public:
        Application(const ApplicationSpecification& spec = ApplicationSpecification());
        ~Application();

        void Run();
        void OnUpdate();
        void OnRender();
        void OnImGuiRender();

    public:
        static Application& Get();

    private:
        void Loop();

    private:
        ApplicationSpecification m_Specification;
        std::unique_ptr<Game> m_Game;

        float m_Time = 0.0f;
    };
}
