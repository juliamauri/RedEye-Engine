#ifndef __DEFAULTSHADERS_H__
#define __DEFAULTSHADERS_H__

#pragma region NoLightShader&Scale

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
"uniform float useClipPlane;\n"										\
"uniform vec4 clip_plane;\n"										\
"\n"																\
"void main()\n"														\
"{\n"																\
"	vec4 worldPos = model * vec4(aPos, 1.0);\n"						\
"	if (useClipPlane > 0.0f)\n"										\
"		gl_ClipDistance[0] = dot(worldPos, clip_plane);\n"			\
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
"uniform float opacity;\n"													   	\
"\n"																			\
"void main()\n"																	\
"{\n"																			\
"	if (useTexture > 0.0f && useColor > 0.0f)\n"								\
"		color = vec4(texture(tdiffuse0, TexCoord) * vec4(cdiffuse, opacity));\n"\
"	else if (useTexture > 0.0f)\n"												\
"		color = texture(tdiffuse0, TexCoord);\n"						       	\
"	else if (useColor > 0.0f)\n"												\
"		color = vec4(cdiffuse, opacity);\n"									    \
"}\0"

#pragma endregion NoLight&ScaleShader

#pragma region SkyBoxShader

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
"layout(location = 4) out vec4 colorDef;\n"					\
"\n"														\
"in vec3 pos;\n"											\
"\n"														\
"uniform samplerCube cubemap;\n"							\
"uniform bool deferred;\n"									\
"\n"														\
"void main()\n"												\
"{\n"														\
"	if(deferred)\n"										    \
"		colorDef = texture(cubemap, normalize(pos).stp);\n"	\
"	else\n"													\
"		color = texture(cubemap, normalize(pos).stp);\n"	\
"}\0"

#pragma endregion SkyBoxShader

#pragma region DefferedShader

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
"uniform float useClipPlane;\n"								\
"uniform vec4 clip_plane;\n"								\
"\n"														\
"void main()\n"												\
"{\n"														\
"	vec4 worldPos = model * vec4(aPos, 1.0);\n"				\
"	if (useClipPlane > 0.0f)\n"								\
"		gl_ClipDistance[0] = dot(worldPos, clip_plane);\n"	\
"\n"														\
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
"layout (location = 3) out vec3 gSpec;\n"							\
"\n"																\
"in vec3 FragPos;\n"												\
"in vec2 TexCoord;\n"												\
"in vec3 Normal;\n"													\
"\n"																\
"uniform float useTexture;\n"										\
"uniform sampler2D tdiffuse0;\n"									\
"uniform sampler2D tspecular0;\n"									\
"uniform float shininess;\n"										\
"\n"																\
"uniform float useColor;\n"											\
"uniform vec3 cdiffuse;\n"											\
"uniform vec3 cspecular;\n"											\
"uniform float opacity;\n"											\
"\n"																\
"void main()\n"														\
"{\n"																\
"	gPosition = FragPos;\n"											\
"	gNormal = normalize(Normal);\n"									\
"\n"																\
"	if (useTexture > 0.0f && useColor > 0.0f)\n"					\
"	{\n"															\
"		gAlbedo = texture(tdiffuse0, TexCoord).rgb * cdiffuse;\n"	\
"		gSpec = vec3(texture(tspecular0, TexCoord).r, shininess,opacity);\n"\
"	}\n"															\
"	else if (useTexture > 0.0f)\n"									\
"	{\n"															\
"		gAlbedo = texture(tdiffuse0, TexCoord).rgb;\n"				\
"		gSpec = vec3(texture(tspecular0, TexCoord).r, shininess,opacity);\n"\
"	}\n"															\
"	else if (useColor > 0.0f)\n"									\
"	{\n"															\
"		gAlbedo = cdiffuse;\n"									    \
"		gSpec = vec3(cspecular.x, shininess, opacity);\n"			\
"	}\n"															\
"}\0"

#pragma region DefferedLightPassShader

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

