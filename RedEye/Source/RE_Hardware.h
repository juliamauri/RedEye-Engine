#ifndef __RE_HARDWARE_H__
#define __RE_HARDWARE_H__

#include <EASTL/string.h>

namespace RE_Hardware
{
	void DrawEditor();
	void Clear();

	namespace Internal
	{
		// Memory
		struct HardwareData
		{
			eastl::string
				// CPU
				cpu_brand, cpu_vendor, core_count, caps1, caps2,
				// Display
				display_drivers,
				// GPU
				vendor_id, device_id, gfx_brand, gt_generation, gpu_renderer, gpu_vendor,
				//VRAM
				vram_total, vram_used, vram_available, vram_reserved;

			enum MEMORY_TYPES : unsigned int
			{
				Working_MEM = 0u, // amount of memory physically mapped to the process context at a given time
				Pageable_MEM,	  // system memory that can be transferred to the paging file on disk (paged) when it is not being used
				Nonpageable_MEM,  // system memory that cannot be paged to disk as long as the corresponding objects are allocated
				Paged_MEM,		  // memory set aside for the process in the system paging file
				MAX_MEM_TYPES
			};
			eastl::string ram_capacity, mem_peaks[100];
			float mem[MAX_MEM_TYPES][100]{};
			bool pause_plotting = false;

			void QueryData();
			void DrawEditor();

		} static *data = nullptr;

		void BytesString(eastl::string& stat, const double val);
		void KBytesString(eastl::string& stat, const double val);
		void MBytesString(eastl::string& stat, const double val);
	}
};

#endif // !__RE_HARDWARE_H__