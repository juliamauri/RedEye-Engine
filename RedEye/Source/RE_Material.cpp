#include "RE_Material.h"

#include "Application.h"
#include "FileSystem.h"
#include "ResourceManager.h"

#include "Globals.h"

RE_Material::RE_Material(const char* name) : name(name), ResourceContainer(name)
{
}

RE_Material::RE_Material(const char* name, rapidjson::Value * val) : name(name), ResourceContainer(name)
{
	shadingType = (RE_ShadingMode)val->FindMember("Shader Type")->value.GetInt();

	rapidjson::Value& valueArrays = val->FindMember("Diffuse Textures")->value;
	for (uint i = 0; i < (uint)valueArrays.GetArray().Size(); i += 2)
	{
		App->resources->CheckFileLoaded(valueArrays.GetArray()[i + 1].GetString(), valueArrays.GetArray()[i].GetString(), R_TEXTURE);
		tDiffuse.push_back(App->resources->At(valueArrays.GetArray()[i].GetString())->GetMD5());
	}

	valueArrays = val->FindMember("Diffuse Color")->value;
	cDiffuse.Set(valueArrays.GetArray()[0].GetFloat(), valueArrays.GetArray()[1].GetFloat(), valueArrays.GetArray()[2].GetFloat());

	valueArrays = val->FindMember("Specular Textures")->value;
	for (uint i = 0; i < (uint)valueArrays.GetArray().Size(); i += 2)
	{
		App->resources->CheckFileLoaded(valueArrays.GetArray()[i + 1].GetString(), valueArrays.GetArray()[i].GetString(), R_TEXTURE);
		tSpecular.push_back(App->resources->At(valueArrays.GetArray()[i].GetString())->GetMD5());
	}

	valueArrays = val->FindMember("Specular Color")->value;
	cSpecular.Set(valueArrays.GetArray()[0].GetFloat(), valueArrays.GetArray()[1].GetFloat(), valueArrays.GetArray()[2].GetFloat());

	valueArrays = val->FindMember("Ambient Textures")->value;
	for (uint i = 0; i < (uint)valueArrays.GetArray().Size(); i += 2)
	{
		App->resources->CheckFileLoaded(valueArrays.GetArray()[i + 1].GetString(), valueArrays.GetArray()[i].GetString(), R_TEXTURE);
		tAmbient.push_back(App->resources->At(valueArrays.GetArray()[i].GetString())->GetMD5());
	}

	valueArrays = val->FindMember("Ambient Color")->value;
	cAmbient.Set(valueArrays.GetArray()[0].GetFloat(), valueArrays.GetArray()[1].GetFloat(), valueArrays.GetArray()[2].GetFloat());

	valueArrays = val->FindMember("Emissive Textures")->value;
	for (uint i = 0; i < (uint)valueArrays.GetArray().Size(); i += 2)
	{
		App->resources->CheckFileLoaded(valueArrays.GetArray()[i + 1].GetString(), valueArrays.GetArray()[i].GetString(), R_TEXTURE);
		tEmissive.push_back(App->resources->At(valueArrays.GetArray()[i].GetString())->GetMD5());
	}

	valueArrays = val->FindMember("Emissive Color")->value;
	cEmissive.Set(valueArrays.GetArray()[0].GetFloat(), valueArrays.GetArray()[1].GetFloat(), valueArrays.GetArray()[2].GetFloat());

	valueArrays = val->FindMember("Transparent Color")->value;
	cTransparent.Set(valueArrays.GetArray()[0].GetFloat(), valueArrays.GetArray()[1].GetFloat(), valueArrays.GetArray()[2].GetFloat());

	backFaceCulling = val->FindMember("BackFace Culling")->value.GetBool();

	blendMode = val->FindMember("Blend Mode")->value.GetBool();

	valueArrays = val->FindMember("Opacity Textures")->value;
	for (uint i = 0; i < (uint)valueArrays.GetArray().Size(); i += 2)
	{
		App->resources->CheckFileLoaded(valueArrays.GetArray()[i + 1].GetString(), valueArrays.GetArray()[i].GetString(), R_TEXTURE);
		tOpacity.push_back(App->resources->At(valueArrays.GetArray()[i].GetString())->GetMD5());
	}
	
	opacity = val->FindMember("Opacity")->value.GetFloat();

	valueArrays = val->FindMember("Shininess Textures")->value;
	for (uint i = 0; i < (uint)valueArrays.GetArray().Size(); i += 2)
	{
		App->resources->CheckFileLoaded(valueArrays.GetArray()[i + 1].GetString(), valueArrays.GetArray()[i].GetString(), R_TEXTURE);
		tShininess.push_back(App->resources->At(valueArrays.GetArray()[i].GetString())->GetMD5());
	}

	shininess = val->FindMember("Shininess")->value.GetFloat();
	shininessStrenght = val->FindMember("Shininess Strenght")->value.GetFloat();

	refraccti = val->FindMember("Refraccti")->value.GetFloat();

	valueArrays = val->FindMember("Height Textures")->value;
	for (uint i = 0; i < (uint)valueArrays.GetArray().Size(); i += 2)
	{
		App->resources->CheckFileLoaded(valueArrays.GetArray()[i + 1].GetString(), valueArrays.GetArray()[i].GetString(), R_TEXTURE);
		tHeight.push_back(App->resources->At(valueArrays.GetArray()[i].GetString())->GetMD5());
	}

	valueArrays = val->FindMember("Normals Textures")->value;
	for (uint i = 0; i < (uint)valueArrays.GetArray().Size(); i += 2)
	{
		App->resources->CheckFileLoaded(valueArrays.GetArray()[i + 1].GetString(), valueArrays.GetArray()[i].GetString(), R_TEXTURE);
		tNormals.push_back(App->resources->At(valueArrays.GetArray()[i].GetString())->GetMD5());
	}

	valueArrays = val->FindMember("Reflection Textures")->value;
	for (uint i = 0; i < (uint)valueArrays.GetArray().Size(); i += 2)
	{
		App->resources->CheckFileLoaded(valueArrays.GetArray()[i + 1].GetString(), valueArrays.GetArray()[i].GetString(), R_TEXTURE);
		tReflection.push_back(App->resources->At(valueArrays.GetArray()[i].GetString())->GetMD5());
	}

	valueArrays = val->FindMember("Unknown Textures")->value;
	for (uint i = 0; i < (uint)valueArrays.GetArray().Size(); i += 2)
	{
		App->resources->CheckFileLoaded(valueArrays.GetArray()[i + 1].GetString(), valueArrays.GetArray()[i].GetString(), R_TEXTURE);
		tUnknown.push_back(App->resources->At(valueArrays.GetArray()[i].GetString())->GetMD5());
	}
}
     

