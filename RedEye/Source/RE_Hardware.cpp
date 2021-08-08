#include "RE_Hardware.h"

#include "RE_Profiler.h"
#include "SDL2\include\SDL.h"
#include "imgui\imgui.h"
#include "gpudetect\DeviceId.h"
#include <Glew/glew.h>
#include <EAStdC/EAString.h>

#include <Psapi.h> // WINDOWS memory calls

#define KILOBYTE 1024.0
#define MEGABYTE 1048576.0
#define GIGABYTE 1073741824.0

void RE_Hardware::Init()
{
	RE_PROFILE(PROF_Init, PROF_Hardware);
	// CPU
	std::string brand, vendor;
	getCPUInfo(&brand, &vendor);
	(cpu_brand = "Brand: ") += brand.c_str();
	(cpu_vendor = "Vendor: ") += vendor.c_str();
	core_count = "Cores: " + eastl::to_string(SDL_GetCPUCount()) + " cores";

	// Caps - CPU supported features
	if (SDL_HasRDTSC()) caps1 += " RDTSC";
	if (SDL_HasAltiVec()) caps1 += " AltiVec";
	if (SDL_HasMMX()) caps1 += " MMX";
	if (SDL_Has3DNow()) caps1 += " 3DNow";
	if (SDL_HasAVX()) caps1 += " AVX";
	if (SDL_HasAVX2()) caps1 += " AVX2";
	if (SDL_HasNEON()) caps1 += " NEON (ARM SIMD)";
	if (SDL_HasSSE()) caps2 += " SSE";
	if (SDL_HasSSE2()) caps2 += " SSE2";
	if (SDL_HasSSE3()) caps2 += " SSE3";
	if (SDL_HasSSE41()) caps2 += " SSE41";
	if (SDL_HasSSE42()) caps2 += " SSE42";

	// Memory
	MBytesString(ram_capacity = "Total space: ", static_cast<double>(SDL_GetSystemRAM()));
	memset(mem, 0, sizeof(mem));

	// Display Drivers
	display_drivers = "2D display drivers: ";
	SDL_RendererInfo info;
	int driver_count = SDL_GetNumRenderDrivers();
	if (driver_count > 0 && SDL_GetRenderDriverInfo(0, &info) != -1)
	{
		display_drivers += info.name;
		for (int i = 1; i < driver_count && SDL_GetRenderDriverInfo(i, &info) == 0; i++)
			(display_drivers += ", ") += info.name;
	}
	else display_drivers += "empty";

	// GPU
	unsigned int VendorId, DeviceId; std::wstring GFXBrand;
	unsigned long long total, used, available, reserved;
	if (getGraphicsDeviceInfo(&VendorId, &DeviceId, &GFXBrand, &total, &used, &available, &reserved))
	{
		vendor_id = "Vendor ID: " + eastl::to_string(VendorId);
		device_id = "Device ID: " + eastl::to_string(DeviceId);
		(gfx_brand = "") += reinterpret_cast<const char*>(GFXBrand.c_str());
		gt_generation = "GT generation: " + eastl::to_string(static_cast<int>(getGTGeneration(DeviceId)));

		BytesString((vram_total = "Total VRAM: "), static_cast<double>(total));
		BytesString((vram_used = "VRAM used: "), static_cast<double>(used));
		BytesString((vram_available = "VRAM available: "), static_cast<double>(available));
		BytesString((vram_reserved = "VRAM reserved: "), static_cast<double>(reserved));
	}

	(gpu_renderer = "GPU: ") += reinterpret_cast<const char*>(glGetString(GL_RENDERER));
	(gpu_vendor = "Brand: ") += reinterpret_cast<const char*>(glGetString(GL_VENDOR));
}

