#include "ResourceManager.h"
#include <array>
#include <sstream>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include "core.h"
#include "Mesh.h"
#include "Texture2D.h"
#include "Shader.h"
#include "Material.h"
#include "math_utility.h"
#include "Entity.h"
#include "MeshInstance.h"
#include "Transform.h"
#include "Font.h"

ResourceManager::ResourceManager()
{
	IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
	TTF_Init();
}

ResourceManager::~ResourceManager()
{
	IMG_Quit();
}

//Audio::Sound* LoadSoundFromFile(const std::string& fullPath)
//{
//	return new Audio::Sound{fullPath};
//}
//
//std::shared_ptr<Audio::Sound> ResourceManager::LoadSound(const std::string& shortPath)
//{
//	return ExistingOrLoad(shortPath, LoadSoundFromFile);
//}

std::string* LoadTextFromFile(const std::string& fullPath)
{
	return new std::string{ ReadTextFile(fullPath) };
}

std::shared_ptr<std::string> ResourceManager::LoadText(const std::string& shortPath)
{
	return ExistingOrLoad(shortPath, LoadTextFromFile);
}

Mesh* CreateMesh(aiMesh* meshData, const glm::mat4 transformation = glm::mat4{})
{
	Mesh* mesh = new Mesh();

	auto importedVerts = meshData->mVertices;
	auto importedNormals = meshData->mNormals;
	auto importedTangents = meshData->mTangents;
	auto importedUvs = meshData->mTextureCoords;
	auto numVertices = meshData->mNumVertices;

	for (decltype(numVertices) i = 0; i < numVertices; ++i)
	{
		glm::vec4 vertex{ importedVerts[i].x, importedVerts[i].y, importedVerts[i].z, 1 };
		mesh->vertices.push_back(transformation * vertex);

		glm::vec4 normal{ importedNormals[i].x, importedNormals[i].y, importedNormals[i].z, 0 };
		mesh->normals.push_back(glm::inverseTranspose(transformation) * normal);

		if (meshData->HasTangentsAndBitangents())
		{
			glm::vec4 tangent{ importedTangents[i].x, importedTangents[i].y, importedTangents[i].z, 0 };
			mesh->tangents.push_back(transformation * tangent);
		}

		glm::vec2 uv;
		if (meshData->HasTextureCoords(0))
		{
			uv.x = importedUvs[0][i].x;
			uv.y = importedUvs[0][i].y;
		}

		mesh->uvs.push_back(uv);
	}
	auto faces = meshData->mFaces;
	auto numFaces = meshData->mNumFaces;

	for (decltype(numFaces) i = 0; i < numFaces; ++i)
	{
		mesh->indices.push_back(faces[i].mIndices[0]);
		mesh->indices.push_back(faces[i].mIndices[1]);
		mesh->indices.push_back(faces[i].mIndices[2]);
	}

	return mesh;
}

Mesh* LoadMeshFromFile(const std::string& path, bool ignoreRootTransform)
{
	Assimp::Importer importer{};
	const aiScene* scene = importer.ReadFile(
		path,
		aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality
	);
	SDL_assert(scene->HasMeshes());
	if (ignoreRootTransform)
		return CreateMesh(scene->mMeshes[0]);
	else
	{
		glm::mat4 matrix;
		auto trans = scene->mRootNode->mTransformation * scene->mRootNode->mChildren[0]->mTransformation;
		AiToGlmMatrix(matrix, trans);
		return CreateMesh(scene->mMeshes[0], matrix);
	}
}

std::shared_ptr<Mesh> ResourceManager::LoadMesh(const std::string& shortPath, bool ignoreRootTransform)
{
	return ExistingOrLoad(shortPath, LoadMeshFromFile, ignoreRootTransform);
}

