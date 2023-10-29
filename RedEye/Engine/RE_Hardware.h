#ifndef __RE_HARDWARE_H__
#define __RE_HARDWARE_H__

#include <EASTL/string.h>
#include <EASTL/array.h>

namespace RE_Hardware
{
	namespace
	{
		constexpr double kilobyte = 1024.0;
		constexpr double megabyte = kilobyte * kilobyte;
		constexpr double gigabyte = kilobyte * megabyte;

		// RAM
		eastl::string ram_capacity;

		// CPU
		eastl::string cpu_brand;
		eastl::string cpu_vendor;
		eastl::string core_count;
		eastl::string caps1;
		eastl::string caps2;

		// Display
		eastl::string display_drivers;

		// GPU
		eastl::string vendor_id;
		eastl::string device_id;
		eastl::string gfx_brand;
		eastl::string gt_generation;
		eastl::string gpu_renderer;
		eastl::string gpu_vendor;

		//VRAM
		eastl::string vram_total;
		eastl::string vram_used;
		eastl::string vram_available;
		eastl::string vram_reserved;

		// Memory Plotting
		bool pause_memory_plotting = false;
		constexpr size_t mem_plotting_range = 100;
		eastl::array<float, mem_plotting_range> working_memory{}; // amount of memory physically mapped to the process context at a given time
		eastl::array<float, mem_plotting_range> pageable_memory{}; // system memory that can be transferred to the paging file on disk (paged) when it is not being used
		eastl::array<float, mem_plotting_range> nonpageable_memory{}; // system memory that cannot be paged to disk as long as the corresponding objects are allocated
		eastl::array<float, mem_plotting_range> paged_memory{}; // memory set aside for the process in the system paging file
	}

	void CheckHardware();
	void DrawEditor();

	namespace Internal
	{
		eastl::string BytesString(const char* stat, const double val);
		eastl::string KBytesString(const char* stat, const double val);
		eastl::string MBytesString(const char* stat, const double val);

		void PlotMemory(
			const char* name,
			eastl::array<float, mem_plotting_range>& data,
			double usage,
			double peak);
	}
};

#endif // !__RE_HARDWARE_H__