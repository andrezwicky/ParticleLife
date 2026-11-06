#pragma once
#include "Lifeform.h"

namespace PLife
{
	class Eddy_Particle : public LifeForm
	{
	public:
		Eddy_Particle() = default;
		~Eddy_Particle() = default;
		void Update(float deltaTime) override;
	};
}