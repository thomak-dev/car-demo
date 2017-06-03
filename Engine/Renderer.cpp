#include "Renderer.h"
#include <algorithm>
#include <cctype>
#include <glm/gtc/type_ptr.hpp>
#include "Entity.h"
#include "Camera.h"
#include "Drawable.h"
#include "Transform.h"
#include "Light.h"
#include "math_utility.h"
#include "Material.h"
#include "Texture2D.h"
#include "GameWindow.h"
#include "Physics.h"
#include "Font.h"

struct ViewProjection
{
	glm::mat4 view;
	glm::mat4 proj;
	glm::vec3 viewPos;
};

struct LightData
{
	glm::vec4 dirPos;
	glm::vec4 color;
	float linearAtten{ 0 };
	float squareAtten{ 0 };
	float cutoff{ 1 / 32.0f };
};

Renderer::Renderer()
{
	glClearColor(1, 0, 1, 1);

	glGenBuffers(UniformBufferIndex::Count, ubos);
	glBindBuffer(GL_UNIFORM_BUFFER, ubos[UniformBufferIndex::ViewProjection]);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(ViewProjection), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, ubos[UniformBufferIndex::Lights]);
	glBufferData(GL_UNIFORM_BUFFER, LightCount * CeilToBoundary(sizeof(LightData), 16), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, UniformBufferIndex::ViewProjection, ubos[UniformBufferIndex::ViewProjection]);
	glBindBufferBase(GL_UNIFORM_BUFFER, UniformBufferIndex::Lights, ubos[UniformBufferIndex::Lights]);

	if (cull)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);

	if (writeDepth)
		glDepthMask(GL_TRUE);
	else
		glDepthMask(GL_FALSE);

	if (testDepth)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);


	glEnable(GL_BLEND);
	glBlendEquationSeparate(blendOpColor, blendOpAlpha);
	glBlendFuncSeparate(blendSrcColor, blendDstColor, blendSrcAlpha, blendDstAlpha);
	physicsDebugDrawer.setDebugMode(
		PhysicsDebugDrawer::DBG_DrawWireframe |
		PhysicsDebugDrawer::DBG_DrawAabb |
		/*PhysicsDebugDrawer::DBG_DrawNormals |*/
		PhysicsDebugDrawer::DBG_DrawContactPoints |
		PhysicsDebugDrawer::DBG_DrawConstraints |
		PhysicsDebugDrawer::DBG_DrawConstraintLimits |
		PhysicsDebugDrawer::DBG_DrawFrames
	);

	glCreateBuffers(1, &textVbo);
	glCreateVertexArrays(1, &textVao);
	glBindBuffer(GL_ARRAY_BUFFER, textVbo);
	glBindVertexArray(textVao);
	glEnableVertexAttribArray(VertexAttributeLocation::Position);
	glVertexAttribPointer(VertexAttributeLocation::Position, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
	glEnableVertexAttribArray(VertexAttributeLocation::UV);
	glVertexAttribPointer(VertexAttributeLocation::UV, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
}


Renderer::~Renderer()
{
	glDeleteBuffers(2, ubos);
	glDeleteBuffers(1, &textVbo);
	glDeleteVertexArrays(1, &textVao);
}

void Renderer::RegisterCamera(Camera* camera)
{
	cameras.push_back(camera);
}

void Renderer::RegisterDrawable(Drawable* renderable)
{
	drawables.push_back(renderable);
}

void Renderer::RegisterLight(Light* light)
{
	lights.push_back(light);
}

void Renderer::UnregisterCamera(Camera* camera)
{
	cameras.erase(std::find(cameras.begin(), cameras.end(), camera));
}

void Renderer::UnregisterDrawable(Drawable* renderable)
{
	drawables.erase(std::find(drawables.begin(), drawables.end(), renderable));
}

void Renderer::UnregisterLight(Light* light)
{
	lights.erase(std::find(lights.begin(), lights.end(), light));
}

void Renderer::SetCull(bool shouldCull)
{
	if (cull != shouldCull)
	{
		cull = shouldCull;
		if (cull)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);
	}
}