#define LIGHTPASSFRAGMENTSHADER																														\
"#version 330 core\n"																																\
"layout (location = 4) out vec4 aRes;\n"																											\
"\n"																																				\
"in vec2 TexCoord;\n"																																\
"\n"																																				\
"struct Light {\n"																																	\
"    vec4 positionType;\n"																															\
"    vec4 directionIntensity;\n"																													\
"    vec4 diffuseSpecular;\n"																														\
"    vec4 clq; //constant linear quadratic\n"																										\
"    vec4 co; //cutoff outercutoff\n"																												\
"};\n"																																				\
"const int NR_LIGHTS = 203;\n"																														\
"uniform Light lights[NR_LIGHTS];\n"																												\
"uniform int count;\n"																															\
"uniform vec3 viewPos;\n"																															\
"\n"																																				\
"uniform sampler2D gPosition;\n"																													\
"uniform sampler2D gNormal;\n"																														\
"uniform sampler2D gAlbedo;\n"																														\
"uniform sampler2D gSpec;\n"																														\
"\n"																																				\
"vec3 calculateLight(float type, vec3 viewDir, vec3 objectPosition, vec3 lightPositon, vec3 objectNormal, vec3 objectDiffuse, vec3 lightDiffuse, float shininess, float objectSpecular, float lightSpecular, float intensity, float constant, float linear, float quadratic, vec3 direction, float cutoff, float outercutoff)\n"																																		\
"{\n"																																				\
"			vec3 lightDir = normalize(lightPositon - objectPosition);\n"																			\
"			vec3 specular = vec3(0.0, 0.0, 0.0);\n"																									\
"			vec3 res_light = vec3(0.0, 0.0, 0.0);\n"																								\
"			\n"																																		\
"			if (type == 0.0) // DIRECTIONAL_LIGHT\n"																						\
"			{\n"																																	\
"				vec3 diffuse = lightDiffuse * max(dot(objectNormal, lightDir), 0.0) * objectDiffuse;\n"													\
"				\n"																																	\
"				if (dot(objectNormal, lightDir) > 0.0)\n"																									\
"					specular = vec3(pow(max(dot(viewDir, reflect(-lightDir, objectNormal)), 0.0), shininess) * lightSpecular * objectSpecular);\n"			\
"				\n"																																	\
"				res_light = diffuse + specular;\n"																									\
"			}\n"																																	\
"			else if (type == 1.0) // POINT_LIGHT\n"																						\
"			{\n"																																	\
"				vec3 diffuse = lightDiffuse * max(dot(objectNormal, lightDir), 0.0) * objectDiffuse;\n"													\
"				\n"																																	\
"				if (dot(objectNormal, lightDir) > 0.0)\n"																									\
"					specular = vec3(pow(max(dot(viewDir, reflect(-lightDir, objectNormal)), 0.0), shininess) * lightSpecular * objectSpecular);\n"			\
"				\n"																																	\
"				float distance = length(lightPositon - objectPosition);\n"																			\
"				float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));\n"		\
"				\n"																																	\
"				diffuse *= attenuation;\n"																											\
"				specular *= attenuation;\n"																											\
"				res_light = diffuse + specular;\n"																									\
"			}\n"																																	\
"			else if (type == 2.0) // SPOT_LIGHT\n"																						\
"			{\n"																																	\
"				float theta = dot(lightDir, normalize(-direction));\n"																	\
"				if(theta > outercutoff)\n"																								\
"				{\n"																																\
"					vec3 diffuse = lightDiffuse * max(dot(objectNormal, lightDir), 0.0) * objectDiffuse;\n"												\
"					\n"																																\
"					if (dot(objectNormal, lightDir) > 0.0)\n"																								\
"						specular = vec3(pow(max(dot(viewDir, reflect(-lightDir, objectNormal)), 0.0), shininess) * lightSpecular * objectSpecular);\n"\
"					\n"																																\
"					float smoothness = clamp((theta - outercutoff) / (cutoff - outercutoff), 0.0, 1.0);\n"			\
"					\n"																																\
"					float distance = length(lightPositon - objectPosition);\n"																		\
"					float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));\n"	\
"					\n"																																\
"					diffuse *= attenuation * smoothness;\n"																							\
"					specular *= attenuation * smoothness;\n"																						\
"					res_light = diffuse + specular;\n"																								\
"				}\n"																																\
"			}\n"																																	\
"			return res_light * intensity;\n"																							\
"}\n"\
"void main()\n"																																		\
"{\n"																																				\
"	vec3 Position = texture(gPosition, TexCoord).rgb;\n"																							\
"	vec3 Normal = normalize(texture(gNormal, TexCoord).rgb);\n"																						\
"	vec3 Diffuse = texture(gAlbedo, TexCoord).rgb;\n"																								\
"	float Specular = texture(gSpec, TexCoord).r;\n"																									\
"	float shininess = texture(gSpec, TexCoord).g;\n"																								\
"	float opacity = texture(gSpec, TexCoord).b;\n"																									\
"	\n"																																				\
"   vec3 lighting = vec3(0.0, 0.0, 0.0);\n"				    																						\
"	vec3 viewDir = normalize(viewPos - Position);\n"																								\
"	\n"																																				\
"	for (int i = 0; i < count; ++i)\n"				        																					\
"   {\n"																																			\
"		lighting += calculateLight(lights[i].positionType.w, viewDir, Position, lights[i].positionType.xyz, Normal, Diffuse, lights[i].diffuseSpecular.xyz, shininess, Specular, lights[i].diffuseSpecular.w, lights[i].directionIntensity.w, lights[i].clq.x, lights[i].clq.y, lights[i].clq.z, lights[i].directionIntensity.xyz, lights[i].co.x, lights[i].co.y);\n"																							\
"   }\n"																																			\
"	aRes = vec4(lighting,opacity);\n"																												\
"}\0"

