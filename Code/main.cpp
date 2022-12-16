// Simple basecode showing how to create a window and attatch a vulkansurface
#define GATEWARE_ENABLE_CORE // All libraries need this
#define GATEWARE_ENABLE_SYSTEM // Graphics libs require system level libraries
#define GATEWARE_ENABLE_GRAPHICS // Enables all Graphics Libraries
#define GATEWARE_ENABLE_MATH
#define GATEWARE_ENABLE_INPUT
// TODO: Part 3a
// Ignore some GRAPHICS libraries we aren't going to use
#define GATEWARE_DISABLE_GDIRECTX11SURFACE // we have another template for this
#define GATEWARE_DISABLE_GDIRECTX12SURFACE // we have another template for this
#define GATEWARE_DISABLE_GRASTERSURFACE // we have another template for this
#define GATEWARE_DISABLE_GOPENGLSURFACE // we have another template for this
// With what we want & what we don't defined we can include the API
#include "../Gateware/Gateware.h"
#include "h2bParser.h"
#include "LoadLevelData.h"
#include "ManageRenderer.h"
#include "Model.h"
#include "renderer.h"

#include <fstream>

// open some namespaces to compact the code a bit
using namespace GW;
using namespace CORE;
using namespace SYSTEM;
using namespace GRAPHICS;
// lets pop a window and use Vulkan to clear to a red screen
int main()
{
	GWindow win;
	GEventResponder msgs;
	GVulkanSurface vulkan;
	if (+win.Create(0, 0, 1920, 1080, GWindowStyle::FULLSCREENBORDERED))
	{
		// TODO: Part 1a
		win.SetWindowName("Ethan Stanks Vulkan Level Renderer");
		VkClearValue clrAndDepth[2];
		clrAndDepth[0].color = { {0, 0, 0, 0} };
		clrAndDepth[1].depthStencil = { 1.0f, 0u };
		msgs.Create([&](const GW::GEvent& e) {
			GW::SYSTEM::GWindow::Events q;
			if (+e.Read(q) && q == GWindow::Events::RESIZE)
				clrAndDepth[0].color.float32[2] += 0.01f; // disable
			});
		win.Register(msgs);
#ifndef NDEBUG
		const char* debugLayers[] = {
			"VK_LAYER_KHRONOS_validation", // standard validation layer
			//"VK_LAYER_LUNARG_standard_validation", // add if not on MacOS
			//"VK_LAYER_RENDERDOC_Capture" // add this if you have installed RenderDoc
		};
		if (+vulkan.Create(	win, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT | DEPTH_STENCIL_SUPPORT | MSAA_8X_SUPPORT,
							sizeof(debugLayers)/sizeof(debugLayers[0]),
							debugLayers, 0, nullptr, 0, nullptr, false))
#else
		if (+vulkan.Create(win, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT))
#endif
		{
			Renderer renderer(win, vulkan);
			while (+win.ProcessWindowEvents())
			{
				if (+vulkan.StartFrame(2, clrAndDepth))
				{
					renderer.UpdateCamera();
					renderer.Render();
					vulkan.EndFrame(true);
				}
			}
		}
	}
	return 0; // that's all folks
}