std::shared_ptr<Entity> ResourceManager::ProcessNode(aiNode* node, Entity* parent, const aiScene* scene, std::vector<bool>& loadedMeshes, std::vector<std::string>& loadedMeshNames, const std::vector<std::shared_ptr<Material>>& materials, const std::string& shortPath, bool bakeTransform)
{
	auto entity = parent->CreateChild(node->mName.C_Str());
	
	Transform* transform = entity->AddComponent<Transform>();
	glm::mat4 matrix;
	AiToGlmMatrix(matrix, node->mTransformation);
	if(!bakeTransform)
		transform->SetMatrix(matrix);
	std::stringstream sstream;
	sstream << shortPath << '/' << node->mName.C_Str();
	std::string newPath{ sstream.str() };

	for (unsigned int i = 0; i < node->mNumMeshes; ++i)
	{
		std::shared_ptr<Mesh> mesh;
		sstream = std::stringstream{};
		sstream << newPath << '_' << i;
		std::string name{ sstream.str() };
		if (!loadedMeshes[node->mMeshes[i]])
		{
			Mesh* newMesh = CreateMesh(scene->mMeshes[node->mMeshes[i]], bakeTransform? matrix : glm::mat4{});
			mesh = std::shared_ptr<Mesh>{ newMesh };
			resources[name] = mesh;
			loadedMeshes[node->mMeshes[i]] = true;
			loadedMeshNames[node->mMeshes[i]] = name;
		}
		else
			mesh = LoadMesh(loadedMeshNames[node->mMeshes[i]]);

		if (i == 0)
		{
			MeshInstance* meshInstance = entity->AddComponent<MeshInstance>();
			meshInstance->SetMeshAndMaterial(mesh, materials[scene->mMeshes[node->mMeshes[i]]->mMaterialIndex]);
		}
		else
		{
			auto child = entity->CreateChild("submesh_" + std::to_string(i));
			child->AddComponent<Transform>();
			MeshInstance* meshInstance = child->AddComponent<MeshInstance>();
			meshInstance->SetMeshAndMaterial(mesh, materials[scene->mMeshes[node->mMeshes[i]]->mMaterialIndex]);
		}
	}

	for (unsigned i = 0; i < node->mNumChildren; ++i)
	{
		if (bakeTransform)
			node->mChildren[i]->mTransformation = node->mTransformation * node->mChildren[i]->mTransformation;
		ProcessNode(node->mChildren[i], entity.get(), scene, loadedMeshes, loadedMeshNames, materials, newPath);
	}
		

	return entity;
}

rapidjson::Document* LoadJsonFile(const std::string& path)
{
	rapidjson::Document* jsonDoc = new rapidjson::Document;
	jsonDoc->Parse(ReadTextFile(path));
	return jsonDoc;
}

std::shared_ptr<Json> ResourceManager::LoadJson(const std::string& shortPath)
{
	return ExistingOrLoad(shortPath, LoadJsonFile);
}

std::shared_ptr<Entity> ResourceManager::LoadEntity(const std::string& shortPath, Entity* parent)
{
	Assimp::Importer importer{};
	std::string nativePath{ MakeNativePath(shortPath) };
	const aiScene* scene = importer.ReadFile(nativePath, aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality);
	SDL_assert(scene);
	std::string settingsJson{ ReadTextFile(MakeNativePath(shortPath + ".settings")) };
	std::unordered_map<std::string, std::string> materialMap;
	if (settingsJson.length() > 0)
	{
		rapidjson::Document doc;
		doc.Parse(settingsJson);
		SDL_assert(doc.IsObject());
		if (doc.HasMember("material_map"))
		{
			for (auto& mapping : doc["material_map"].GetObject())
			{
				materialMap[mapping.name.GetString()] = mapping.value.GetString();
			}
		}
	}
	std::vector<bool> loadedMeshes(scene->mNumMeshes);
	std::vector<std::string> loadedMeshNames(scene->mNumMeshes);
	std::vector<std::shared_ptr<Material>> materials;
	for (unsigned i = 0; i < scene->mNumMaterials; ++i)
	{
		aiString matName;
		scene->mMaterials[i]->Get(AI_MATKEY_NAME, matName);
		materials.push_back(LoadMaterial(materialMap[matName.C_Str()]));
	}
	scene->mRootNode->mChildren[0]->mTransformation = scene->mRootNode->mTransformation * scene->mRootNode->mChildren[0]->mTransformation;
	std::shared_ptr<Entity> entity{ ProcessNode(scene->mRootNode->mChildren[0], parent, scene, loadedMeshes, loadedMeshNames, materials, shortPath, true) };

	return entity;
}

Texture2D* LoadTexture2DFromFileOrEmpty(const std::string& fullPath)
{
	SDL_Surface* surface = IMG_Load(fullPath.c_str());
	if (surface == nullptr)
		return nullptr;

	Texture2D* texture = new Texture2D{surface};
	rapidjson::Document json;
	std::string jsonString{ ReadTextFile(fullPath + ".settings") };
	if (jsonString.length() > 0)
	{
		json.Parse(jsonString);
		texture->SetFiltering(Texture2D::FilteringFromString(json["filtering"].GetString()));
		texture->SetWrapping(Texture2D::WrappingFromString(json["wrapping"].GetString()));
	}
	return texture;
}

std::shared_ptr<Texture2D> ResourceManager::LoadTexture2D(const std::string& shortPath)
{
	return ExistingOrLoad(shortPath, LoadTexture2DFromFileOrEmpty);
}

ShaderElement* LoadShaderElementFromFile(const std::string& fullPath, ShaderElement::Type type)
{
	return new ShaderElement{ ReadTextFile(fullPath), type };
}

std::shared_ptr<ShaderElement> ResourceManager::LoadShaderElement(const std::string& shortPath, ShaderElement::Type type)
{
	return ExistingOrLoad(shortPath, LoadShaderElementFromFile, type);
}