#define PARTICLELIGHTPASSFRAGMENTSHADER																														\
"#version 330 core\n"																																\
"layout (location = 4) out vec4 aRes;\n"																											\
"\n"																																				\
"in vec2 TexCoord;\n"																																\
"\n"																																				\
"struct ParticleLight {\n"																																	\
"    vec4 positionIntensity;\n"																															\
"    vec4 diffuseSpecular;\n"																														\
"};\n"																																				\
"struct ParticleInfo {\n"																																	\
"    vec4 tclq;\n"																															\
"    int pCount;\n"																														\
"};\n"																																				\
"const int NR_PLIGHTS = 508;\n"																														\
"uniform ParticleLight plights[NR_PLIGHTS];\n"																												\
"uniform ParticleInfo pInfo;\n"																												\
"uniform vec3 viewPos;\n"																															\
"\n"																																				\
"uniform sampler2D gPosition;\n"																													\
"uniform sampler2D gNormal;\n"																														\
"uniform sampler2D gAlbedo;\n"																														\
"uniform sampler2D gSpec;\n"																														\
"uniform sampler2D gLighting;\n"																													\
"\n"																																				\
"vec3 calculateLight(float type, vec3 viewDir, vec3 objectPosition, vec3 lightPositon, vec3 objectNormal, vec3 objectDiffuse, vec3 lightDiffuse, float shininess, float objectSpecular, float lightSpecular, float intensity, float constant, float linear, float quadratic, vec3 direction, float cutoff, float outercutoff)\n"																																		\
"{\n"																																				\
"			vec3 lightDir = normalize(lightPositon - objectPosition);\n"																			\
"			vec3 specular = vec3(0.0, 0.0, 0.0);\n"																									\
"			vec3 res_light = vec3(0.0, 0.0, 0.0);\n"																								\
"			\n"																																		\
"			if (type == 0.0) // DIRECTIONAL_LIGHT\n"																						\
"			{\n"																																	\
"				vec3 diffuse = lightDiffuse * max(dot(objectNormal, lightDir), 0.0) * objectDiffuse;\n"													\
"				\n"																																	\
"				if (dot(objectNormal, lightDir) > 0.0)\n"																									\
"					specular = vec3(pow(max(dot(viewDir, reflect(-lightDir, objectNormal)), 0.0), shininess) * lightSpecular * objectSpecular);\n"			\
"				\n"																																	\
"				res_light = diffuse + specular;\n"																									\
"			}\n"																																	\
"			else if (type == 1.0) // POINT_LIGHT\n"																						\
"			{\n"																																	\
"				vec3 diffuse = lightDiffuse * max(dot(objectNormal, lightDir), 0.0) * objectDiffuse;\n"													\
"				\n"																																	\
"				if (dot(objectNormal, lightDir) > 0.0)\n"																									\
"					specular = vec3(pow(max(dot(viewDir, reflect(-lightDir, objectNormal)), 0.0), shininess) * lightSpecular * objectSpecular);\n"			\
"				\n"																																	\
"				float distance = length(lightPositon - objectPosition);\n"																			\
"				float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));\n"		\
"				\n"																																	\
"				diffuse *= attenuation;\n"																											\
"				specular *= attenuation;\n"																											\
"				res_light = diffuse + specular;\n"																									\
"			}\n"																																	\
"			else if (type == 2.0) // SPOT_LIGHT\n"																						\
"			{\n"																																	\
"				float theta = dot(lightDir, normalize(-direction));\n"																	\
"				if(theta > outercutoff)\n"																								\
"				{\n"																																\
"					vec3 diffuse = lightDiffuse * max(dot(objectNormal, lightDir), 0.0) * objectDiffuse;\n"												\
"					\n"																																\
"					if (dot(objectNormal, lightDir) > 0.0)\n"																								\
"						specular = vec3(pow(max(dot(viewDir, reflect(-lightDir, objectNormal)), 0.0), shininess) * lightSpecular * objectSpecular);\n"\
"					\n"																																\
"					float smoothness = clamp((theta - outercutoff) / (cutoff - outercutoff), 0.0, 1.0);\n"			\
"					\n"																																\
"					float distance = length(lightPositon - objectPosition);\n"																		\
"					float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));\n"	\
"					\n"																																\
"					diffuse *= attenuation * smoothness;\n"																							\
"					specular *= attenuation * smoothness;\n"																						\
"					res_light = diffuse + specular;\n"																								\
"				}\n"																																\
"			}\n"																																	\
"			return res_light * intensity;\n"																							\
"}\n"\
"void main()\n"																																		\
"{\n"																																				\
"	vec3 Position = texture(gPosition, TexCoord).rgb;\n"																							\
"	vec3 Normal = normalize(texture(gNormal, TexCoord).rgb);\n"																						\
"	vec3 Diffuse = texture(gAlbedo, TexCoord).rgb;\n"																								\
"	float Specular = texture(gSpec, TexCoord).r;\n"																									\
"	float shininess = texture(gSpec, TexCoord).g;\n"																								\
"	float opacity = texture(gSpec, TexCoord).b;\n"																									\
"	\n"																																				\
"   vec3 lighting = texture(gLighting, TexCoord).rgb;\n"				    																						\
"	vec3 viewDir = normalize(viewPos - Position);\n"																								\
"	\n"																																				\
"	for (int i = 0; i < pInfo.pCount; ++i)\n"				        																					\
"   {\n"																																			\
"		lighting += calculateLight(pInfo.tclq.x, viewDir, Position, plights[i].positionIntensity.xyz, Normal, Diffuse, plights[i].diffuseSpecular.xyz, shininess, Specular, plights[i].diffuseSpecular.w, plights[i].positionIntensity.w, pInfo.tclq.y, pInfo.tclq.z, pInfo.tclq.w, lighting, 0.0, 0.0);\n"																							\
"   }\n"																																			\
"	aRes = vec4(lighting,opacity);\n"																												\
"}\0"

