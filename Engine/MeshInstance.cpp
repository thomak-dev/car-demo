#include "MeshInstance.h"
#include <iostream>
#include <SDL.h>
#include <glm/gtc/type_ptr.hpp>
#include "Renderer.h"
#include "Transform.h"
#include "Mesh.h"
#include "Material.h"
#include "ShaderProgram.h"
#include "Texture2D.h"
#include "Material.h"
#include "ResourceManager.h"
#include "Entity.h"

MeshInstance::MeshInstance(Entity& entity)
	: Drawable{ entity }
{
	glGenVertexArrays(1, &vao);
	SetMeshAndMaterial(nullptr, nullptr);
}

void MeshInstance::Initialize()
{
	Component::Initialize();
	transform = entity.GetComponent<Transform>();
}

void MeshInstance::Deserialize(const Json& json)
{
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;
	if (json.HasMember("mesh"))
		mesh = ResourceManager::Instance().LoadMesh(json["mesh"].GetString());
	if (json.HasMember("material"))
		material = ResourceManager::Instance().LoadMaterial(json["material"].GetString());
	SetMeshAndMaterial(mesh, material);
}

MeshInstance::~MeshInstance()
{
	glDeleteVertexArrays(1, &vao);
}

void MeshInstance::Draw()
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, RenderDebugId::MeshInstance, -1, "MeshInstance::Draw");
	Renderer::Instance().SetModel(transform->WorldMatrix());
	
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, mesh->indices.size(), GL_UNSIGNED_INT, nullptr);
	glPopDebugGroup();
}

void MeshInstance::SetMaterial(const std::shared_ptr<Material>& material)
{
	if (!material)
		this->material = ResourceManager::Instance().LoadMaterial("");
	else
		this->material = material;

	ApplyAppearance();
}

void MeshInstance::SetMesh(const std::shared_ptr<Mesh>& mesh)
{
	if (!mesh)
		this->mesh = std::make_shared<Mesh>();
	else
		this->mesh = mesh;

	ApplyAppearance();
}

void MeshInstance::SetMeshAndMaterial(const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Material>& material)
{
	if (!mesh)
		this->mesh = std::make_shared<Mesh>();
	else
		this->mesh = mesh;

	if (!material)
		this->material = ResourceManager::Instance().LoadMaterial("");
	else
		this->material = material;

	ApplyAppearance();
}

void MeshInstance::ApplyAppearance()
{
	glBindVertexArray(vao);
	mesh->Bind();
	EnableVertexAttributes();
}

void MeshInstance::EnableVertexAttributes()
{
	const auto& shaderProgram = material->GetShaderProgram();
	shaderProgram.Use();
	GLuint programObject = shaderProgram.GetProgramObject();

	for(GLint attrib : enabledAttribs)
	{
		glDisableVertexAttribArray(attrib);
	}
	enabledAttribs.clear();

	GLint attrib = glGetAttribLocation(programObject, "position");
	if (attrib != -1)
	{
		enabledAttribs.insert(attrib);
		glEnableVertexAttribArray(attrib);
		glVertexAttribPointer(attrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}

	attrib = glGetAttribLocation(programObject, "normal");
	if (attrib != -1)
	{
		enabledAttribs.insert(attrib);
		glEnableVertexAttribArray(attrib);
		glVertexAttribPointer(attrib, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(sizeof(mesh->vertices[0]) * mesh->vertices.size()));
	}

	attrib = glGetAttribLocation(programObject, "tangent");
	if (attrib != -1)
	{
		enabledAttribs.insert(attrib);
		glEnableVertexAttribArray(attrib);
		glVertexAttribPointer(attrib, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(sizeof(mesh->vertices[0]) * mesh->vertices.size() * 2));
	}

	attrib = glGetAttribLocation(programObject, "uv");
	if (attrib != -1)
	{
		enabledAttribs.insert(attrib);
		glEnableVertexAttribArray(attrib);
		glVertexAttribPointer(attrib, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(sizeof(mesh->vertices[0]) * mesh->vertices.size() * 3));
	}

	attrib = glGetAttribLocation(programObject, "color");
	if (attrib != -1)
	{
		SDL_assert(false);
		enabledAttribs.insert(attrib);
		//glEnableVertexAttribArray(uvAttrib);
		//glVertexAttribPointer(uvAttrib, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(sizeof(mesh->vertices[0]) * mesh->vertices.size() * 3));
	}
}
