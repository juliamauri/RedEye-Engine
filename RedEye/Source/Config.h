#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "RapidJson\include\document.h"
#include "RapidJson\include\stringbuffer.h"
#include "RapidJson\include\writer.h"
#include "RapidJson\include\filereadstream.h"
#include "RapidJson\include\filewritestream.h"
using namespace rapidjson;

class Config 
{
public:

	void Init();

	Document LoadConfig();

	void TestRead();
	void TestWrite();
	void TestStreams();
};

#endif //__CONFIG_H__