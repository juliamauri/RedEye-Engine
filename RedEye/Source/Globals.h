#pragma once

#define DEGTORAD 0.0174532925199432957f
#define RADTODEG 57.295779513082320876f
#define PI 3.14159265358979323846f
#define TWO_PI 6.28318530717958647692f
#define HALF_PI 1.57079632679489661923f
#define QUARTER_PI 0.78539816339744830961f
#define INV_PI 0.31830988618379067154f
#define INV_TWO_PI 0.15915494309189533576f
#define HAVE_M_PI

#define KILOBYTE 1024.0
#define MEGABYTE 1048576.0
#define GIGABYTE 1073741824.0

// New useful types
typedef unsigned int uint;
typedef unsigned short int ushortint;
typedef unsigned char uchar;
typedef unsigned long ulong;
typedef unsigned long long UID;

enum update_status
{
	UPDATE_ERROR = -1,
	UPDATE_STOP = 0,
	UPDATE_CONTINUE = 1
};

enum Dir
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

// Useful macros
#define RE_CAP(n) ((n <= 0.0f) ? n=0.0f : (n >= 1.0f) ? n=1.0f : n=n)
#define RE_CAPTO(n, t) ((n <= 0.0f) ? n=0.0f : (n >= t) ? n=t : n=n)
#define RE_MIN(a,b) ((a)<(b)) ? (a) : (b)
#define RE_MAX(a,b) ((a)>(b)) ? (a) : (b)

/*/ Align 16, use if you have math elemtns in your class like float4x4 or AABB
#define ALIGN_CLASS_TO_16 \
	void* operator new(size_t i) { return _aligned_malloc(i,16); }\
    void operator delete(void* p) { _aligned_free(p); }
	*/


// Deletes a buffer
#define DEL( x )\
    {\
       if( x != nullptr )\
       {\
         delete x;\
	     x = nullptr;\
       }\
    }

// Deletes an array of buffers
#define DEL_A( x )\
	{\
       if( x != nullptr )\
       {\
           delete[] x;\
	       x = nullptr;\
		 }\
	 }


// Warning disabled ---
//#pragma warning( disable : 4577 ) // Warning that exceptions are disabled
//#pragma warning( disable : 4530 ) // Warning that exceptions are disabled

// Disable STL exceptions
#undef _HAS_EXCEPTIONS
#define _HAS_EXCEPTIONS 0

#define _STATIC_CPPLIB
#define _DISABLE_DEPRECATE_STATIC_CPPLIB

//OpenGL GetError
void _CheckGLError(const char* file, int line);

#define CheckGLError() _CheckGLError(__FILE__, __LINE__)