void Renderer::SetWriteDepth(bool shouldWrite)
{
	if (writeDepth != shouldWrite)
	{
		writeDepth = shouldWrite;
		if (writeDepth)
			glDepthMask(GL_TRUE);
		else
			glDepthMask(GL_FALSE);
	}
}

void Renderer::SetTestDepth(bool shouldTest)
{
	if (testDepth != shouldTest)
	{
		testDepth = shouldTest;
		if (testDepth)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
	}
}

void Renderer::SetBlendOperators(Blend::Operation::Type blendOpColor, Blend::Operation::Type blendOpAlpha)
{
	if (this->blendOpColor != blendOpColor || this->blendOpAlpha != blendOpAlpha)
	{
		this->blendOpColor = blendOpColor;
		this->blendOpAlpha = blendOpAlpha;
		glBlendEquationSeparate(blendOpColor, blendOpAlpha);
	}
}

void Renderer::SetBlendCoefficients(Blend::Coefficient::Type blendSrcColor, Blend::Coefficient::Type blendSrcAlpha, Blend::Coefficient::Type blendDstColor, Blend::Coefficient::Type blendDstAlpha)
{
	if (this->blendSrcColor != blendSrcColor || this->blendSrcAlpha != blendSrcAlpha || this->blendDstColor != blendDstColor || this->blendDstAlpha != blendDstAlpha)
	{
		this->blendSrcColor = blendSrcColor;
		this->blendSrcAlpha = blendSrcAlpha;
		this->blendDstColor = blendDstColor;
		this->blendDstAlpha = blendDstAlpha;
		glBlendFuncSeparate(blendSrcColor, blendDstColor, blendSrcAlpha, blendDstAlpha);
	}
}

void Renderer::Render()
{
	ViewProjection vp;

	sort(drawables.begin(), drawables.end(), [](Drawable* a, Drawable* b) { return a->GetMaterial()->ordinal < b->GetMaterial()->ordinal; });

	for (Camera* camera : cameras)
	{
		currentCamera = camera;
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, RenderDebugId::Camera, -1, "Camera");
		glViewport(0, 0, GameWindow::Instance().Width(), GameWindow::Instance().Height());
		GLbitfield clearFlags = 0;
		if(camera->clearDepth)
		{
			SetWriteDepth(true);
			clearFlags |= GL_DEPTH_BUFFER_BIT;
		}
		if(camera->clearColor)
		{
			glClearColor(camera->background.r, camera->background.g, camera->background.b, camera->background.a);
			clearFlags |= GL_COLOR_BUFFER_BIT;
		}
		if(clearFlags)
			glClear(clearFlags);
		vp.view = camera->ViewMatrix();
		vp.proj = camera->ProjMatrix();
		vp.viewPos = camera->GetEntity()->GetComponent<Transform>()->WorldPosition();
		glBindBuffer(GL_UNIFORM_BUFFER, ubos[UniformBufferIndex::ViewProjection]);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ViewProjection), &vp);
		std::sort(lights.begin(), lights.end(), [&](const Light* a, const Light* b) {
			if (a->type != b->type)
				return a->type == Light::Type::Directional;
			if (a->type == Light::Type::Directional)
				return 
					glm::dot(a->GetEntity()->GetComponent<Transform>()->Forward(), camera->GetEntity()->GetComponent<Transform>()->Forward()) >
					glm::dot(b->GetEntity()->GetComponent<Transform>()->Forward(), camera->GetEntity()->GetComponent<Transform>()->Forward());

			return 
				glm::distance(a->GetEntity()->GetComponent<Transform>()->WorldPosition(), vp.viewPos) <
				glm::distance(b->GetEntity()->GetComponent<Transform>()->WorldPosition(), vp.viewPos);
				

		});
		glBindBuffer(GL_UNIFORM_BUFFER, ubos[UniformBufferIndex::Lights]);
		for (size_t i = 0; i < LightCount; i++)
		{
			LightData lightData{};
			if (i < lights.size())
			{
				if(lights[i]->type == Light::Type::Point)
					lightData.dirPos = glm::vec4(lights[i]->GetEntity()->GetComponent<Transform>()->WorldPosition(), 1);
				else
					lightData.dirPos = glm::vec4(lights[i]->GetEntity()->GetComponent<Transform>()->Forward(), 0);
				lightData.cutoff = lights[i]->Cutoff();
				lightData.color = glm::max(lights[i]->color, 0.f);
				lightData.linearAtten = lights[i]->LinearAttenuation();
				lightData.squareAtten = lights[i]->SquareAttenuation();
			}
			glBufferSubData(GL_UNIFORM_BUFFER, i * CeilToBoundary(sizeof(LightData), 16), sizeof(LightData), &lightData);
		}

		for (auto drawable : drawables)
		{
			if(drawable->GetEntity()->flags & camera->GetEntity()->flags)
			{
				SetMaterial(drawable->GetMaterial());
				drawable->Draw();
			}
		}

		if (camera->IsMain() && Physics::Instance().Visualize())
			Physics::Instance().DebugDraw();

		glPopDebugGroup();
	}
}

