#include "RapidJson\include\document.h"
#include "Globals.h"
#include "Config.h"


using namespace rapidjson;

//Tutorial http://rapidjson.org/md_doc_tutorial.html

void Config::TestRead()
{
	const char* json = "{\"hello\":\"world\",\"t\":true,\"f\":false,\"n\":null,\"i\":123,\"pi\":3.1416,\"a\":[1,2,3,4]}";

	Document document;
	document.Parse(json);

		//check root
	assert(document.IsObject());

		//check hello
	assert(document.HasMember("hello"));
	assert(document["hello"].IsString());
	LOG("hello = %s\n", document["hello"].GetString());

		//check bool
	assert(document["t"].IsBool());
	LOG("t = %s\n", document["t"].GetBool() ? "true" : "false");
	
		//check null
	LOG("n = %s\n", document["n"].IsNull() ? "null" : "?");

		//check numbers
	assert(document["i"].IsNumber());

	// In this case, IsUint()/IsInt64()/IsUInt64() also return true.
	assert(document["i"].IsInt());
	LOG("i = %d\n", document["i"].GetInt());
	// Alternative (int)document["i"]

	assert(document["pi"].IsNumber());
	assert(document["pi"].IsDouble());
	LOG("pi = %g\n", document["pi"].GetDouble());

		//check array
	// Using a reference for consecutive access is handy and faster.
	const Value& a = document["a"];
	assert(a.IsArray());
	for (SizeType i = 0; i < a.Size(); i++)
	{
		LOG("a[%d] = %d\n", i, a[i].GetInt());
	}

		//Query array
	for (Value::ConstValueIterator itr = a.Begin(); itr != a.End(); ++itr)
		LOG("%d ", itr->GetInt());
	//other functions
	// SizeType Capacity() const;
	// bool Empty() const

	//Range - based For Loop
	for (auto& v : a.GetArray())
		LOG("%d ", v.GetInt());

		//Query Object
	static const char* kTypeNames[] =
	{ "Null", "False", "True", "Object", "Array", "String", "Number" };

	for (Value::ConstMemberIterator itr = document.MemberBegin(); itr != document.MemberEnd(); ++itr)
	{
		LOG("Type of member %s is %s\n",
			itr->name.GetString(), kTypeNames[itr->value.GetType()]);
	}

		//find member
	Value::ConstMemberIterator itr = document.FindMember("hello");
	if (itr != document.MemberEnd()) //check
		LOG("%s\n", itr->value.GetString());

		//Comparing Numbers
	//if (document["hello"] == document["n"]) /*...*/;    // Compare values
	//if (document["hello"] == "world") /*...*/;          // Compare value with literal string
	//if (document["i"] != 123) /*...*/;                  // Compare with integers
	//if (document["pi"] != 3.14) /*...*/;                // Compare with double.
	//Duplicated member name is always false.
}

void Config::TestWrite()
{
	{//Change value Type
		Document d; //null
		d.SetObject();

		Value v;
		v.SetInt(10);
		//Same -> v = 10;
	}
	{//Overloaded constructors
		Value b(true);    // calls Value(bool)
		Value i(-123);    // calls Value(int)
		Value u(123u);    // calls Value(unsigned)
		Value d(1.5);     // calls Value(double)

		//empty object or array
		Value o(kObjectType);
		Value a(kArrayType);
	}

		//Move Semantics
	{
		Value a(123);
		Value b(456);
		b = a;         // a becomes a Null value, b becomes number 123.
	}
	{
		Document d;
		Value o(kObjectType);
		{
			Value contacts(kArrayType);
			// adding elements to contacts array.
			o.AddMember("contacts", contacts, d.GetAllocator());  // just memcpy() of contacts itself to the value of new member (16 bytes)
																  // contacts became Null here. Its destruction is trivial.
		}
	}
	{//  Move semantics and temporary values
		Document document;
		Value a(kArrayType);
		Document::AllocatorType& allocator = document.GetAllocator();
		// a.PushBack(Value(42), allocator);       // will not compile
		a.PushBack(Value().SetInt(42), allocator); // fluent API
		a.PushBack(Value(42).Move(), allocator);   // same as above
	}

	//Create String
	{
		Document document;
		Value author;
		char buffer[10];
		int len = sprintf_s(buffer, "%s %s", "Milo", "Yip"); // dynamically created string.
		author.SetString(buffer, len, document.GetAllocator());
		memset(buffer, 0, sizeof(buffer));
		// author.GetString() still contains "Milo Yip" after buffer is destroyed
	}
	{
		Value s;
		s.SetString("rapidjson");    // can contain null character, length derived at compile time
		s = "rapidjson";             // shortcut, same as above
	}
	/*
	{
		const char * cstr = getenv("USER");
		size_t cstr_len = 5;                 // in case length is available
		Value s;
		// s.SetString(cstr);                  // will not compile
		s.SetString(StringRef(cstr));          // ok, assume safe lifetime, null-terminated
		s = StringRef(cstr);                   // shortcut, same as above
		s.SetString(StringRef(cstr, cstr_len)); // faster, can contain null character
		s = StringRef(cstr, cstr_len);          // shortcut, same as above
	}
	*/

}