#pragma endregion DeferredLightPassShader

#pragma endregion DeferredShader

#pragma region WaterShader

#pragma region NoLightWater

#define WATERVERTEXSHADER											\
"#version 330 core\n"												\
"layout(location = 0) in vec3 aPos;\n"								\
"layout(location = 1) in vec3 aNormal;\n"							\
"layout(location = 2) in vec3 aTangent;\n"							\
"layout(location = 3) in vec3 aBitangent;\n"						\
"layout(location = 4) in vec2 aTexCoord;\n"							\
"\n"																\
"out vec2 TexCoord;\n"												\
"out vec3 Normal;\n"												\
"out vec3 FragPos;\n"												\
"out float height;\n"												\
"\n"																\
"uniform mat4 model;\n"												\
"uniform mat4 view;\n"												\
"uniform mat4 projection;\n"										\
"\n"																\
"uniform float time;\n"												\
"uniform float waveLength;\n"										\
"uniform float amplitude;\n"										\
"uniform float speed;\n"											\
"uniform bool is_linear;\n"											\
"uniform vec2 direction;\n"											\
"uniform vec2 center;\n"											\
"uniform float steepness;\n"										\
"uniform int numWaves;\n"											\
"\n"																\
"float PI = 3.14;\n"												\
"void main()\n"														\
"{\n"																\
"	float w = 2 * PI / waveLength;\n"								\
"	float Q = steepness / (w * amplitude * numWaves);\n"			\
"	float phi = speed * 2 * PI / waveLength;\n"						\
"\n"																\
"	vec2 D = vec2(0.0, 0.0);\n"										\
"	float dist = 0;\n"												\
"	if (is_linear) {\n"												\
"		D = normalize(direction);\n"								\
"		dist = dot(D, vec2(aPos.x, aPos.y));\n"						\
"	}\n"															\
"	else {\n"														\
"		D = vec2(aPos.x, aPos.y) - center;\n"						\
"		D = D / length(D);\n"										\
"		dist = dot(D, vec2(aPos.x, aPos.y));\n"						\
"	}\n"															\
"\n"																\
"	float QA = Q * amplitude;\n"									\
"	float tmp = w * dist + phi * time;\n"							\
"	float WA = w * amplitude;\n"									\
"	float S = sin(tmp);\n"											\
"	float C = cos(tmp);\n"											\
"\n"																\
"	vec3 pos = vec3(aPos.x + (QA * D.x * cos(tmp)),\n"				\
"		aPos.y + (QA * D.y * cos(tmp)),\n"							\
"		amplitude * sin(tmp));\n"									\
"\n"																\
"	vec3 bitangent = vec3(1 - (Q * pow(D.x, 2) * WA * S),\n"		\
"		-(Q * D.x * D.y * WA * S),\n"								\
"		D.x * WA * C);\n"											\
"\n"																\
"	vec3 tangent = vec3(bitangent.y,\n"								\
"		1 - (Q * pow(D.y, 2) * WA * S),\n"							\
"		D.y * WA * C);\n"											\
"\n"																\
"	Normal = mat3(transpose(inverse(model))) *\n"					\
"		vec3(-(D.x * WA * C),\n"									\
"			-(D.y * WA * C),\n"										\
"			1 - (Q * WA * S));\n"									\
"\n"																\
"	gl_Position = projection * view * model * vec4(pos, 1.0);\n"	\
"	TexCoord = aTexCoord;\n"										\
"	FragPos = vec3(model * vec4(pos, 1.0));\n"						\
"	height = normalize(pos.z);\n"									\
"}\0"																\
															 

