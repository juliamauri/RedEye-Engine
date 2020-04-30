#ifndef __SYSTEMINFO_H__
#define __SYSTEMINFO_H__

#include "EventListener.h"
#include <EASTL/vector.h>
#include <EASTL/string.h>

class SystemInfo : public EventListener
{
public:
	SystemInfo();
	~SystemInfo();

	void Init();

	// Check Hardware
	void WhatAreWeRunningOn();

	// Editor Drawing to Configuration Window
	void MemoryDraw();
	void HardwareDraw();

private:

	void PlotMemory(const unsigned int current_mem);

	void CheckVRAM();
	void MemValAsString(eastl::string& stat, const unsigned long long val) const;

private:

	float mem[100] = {};
	bool pause_plotting = false;

	// Hardware
	eastl::string cpus;
	eastl::string ram;
	eastl::string caps1;
	eastl::string caps2;
	eastl::string drivers;

	// GPU
	eastl::string gpu_renderer;
	eastl::string gpu_vendor;
	eastl::string gpu_version;
	eastl::string gpu_shading;

	//VRAM
	eastl::string vram_total;
	eastl::string vram_used;
	eastl::string vram_available;
	eastl::string vram_reserved;
};

/* extra functions

bool getGraphicsDeviceInfo(
	unsigned int* VendorId,
	unsigned int* DeviceId,
	eastl::wstring* GFXBrand,
	unsigned __int64* VideoMemoryBudget,
	unsigned __int64* VideoMemoryCurrentUsage,
	unsigned __int64* VideoMemoryAvailable,
	unsigned __int64* VideoMemoryReserved)

getCPUInfo(
	eastl::string* cpubrand,
	eastl::string* cpuvendor)

getGTGeneration(unsigned int deviceId) */


#endif // !__SYSTEMINFO_H__