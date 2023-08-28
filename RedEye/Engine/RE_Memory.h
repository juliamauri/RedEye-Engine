#ifndef __MEMORY__
#define __MEMORY__

// Delete Macros
#define DEL(x) if (x != nullptr) { delete x; x = nullptr; }
#define DEL_A(x) if (x != nullptr) { delete[] x; x = nullptr; }

// TODO Julius: #define RE_NEW new

#endif // !__MEMORY__