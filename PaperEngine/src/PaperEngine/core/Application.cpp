

#include <nvrhi/nvrhi.h>
#include "Application.h"

#include <PaperEngine/core/Assert.h>
#include <PaperEngine/core/Logger.h>
#include <PaperEngine/events/ApplicationEvent.h>
#include <PaperEngine/utils/Clock.h>

#include <PaperEngine/debug/Instrumentor.h>

namespace PaperEngine {

	Application* Application::s_instance = nullptr;

	Application::Application(const ApplicationProps& props) :
		m_renderAPI(props.renderAPI)
	{
		PE_CORE_ASSERT(!s_instance, "Application already created.");
		s_instance = this;

		initThreadPool();

		m_window = Window::Create(WindowProps(props.name, props.width, props.height));
		m_window->init();
		m_window->setEventCallback(PE_BIND_EVENT_FN(Application::onEvent));
	}

	Application::~Application()
	{
	}

	PE_API void Application::run()
	{
		PE_PROFILE_FUNCTION();

		m_graphicsContext = GraphicsContext::Create(m_window.get());
		m_graphicsContext->setOnBackBufferResizedCallback(PE_BIND_EVENT_FN(Application::onBackBufferResized));
		m_graphicsContext->setOnBackBufferResizingCallback(PE_BIND_EVENT_FN(Application::onBackBufferResizing));
		m_graphicsContext->init();

		m_resourceManager = CreateScope<ResourceManager>();

#ifdef PE_ENABLE_IMGUI
		m_imguiLayer = ImGuiLayer::Create();
		m_layerManager.pushOverlay(m_imguiLayer.get());
#endif // PE_ENABLE_IMGUI


		m_running = true;
		onInit();

		// 測試用Command
		auto cmd = m_graphicsContext->getNVRhiDevice()->createCommandList();
		Clock clock;

		Timestep FPSCounter(std::chrono::seconds(0));

		while (m_running) {
			PE_PROFILE_SCOPE("RunLoop");

			auto deltaTime = clock.resetClock();
			FPSCounter += deltaTime;

			if (FPSCounter.toSeconds() >= 1.0f) {
				// 每秒更新一次FPS
				m_window->setTitle(fmt::format("Sandbox - FPS: {}", m_framePerSecond));
				FPSCounter = Timestep(std::chrono::seconds(0));
				m_framePerSecond = 0;
			}
			// update

			{
				m_window->onUpdate();
				// Update logic, input handling, etc.
				{
					PE_PROFILE_SCOPE("Layers Update");
					for (auto layer : m_layerManager) {
						layer->onUpdate(deltaTime);
					}
				}
			}

			// Render
			{
				if (m_window->getWidth() != 0 && m_window->getHeight() != 0) {
					if (m_graphicsContext->beginFrame()) {
						// Render

						// TODO commit無關swapchain image的繪製
						for (auto layer : m_layerManager) {
							layer->onPreRender();
						}

						m_graphicsContext->waitForSwapchainImageAvailable();

						// TODO commit繪製swpachain image的命令
						for (auto layer : m_layerManager) {
#ifdef PE_ENABLE_IMGUI
							// Imgui 在preRender begin，然後他又是最後render的
							// 所以他能夠處理
							layer->onImGuiRender();
#endif // PE_ENABLE_IMGUI

							layer->onFinalRender(m_graphicsContext->getCurrentFramebuffer());

						}

#pragma region Present this frame
						//cmd->open();

						//cmd->setTextureState(m_graphicsContext->getCurrentSwapchainTexture(),
						//	nvrhi::AllSubresources,
						//	nvrhi::ResourceStates::Present);

						//cmd->close();
						//m_graphicsContext->getNVRhiDevice()->executeCommandList(cmd);

						if (m_graphicsContext->present()) {
							m_framePerSecond++;
						}
#pragma endregion

					}
				}
			}

			m_graphicsContext->getNVRhiDevice()->runGarbageCollection();
		}

		cmd = nullptr;

		m_layerManager.cleanUp();

		m_resourceManager.reset();

		m_graphicsContext->cleanUp();

		m_window->cleanUp();
	}

	nvrhi::IDevice* Application::GetNVRHIDevice() {
		PE_CORE_ASSERT(s_instance, "Application instance is null, cannot get NVRHI device.");
		return s_instance->m_graphicsContext->getNVRhiDevice();
	}

	PE_API Ref<BS::thread_pool<>> Application::GetThreadPool()
	{
		PE_CORE_ASSERT(s_instance, "Application instance is null, cannot get Thread pool.");
		return s_instance->m_thread_pool;
	}