#define WATERFRAGMENTSHADER																		\
"#version 330 core\n"																			\
"#extension GL_ARB_separate_shader_objects : enable\n"											\
"layout(location = 0) out vec4 color;\n"														\
"\n"																							\
"in vec2 TexCoord;\n"																			\
"in vec3 Normal;\n"																				\
"in vec3 FragPos;\n"																			\
"in float height;\n"																			\
"in vec4 gl_FragCoord;\n"																		\
"\n"																							\
"uniform vec3 cdiffuse;\n"																		\
"uniform float shininess;\n"																	\
"\n"																							\
"uniform float foamMin;\n"																		\
"uniform float foamMax;\n"																		\
"uniform vec3 foam_color;\n"																	\
"uniform sampler2D water_foam;\n"																\
"\n"																							\
"uniform float opacity;\n"																		\
"\n"																							\
"uniform sampler2D currentDepth;\n"																\
"uniform float viewport_h;\n"																	\
"uniform float viewport_w;\n"																	\
"uniform float near_plane;\n"																	\
"uniform float far_plane;\n"																	\
"\n"																							\
"uniform float distanceFoam;\n"																	\
"\n"																							\
"float LinearizeDepthTexture(vec2 uv)\n"														\
"{\n"																							\
"	float n = near_plane; // camera z near\n"													\
"	float f = far_plane; // camera z far\n"														\
"	float z = texture2D(currentDepth, uv).x;\n"													\
"	return (2.0 * n) / (f + n - z * (f - n));\n"												\
"}\n"																							\
"\n"																							\
"float LinearizeDepth()\n"																		\
"{\n"																							\
"	float n = near_plane; // camera z near\n"													\
"	float f = far_plane; // camera z far\n"														\
"	float z = gl_FragCoord.z;\n"																\
"	return (2.0 * n) / (f + n - z * (f - n));\n"												\
"}\n"																							\
"\n"																							\
"void main()\n"																					\
"{\n"																							\
"\n"																							\
"	vec4 finalcolor;\n"																			\
"	if (height > foamMax)\n"																	\
"		finalcolor = vec4(foam_color, 1.0);\n"													\
"	else if (height >= foamMin)\n"																\
"	{\n"																						\
"		float n_height = (height - foamMin) / foamMax;\n"										\
"		finalcolor.x = ((1.0f - n_height) * cdiffuse.x) + (n_height * foam_color.x);\n"			\
"		finalcolor.y = ((1.0f - n_height) * cdiffuse.y) + (n_height * foam_color.y);\n"			\
"		finalcolor.z = ((1.0f - n_height) * cdiffuse.z) + (n_height * foam_color.z);\n"			\
"		finalcolor.w = ((1.0f - n_height) * opacity) + n_height;\n"								\
"	}\n"																						\
"	else\n"																						\
"		finalcolor = vec4(cdiffuse, opacity);\n"												\
"\n"																							\
"	vec2 uv = vec2(gl_FragCoord.x / viewport_w, gl_FragCoord.y / viewport_h);\n"				\
"\n"																							\
"	float td = LinearizeDepthTexture(uv);\n"													\
"	float d = LinearizeDepth();\n"																\
"\n"																							\
"	if (td - d < distanceFoam)\n"																\
"		finalcolor = vec4(1.0);\n"																\
"\n"																							\
"	color = finalcolor;\n"																		\
"}\n"																							\

