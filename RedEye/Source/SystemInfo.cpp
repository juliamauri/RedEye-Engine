#include "SystemInfo.h"

#include "Application.h"
#include "Globals.h"
#include "OutputLog.h"
#include "SDL2\include\SDL.h"
#include "imgui\imgui.h"
#include "mmgr\mmgr.h"
#include "gpudetect\DeviceId.h"
#include "Glew\include\glew.h"
#include <gl\GL.h>

SystemInfo::SystemInfo()
{}

SystemInfo::~SystemInfo()
{}

void SystemInfo::Init()
{
	LOG("Initializing System Info");
	WhatAreWeRunningOn();
}

void SystemInfo::WhatAreWeRunningOn()
{
	// CPU
	cpus = "CPUs: ";
	cpus += std::to_string(SDL_GetCPUCount());
	cpus += " cores";

	// RAM
	char ram_s[25];
	float system_ram = SDL_GetSystemRAM();
	if (system_ram > KILOBYTE)
		sprintf_s(ram_s, 25, "System RAM %0.1f Gb", system_ram / KILOBYTE_F);
	else
		sprintf_s(ram_s, 25, "System RAM %0.1f Mb", system_ram);
	ram = ram_s;

	// Caps
	if (SDL_Has3DNow()) caps1 += " 3DNow";
	if (SDL_HasAVX()) caps1 += " AVX";
	if (SDL_HasAltiVec()) caps1 += " AltiVec";
	if (SDL_HasMMX()) caps1 += " MMX";
	if (SDL_HasRDTSC()) caps1 += " RDTSC";

	if (SDL_HasSSE()) caps2 += " SSE";
	if (SDL_HasSSE2()) caps2 += " SSE2";
	if (SDL_HasSSE3()) caps2 += " SSE3";
	if (SDL_HasSSE41()) caps2 += " SSE41";
	if (SDL_HasSSE42()) caps2 += " SSE42";

	// Render Drivers
	drivers = "Render Drivers: ";
	SDL_RendererInfo info;
	if (SDL_GetRenderDriverInfo(0, &info) != -1) drivers += info.name;
	for (int i = 1; i < SDL_GetNumRenderDrivers() && SDL_GetRenderDriverInfo(i, &info) == 0; i++)
	{
		drivers += ", ";
		drivers += info.name;
	}

	// GPU
	gpu_renderer = "GPU: ";
	gpu_renderer += (char*)glGetString(GL_RENDERER);
	gpu_vendor = "Brand: ";
	gpu_vendor += (char*)glGetString(GL_VENDOR);

	App->ReportSoftware("gpudetect");
	App->ReportSoftware("mmgr");
}

void SystemInfo::MemoryDraw()
{
	sMStats stats = m_getMemoryStatistics();

	PlotMemory(stats.totalReportedMemory);
	ImGui::Separator();

	ImGui::Text("Total Reported Mem: %u", stats.totalReportedMemory);
	ImGui::Text("Total Actual Mem: %u", stats.totalActualMemory);
	ImGui::Text("Peak Reported Mem: %u", stats.peakReportedMemory);
	ImGui::Text("Peak Actual Mem: %u", stats.peakActualMemory);
	ImGui::Text("Accumulated Reported Mem: %u", stats.accumulatedReportedMemory);
	ImGui::Text("Accumulated Actual Mem: %u", stats.accumulatedActualMemory);
	ImGui::Text("Accumulated Alloc Unit Count: %u", stats.accumulatedAllocUnitCount);
	ImGui::Text("Total Alloc Unit Count: %u", stats.totalAllocUnitCount);
	ImGui::Text("Peak Alloc Unit Count: %u", stats.peakAllocUnitCount);
}

void SystemInfo::HardwareDraw()
{
	// CPU
	ImGui::Text(cpus.c_str());

	// RAM
	ImGui::Text(ram.c_str());

	// Caps
	ImGui::Text(caps1.c_str());
	ImGui::Text(caps2.c_str());

	// Render Drivers
	ImGui::Text(drivers.c_str());
	ImGui::Separator();

	// GPU
	ImGui::Text(gpu_renderer.c_str());
	ImGui::Text(gpu_vendor.c_str());
	ImGui::TextWrappedV(gpu_version.c_str(), "");
	ImGui::Text(gpu_shading.c_str());
	ImGui::Separator();

	// VRAM
	if (ImGui::Button("Check VRAM"))
		CheckVRAM();

	ImGui::Text(vram_total.c_str());
	ImGui::Text(vram_used.c_str());
	ImGui::Text(vram_available.c_str());
	ImGui::Text(vram_reserved.c_str());
}

void SystemInfo::PlotMemory(const unsigned int current_mem)
{
	float max = 0.f;

	if (!pause_plotting)
	{
		for (int i = 0; i < 99; i++)
		{
			if (mem[i] >= max)
				max = mem[i];

			mem[i] = mem[i + 1];
		}

		mem[99] = current_mem;
	}

	char title[25];
	sprintf_s(title, 25, "Memory Usage: %.1f", mem[99]);
	ImGui::PlotHistogram("##Memory Usage", mem, ((int)(sizeof(mem) / sizeof(*mem))), 0, title, 0.0f, 1.2f * max, ImVec2(310, 100));


	if (ImGui::Checkbox(pause_plotting ? "Restart Plotting" : "Pause Plotting", &pause_plotting))
	{
		if (!pause_plotting)
		{
			for (int i = 0; i < 99; i++) mem[i] = 0u; // Clear mem array
		}
	}
}

void SystemInfo::CheckVRAM()
{
	unsigned __int64 total, used, available, reserved;
	getGraphicsDeviceInfo(nullptr, nullptr, nullptr, &total, &used, &available, &reserved);

	/*char ram_s[25];
	sprintf_s(ram_s, 25, "Total VRAM: %0.1f Gb", total / GIGABYTE_F);
	vram_total = ram_s;*/

	vram_total = "Total VRAM: "; MemValAsString(vram_total, total);
	vram_used = "VRAM used: "; MemValAsString(vram_used, used);
	vram_available = "VRAM available: "; MemValAsString(vram_available, available);
	vram_reserved = "VRAM reserved: "; MemValAsString(vram_reserved, reserved);
}

void SystemInfo::MemValAsString(std::string& stat, const unsigned long long val) const
{
	// TODO: eliminate ghost leaks
	
	if (val >= GIGABYTE)
	{
		stat += std::to_string(val / GIGABYTE_F);
		stat += " Gbs";
	}
	else if (val >= MEGABYTE)
	{
		stat += std::to_string(val / MEGABYTE_F);
		stat += " Mbs";
	}
	else if (val >= KILOBYTE)
	{
		stat += std::to_string(val / KILOBYTE_F);
		stat += " Kbs";
	}
	else
	{
		stat += std::to_string(val);
		stat += " bytes";
	}
}

