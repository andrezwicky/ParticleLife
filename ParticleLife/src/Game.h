#pragma once

#include <memory>

#include "world/world.h"
#include "simulation/Simulation.h"

namespace PLife
{
    class Renderer;

    // Owns the simulation: renderer, entities/particles, world state.
    // Created by Application after the window (and GPU context) exists,
    // and destroyed before the window closes, so it may load and free
    // GPU resources in its constructor/destructor.
    class Game
    {
    public:
        Game();
        ~Game();

        Game(const Game&) = delete;
        Game& operator=(const Game&) = delete;

        void OnUpdate(float deltaTime);
        void OnRender();
        void OnImGuiRender();

    private:
        void HandleViewInput();
        void HandleSimulationInput();
        void DrawRulesPanel();

    private:
        // Declaration order matters: Simulation and Renderer reference the World.
        World m_World;
        Simulation m_Simulation;
        std::unique_ptr<Renderer> m_Renderer;

        // Fixed timestep: the simulation always advances in steps of s_StepTime,
        // independent of the frame rate.
        float m_StepAccumulator = 0.0f;
        bool m_Paused = false;
        bool m_ShowRulesPanel = true;
    };
}
