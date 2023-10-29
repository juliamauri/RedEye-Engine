#include "RE_Hardware.h"

#include "RE_Math.h"
#include "RE_Profiler.h"
#include "ModuleRenderer3D.h"

#include <SDL2/SDL.h>
#include <ImGui/imgui.h>
#include "gpudetect/DeviceId.h"

#include <Psapi.h> // WINDOWS memory calls

void RE_Hardware::CheckHardware()
{
	RE_PROFILE(RE_ProfiledFunc::Init, RE_ProfiledClass::Hardware)

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
	ram_capacity = Internal::MBytesString("Total space: ", static_cast<double>(SDL_GetSystemRAM()));

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

		vram_total = Internal::BytesString("Total VRAM: ", static_cast<double>(total));
		vram_used = Internal::BytesString("VRAM used: ", static_cast<double>(used));
		vram_available = Internal::BytesString("VRAM available: ", static_cast<double>(available));
		vram_reserved = Internal::BytesString("VRAM reserved: ", static_cast<double>(reserved));
	}

	(gpu_renderer = "GPU: ") += ModuleRenderer3D::GetGPURenderer();
	(gpu_vendor = "Brand: ") += ModuleRenderer3D::GetGPUVendor();
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
		if (!pause_memory_plotting)
		{
			PROCESS_MEMORY_COUNTERS memCounter;
			if (GetProcessMemoryInfo(GetCurrentProcess(), &memCounter, sizeof(memCounter)))
			{
				Internal::PlotMemory("Working", working_memory, memCounter.WorkingSetSize / megabyte, memCounter.PeakWorkingSetSize / megabyte);
				Internal::PlotMemory("Pageable", working_memory, memCounter.QuotaPagedPoolUsage / megabyte, memCounter.QuotaPeakPagedPoolUsage / megabyte);
				Internal::PlotMemory("Non-Pageable", working_memory, memCounter.QuotaNonPagedPoolUsage / megabyte, memCounter.QuotaPeakNonPagedPoolUsage / megabyte);
				Internal::PlotMemory("Paged", working_memory, memCounter.PagefileUsage / megabyte, memCounter.PeakPagefileUsage / megabyte);
			}
		}

		if (ImGui::Checkbox(pause_memory_plotting ? "Restart Plotting" : "Pause Plotting", &pause_memory_plotting) && !pause_memory_plotting)
		{
			working_memory.fill(0.f);
			pageable_memory.fill(0.f);
			nonpageable_memory.fill(0.f);
			paged_memory.fill(0.f);
		}

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
		ImGui::Separator();

		// VRAM
		if (ImGui::Button("Check VRAM"))
		{
			unsigned long long total;
			unsigned long long used;
			unsigned long long available;
			unsigned long long reserved;
			getGraphicsDeviceInfo(nullptr, nullptr, nullptr, &total, &used, &available, &reserved);
			vram_total = Internal::MBytesString("Total VRAM: ", static_cast<double>(total));
			vram_used = Internal::MBytesString("VRAM used: ", static_cast<double>(used));
			vram_available = Internal::MBytesString("VRAM available: ", static_cast<double>(available));
			vram_reserved = Internal::MBytesString("VRAM reserved: ", static_cast<double>(reserved));
		}

		ImGui::Text(vram_total.c_str());
		ImGui::Text(vram_used.c_str());
		ImGui::Text(vram_available.c_str());
		ImGui::Text(vram_reserved.c_str());

		ImGui::TreePop();
	}
}

#pragma region Internal

eastl::string RE_Hardware::Internal::BytesString(const char* stat, const double val)
{
	eastl::string ret = stat;
	if (val >= gigabyte)		ret + eastl::to_string(val / gigabyte) + " Gbs";
	else if (val >= megabyte)	ret += eastl::to_string(val / megabyte) + " Mbs";
	else if (val >= kilobyte)	ret += eastl::to_string(val / kilobyte) + " Kbs";
	else						ret += eastl::to_string(val) + " bytes";
	return ret;
}

eastl::string RE_Hardware::Internal::KBytesString(const char* stat, const double val)
{
	eastl::string ret = stat;
	if (val >= megabyte)	  ret += eastl::to_string(val / megabyte) + " Gbs";
	else if (val >= kilobyte) ret += eastl::to_string(val / kilobyte) + " Mbs";
	else					  ret += eastl::to_string(val) + " Kbs";
	return ret;
}

eastl::string RE_Hardware::Internal::MBytesString(const char* stat, const double val)
{
	eastl::string ret = stat;
	if (val >= kilobyte) ret += eastl::to_string(val / kilobyte) + " Gbs";
	else				 ret += eastl::to_string(val) + " Mbs";
	return ret;
}

void RE_Hardware::Internal::PlotMemory(const char* name, eastl::array<float, mem_plotting_range>& data, double usage, double peak)
{
	// Add next value
	float max = data[0];
	for (size_t i = 0; i < mem_plotting_range - 1; i++)
	{
		max = RE_Math::Max(max, data[i]);
		data[i] = data[i + 1];
	}
	data[mem_plotting_range - 1] = static_cast<float>(usage);

	// Plot
	ImGui::PlotHistogram(
		MBytesString((eastl::string("##") + name + ": ").c_str(), static_cast<double>(data[mem_plotting_range - 1])).c_str(),
		data.data(),
		100,
		0,
		name,
		0.0f,
		1.2f * static_cast<float>(peak),
		ImVec2(310, 100));

	// Max Peak
	ImGui::Text(MBytesString("Highest peak: ", peak).c_str());
}

#pragma endregion
