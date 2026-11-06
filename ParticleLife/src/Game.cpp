#include "Game.h"
#include "graphic/Renderer.h"
#include "entity/Lifeform.h"

#include <cmath>
#include <entity/Eddy_Particle.h>

namespace PLife
{
    Game::Game()
        : m_World(1600.0f, 1000.0f), m_Renderer(std::make_unique<Renderer>(m_World))
    {

    }

    Game::~Game()
    {
    }

    void Game::OnUpdate(float deltaTime)
    {
        (void)deltaTime;
        HandleViewInput();
    }

    void Game::OnRender()
    {
        m_Renderer->BeginFrame();

        m_World.Render(*m_Renderer);

        // Placeholder so there is something on screen; replace with particles.
        m_Renderer->DrawText("Particle Life", { 20.0f, 20.0f }, 40.0f, RAYWHITE);

        m_Renderer->EndFrame();

        m_Renderer->DrawScreenText("Wheel: zoom   Right drag: pan   R: reset view", 10.0f, m_Renderer->GetScreenSize().y - 26.0f, 16.0f, GRAY);
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
}