RE_Material::~RE_Material()
{
}

void RE_Material::Serialize(JSONNode * node, rapidjson::Value * materialValue)
{
	rapidjson::Value val(rapidjson::kObjectType);

	val.AddMember(rapidjson::Value::StringRefType("Name"), rapidjson::Value().SetString(name.c_str(), node->GetDocument()->GetAllocator()), node->GetDocument()->GetAllocator());
	
	val.AddMember(rapidjson::Value::StringRefType("Shader Type"), rapidjson::Value().SetInt((int)shadingType), node->GetDocument()->GetAllocator());

	rapidjson::Value valueaAray(rapidjson::kArrayType);
	for (const char* tmd5 : tDiffuse)  valueaAray.PushBack(rapidjson::Value().SetString(tmd5, node->GetDocument()->GetAllocator()), node->GetDocument()->GetAllocator()).PushBack(rapidjson::Value().SetString(App->resources->At(tmd5)->GetFilePath(), node->GetDocument()->GetAllocator()), node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("Diffuse Textures"), valueaAray.Move(), node->GetDocument()->GetAllocator());

	valueaAray.SetArray();
	valueaAray.PushBack(cDiffuse.x, node->GetDocument()->GetAllocator()).PushBack(cDiffuse.y, node->GetDocument()->GetAllocator()).PushBack(cDiffuse.z, node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("Diffuse Color"), valueaAray.Move(), node->GetDocument()->GetAllocator());

	valueaAray.SetArray();
	for (const char* tmd5 : tSpecular)  valueaAray.PushBack(rapidjson::Value().SetString(tmd5, node->GetDocument()->GetAllocator()), node->GetDocument()->GetAllocator()).PushBack(rapidjson::Value().SetString(App->resources->At(tmd5)->GetFilePath(), node->GetDocument()->GetAllocator()), node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("Specular Textures"), valueaAray.Move(), node->GetDocument()->GetAllocator());

	valueaAray.SetArray();
	valueaAray.PushBack(cSpecular.x, node->GetDocument()->GetAllocator()).PushBack(cSpecular.y, node->GetDocument()->GetAllocator()).PushBack(cSpecular.z, node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("Specular Color"), valueaAray.Move(), node->GetDocument()->GetAllocator());

	valueaAray.SetArray();
	for (const char* tmd5 : tAmbient)  valueaAray.PushBack(rapidjson::Value().SetString(tmd5, node->GetDocument()->GetAllocator()), node->GetDocument()->GetAllocator()).PushBack(rapidjson::Value().SetString(App->resources->At(tmd5)->GetFilePath(), node->GetDocument()->GetAllocator()), node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("Ambient Textures"), valueaAray.Move(), node->GetDocument()->GetAllocator());

	valueaAray.SetArray();
	valueaAray.PushBack(cAmbient.x, node->GetDocument()->GetAllocator()).PushBack(cAmbient.y, node->GetDocument()->GetAllocator()).PushBack(cAmbient.z, node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("Ambient Color"), valueaAray.Move(), node->GetDocument()->GetAllocator());

	valueaAray.SetArray();
	for (const char* tmd5 : tEmissive)  valueaAray.PushBack(rapidjson::Value().SetString(tmd5, node->GetDocument()->GetAllocator()), node->GetDocument()->GetAllocator()).PushBack(rapidjson::Value().SetString(App->resources->At(tmd5)->GetFilePath(), node->GetDocument()->GetAllocator()), node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("Emissive Textures"), valueaAray.Move(), node->GetDocument()->GetAllocator());

	valueaAray.SetArray();
	valueaAray.PushBack(cEmissive.x, node->GetDocument()->GetAllocator()).PushBack(cEmissive.y, node->GetDocument()->GetAllocator()).PushBack(cEmissive.z, node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("Emissive Color"), valueaAray.Move(), node->GetDocument()->GetAllocator());

	valueaAray.SetArray();
	valueaAray.PushBack(cTransparent.x, node->GetDocument()->GetAllocator()).PushBack(cTransparent.y, node->GetDocument()->GetAllocator()).PushBack(cTransparent.z, node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("Transparent Color"), valueaAray.Move(), node->GetDocument()->GetAllocator());

	val.AddMember(rapidjson::Value::StringRefType("BackFace Culling"), rapidjson::Value().SetBool(backFaceCulling), node->GetDocument()->GetAllocator());
	
	val.AddMember(rapidjson::Value::StringRefType("Blend Mode"), rapidjson::Value().SetBool(blendMode), node->GetDocument()->GetAllocator());

	valueaAray.SetArray();
	for (const char* tmd5 : tOpacity)  valueaAray.PushBack(rapidjson::Value().SetString(tmd5, node->GetDocument()->GetAllocator()), node->GetDocument()->GetAllocator()).PushBack(rapidjson::Value().SetString(App->resources->At(tmd5)->GetFilePath(), node->GetDocument()->GetAllocator()), node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("Opacity Textures"), valueaAray.Move(), node->GetDocument()->GetAllocator());

	val.AddMember(rapidjson::Value::StringRefType("Opacity"), rapidjson::Value().SetFloat(opacity), node->GetDocument()->GetAllocator());

	valueaAray.SetArray();
	for (const char* tmd5 : tShininess)  valueaAray.PushBack(rapidjson::Value().SetString(tmd5, node->GetDocument()->GetAllocator()), node->GetDocument()->GetAllocator()).PushBack(rapidjson::Value().SetString(App->resources->At(tmd5)->GetFilePath(), node->GetDocument()->GetAllocator()), node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("Shininess Textures"), valueaAray.Move(), node->GetDocument()->GetAllocator());

	val.AddMember(rapidjson::Value::StringRefType("Shininess"), rapidjson::Value().SetFloat(shininess), node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("Shininess Strenght"), rapidjson::Value().SetFloat(shininessStrenght), node->GetDocument()->GetAllocator());

	val.AddMember(rapidjson::Value::StringRefType("Refraccti"), rapidjson::Value().SetFloat(refraccti), node->GetDocument()->GetAllocator());

	valueaAray.SetArray();
	for (const char* tmd5 : tHeight)  valueaAray.PushBack(rapidjson::Value().SetString(tmd5, node->GetDocument()->GetAllocator()), node->GetDocument()->GetAllocator()).PushBack(rapidjson::Value().SetString(App->resources->At(tmd5)->GetFilePath(), node->GetDocument()->GetAllocator()), node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("Height Textures"), valueaAray.Move(), node->GetDocument()->GetAllocator());

	valueaAray.SetArray();
	for (const char* tmd5 : tNormals)  valueaAray.PushBack(rapidjson::Value().SetString(tmd5, node->GetDocument()->GetAllocator()), node->GetDocument()->GetAllocator()).PushBack(rapidjson::Value().SetString(App->resources->At(tmd5)->GetFilePath(), node->GetDocument()->GetAllocator()), node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("Normals Textures"), valueaAray.Move(), node->GetDocument()->GetAllocator());

	valueaAray.SetArray();
	for (const char* tmd5 : tReflection)  valueaAray.PushBack(rapidjson::Value().SetString(tmd5, node->GetDocument()->GetAllocator()), node->GetDocument()->GetAllocator()).PushBack(rapidjson::Value().SetString(App->resources->At(tmd5)->GetFilePath(), node->GetDocument()->GetAllocator()), node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("Reflection Textures"), valueaAray.Move(), node->GetDocument()->GetAllocator());

	valueaAray.SetArray();
	for (const char* tmd5 : tUnknown)  valueaAray.PushBack(rapidjson::Value().SetString(tmd5, node->GetDocument()->GetAllocator()), node->GetDocument()->GetAllocator()).PushBack(rapidjson::Value().SetString(App->resources->At(tmd5)->GetFilePath(), node->GetDocument()->GetAllocator()), node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("Unknown Textures"), valueaAray.Move(), node->GetDocument()->GetAllocator());

	materialValue->PushBack(val, node->GetDocument()->GetAllocator());
}
