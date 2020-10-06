#ifndef __DEFAULTSHADERS_H__
#define __DEFAULTSHADERS_H__

#define DEFVERTEXSCALESHADER																\
"#version 330 core\n"																		\
"layout(location = 0) in vec3 aPos;\n"														\
"layout(location = 1) in vec3 aNormal;\n"													\
"layout(location = 2) in vec3 aTangent;\n"													\
"layout(location = 3) in vec3 aBitangent;\n"												\
"layout(location = 4) in vec2 aTexCoord;\n"													\
"\n"																						\
"out vec2 TexCoord;\n"																		\
"\n"																						\
"uniform mat4 model;\n"																		\
"uniform mat4 view;\n"																		\
"uniform mat4 projection;\n"																\
"\n"																						\
"uniform float scaleFactor;\n"																\
"uniform vec3 center;\n"																	\
"\n"																						\
"void main()\n"																				\
"{\n"																						\
"	mat4 modelviewMatrix = view*model;\n"													\
"	mat3 transformMatrix = mat3(modelviewMatrix[0].xyz,\n"									\
"		modelviewMatrix[1].xyz,\n"															\
"		modelviewMatrix[2].xyz);\n"															\
"	vec3 viewSpaceNormal = transformMatrix * normalize(aPos - center);\n"					\
"	vec4 replacementPosition = projection * vec4(viewSpaceNormal, 0.0) * scaleFactor;\n"	\
"	gl_Position = projection * modelviewMatrix * vec4(aPos,1.0) + replacementPosition;\n"	\
"	TexCoord = aTexCoord;\n"																\
"}\0"

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

#define DEFFRAGMENTSHADER														\
"#version 330 core\n"															\
"#extension GL_ARB_separate_shader_objects : enable\n"						   	\
"layout(location = 0) out vec4 color;\n"										\
"\n"																			\
"in vec2 TexCoord;\n"															\
"\n"																			\
"uniform float useTexture;\n"													\
"uniform sampler2D tdiffuse0;\n"											   	\
"\n"																			\
"uniform float useColor;\n"														\
"uniform vec3 cdiffuse;\n"													   	\
"\n"																			\
"void main()\n"																	\
"{\n"																			\
"	if (useTexture > 0.0f && useColor > 0.0f)\n"								\
"		color = vec4(texture(tdiffuse0, TexCoord) * vec4(cdiffuse, 1.0));\n"   	\
"	else if (useTexture > 0.0f)\n"												\
"		color = texture(tdiffuse0, TexCoord);\n"						       	\
"	else if (useColor > 0.0f)\n"												\
"		color = vec4(cdiffuse, 1.0);\n"									       	\
"}\0"

// Skybox
#define SKYBOXVERTEXSHADER									\
"#version 330 core\n"										\
"layout(location = 0) in vec3 aPos;\n"						\
"\n"														\
"out vec3 pos;\n"											\
"\n"														\
"uniform mat4 projection;\n"								\
"uniform mat4 view;\n"										\
"\n"														\
"void main()\n"												\
"{\n"														\
"	pos = aPos;\n"											\
"	gl_Position = projection * view * vec4(aPos, 1.0);\n"	\
"}\0"

#define SKYBOXFRAGMENTSHADER								\
"#version 330 core\n"										\
"#extension GL_ARB_separate_shader_objects : enable\n"		\
"layout(location = 0) out vec4 color;\n"					\
"\n"														\
"in vec3 pos;\n"											\
"\n"														\
"uniform samplerCube cubemap;\n"							\
"\n"														\
"void main()\n"												\
"{\n"														\
"	color = texture(cubemap, normalize(pos).stp);\n"	    \
"}\0"

