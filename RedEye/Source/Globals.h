#ifndef __GLOBALS__
#define __GLOBALS__

// Engine data
constexpr auto ENGINE_NAME = "RedEye Engine";
constexpr auto ENGINE_VERSION = "5.0";
constexpr auto ENGINE_ORGANIZATION = "RedEye";
constexpr auto ENGINE_AUTHOR_1 = "Juli Mauri Costa";
constexpr auto ENGINE_AUTHOR_2 = "Ruben Sardon Roldan";
constexpr auto ENGINE_DESCRIPTION = "RedEye Engine is a 3D Game Engine Sofware.";
constexpr auto ENGINE_LICENSE = "GNU General Public License v3.0";

// Useful types
typedef unsigned int uint;
typedef unsigned short int ushortint;
typedef unsigned char uchar;
typedef unsigned long ulong;
typedef unsigned long long UID;

// Delete Macros
#define DEL(x) if (x != nullptr) { delete x; x = nullptr; }
#define DEL_A(x) if (x != nullptr) { delete[] x; x = nullptr; }

// Build instructions
#define _STATIC_CPPLIB

// Disable STL exceptions
#undef _HAS_EXCEPTIONS
#define _HAS_EXCEPTIONS 0

// Disable depricate code warnings
#define _DISABLE_DEPRECATE_STATIC_CPPLIB
//#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS

// Define Assert to call EA_ASSERT
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

#endif // !__GLOBALS__