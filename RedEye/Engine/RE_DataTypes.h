#ifndef __DATA_TYPES__
#define __DATA_TYPES__

typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;
typedef unsigned long ulong;
typedef unsigned long long ulonglong;

typedef unsigned long long GO_UID;
typedef unsigned long long COMP_UID;

// 3D Directions
enum class Direction : uchar
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

#endif // !__DATA_TYPES__