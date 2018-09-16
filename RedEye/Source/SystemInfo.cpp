#include "SystemInfo.h"

#include "Globals.h"
#include "SDL2\include\SDL.h"
#include "imgui\imgui.h"
#include "mmgr\mmgr.h"
#include "gpudetect\DeviceId.h"
#include "Glew\include\glew.h"
#include <gl\GL.h>

//SystemInfo::SystemInfo() {}

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
		sprintf_s(ram_s, 25, "System RAM %0.1f Gb/s", system_ram / KILOBYTE_F);
	else
		sprintf_s(ram_s, 25, "System RAM %0.1f Mb/s", system_ram);
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
	gpu_version = "Version:: ";
	gpu_version += (char*)glGetString(GL_VERSION);
	gpu_shading = "GLSL: ";
	gpu_shading += (char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
}

void SystemInfo::CheckMemory()
{
	unsigned __int64 total, used, available, reserved;
	getGraphicsDeviceInfo(nullptr, nullptr, nullptr, &total, &used, &available, &reserved);

	// Total
	vram_total = "Total VRAM: ";
	if (total > GIGABYTE)
	{
		vram_total += std::to_string(total / GIGABYTE_F);
		vram_total += " Gbs";
	}
	else if (total > GIGABYTE)
	{
		vram_total += std::to_string(total / MEGABYTE_F);
		vram_total += " Mbs";
	}
	else
	{
		vram_total += std::to_string(total);
		vram_total += " Kbs";
	}

	// Used
	vram_used = "VRAM used: ";
	if (used > GIGABYTE)
	{
		vram_used += std::to_string(used / GIGABYTE_F);
		vram_used += " Gbs";
	}
	else if (used > GIGABYTE)
	{
		vram_used += std::to_string(used / MEGABYTE_F);
		vram_used += " Mbs";
	}
	else
	{
		vram_used += std::to_string(used);
		vram_used += " Kbs";
	}

	// Available
	vram_available = "VRAM available: ";
	if (available > GIGABYTE)
	{
		vram_available += std::to_string(available / GIGABYTE_F);
		vram_available += " Gbs";
	}
	else if (available > GIGABYTE)
	{
		vram_available += std::to_string(available / MEGABYTE_F);
		vram_available += " Mbs";
	}
	else
	{
		vram_available += std::to_string(available);
		vram_available += " Kbs";
	}

	// Reserved
	vram_reserved = "VRAM reserved: ";
	if (reserved > GIGABYTE)
	{
		vram_reserved += std::to_string(reserved / GIGABYTE_F);
		vram_reserved += " Gbs";
	}
	else if (reserved > GIGABYTE)
	{
		vram_reserved += std::to_string(reserved / MEGABYTE_F);
		vram_reserved += " Mbs";
	}
	else
	{
		vram_reserved += std::to_string(reserved);
		vram_reserved += " Kbs";
	}
}

void SystemInfo::MemoryDraw()
{
	CheckMemory();

	/*/ Mem Plotter
	char title[25];
	int max = AddPlotValue(mem, stats.totalReportedMemory);
	sprintf_s(title, 25, "Memory Usage %d", stats.totalReportedMemory);
	ImGui::PlotHistogram("##Memory Usage", &mem[0], mem.size(), 0, title, 0.f, 1.2f * max, ImVec2(310, 100));*/

	sMStats stats = m_getMemoryStatistics();
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
	ImGui::Text(gpu_version.c_str());
	ImGui::Text(gpu_shading.c_str());

	// VRAM
	ImGui::Text(vram_total.c_str());
	ImGui::Text(vram_used.c_str());
	ImGui::Text(vram_available.c_str());
	ImGui::Text(vram_reserved.c_str());
}