#pragma endregion NoLightWater

#pragma region DeferredLightWater

#define WATERPASSVERTEXSHADER										\
"#version 330 core\n"												\
"layout(location = 0) in vec3 aPos;\n"								\
"layout(location = 1) in vec3 aNormal;\n"							\
"layout(location = 4) in vec2 aTexCoord;\n"							\
"\n"																\
"out vec3 FragPos;\n"												\
"out vec2 TexCoord;\n"												\
"out vec3 Normal;\n"												\
"\n"																\
"out float height;\n"												\
"\n"																\
"uniform mat4 model;\n"												\
"uniform mat4 view;\n"												\
"uniform mat4 projection;\n"										\
"\n"																\
"uniform float time;\n"												\
"uniform float waveLength;\n"										\
"uniform float amplitude;\n"										\
"uniform float speed;\n"											\
"uniform bool is_linear;\n"											\
"uniform vec2 direction;\n"											\
"uniform vec2 center;\n"											\
"uniform float steepness;\n"										\
"uniform int numWaves;\n"											\
"\n"																\
"float PI = 3.14;\n"												\
"void main()\n"														\
"{\n"																\
"	float w = 2 * PI / waveLength;\n"								\
"	float Q = steepness / (w * amplitude * numWaves);\n"			\
"	float phi = speed * 2 * PI / waveLength;\n"						\
"\n"																\
"	vec2 D = vec2(0.0, 0.0);\n"										\
"	float dist = 0;\n"												\
"	if (is_linear) {\n"												\
"		D = normalize(direction);\n"								\
"		dist = dot(D, vec2(aPos.x, aPos.y));\n"						\
"	}\n"															\
"	else {\n"														\
"		D = vec2(aPos.x, aPos.y) - center;\n"						\
"		D = D / length(D);\n"										\
"		dist = dot(D, vec2(aPos.x, aPos.y));\n"						\
"	}\n"															\
"\n"																\
"	float QA = Q * amplitude;\n"									\
"	float tmp = w * dist + phi * time;\n"							\
"	float WA = w * amplitude;\n"									\
"	float S = sin(tmp);\n"											\
"	float C = cos(tmp);\n"											\
"\n"																\
"	vec3 pos = vec3(aPos.x + (QA * D.x * cos(tmp)),\n"				\
"		aPos.y + (QA * D.y * cos(tmp)),\n"							\
"		amplitude * sin(tmp));\n"									\
"\n"																\
"	Normal = mat3(transpose(inverse(model))) *\n"					\
"		vec3(-(D.x * WA * C),\n"									\
"			-(D.y * WA * C),\n"										\
"			1 - (Q * WA * S));\n"									\
"\n"																\
"	gl_Position = projection * view * model * vec4(pos, 1.0);\n"	\
"	TexCoord = aTexCoord;\n"										\
"	FragPos = vec3(model * vec4(pos, 1.0));\n"						\
"	height = normalize(pos.z);\n"									\
"}\0"																\