// Deferred Geo Pass
#define GEOPASSVERTEXSHADER									\
"#version 330 core\n"										\
"layout(location = 0) in vec3 aPos;\n"						\
"layout(location = 1) in vec3 aNormal;\n"					\
"layout(location = 4) in vec2 aTexCoord;\n"					\
"\n"														\
"out vec3 FragPos;\n"										\
"out vec2 TexCoord;\n"										\
"out vec3 Normal;\n"										\
"\n"														\
"uniform mat4 model;\n"										\
"uniform mat4 view;\n"										\
"uniform mat4 projection;\n"								\
"\n"														\
"void main()\n"												\
"{\n"														\
"	vec4 worldPos = model * vec4(aPos, 1.0);\n"				\
"	FragPos = worldPos.xyz;\n"								\
"	TexCoord = aTexCoord;\n"								\
"	mat3 normalMatrix = transpose(inverse(mat3(model)));\n"	\
"	Normal = normalMatrix * aNormal;\n"						\
"	gl_Position = projection * view * worldPos;\n"			\
"}\0"

#define GEOPASSFRAGMENTSHADER										\
"#version 330 core\n"												\
"layout (location = 0) out vec3 gPosition;\n"						\
"layout (location = 1) out vec3 gNormal;\n"							\
"layout (location = 2) out vec3 gAlbedo;\n"							\
"layout (location = 3) out float gSpec;\n"							\
"\n"																\
"in vec3 FragPos;\n"												\
"in vec2 TexCoord;\n"												\
"in vec3 Normal;\n"													\
"\n"																\
"uniform float useTexture;\n"										\
"uniform sampler2D tdiffuse0;\n"									\
"uniform sampler2D tspecular0;\n"									\
"\n"																\
"uniform float useColor;\n"											\
"uniform vec3 cdiffuse;\n"											\
"\n"																\
"void main()\n"														\
"{\n"																\
"	gPosition = FragPos;\n"											\
"	gNormal = normalize(Normal);\n"									\
"\n"																\
"	if (useTexture > 0.0f && useColor > 0.0f)\n"					\
"	{\n"															\
"		gAlbedo = texture(tdiffuse0, TexCoord).rgb * cdiffuse;\n"	\
"		gSpec = texture(tspecular0, TexCoord).r;\n"				\
"	}\n"															\
"	else if (useTexture > 0.0f)\n"									\
"	{\n"															\
"		gAlbedo = texture(tdiffuse0, TexCoord).rgb;\n"				\
"		gSpec = texture(tspecular0, TexCoord).r;\n"				\
"	}\n"															\
"	else if (useColor > 0.0f)\n"									\
"		gAlbedo = cdiffuse;\n"									    \
"}\0"

// Deferred Light Pass
#define LIGHTPASSVERTEXSHADER				\
"#version 330 core\n"						\
"layout(location = 0) in vec3 aPos;\n"		\
"layout(location = 1) in vec2 aTexCoord;\n"	\
"\n"										\
"out vec2 TexCoord;\n"						\
"\n"										\
"void main()\n"								\
"{\n"										\
"	TexCoord = aTexCoord;\n"				\
"	gl_Position = vec4(aPos, 1.0);\n"		\
"}\0"

#define LIGHTPASSFRAGMENTSHADER										\
"#version 330 core\n"												\
"layout (location = 4) out vec4 aRes;\n"							\
"\n"																\
"in vec2 TexCoord;\n"												\
"\n"																\
"uniform sampler2D gPosition;\n"									\
"uniform sampler2D gNormal;\n"										\
"uniform sampler2D gAlbedo;\n"										\
"uniform sampler2D gSpec;\n"										\
"\n"																\
"void main()\n"														\
"{\n"																\
"	vec3 pos = texture(gPosition, TexCoord).rgb;\n"		\
"	vec3 Normal = normalize(texture(gNormal, TexCoord).rgb);\n"		\
"	vec3 Diffuse = texture(gAlbedo, TexCoord).rgb;\n"				\
"	float Specular = texture(gSpec, TexCoord).r;\n"					\
"	vec3 lighting = vec3(Specular, 0, 0);\n"						\
"	aRes = vec4(Normal, 1.0);\n"										\
"}\0"



#endif // !__DEFAULTSHADERS_H__