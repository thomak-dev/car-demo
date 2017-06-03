#pragma once
#include <vector>
#include <memory>
#include <btBulletDynamicsCommon.h>
#include "core.h"
#include "Singleton.h"
#include "PhysicsDebugDrawer.h"

class Font;
class Camera;
class Drawable;
class Light;

namespace VertexAttributeLocation
{
	enum : GLuint
	{
		Position,
		Normal,
		Tangent,
		UV,
		Color
	};
}

namespace UniformBufferIndex
{
	enum : GLuint
	{
		ViewProjection = 0, Lights, Count
	};
}

class Material;

namespace RenderDebugId
{
	enum : GLuint
	{
		Frame,
		Camera,
		MeshInstance,
		Text
	};
}

namespace Blend
{
	namespace Operation
	{
		enum Type : GLenum
		{
			Add = GL_FUNC_ADD,
			Subtract = GL_FUNC_SUBTRACT,
			ReverseSubtract = GL_FUNC_REVERSE_SUBTRACT
		};
	}
	namespace Coefficient
	{
		enum Type : GLenum
		{
			Zero = GL_ZERO,
			One = GL_ONE,
			SrcColor = GL_SRC_COLOR,
			OneMinusSrcColor = GL_ONE_MINUS_SRC_COLOR,
			DstColor = GL_DST_COLOR,
			OneMinusDstColor = GL_ONE_MINUS_DST_COLOR,
			SrcAlpha = GL_SRC_ALPHA,
			OneMinusSrcAlpha = GL_ONE_MINUS_SRC_ALPHA,
			DstAlpha = GL_DST_ALPHA,
			OneMinusDstAlpha = GL_ONE_MINUS_DST_ALPHA,
			ConstantColor = GL_CONSTANT_COLOR,
			OneMinusConstantColor = GL_ONE_MINUS_CONSTANT_COLOR,
			ConstantAlpha = GL_CONSTANT_ALPHA,
			OneMinusConstantAlpha = GL_ONE_MINUS_CONSTANT_ALPHA,
			SrcAlphaSaturate = GL_SRC_ALPHA_SATURATE,
			Src1Color = GL_SRC1_COLOR,
			Src1Alpha = GL_SRC1_ALPHA
		};
	}

	Operation::Type OperationFromString(const std::string& value);
	Coefficient::Type CoefficientFromString(const std::string& value);
}

class Renderer : public virtual Singleton<Renderer>
{
	DELETE_COPY_MOVE(Renderer)
public:
	static constexpr int LightCount{4};
	Renderer();
	virtual ~Renderer();

	const std::vector<Camera*>& Cameras() const { return cameras; }
	void RegisterCamera(Camera* camera);
	void RegisterDrawable(Drawable* renderable);
	void RegisterLight(Light* light);
	void UnregisterCamera(Camera* camera);
	void UnregisterDrawable(Drawable* renderable);
	void UnregisterLight(Light* light);
	void Render();
	void SetMaterial(const std::shared_ptr<Material>& material);
	void SetModel(const glm::mat4& modelMatrix);
	void DrawText(const std::string& text, Font& font, int pointSize, int horizontalAnchor = 0, int verticalAnchor = 0, int alignment = 0);
	const Camera& CurrentCamera() const { return *currentCamera; }
	btIDebugDraw* GetPhysicsDebugDrawer() { return &physicsDebugDrawer; }

private:
	struct TextVertex
	{
		TextVertex(float x, float y, float z, float u, float v) : x{x}, y{y}, z{z}, u{u}, v{v} {}
		float x, y, z, u, v;
	};
	struct TextQuad
	{
		TextQuad(float x, float y, float w, float h, float u, float v) : x{ x }, y{ y }, w{ w }, h{ h }, u{ u }, v{ v } {}
		float x, y, w, h, u, v;
	};
	struct Line
	{
		std::vector<TextQuad> quads;
		float width{};
	};
	std::vector<Line> lines;
	std::vector<TextVertex> textVertices;
	GLuint textVao{};
	GLuint textVbo{};

	std::vector<Drawable*> drawables;
	std::vector<Camera*> cameras;
	std::vector<Light*> lights;
	GLuint ubos[UniformBufferIndex::Count]{};
	bool cull{true};
	bool testDepth{true};
	bool writeDepth{true};
	Blend::Operation::Type blendOpColor{ Blend::Operation::Add };
	Blend::Operation::Type blendOpAlpha{ Blend::Operation::Add };
	Blend::Coefficient::Type blendSrcColor{ Blend::Coefficient::One };
	Blend::Coefficient::Type blendSrcAlpha{ Blend::Coefficient::One };
	Blend::Coefficient::Type blendDstColor{ Blend::Coefficient::Zero };
	Blend::Coefficient::Type blendDstAlpha{ Blend::Coefficient::Zero };
	std::shared_ptr<Material> currentMaterial;
	PhysicsDebugDrawer physicsDebugDrawer;
	Camera* currentCamera{};
	

	void SetCull(bool);
	void SetWriteDepth(bool);
	void SetTestDepth(bool);
	void SetBlendOperators(Blend::Operation::Type blendOpColor, Blend::Operation::Type blendOpAlpha);
	void SetBlendCoefficients(Blend::Coefficient::Type srcColor, Blend::Coefficient::Type srcAlpha, Blend::Coefficient::Type dstColor, Blend::Coefficient::Type dstAlpha);
};

