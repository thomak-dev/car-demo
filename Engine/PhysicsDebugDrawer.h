#pragma once
#include <vector>
#include <btBulletDynamicsCommon.h>
#include <GL/glew.h>
#include <glm/common.hpp>
#include "math_utility.h"
#include <memory>

class Renderer;
class Material;
class Font;

class PhysicsDebugDrawer : public btIDebugDraw
{
public:
	PhysicsDebugDrawer();
	virtual ~PhysicsDebugDrawer();


	void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;
	void drawLine(const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor) override;
	void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override;
	void reportErrorWarning(const char* warningString) override;
	void draw3dText(const btVector3& location, const char* textString) override;
	void setDebugMode(int debugMode) override;
	int getDebugMode() const override;
	void flushLines() override;

private:
	struct Vertex
	{
		Vertex(const glm::vec3& position, const glm::vec4& color)
			:position{position}, color{color}
		{			
		}
		Vertex(const btVector3& position, const btVector3& color)
			:Vertex{ BtToGlmVec3(position), glm::vec4(BtToGlmVec3(color), 1) }
		{
		}
		glm::vec3 position;
		glm::vec4 color;
	};
	
	static constexpr int TextPointSize{ 16 };
	static constexpr float TextScale{ 0.026f };

	std::vector<Vertex> buffer;
	int debugMode{};
	GLuint vbo;
	GLuint vao;
	std::shared_ptr<Material> lineMaterial;
	std::shared_ptr<Material> textMaterial;
	std::shared_ptr<Font> font;
	Renderer& renderer;
};

