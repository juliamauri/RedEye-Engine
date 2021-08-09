#ifndef __RE_HARDWARE_H__
#define __RE_HARDWARE_H__

class RE_Hardware
{
public:

	RE_Hardware() {}
	~RE_Hardware() {}

	void Init();
	void DrawEditor();

private:

	void QueryData();

	void BytesString(eastl::string& stat, const double val);
	void KBytesString(eastl::string& stat, const double val);
	void MBytesString(eastl::string& stat, const double val);

private:

	eastl::string
		// RAM
		ram_capacity,
		// CPU
		cpu_brand, cpu_vendor, core_count, caps1, caps2,
		// Display
		display_drivers,
		// GPU
		vendor_id, device_id, gfx_brand, gt_generation, gpu_renderer, gpu_vendor,
		//VRAM
		vram_total, vram_used, vram_available, vram_reserved;

	bool pause_plotting = false;

	// Memory
	enum MEMORY_TYPES : unsigned int
	{
		Working_MEM = 0u, // amount of memory physically mapped to the process context at a given time
		Pageable_MEM,	  // system memory that can be transferred to the paging file on disk (paged) when it is not being used
		Nonpageable_MEM,  // system memory that cannot be paged to disk as long as the corresponding objects are allocated
		Paged_MEM,		  // memory set aside for the process in the system paging file
		MAX_MEM_TYPES
	};
	float mem[MAX_MEM_TYPES][100]{};
};

#endif // !__RE_HARDWARE_H__