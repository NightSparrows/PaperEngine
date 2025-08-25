#pragma once

#include <nvrhi/nvrhi.h>

#include <PaperEngine/core/Base.h>

namespace PaperEngine {

	struct BindingLayout {
		nvrhi::BindingLayoutHandle handle;
	};

	typedef Ref<BindingLayout> BindingLayoutHandle;
}