void RE_Hardware::DrawEditor()
{
	// CPU
	if (ImGui::TreeNodeEx("CPU", ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow))
	{
		ImGui::Text(cpu_brand.c_str());
		ImGui::Text(cpu_vendor.c_str());
		ImGui::Text(core_count.c_str());
		ImGui::Text(caps1.c_str());
		ImGui::Text(caps2.c_str());
		ImGui::TreePop();
	}

	// Memory
	if (ImGui::TreeNodeEx("RAM", ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow))
	{
		PROCESS_MEMORY_COUNTERS memCounter;
		if (!pause_plotting && GetProcessMemoryInfo(GetCurrentProcess(), &memCounter, sizeof(memCounter)))
		{
			static eastl::string names[MAX_MEM_TYPES] = { "Working", "Pageable", "Non-Pageable", "Paged" };
			float values[MAX_MEM_TYPES][2] = {
				{ static_cast<float>(memCounter.WorkingSetSize / MEGABYTE),			static_cast<float>(memCounter.PeakWorkingSetSize / MEGABYTE) },
				{ static_cast<float>(memCounter.QuotaPagedPoolUsage / MEGABYTE),	static_cast<float>(memCounter.QuotaPeakPagedPoolUsage / MEGABYTE) },
				{ static_cast<float>(memCounter.QuotaNonPagedPoolUsage / MEGABYTE),	static_cast<float>(memCounter.QuotaPeakNonPagedPoolUsage / MEGABYTE) },
				{ static_cast<float>(memCounter.PagefileUsage / MEGABYTE),			static_cast<float>(memCounter.PeakPagefileUsage / MEGABYTE) } };

			for (unsigned short i = 0; i < MAX_MEM_TYPES; ++i)
			{
				float peak = values[i][0];
				for (int i2 = 0; i2 < 99; i2++)
				{
					float tmp[2] = { peak, mem[i][i2] };
					peak = tmp[mem[i][i2] > peak];
					mem[i][i2] = mem[i][i2 + 1];
				}

				mem[i][99] = values[i][0];

				// Plot
				eastl::string title = names[i];
				MBytesString(title + " Memory: ", static_cast<double>(mem[i][99]));
				ImGui::PlotHistogram((eastl::string("##") + names[i] + " Memory Usage").c_str(), mem[i], 100, 0, title.c_str(), 0.0f, 1.2f * peak, ImVec2(310, 100));

				// Max Peak
				title = "Highest peak: ";
				MBytesString(title, peak);
				ImGui::Text(title.c_str());
			}
		}

		if (ImGui::Checkbox(pause_plotting ? "Restart Plotting" : "Pause Plotting", &pause_plotting) && !pause_plotting)
			memset(mem, 0, sizeof(mem));

		ImGui::TreePop();
	}

	// Display Drivers
	if (ImGui::TreeNodeEx("Display", ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow))
	{
		ImGui::Text(display_drivers.c_str());
		ImGui::TreePop();
	}

	// GPU
	if (ImGui::TreeNodeEx("GPU", ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow))
	{
		// GPU
		ImGui::Text(gpu_renderer.c_str());
		ImGui::Text(gpu_vendor.c_str());
		//ImGui::TextWrappedV(gpu_version.c_str(), "");
		//ImGui::Text(gpu_shading.c_str());
		ImGui::Separator();

		// VRAM
		if (ImGui::Button("Check VRAM"))
		{
			unsigned long long total, used, available, reserved;
			getGraphicsDeviceInfo(nullptr, nullptr, nullptr, &total, &used, &available, &reserved);
			MBytesString(vram_total = "Total VRAM: ", static_cast<double>(total));
			MBytesString(vram_used = "VRAM used: ", static_cast<double>(used));
			MBytesString(vram_available = "VRAM available: ", static_cast<double>(available));
			MBytesString(vram_reserved = "VRAM reserved: ", static_cast<double>(reserved));
		}

		ImGui::Text(vram_total.c_str());
		ImGui::Text(vram_used.c_str());
		ImGui::Text(vram_available.c_str());
		ImGui::Text(vram_reserved.c_str());

		ImGui::TreePop();
	}

}

