#ifndef __MEMORY__
#define __MEMORY__

// Delete Macros
#define DEL(x) if (x != nullptr) { delete x; x = nullptr; }
#define DEL_A(x) if (x != nullptr) { delete[] x; x = nullptr; }

// TODO Julius: #define RE_NEW new



/*
void* operator new(std::size_t sz)
{
	std::printf("1) new(size_t), size = %zu\n", sz);
	if (sz == 0)
		++sz; // avoid std::malloc(0) which may return nullptr on success

	if (void *ptr = std::malloc(sz))
		return ptr;

	throw std::bad_alloc{}; // required by [new.delete.single]/3
}

void* operator new[](std::size_t sz)
{
	std::printf("2) new[](size_t), size = %zu\n", sz);
	if (sz == 0)
		++sz; // avoid std::malloc(0) which may return nullptr on success

	if (void *ptr = std::malloc(sz))
		return ptr;

	throw std::bad_alloc{}; // required by [new.delete.single]/3
}
*/

/*void operator delete(void* ptr, std::size_t size) noexcept
{
	//std::printf("4) delete(void*, size_t), size = %zu\n", size);
	std::free(ptr);
}

void operator delete[](void* ptr, std::size_t size) noexcept
{
	//std::printf("6) delete[](void*, size_t), size = %zu\n", size);
	std::free(ptr);
}*/


#endif // !__MEMORY__