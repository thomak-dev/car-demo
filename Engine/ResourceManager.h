#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <assimp/scene.h>
#include "json.h"
#include "core.h"
#include "Singleton.h"
#include "ShaderElement.h"

class Mesh;
class Texture2D;
class Shader;
class Material;
class Entity;
class Font;

using Prefab = rapidjson::Document;

class ResourceManager : public virtual Singleton<ResourceManager>
{
	friend class ResourceManagerTest;
	DELETE_COPY_MOVE(ResourceManager)
public:
	ResourceManager();
	virtual ~ResourceManager();
	
	std::shared_ptr<Prefab> LoadPrefab(const std::string& shortPath);
	Entity* LoadEntity(const std::string& shortPath, Entity* parent);
	std::shared_ptr<std::string> LoadText(const std::string& shortPath);
	std::shared_ptr<Mesh> LoadMesh(const std::string& shortPath, bool ignoreRootTransform = false);
	std::shared_ptr<Texture2D> LoadTexture2D(const std::string& shortPath);
	std::shared_ptr<ShaderElement> LoadShaderElement(const std::string& shortPath, ShaderElement::Type);
	std::shared_ptr<Shader> LoadShader(const std::string& shortPath);
	std::shared_ptr<Material> LoadMaterial(const std::string& shortPath);
	std::shared_ptr<Font> LoadFont(const std::string& shortPath);

	void CleanUp();
	
private:
	std::unordered_map<std::string, std::shared_ptr<void>> resources;

	template<typename T, typename... Targs>
	std::shared_ptr<T> ExistingOrLoad(const std::string& shortPath, T*loadFunction(const std::string&, Targs...), Targs... args);
	Entity* ProcessNode(
		aiNode* node, 
		Entity* parent,
		const aiScene* scene, 
		std::vector<bool>& loadedMeshes,
		std::vector<std::string>& loadedMeshNames, 
		const std::vector<std::shared_ptr<Material>>& materials,
		const std::string& shortPath,
		bool bakeTransform = false);
};

template<typename T, typename... Targs>
std::shared_ptr<T> ResourceManager::ExistingOrLoad(const std::string& shortPath, T*loadFunction(const std::string&, Targs...), Targs... args)
{
	auto existing = resources.find(shortPath);
	if (existing == resources.end())
	{
		std::string nativePath{ MakeNativePath(shortPath) };

		std::shared_ptr<T> sharedResource{ loadFunction(nativePath, args...) };
		resources[shortPath] = sharedResource;
		return sharedResource;
	}
	else
		return std::static_pointer_cast<T>((*existing).second);
}