void RE_Hardware::QueryData()
{
	// CPU
	std::string brand, vendor;
	getCPUInfo(&brand, &vendor);
	(cpu_brand = "Brand: ") += brand.c_str();
	(cpu_vendor = "Vendor: ") += vendor.c_str();
	core_count = "Cores: " + eastl::to_string(SDL_GetCPUCount()) + " cores";

	// Caps - CPU supported features
	if (SDL_HasRDTSC()) caps1 += " RDTSC";
	if (SDL_HasAltiVec()) caps1 += " AltiVec";
	if (SDL_HasMMX()) caps1 += " MMX";
	if (SDL_Has3DNow()) caps1 += " 3DNow";
	if (SDL_HasAVX()) caps1 += " AVX";
	if (SDL_HasAVX2()) caps1 += " AVX2";
	if (SDL_HasNEON()) caps1 += " NEON (ARM SIMD)";
	if (SDL_HasSSE()) caps2 += " SSE";
	if (SDL_HasSSE2()) caps2 += " SSE2";
	if (SDL_HasSSE3()) caps2 += " SSE3";
	if (SDL_HasSSE41()) caps2 += " SSE41";
	if (SDL_HasSSE42()) caps2 += " SSE42";

	// Memory
	MBytesString(ram_capacity = "Total space: ", static_cast<double>(SDL_GetSystemRAM()));
	memset(mem, 0, sizeof(mem));

	// Display Drivers
	display_drivers = "2D display drivers: ";
	SDL_RendererInfo info;
	int driver_count = SDL_GetNumRenderDrivers();
	if (driver_count > 0 && SDL_GetRenderDriverInfo(0, &info) != -1)
	{
		display_drivers += info.name;
		for (int i = 1; i < driver_count && SDL_GetRenderDriverInfo(i, &info) == 0; i++)
			(display_drivers += ", ") += info.name;
	}
	else display_drivers += "empty";

	// GPU
	unsigned int VendorId, DeviceId; std::wstring GFXBrand;
	unsigned long long total, used, available, reserved;
	if (getGraphicsDeviceInfo(&VendorId, &DeviceId, &GFXBrand, &total, &used, &available, &reserved))
	{
		vendor_id = "Vendor ID: " + eastl::to_string(VendorId);
		device_id = "Device ID: " + eastl::to_string(DeviceId);
		(gfx_brand = "") += reinterpret_cast<const char*>(GFXBrand.c_str());
		gt_generation = "GT generation: " + eastl::to_string(static_cast<int>(getGTGeneration(DeviceId)));

		BytesString((vram_total = "Total VRAM: "), static_cast<double>(total));
		BytesString((vram_used = "VRAM used: "), static_cast<double>(used));
		BytesString((vram_available = "VRAM available: "), static_cast<double>(available));
		BytesString((vram_reserved = "VRAM reserved: "), static_cast<double>(reserved));
	}

	(gpu_renderer = "GPU: ") += reinterpret_cast<const char*>(glGetString(GL_RENDERER));
	(gpu_vendor = "Brand: ") += reinterpret_cast<const char*>(glGetString(GL_VENDOR));
}

void RE_Hardware::BytesString(eastl::string& stat, const double val)
{
	if (val >= GIGABYTE) stat += eastl::to_string(static_cast<float>(val / GIGABYTE)) + " Gbs";
	else if (val >= MEGABYTE) stat += eastl::to_string(static_cast<float>(val / MEGABYTE)) + " Mbs";
	else if (val >= KILOBYTE) stat += eastl::to_string(static_cast<float>(val / KILOBYTE)) + " Kbs";
	else					  stat += eastl::to_string(val) + " bytes";
}

void RE_Hardware::KBytesString(eastl::string& stat, const double val)
{
	if (val >= MEGABYTE)	  stat += eastl::to_string(static_cast<float>(val / MEGABYTE)) + " Gbs";
	else if (val >= KILOBYTE) stat += eastl::to_string(static_cast<float>(val / KILOBYTE)) + " Mbs";
	else					  stat += eastl::to_string(val) + " Kbs";
}

void RE_Hardware::MBytesString(eastl::string& stat, const double val)
{
	if (val >= KILOBYTE) stat += eastl::to_string(static_cast<float>(val / KILOBYTE)) + " Gbs";
	else				 stat += eastl::to_string(val) + " Mbs";
}