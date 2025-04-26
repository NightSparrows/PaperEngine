#pragma once


namespace PaperEngine {

	class Timestep
	{
	public:

		template<typename Rep, typename Period>
		Timestep(std::chrono::duration<Rep, Period> duration)
			: m_time(std::chrono::duration_cast<std::chrono::nanoseconds>(duration))
		{
		}

		float to_seconds() {
			return m_time.count() * 1e-9f;
		}

	private:
		std::chrono::nanoseconds m_time;
	};

}
