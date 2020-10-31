#ifdef _DEBUG

#include "SystemInfo.h"

#include "Application.h"
#include "ModuleRenderer3D.h"
#include "Globals.h"
#include "OutputLog.h"

#include "SDL2\include\SDL.h"
#include "imgui\imgui.h"
#include "gpudetect\DeviceId.h"
#include <EAStdC/EAString.h>

#include <Psapi.h> // WINDOWS memory calls

SystemInfo::SystemInfo()
{}

SystemInfo::~SystemInfo()
{}

void SystemInfo::Init()
{
	RE_LOG("Initializing System Info");

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
	for (unsigned int i = 0; i < MAX_MEM_TYPES; ++i)
		for (int j = 0; j < MEMORY_PLOT_ARRAY_SIZE; ++j)
			mem[i][j] = 0.f;

	// Display Drivers
	drivers = "2D display drivers: ";
	SDL_RendererInfo info;
	int driver_count = SDL_GetNumRenderDrivers();
	if (driver_count > 0 && SDL_GetRenderDriverInfo(0, &info) != -1)
	{
		drivers += info.name;
		for (int i = 1; i < driver_count && SDL_GetRenderDriverInfo(i, &info) == 0; i++)
		{
			drivers += ", ";
			drivers += info.name;
		}
	}
	else
		drivers += "empty";

	// GPU
	unsigned int VendorId, DeviceId; std::wstring GFXBrand;
	unsigned long long total, used, available, reserved;
	if (getGraphicsDeviceInfo(&VendorId, &DeviceId, &GFXBrand, nullptr, nullptr, nullptr, nullptr))
	{
		vendor_id = "Vendor ID: " + eastl::to_string(VendorId);
		device_id = "Device ID: " + eastl::to_string(DeviceId);
		(gfx_brand = "") += reinterpret_cast<const char*>(GFXBrand.c_str());
		gt_generation = "GT generation: " + eastl::to_string(static_cast<int>(getGTGeneration(DeviceId)));

		BytesString(vram_total = "Total VRAM: ", static_cast<double>(total));
		BytesString(vram_used = "VRAM used: ", static_cast<double>(used));
		BytesString(vram_available = "VRAM available: ", static_cast<double>(available));
		BytesString(vram_reserved = "VRAM reserved: ", static_cast<double>(reserved));
	}

	(gpu_renderer = "GPU: ") += App::renderer3d->GetGPURenderer();
	(gpu_vendor = "Brand: ") += App::renderer3d->GetGPUVendor();

	App->ReportSoftware("gpudetect");
}

void SystemInfo::DrawEditor()
{
	if (ImGui::CollapsingHeader("Hardware"))
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
					for (int i2 = 0; i2 < MEMORY_PLOT_ARRAY_SIZE - 1; i2++)
					{
						float tmp[2] = { peak, mem[i][i2] };
						peak = tmp[mem[i][i2] > peak];
						mem[i][i2] = mem[i][i2 + 1];
					}

					mem[i][MEMORY_PLOT_ARRAY_SIZE - 1] = values[i][0];

					// Plot
					eastl::string title;
					MBytesString(title = names[i] + " Memory: ", static_cast<double>(mem[i][MEMORY_PLOT_ARRAY_SIZE - 1]));
					ImGui::PlotHistogram((eastl::string("##") + names[i] + " Memory Usage").c_str(), mem[i], MEMORY_PLOT_ARRAY_SIZE, 0, title.c_str(), 0.0f, 1.2f * peak, ImVec2(310, 100));

					// Max Peak
					MBytesString(mem_peaks[i] = "Highest peak: ", values[i][1]);
					ImGui::Text(mem_peaks[i].c_str());
				}
			}

			if (ImGui::Checkbox(pause_plotting ? "Restart Plotting" : "Pause Plotting", &pause_plotting) && !pause_plotting)
				for (unsigned short i = 0; i < MAX_MEM_TYPES; ++i)
					for (int i2 = 0; i2 < MEMORY_PLOT_ARRAY_SIZE; ++i2)
						mem[i][i2] = 0.f;

			ImGui::TreePop();
		}

		// Display Drivers
		if (ImGui::TreeNodeEx("Display", ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow))
		{
			ImGui::Text(drivers.c_str());
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
}

void SystemInfo::BytesString(eastl::string& stat, const double val) const
{
	if		(val >= GIGABYTE) stat += eastl::to_string(static_cast<float>(val / GIGABYTE)) + " Gbs";
	else if (val >= MEGABYTE) stat += eastl::to_string(static_cast<float>(val / MEGABYTE)) + " Mbs";
	else if (val >= KILOBYTE) stat += eastl::to_string(static_cast<float>(val / KILOBYTE)) + " Kbs";
	else					  stat += eastl::to_string(val) + " bytes";
}

void SystemInfo::KBytesString(eastl::string& stat, const double val) const
{
	if (val >= MEGABYTE)	  stat += eastl::to_string(static_cast<float>(val / MEGABYTE)) + " Gbs";
	else if (val >= KILOBYTE) stat += eastl::to_string(static_cast<float>(val / KILOBYTE)) + " Mbs";
	else					  stat += eastl::to_string(val) + " Kbs";
}

void SystemInfo::MBytesString(eastl::string& stat, const double val) const
{
	if (val >= KILOBYTE) stat += eastl::to_string(static_cast<float>(val / KILOBYTE)) + " Gbs";
	else				 stat += eastl::to_string(val) + " Mbs";
}

#endif // _DEBUG
