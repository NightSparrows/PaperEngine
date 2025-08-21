#pragma once

#include <chrono>

namespace PaperEngine {

	class Timestep
	{
	public:

		template<typename Rep, typename Period>
		Timestep(std::chrono::duration<Rep, Period> duration)
			: m_time(std::chrono::duration_cast<std::chrono::nanoseconds>(duration))
		{
		}

		float toSeconds() const {
			return m_time.count() * 1e-9f;
		}

		Timestep& operator+= (const Timestep& other) {
			m_time += other.m_time;
			return *this;
		}

	private:
		std::chrono::nanoseconds m_time;
	};

}
