#pragma once
#include <memory>
#include <unordered_set>
#include "Component.h"
#include "Drawable.h"

class Mesh;
class Transform;

class MeshInstance : public Drawable
{
	DELETE_COPY_MOVE(MeshInstance)
public:
	MeshInstance() = delete;
	explicit MeshInstance(Entity*);
	
	virtual ~MeshInstance();

	void Draw() override;
	void Initialize() override;
	void Deserialize(const Json& json) override;
	void SetMeshAndMaterial(const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Material>& material);
	void SetMaterial(const std::shared_ptr<Material>& material);
	std::shared_ptr<Material> GetMaterial() const override { return material; }
	void SetMesh(const std::shared_ptr<Mesh>& mesh);
	std::shared_ptr<Mesh> GetMesh() const { return mesh; }
	
private:
	std::shared_ptr<Mesh> mesh{};
	std::shared_ptr<Material> material{};
	std::unordered_set<GLint> enabledAttribs;
	GLuint vao{};
	Transform* transform{};

	void ApplyAppearance();
	void EnableVertexAttributes();
};

