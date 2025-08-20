#pragma once


namespace PaperEngine {

	class GraphicsContext {
	public:
		virtual ~GraphicsContext() = default;

		virtual void init() = 0;

		virtual void cleanUp() = 0;

	};

}
