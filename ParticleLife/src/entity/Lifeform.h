#pragma once
#include <raylib.h>
#include <vector>

#include "Types.h"

namespace PLife
{
	struct Particle
	{
		float	Size;
		Vector2	Position;
		Vector2	Direction;
		Vector2	Velocity;
		Vector2	Acceleration;
		Color	Color;
	};

	class LifeForm
	{
	public:
		LifeForm() = default;
		~LifeForm() = default;
		
		virtual void Update(float deltaTime) = 0;
		void Render();
		
		void EmplaceParticle(const Particle& particle);

	protected:
		std::vector<Particle> m_Particles;
	};
}	