	void Application::Shutdown()
	{
		PE_CORE_ASSERT(s_instance, "Application instance is null, cannot shutdown.");
		if (!s_instance) {
			PE_CORE_ERROR("Application instance is null, cannot shutdown.");
			return;
		}
		s_instance->m_running = false;
	}

	PE_API ResourceManager* Application::GetResourceManager()
	{
		PE_CORE_ASSERT(s_instance->m_resourceManager, "ResourceManager is not created. Application not run?");
		return s_instance->m_resourceManager.get();
	}

	void Application::onEvent(Event& e)
	{
		for (auto it = m_layerManager.rbegin(); it != m_layerManager.rend(); ++it)
		{
			Layer* layer = *it;
			layer->onEvent(e);
			if (e.Handled) {
				break; // 如果事件已經被處理，則不再繼續傳遞
			}
		}
	}

	void Application::onBackBufferResizing()
	{
		for (auto layer : m_layerManager) {
			layer->onBackBufferResizing();
		}
	}

	void Application::onBackBufferResized()
	{
		for (auto layer : m_layerManager) {
			layer->onBackBufferResized();
		}
	}

	void Application::initThreadPool()
	{
		uint32_t core_count = std::thread::hardware_concurrency();
		std::vector<bool> affinity(std::thread::hardware_concurrency(), false);
#ifdef PE_PLATFORM_WINDOWS


		struct CoreInfo {
			DWORD id;
			DWORD efficiency;
			BYTE coreIndex;
			BYTE schedulingClass;
		};

		DWORD len = 0;
		GetSystemCpuSetInformation(nullptr, 0, &len, GetCurrentProcess(), 0);

		std::vector<char> buffer(len);
		if (!GetSystemCpuSetInformation((SYSTEM_CPU_SET_INFORMATION*)buffer.data(), len, &len, GetCurrentProcess(), 0)) {
			PE_CORE_ERROR("GetSystemCpuSetInformation failed");
		}

		std::vector<CoreInfo> pcs, ecs;

		size_t offset = 0;
		while (offset < len) {
			auto* info = (SYSTEM_CPU_SET_INFORMATION*)((char*)buffer.data() + offset);

			if (info->Type == CpuSetInformation) {
				const auto& s = info->CpuSet;
				std::cout << "Core Id: " << s.Id << " logical processor index: " << (uint32_t)s.LogicalProcessorIndex << " EfficiencyClass: " << (uint32_t)s.EfficiencyClass
					<< " CoreIndex: " << (uint32_t)s.CoreIndex << " SchedulingClass: " << (uint32_t)s.SchedulingClass << "\n";
				// 0: E-core, 1: P-core
				if (s.EfficiencyClass == 1) {
					pcs.emplace_back(s.Id, s.EfficiencyClass, s.CoreIndex, s.SchedulingClass);
					affinity[s.CoreIndex] = true;
				}
				else
					ecs.emplace_back(s.Id, s.EfficiencyClass, s.CoreIndex, s.SchedulingClass);
			}

			offset += info->Size;
		}

		std::stringstream ss;
		ss << "[";
		for (size_t i = 0; i < affinity.size(); ++i) {
			ss << (affinity[i] ? "1" : "0");
			if (i + 1 != affinity.size()) ss << ", ";
		}
		ss << "]";

		PE_CORE_TRACE("Affinity content: {}", ss.str());
		std::cout << "P-cores:\n";
		for (auto& c : pcs) std::cout << "    class: " << c.efficiency << " index: " << (uint32_t)c.coreIndex << " sch class: " << (uint32_t)c.schedulingClass  << "\n";
		std::cout << "\nE-cores:\n";
		for (auto& c : ecs) std::cout << "    class: " << c.efficiency << " index: " << (uint32_t)c.coreIndex << " sch class: " << (uint32_t)c.schedulingClass << "\n";
		std::cout << "\n";

		// 只使用P-cores
		core_count = static_cast<uint32_t>(pcs.size());
		PE_CORE_TRACE("Using {} P-cores for Thread Pool", core_count);

		m_thread_pool = std::make_shared<BS::thread_pool<>>(core_count, [core_count, affinity](std::size_t idx) {
			PE_CORE_TRACE("ThreadPoolWorker header {}", idx);

			BS::this_thread::set_os_thread_affinity(affinity);
			});
#elifdef PE_PLATFORM_LINUX
		
		m_thread_pool = std::make_shared<BS::thread_pool<>>(core_count);
#endif // PE_PLATFORM_WINDOWS

	}

}