void Renderer::SetModel(const glm::mat4& modelMatrix)
{
	glUniformMatrix4fv(currentMaterial->uniformModel, 1, false, glm::value_ptr(modelMatrix));
}

void Renderer::SetMaterial(const std::shared_ptr<Material>& material)
{
	if (material != currentMaterial)
	{
		SetCull(material->cull);
		SetWriteDepth(material->writeDepth);
		SetTestDepth(material->testDepth);
		SetBlendOperators(material->blendOpColor, material->blendOpAlpha);
		SetBlendCoefficients(material->blendSrcColor, material->blendSrcAlpha, material->blendDstColor, material->blendDstAlpha);

		material->GetShaderProgram().Use();

		int textureUnitOffset = 0;
		int textureUnitStart = GL_TEXTURE0;
		for (auto& property : material->GetProperties())
		{
			if (property.type == Material::Property::Type::Float && property.dirty)
			{
				glUniform1f(material->uniformLocations[property.name], property.GetFloat());
				property.dirty = false;
			}
			else if (property.type == Material::Property::Type::Texture2D)
			{
				int currentTextureUnit = textureUnitStart + textureUnitOffset;
				SDL_assert(currentTextureUnit < GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
				glActiveTexture(currentTextureUnit);
				property.GetTexture2D()->Bind();
				++textureUnitOffset;
			}
			else if (property.type == Material::Property::Type::Vector4 && property.dirty)
			{
				glUniform4fv(material->uniformLocations[property.name], 1, glm::value_ptr(property.GetVector4()));
				property.dirty = false;
			}
		}

		currentMaterial = material;
	}
}

void Renderer::DrawText(const std::string& text, Font& font, int pointSize, int horizontalAnchor, int verticalAnchor, int alignment)
{
	if (text.size() == 0 || pointSize == 0)
		return;

	const auto& textureAndMetrics = font.GetTextureAndMetrics(pointSize);
	auto texture = textureAndMetrics.first;
	const auto& metrics = textureAndMetrics.second;

	textVertices.clear();
	lines.clear(); 

	float totalWidth = text.length() * metrics.advance;
	int line = 0;
	lines.emplace_back();
	float maxWidth = 0;
	for (size_t i = 0; i < text.length(); ++i)
	{
		char ch = text[i];
		if (ch == '\n')
		{
			if (lines[line].width > maxWidth)
				maxWidth = lines[line].width;
			lines.emplace_back();
			++line;
			continue;
		}			
		else if (std::isspace(ch))
			ch = ' ';

		unsigned charIndex = ch - ' ';

		lines[line].quads.emplace_back(
			lines[line].quads.size() * metrics.advance,
			0,
			metrics.advance,
			metrics.height,
			static_cast<float>(((charIndex % metrics.columns) * metrics.tileWidth)) / texture->Width(),
			static_cast<float>((texture->Height() - (charIndex / metrics.columns) * metrics.tileHeight - 1)) / texture->Height()
		);
		lines[line].width += metrics.advance;
	}

	float totalHeight = lines.size() * metrics.lineSkip;
	float padding = metrics.lineSkip - metrics.height;

	for (int i = 0; i < lines.size(); ++i)
	{
		for (int j = 0; j < lines[i].quads.size(); ++j)
		{
			const TextQuad& quad = lines[i].quads[j];
			float horizontalOffset = -lines[i].width / 2 + alignment * (maxWidth - lines[i].width) / 2 - horizontalAnchor * (maxWidth / 2);
			float verticalOffset = totalHeight/2 - verticalAnchor * (totalHeight / 2) - i * metrics.lineSkip + (verticalAnchor - 1) * (padding/2);
			// TL
			textVertices.emplace_back( 
				quad.x + horizontalOffset, quad.y + verticalOffset,
				0, quad.u, quad.v 
			);
			// BL
			textVertices.emplace_back(
				quad.x + horizontalOffset, quad.y - metrics.height + verticalOffset,
				0, quad.u, quad.v - metrics.height / static_cast<float>(texture->Height())
			);
			// TR
			textVertices.emplace_back(
				quad.x + metrics.advance + horizontalOffset, quad.y + verticalOffset,
				0, quad.u + metrics.advance / static_cast<float>(texture->Width()), quad.v 
			);
			// BL
			textVertices.emplace_back(
				quad.x + horizontalOffset, quad.y - metrics.height + verticalOffset,
				0, quad.u, quad.v - metrics.height / static_cast<float>(texture->Height())
			);
			// BR
			textVertices.emplace_back(
				quad.x + metrics.advance + horizontalOffset, quad.y - metrics.height + verticalOffset,
				0, quad.u + metrics.advance / static_cast<float>(texture->Width()), quad.v - metrics.height / static_cast<float>(texture->Height())
			);
			// TR
			textVertices.emplace_back(
				quad.x + metrics.advance + horizontalOffset, quad.y + verticalOffset,
				0, quad.u + metrics.advance / static_cast<float>(texture->Width()), quad.v
			);
		}
	}
	if(textVertices.size() > 0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, textVbo);
		glBufferData(GL_ARRAY_BUFFER, textVertices.size() * sizeof(TextVertex), textVertices.data(), GL_STREAM_DRAW);
		glBindVertexArray(textVao);
		glDrawArrays(GL_TRIANGLES, 0, textVertices.size());
	}
}

