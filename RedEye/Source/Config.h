#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "RapidJson\include\document.h"
#include "RapidJson\include\stringbuffer.h"
#include "RapidJson\include\writer.h"
#include "RapidJson\include\filereadstream.h"
#include "RapidJson\include\filewritestream.h"
using namespace rapidjson;

/*
union CValue
{
	int i;
	double d;
	bool b;
	const char* c;
	Value::ConstValueIterator itr;
};

enum TDOCUMENTS
{
	TD_CONFIG,
	TD_SCENE
};
*/
class Config 
{
private:
	Document config;
public:

	void Init();

	bool LoadJsons();
	Document LoadConfig();

	void TestRead();
	void TestWrite();
	void TestStreams();
};

#endif //__CONFIG_H__