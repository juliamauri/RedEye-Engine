#ifndef __SYSTEMINFO_H__
#define __SYSTEMINFO_H__

#include "EventListener.h"
#include <vector>
#include <string>

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
	void MemValAsString(std::string& stat, const unsigned long long val) const;

private:

	float mem[100] = {};
	bool pause_plotting = false;

	// Hardware
	std::string cpus;
	std::string ram;
	std::string caps1;
	std::string caps2;
	std::string drivers;

	// GPU
	std::string gpu_renderer;
	std::string gpu_vendor;
	std::string gpu_version;
	std::string gpu_shading;

	//VRAM
	std::string vram_total;
	std::string vram_used;
	std::string vram_available;
	std::string vram_reserved;
};

/* extra functions

bool getGraphicsDeviceInfo(
	unsigned int* VendorId,
	unsigned int* DeviceId,
	std::wstring* GFXBrand,
	unsigned __int64* VideoMemoryBudget,
	unsigned __int64* VideoMemoryCurrentUsage,
	unsigned __int64* VideoMemoryAvailable,
	unsigned __int64* VideoMemoryReserved)

getCPUInfo(
	std::string* cpubrand,
	std::string* cpuvendor)

getGTGeneration(unsigned int deviceId) */


#endif // !__SYSTEMINFO_H__