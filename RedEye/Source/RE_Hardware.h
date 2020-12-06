#ifndef __SYSTEMINFO_H__
#define __SYSTEMINFO_H__

#include <EASTL/vector.h>
#include <EASTL/string.h>

#define MEMORY_PLOT_ARRAY_SIZE 100

class RE_Hardware
{
public:
	RE_Hardware() {}
	~RE_Hardware() {}

	void Init();
	void DrawEditor();

private:

	void BytesString(eastl::string& stat, const double val) const;
	void KBytesString(eastl::string& stat, const double val) const;
	void MBytesString(eastl::string& stat, const double val) const;

private:

	// CPU
	eastl::string cpu_brand;
	eastl::string cpu_vendor;
	eastl::string core_count;
	eastl::string caps1;
	eastl::string caps2;

	// Memory
	bool pause_plotting = false;
	enum MEMORY_TYPES : unsigned int
	{
		Working_MEM = 0u, // amount of memory physically mapped to the process context at a given time
		Pageable_MEM,	  // system memory that can be transferred to the paging file on disk (paged) when it is not being used
		Nonpageable_MEM,  // system memory that cannot be paged to disk as long as the corresponding objects are allocated
		Paged_MEM,		  // memory set aside for the process in the system paging file
		MAX_MEM_TYPES
	};
	eastl::string ram_capacity;
	eastl::string mem_peaks[MEMORY_PLOT_ARRAY_SIZE];
	float mem[MAX_MEM_TYPES][MEMORY_PLOT_ARRAY_SIZE]{};
																		  
	// Display
	eastl::string drivers;

	// GPU
	eastl::string vendor_id;
	eastl::string device_id;
	eastl::string gfx_brand;
	eastl::string gt_generation;

	eastl::string gpu_renderer;
	eastl::string gpu_vendor;

	//eastl::string gpu_version;
	//eastl::string gpu_shading;

	//VRAM
	eastl::string vram_total;
	eastl::string vram_used;
	eastl::string vram_available;
	eastl::string vram_reserved;
};

#endif // !__SYSTEMINFO_H__