namespace Blend
{
	static std::unordered_map<std::string, Operation::Type> strToOp
	{
		{ "Add", Operation::Add },
		{ "Subtract", Operation::Subtract },
		{ "ReverseSubtract", Operation::ReverseSubtract }
	};
	static std::unordered_map<std::string, Coefficient::Type> strToCoeff
	{
		{ "Zero", Coefficient::Zero },
		{ "One", Coefficient::One },
		{ "SrcColor", Coefficient::SrcColor },
		{ "OneMinusSrcColor", Coefficient::OneMinusSrcColor },
		{ "DstColor", Coefficient::DstColor },
		{ "OneMinusDstColor", Coefficient::OneMinusDstColor },
		{ "SrcAlpha", Coefficient::SrcAlpha },
		{ "OneMinusSrcAlpha", Coefficient::OneMinusSrcAlpha },
		{ "DstAlpha", Coefficient::DstAlpha },
		{ "OneMinusDstAlpha", Coefficient::OneMinusDstAlpha },
		{ "ConstantColor", Coefficient::ConstantColor },
		{ "OneMinusConstantColor", Coefficient::OneMinusConstantColor },
		{ "ConstantAlpha", Coefficient::ConstantAlpha },
		{ "OneMinusConstantAlpha", Coefficient::OneMinusConstantAlpha },
		{ "SrcAlphaSaturate", Coefficient::SrcAlphaSaturate },
		{ "Src1Color", Coefficient::Src1Color },
		{ "Src1Alpha", Coefficient::Src1Alpha }
	};

	Operation::Type OperationFromString(const std::string& value)
	{
		const auto& found = strToOp.find(value);
		SDL_assert(found != strToOp.end());
		return found->second;
	}

	Coefficient::Type CoefficientFromString(const std::string& value)
	{
		const auto& found = strToCoeff.find(value);
		SDL_assert(found != strToCoeff.end());
		return found->second;
	}
}