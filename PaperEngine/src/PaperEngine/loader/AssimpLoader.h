#pragma once

#include <PaperEngine/renderer/Model.h>

namespace PaperEngine {

	class AssimpLoader {
	public:


		static ModelHandle LoadModel(const std::string& path);


	};

}
