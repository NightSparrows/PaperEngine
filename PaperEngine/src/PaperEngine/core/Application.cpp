

#include <nvrhi/nvrhi.h>
#include "Application.h"

#include <PaperEngine/core/Assert.h>
#include <PaperEngine/core/Logger.h>
#include <PaperEngine/events/ApplicationEvent.h>
#include <PaperEngine/utils/Clock.h>

namespace PaperEngine {

	Application* Application::s_instance = nullptr;

	Application::Application(const ApplicationProps& props) :
		m_renderAPI(props.renderAPI)
	{
		PE_CORE_ASSERT(!s_instance, "Application already created.");
		s_instance = this;

		m_window = Window::Create(WindowProps(props.name, props.width, props.height));
		m_window->init();
		m_window->setEventCallback(PE_BIND_EVENT_FN(Application::onEvent));
	}

	Application::~Application()
	{
	}

	PE_API void Application::run()
	{
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
				for (auto layer : m_layerManager) {
					layer->onUpdate(deltaTime); // TODO 修改
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
						cmd->open();

						cmd->setTextureState(m_graphicsContext->getCurrentSwapchainTexture(),
							nvrhi::TextureSubresourceSet(),
							nvrhi::ResourceStates::Present);

						cmd->close();
						m_graphicsContext->getNVRhiDevice()->executeCommandList(cmd);

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
		// TODO do the application event handling
		EventDispatcher dispatcher(e);
#ifdef PE_DEBUG
		dispatcher.dispatch<WindowCloseEvent>([this](Event& e) {
			m_running = false;
			return true;
			});
#endif // PE_DEBUG
		// TODO: Do the imgui event first
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

}