#define WATERPASSFRAGMENTSHADER																	\
"#version 330 core\n"																			\
"#extension GL_ARB_separate_shader_objects : enable\n"											\
"layout (location = 0) out vec3 gPosition;\n"													\
"layout (location = 1) out vec3 gNormal;\n"														\
"layout (location = 2) out vec3 gAlbedo;\n"														\
"layout (location = 3) out vec3 gSpec;\n"														\
"\n"																							\
"in vec3 FragPos;\n"																			\
"in vec2 TexCoord;\n"																			\
"in vec3 Normal;\n"																				\
"in float height;\n"																			\
"\n"																							\
"in vec4 gl_FragCoord;\n"																		\
"\n"																							\
"uniform vec3 cdiffuse;\n"																		\
"uniform float shininess;\n"																	\
"\n"																							\
"uniform float foamMin;\n"																		\
"uniform float foamMax;\n"																		\
"uniform vec3 foam_color;\n"																	\
"uniform sampler2D water_foam;\n"																\
"\n"																							\
"uniform float opacity;\n"																		\
"\n"																							\
"uniform sampler2D currentDepth;\n"																\
"uniform float viewport_h;\n"																	\
"uniform float viewport_w;\n"																	\
"uniform float near_plane;\n"																	\
"uniform float far_plane;\n"																	\
"\n"																							\
"uniform float distanceFoam;\n"																	\
"\n"																							\
"float LinearizeDepthTexture(vec2 uv)\n"														\
"{\n"																							\
"	float n = near_plane; // camera z near\n"													\
"	float f = far_plane; // camera z far\n"														\
"	float z = texture2D(currentDepth, uv).x;\n"													\
"	return (2.0 * n) / (f + n - z * (f - n));\n"												\
"}\n"																							\
"\n"																							\
"float LinearizeDepth()\n"																		\
"{\n"																							\
"	float n = near_plane; // camera z near\n"													\
"	float f = far_plane; // camera z far\n"														\
"	float z = gl_FragCoord.z;\n"																\
"	return (2.0 * n) / (f + n - z * (f - n));\n"												\
"}\n"																							\
"\n"																							\
"void main()\n"																					\
"{\n"																							\
"\n"																							\
"	vec4 finalcolor;\n"																			\
"	if (height > foamMax)\n"																	\
"		finalcolor = vec4(foam_color, opacity);\n"												\
"	else if (height >= foamMin)\n"																\
"	{\n"																						\
"		float n_height = (height - foamMin) / foamMax;\n"										\
"		finalcolor.x = ((1.0f - n_height) * cdiffuse.x) + (n_height * foam_color.x);\n"			\
"		finalcolor.y = ((1.0f - n_height) * cdiffuse.y) + (n_height * foam_color.y);\n"			\
"		finalcolor.z = ((1.0f - n_height) * cdiffuse.z) + (n_height * foam_color.z);\n"			\
"		finalcolor.w = ((1.0f - n_height) * opacity) + n_height;\n"								\
"	}\n"																						\
"	else\n"																						\
"		finalcolor = vec4(cdiffuse, opacity);\n"												\
"\n"																							\
"	vec2 uv = vec2(gl_FragCoord.x / viewport_w, gl_FragCoord.y / viewport_h);\n"				\
"\n"																							\
"	float td = LinearizeDepthTexture(uv);\n"													\
"	float d = LinearizeDepth();\n"																\
"\n"																							\
"	if (td - d < distanceFoam)\n"																\
"		finalcolor = vec4(vec3(1.0), opacity);\n"												\
"\n"																							\
"	gAlbedo = vec3(finalcolor.x, finalcolor.y, finalcolor.z);\n"								\
"	gPosition = FragPos;\n"																		\
"	gNormal = normalize(Normal);\n"																\
"	gSpec = vec3(0.0f, shininess, finalcolor.w);\n"												\
"}\n"																							\


#pragma endregion DefferedLightWater

