#pragma once

#include <chrono>

#include <PaperEngine/core/Timestep.h>

namespace PaperEngine {

	class Clock {
	public:
		Clock() {
			m_startTime = std::chrono::high_resolution_clock::now();
		}

		Timestep resetClock() {
			auto current_time = std::chrono::high_resolution_clock::now();
			auto delta_time = current_time - m_startTime;
			m_startTime = current_time;
			return Timestep(delta_time);
		}

		/// <summary>
		/// Get the delta time 
		/// Not reset the clock.
		/// </summary>
		/// <returns></returns>
		Timestep getDeltaTime() const {
			auto current_time = std::chrono::high_resolution_clock::now();
			return Timestep(current_time - m_startTime);
		}

	private:

		std::chrono::steady_clock::time_point m_startTime;
	};

}