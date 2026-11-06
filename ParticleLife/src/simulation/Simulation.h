#pragma once

#include <algorithm>
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
        float   EddyDiffusion;  // how strongly this particle reacts to temperature (world units^2/s at temperature 1)
        uint8_t Type;           // index into the rule matrix / color table
    };

    // Force settings of one particle type. They apply to the type's particles as the
    // ones reacting: type a feels type b within a's radius, shaped by a's repulsion range.
    struct TypeSettings
    {
        float InteractionRadius = 80.0f;  // rMax: how far particles of this type sense others (world units)
        float RepulsionRange    = 0.3f;   // beta: fraction of rMax below which they always repel
    };

    struct SimulationSettings
    {
        // Particles
        int   ParticleCount     = 2000;
        int   TypeCount         = 6;

        // Forces (interaction radius and repulsion range are per type, see TypeSettings)
        float ForceFactor       = 10.0f;  // overall force strength
        float FrictionHalfLife  = 0.04f;  // seconds until velocity halves without forces

        // Temperature: random thermal motion. Each step a particle is displaced by
        // N(0,1) * sqrt(2 * D * dt) per axis, with D = Temperature * particle.EddyDiffusion.
        float Temperature       = 1.0f;
        float EddyDiffusionMin  = 0.0f;   // per-particle factor is drawn uniformly from [min, max]
        float EddyDiffusionMax  = 20.0f;

        // Display
        float ParticleRadius    = 2.5f;   // world units
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
        // Spawns with a random eddy diffusion factor from the settings' range.
        void Spawn(Vector2 position, uint8_t type);
        // Draw a new eddy diffusion factor for every particle from the settings' range.
        void RerollEddyDiffusion();

        const TypeSettings& GetTypeSettings(int type) const { return m_TypeSettings[type]; }
        // Applies live; rebuilds the neighbor grid if the largest radius changes.
        void SetTypeSettings(int type, const TypeSettings& settings);
        // All types back to TypeSettings defaults.
        void ResetTypeSettings();

        // Applies changed settings live: resizes the rule matrix and type settings (keeping existing ones),
        // adds/removes particles, rebuilds the neighbor grid and rerolls eddy factors as needed.
        void SetSettings(const SimulationSettings& settings);

        // rules[a][b]: how strongly type a is attracted to (+) / repelled by (-) type b.
        float GetRule(int a, int b) const { return m_Rules[a * m_Settings.TypeCount + b]; }
        void  SetRule(int a, int b, float value) { m_Rules[a * m_Settings.TypeCount + b] = std::clamp(value, -1.0f, 1.0f); }
        void  ClearRules() { std::fill(m_Rules.begin(), m_Rules.end(), 0.0f); }

        const std::vector<Particle>& GetParticles() const { return m_Particles; }
        const SimulationSettings& GetSettings() const { return m_Settings; }
        Color GetTypeColor(int type) const { return m_TypeColors[type]; }

    private:
        static float Force(float normalizedDistance, float attraction, float repulsionRange);
        float RandomEddyDiffusion();
        void  SetupTypes();     // colors, rule matrix and type settings sized to TypeCount
        void  SetupGrid();      // cell layout from the largest interaction radius
        void  BuildGrid();      // sort particles into cells

    private:
        const World& m_World;
        SimulationSettings m_Settings;

        std::vector<Particle> m_Particles;
        std::vector<float>    m_Rules;      // TypeCount * TypeCount, row = acting type
        std::vector<Color>    m_TypeColors;
        std::vector<TypeSettings> m_TypeSettings;

        // Uniform grid for neighbor search; cells are at least as wide as the largest interaction radius,
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