/*"\n"		----Unused code but usefull for future uses----			\
"	vec3 bitangent = vec3(1 - (Q * pow(D.x, 2) * WA * S),\n"		\
"		-(Q * D.x * D.y * WA * S),\n"								\
"		D.x * WA * C);\n"											\
"\n"																\
"	vec3 tangent = vec3(bitangent.y,\n"								\
"		1 - (Q * pow(D.y, 2) * WA * S),\n"							\
"		D.y * WA * C);\n"											\*/

#pragma endregion WaterShader

#pragma region ParticleShader

#pragma region pNormalShading

#define PARTICLEVERTEXSHADER										\
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

#define PARTICLEFRAGMENTSHADER													\
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
"uniform float opacity;\n"													   	\
"\n"																			\
"void main()\n"																	\
"{\n"																			\
"	if (useTexture > 0.0f && useColor > 0.0f)\n"								\
"		color = vec4(texture(tdiffuse0, TexCoord) * vec4(cdiffuse, opacity));\n"\
"	else if (useTexture > 0.0f)\n"												\
"		color = texture(tdiffuse0, TexCoord);\n"						       	\
"	else if (useColor > 0.0f)\n"												\
"		color = vec4(cdiffuse, opacity);\n"									    \
"}\0"

#pragma endregion pNormalShading

#pragma region pDeffShading
// Deferred Geo Pass
#define PARTICLEGEOPASSVERTEXSHADER							\
"#version 330 core\n"										\
"layout(location = 0) in vec3 aPos;\n"						\
"layout(location = 1) in vec3 aNormal;\n"					\
"layout(location = 4) in vec2 aTexCoord;\n"					\
"\n"														\
"out vec3 FragPos;\n"										\
"out vec2 TexCoord;\n"										\
"out vec3 Normal;\n"										\
"\n"														\
"uniform float customNormal;\n"								\
"uniform vec3 normal;\n"									\
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
"	if(customNormal > 0.0)"									\
"		Normal = normalMatrix * normal;\n"					\
"	else\n"													\
"		Normal = normalMatrix * aNormal;\n"					\
"	gl_Position = projection * view * worldPos;\n"			\
"}\0"

#define PARTICLEGEOPASSFRAGMENTSHADER								\
"#version 330 core\n"												\
"layout (location = 0) out vec3 gPosition;\n"						\
"layout (location = 1) out vec3 gNormal;\n"							\
"layout (location = 2) out vec3 gAlbedo;\n"							\
"layout (location = 3) out vec3 gSpec;\n"							\
"\n"																\
"in vec3 FragPos;\n"												\
"in vec2 TexCoord;\n"												\
"in vec3 Normal;\n"													\
"\n"																\
"uniform float useTexture;\n"										\
"uniform sampler2D tdiffuse0;\n"									\
"uniform sampler2D tspecular0;\n"									\
"uniform float shininess;\n"										\
"\n"																\
"uniform float useColor;\n"											\
"uniform vec3 cdiffuse;\n"											\
"uniform vec3 cspecular;\n"											\
"uniform float opacity;\n"											\
"\n"																\
"void main()\n"														\
"{\n"																\
"	gPosition = FragPos;\n"											\
"	gNormal = normalize(Normal);\n"									\
"\n"																\
"	if (useTexture > 0.0f && useColor > 0.0f)\n"					\
"	{\n"															\
"		gAlbedo = texture(tdiffuse0, TexCoord).rgb * cdiffuse;\n"	\
"		gSpec = vec3(texture(tspecular0, TexCoord).r, shininess,opacity);\n"\
"	}\n"															\
"	else if (useTexture > 0.0f)\n"									\
"	{\n"															\
"		gAlbedo = texture(tdiffuse0, TexCoord).rgb;\n"				\
"		gSpec = vec3(texture(tspecular0, TexCoord).r, shininess,opacity);\n"\
"	}\n"															\
"	else if (useColor > 0.0f)\n"									\
"	{\n"															\
"		gAlbedo = cdiffuse;\n"									    \
"		gSpec = vec3(cspecular.x, shininess, opacity);\n"			\
"	}\n"															\
"}\0"
#pragma endregion pDeffShading

#pragma endregion ParticleShader

#endif // !__DEFAULTSHADERS_H__