Shader* LoadShaderFromFile(const std::string& path)
{
	Shader* result = new Shader;
	auto json = ReadTextFile(path);
	rapidjson::Document doc;
	doc.Parse(json);
	SDL_assert(doc.IsObject());
	if(doc["elements"].HasMember("vertex"))
		for (auto& elem : doc["elements"]["vertex"].GetArray())
			result->AddElement(ResourceManager::Instance().LoadShaderElement(elem.GetString(), ShaderElement::Type::Vertex));
	if (doc["elements"].HasMember("geometry"))
		for (auto& elem : doc["elements"]["geometry"].GetArray())
			result->AddElement(ResourceManager::Instance().LoadShaderElement(elem.GetString(), ShaderElement::Type::Geometry));
	if (doc["elements"].HasMember("fragment"))
		for (auto& elem : doc["elements"]["fragment"].GetArray())
			result->AddElement(ResourceManager::Instance().LoadShaderElement(elem.GetString(), ShaderElement::Type::Fragment));
	return result;
}

std::shared_ptr<Shader> ResourceManager::LoadShader(const std::string& shortPath)
{
	return ExistingOrLoad(shortPath, LoadShaderFromFile);
}

Material* LoadMaterialFromFile(const std::string& path)
{
	auto json = ReadTextFile(path);
	rapidjson::Document doc;
	doc.Parse(json);
	SDL_assert(doc.IsObject());
	Material* mat = new Material{ ShaderProgram{ResourceManager::Instance().LoadShader(doc["shader"].GetString())} };

	if(doc.HasMember("properties"))
		for (const auto& prop : doc["properties"].GetArray())
		{
			const auto& entry = prop.GetObject().begin();
			const auto& name = entry->name;
			const auto& value = entry->value;
			SDL_assert(value.IsFloat() || value.IsString() || (value.IsArray() && value.Size() == 4));
			if (value.IsFloat())
				mat->SetProperty(name.GetString(), value.GetFloat());
			else if (value.IsString())
				mat->SetProperty(name.GetString(), ResourceManager::Instance().LoadTexture2D(value.GetString()));
			else if (value.IsArray())
			{
				glm::vec4 vec{ value[0].GetFloat(), value[1].GetFloat(), value[2].GetFloat(), value[3].GetFloat() };
				mat->SetProperty(name.GetString(), vec);
			}
		}

	if(doc.HasMember("cull"))
		mat->cull =  doc["cull"].GetBool();
	if(doc.HasMember("test_depth"))
		mat->testDepth = doc["test_depth"].GetBool();
	if(doc.HasMember("write_depth"))
		mat->writeDepth = doc["write_depth"].GetBool();
	if (doc.HasMember("blend_op_color"))
		mat->blendOpColor = Blend::OperationFromString(doc["blend_op_color"].GetString());
	if (doc.HasMember("blend_op_alpha"))
		mat->blendOpAlpha = Blend::OperationFromString(doc["blend_op_alpha"].GetString());
	if (doc.HasMember("blend_src_color"))
		mat->blendSrcColor = Blend::CoefficientFromString(doc["blend_src_color"].GetString());
	if (doc.HasMember("blend_src_alpha"))
		mat->blendSrcAlpha = Blend::CoefficientFromString(doc["blend_src_alpha"].GetString());
	if (doc.HasMember("blend_dst_color"))
		mat->blendDstColor = Blend::CoefficientFromString(doc["blend_dst_color"].GetString());
	if (doc.HasMember("blend_dst_alpha"))
		mat->blendDstAlpha = Blend::CoefficientFromString(doc["blend_dst_alpha"].GetString());
	if (doc.HasMember("ordinal"))
		mat->ordinal = doc["ordinal"].GetInt();

	return mat;
}

std::shared_ptr<Material> ResourceManager::LoadMaterial(const std::string& shortPath)
{
	if (shortPath.length() > 0)
		return ExistingOrLoad(shortPath, LoadMaterialFromFile);
	else
		return LoadMaterial("Materials/default_lambert.mat");
}

Font* LoadFontFromFile(const std::string& fullPath)
{
	SDL_RWops* fontFile = SDL_RWFromFile(fullPath.c_str(), "r");
	SDL_assert(fontFile);
	Font* font = new Font(fontFile);
	return font;
}

std::shared_ptr<Font> ResourceManager::LoadFont(const std::string& shortPath)
{
	return ExistingOrLoad(shortPath, LoadFontFromFile);
}

void ResourceManager::CleanUp()
{
	size_t oldSize = resources.size();
	for (auto it = resources.begin(); it != resources.end();)
	{
		if (it->second.unique())
			it = resources.erase(it);
		else
			++it;
	}

	// clean up until there is no change in size, to clean up all dependencies
	if (resources.size() == oldSize)
		return;
	else
		CleanUp();
}