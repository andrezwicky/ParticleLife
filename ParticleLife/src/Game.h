#pragma once

#include <memory>

#include "world/world.h"

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

    private:
        void HandleViewInput();

    private:
        World m_World;
        std::unique_ptr<Renderer> m_Renderer;
    };
}
