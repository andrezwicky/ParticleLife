#include "Game.h"
#include "graphic/Renderer.h"

#include <cmath>

namespace PLife
{
    static constexpr float s_StepTime = 1.0f / 60.0f;
    static constexpr int s_MaxStepsPerFrame = 4; // drop time instead of spiraling when a frame is slow

    Game::Game()
        : m_World(1600.0f, 1000.0f), m_Simulation(m_World), m_Renderer(std::make_unique<Renderer>(m_World))
    {
    }

    Game::~Game()
    {
    }

    void Game::OnUpdate(float deltaTime)
    {
        HandleViewInput();
        HandleSimulationInput();

        if (m_Paused)
            return;

        m_StepAccumulator += deltaTime;
        int steps = 0;
        while (m_StepAccumulator >= s_StepTime && steps < s_MaxStepsPerFrame)
        {
            m_Simulation.Step(s_StepTime);
            m_StepAccumulator -= s_StepTime;
            ++steps;
        }
        if (steps == s_MaxStepsPerFrame)
            m_StepAccumulator = 0.0f;
    }

    void Game::OnRender()
    {
        m_Renderer->BeginFrame();

        m_World.Render(*m_Renderer);
        m_Simulation.Render(*m_Renderer);

        m_Renderer->EndFrame();

        const float bottom = m_Renderer->GetScreenSize().y;
        m_Renderer->DrawScreenText(TextFormat("%d particles%s", (int)m_Simulation.GetParticles().size(), m_Paused ? "   PAUSED" : ""),
            10.0f, 10.0f, 18.0f, RAYWHITE);
        m_Renderer->DrawScreenText("Wheel: zoom   Right drag: pan   R: reset view   Space: pause   S: single step   N: new rules   P: respawn",
            10.0f, bottom - 26.0f, 16.0f, GRAY);
    }

    void Game::HandleViewInput()
    {
        const float wheel = GetMouseWheelMove();
        if (wheel != 0.0f)
            m_Renderer->ZoomAt(GetMousePosition(), std::pow(1.15f, wheel));

        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
            m_Renderer->Pan(GetMouseDelta());

        if (IsKeyPressed(KEY_R))
            m_Renderer->ResetView();
    }

    void Game::HandleSimulationInput()
    {
        if (IsKeyPressed(KEY_SPACE))
        {
            m_Paused = !m_Paused;
            m_StepAccumulator = 0.0f;
        }
        if (IsKeyPressed(KEY_S) && m_Paused)
            m_Simulation.Step(s_StepTime);
        if (IsKeyPressed(KEY_N))
            m_Simulation.RandomizeRules();
        if (IsKeyPressed(KEY_P))
            m_Simulation.RespawnParticles();
    }
}
