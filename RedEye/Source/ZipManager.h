#pragma once


#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include "libzip/include/zip.h"


#ifdef _DEBUG
#pragma comment( lib, "libzip/zip_d.lib" )

#else
#pragma comment( lib, "libzip/zip_r.lib" )
#endif // _DEBUG

#include <fstream>
#include <string>


void TestZIP(const char* zip_path, const char* filename, const char * buffer, unsigned int size)
{
	struct zip *f_zip = NULL;
	int error = 0;
	f_zip = zip_open(zip_path, ZIP_CHECKCONS, &error); /* on ouvre l'archive zip */
	if (error)	LOG("could not open or create archive: %s", zip_path);
	
	zip_source_t *s;

	s = zip_source_buffer(f_zip, buffer, size, 0);

	if (s == NULL ||
		zip_file_add(f_zip, filename, s, ZIP_FL_OVERWRITE + ZIP_FL_ENC_UTF_8) < 0) {
		zip_source_free(s);
		LOG("error adding file: %s\n", zip_strerror(f_zip));
	}

	zip_close(f_zip);
	f_zip = NULL;
}