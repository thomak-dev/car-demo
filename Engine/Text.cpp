#include "Text.h"
#include "Entity.h"
#include "Material.h"
#include "Renderer.h"
#include "Transform.h"
#include "Texture2D.h"
#include "Font.h"

void Text::Initialize()
{
	Component::Initialize();
	material = std::make_shared<Material>(*material);
	material->SetProperty("colorMap", font->GetTexture(pointSize));
	transform = entity->GetComponent<Transform>();
}

void Text::Deserialize(const Json& json)
{
	if (json.HasMember("point_size"))
		pointSize = json["point_size"].GetUint();
	if (json.HasMember("content"))
		content = json["content"].GetString();
	if (json.HasMember("material"))
		material = ResourceManager::Instance().LoadMaterial(json["material"].GetString());
	else
		material = ResourceManager::Instance().LoadMaterial("");
	if(json.HasMember("alignment"))
	{
		alignment = json["alignment"].GetInt();
	}
	if (json.HasMember("anchor"))
	{
		const auto& arr = json["anchor"].GetArray();
		anchor[0] = arr[0].GetInt();
		anchor[1] = arr[1].GetInt();
	}
	if(json.HasMember("font"))
	{
		font = ResourceManager::Instance().LoadFont(json["font"].GetString());
	}
	else
		font = ResourceManager::Instance().LoadFont("Fonts/consola.ttf");

}

void Text::Draw()
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, RenderDebugId::Text, -1, "Text::Draw");
	Renderer::Instance().SetModel(transform->WorldMatrix());
	Renderer::Instance().DrawText(content, *font, pointSize, anchor[0], anchor[1], alignment);
	glPopDebugGroup();
}
