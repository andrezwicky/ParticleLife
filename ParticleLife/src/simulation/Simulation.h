#pragma once

#include <cstdint>
#include <random>
#include <vector>

#include "raylib.h"

namespace PLife
{
    class World;
    class Renderer;

    struct Particle
    {
        Vector2 Position;
        Vector2 Velocity;
        uint8_t Type;       // index into the rule matrix / color table
    };

    struct SimulationSettings
    {
        int   ParticleCount   = 2000;
        int   TypeCount       = 6;
        float InteractionRadius = 80.0f;  // rMax: particles further apart than this ignore each other (world units)
        float RepulsionRange  = 0.3f;     // beta: fraction of rMax below which particles always repel
        float ForceFactor     = 10.0f;    // overall force strength
        float FrictionHalfLife = 0.04f;   // seconds until velocity halves without forces
        float ParticleRadius  = 2.5f;     // drawing only (world units)
    };

    // Particle Life: particles of different types attract/repel each other according
    // to a (non-symmetric) rule matrix. The world wraps around at its edges (torus).
    // Keeps a reference to the world: the world must outlive the simulation.
    class Simulation
    {
    public:
        Simulation(const World& world, const SimulationSettings& settings = SimulationSettings());

        Simulation(const Simulation&) = delete;
        Simulation& operator=(const Simulation&) = delete;

        // Advance by a fixed time step (call from a fixed-timestep loop).
        void Step(float dt);

        // World space (call between Renderer::BeginFrame/EndFrame).
        void Render(Renderer& renderer) const;

        // New random attraction values in [-1, 1].
        void RandomizeRules();
        // Replace all particles with SimulationSettings::ParticleCount random ones.
        void RespawnParticles();
        void Spawn(Vector2 position, uint8_t type);

        // rules[a][b]: how strongly type a is attracted to (+) / repelled by (-) type b.
        float GetRule(int a, int b) const { return m_Rules[a * m_Settings.TypeCount + b]; }
        void  SetRule(int a, int b, float value) { m_Rules[a * m_Settings.TypeCount + b] = value; }

        const std::vector<Particle>& GetParticles() const { return m_Particles; }
        const SimulationSettings& GetSettings() const { return m_Settings; }
        Color GetTypeColor(int type) const { return m_TypeColors[type]; }

    private:
        float Force(float normalizedDistance, float attraction) const;
        void  BuildGrid();

    private:
        const World& m_World;
        SimulationSettings m_Settings;

        std::vector<Particle> m_Particles;
        std::vector<float>    m_Rules;      // TypeCount * TypeCount, row = acting type
        std::vector<Color>    m_TypeColors;

        // Uniform grid for neighbor search; cells are at least InteractionRadius wide,
        // so all neighbors of a particle are in its own or the 8 surrounding cells.
        int m_CellsX = 1;
        int m_CellsY = 1;
        float m_CellWidth = 0.0f;
        float m_CellHeight = 0.0f;
        std::vector<int> m_CellStart;       // CellsX * CellsY + 1, offsets into m_CellParticles
        std::vector<int> m_CellParticles;   // particle indices sorted by cell
        std::vector<int> m_ParticleCell;    // cell of each particle

        std::vector<Vector2> m_Accelerations;

        std::mt19937 m_Rng;
    };
}
