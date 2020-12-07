#pragma once

// Disable STL exceptions
#undef _HAS_EXCEPTIONS
#define _HAS_EXCEPTIONS 0

#define _STATIC_CPPLIB
#define _DISABLE_DEPRECATE_STATIC_CPPLIB
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS

#include <EAAssert/eaassert.h>
#define RE_ASSERT(expression) EA_ASSERT(expression)

// OpenGL Output Debug
#include "Glew/include/glew.h"
void GLAPIENTRY MessageCallback(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam);

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

// Useful types
typedef unsigned int uint;
typedef unsigned short int ushortint;
typedef unsigned char uchar;
typedef unsigned long ulong;
typedef unsigned long long UID;

// Delete Macros
#define DEL(x) if (x != nullptr) { delete x; x = nullptr; }
#define DEL_A(x) if (x != nullptr) { delete[] x; x = nullptr; }
