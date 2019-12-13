#ifndef __DEFAULTSHADERS_H__
#define __DEFAULTSHADERS_H__

#define DEFVERTEXSHADER												\
"#version 330 core\n"												\
"layout(location = 0) in vec3 aPos;\n"								\
"layout(location = 1) in vec3 aNormal;\n"							\
"layout(location = 2) in vec3 aTangent;\n"							\
"layout(location = 3) in vec3 aBitangent;\n"						\
"layout(location = 4) in vec2 aTexCoord;\n"							\
"\n"																\
"out vec2 TexCoord;\n"												\
"\n"																\
"uniform mat4 model;\n"												\
"uniform mat4 view;\n"												\
"uniform mat4 projection;\n"										\
"\n"																\
"void main()\n"														\
"{\n"																\
"	gl_Position = projection * view * model * vec4(aPos, 1.0);\n"	\
"	TexCoord = aTexCoord;\n"										\
"}\0"																  

#define DEFFRAGMENTSHADER														       \
"#version 330 core\n"															       \
"#extension GL_ARB_separate_shader_objects : enable\n"						  \
"layout(location = 0) out vec4 color;\n"															       \
"\n"																				   \
"in vec2 TexCoord;\n"																   \
"\n"																				   \
"uniform float useTexture;\n"														   \
"uniform sampler2D tdiffuse0;\n"											       \
"\n"																				   \
"uniform float useColor;\n"														       \
"uniform vec3 cdiffuse;\n"													       \
"\n"																			       \
"void main()\n"																	       \
"{\n"																				   \
"	if (useTexture > 0.0f && useColor > 0.0f)\n"									   \
"		color = vec4(texture(tdiffuse0, TexCoord) * vec4(cdiffuse, 1.0));\n"   \
"	else if (useTexture > 0.0f)\n"												       \
"		color = texture(tdiffuse0, TexCoord);\n"						       \
"	else if (useColor > 0.0f)\n"													   \
"		color = vec4(cdiffuse, 1.0);\n"									       \
"}\0"

#define SKYBOXVERTEXSHADER							  \
"#version 330 core\n"								  \
"layout(location = 0) in vec3 aPos;\n"				  \
"\n"													  \
"out vec3 TexCoords;\n"								  \
"\n"													  \
"uniform mat4 projection;\n"							  \
"uniform mat4 view;\n"								  \
"\n"													  \
"void main()\n"										  \
"{\n"													  \
"	TexCoords = aPos;\n"								  \
"	vec4 pos = projection * view * vec4(aPos, 1.0);\n"  \
"	gl_Position = pos.xyww;\n"						  \
"}\0"

#define SKYBOXFRAGMENTSHADER				  \
"#version 330 core\n"						  \
"#extension GL_ARB_separate_shader_objects : enable\n"						  \
"layout(location = 0) out vec4 color;\n"		\
"\n"											  \
"in vec3 TexCoords;\n"						  \
"\n"											  \
"uniform samplerCube skybox;\n"				  \
"\n"											  \
"void main()\n"								  \
"{\n"											  \
"	color = texture(skybox, TexCoords);\n"  \
"}\0"



#endif // !__DEFAULTSHADERS_H__