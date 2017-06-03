#include "PhysicsDebugDrawer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "ResourceManager.h"
#include "Renderer.h"
#include "Camera.h"
#include "Entity.h"
#include "Transform.h"
#include "Material.h"
#include "Font.h"

PhysicsDebugDrawer::PhysicsDebugDrawer()
	:renderer{ Renderer::Instance() }
{
	glGenBuffers(1, &vbo);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glEnableVertexAttribArray(VertexAttributeLocation::Position);
	glVertexAttribPointer(VertexAttributeLocation::Position, 3, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
	glEnableVertexAttribArray(VertexAttributeLocation::Color);
	glVertexAttribPointer(VertexAttributeLocation::Color, 4, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, color)));
	lineMaterial = ResourceManager::Instance().LoadMaterial("Materials/line.mat");
	textMaterial = ResourceManager::Instance().LoadMaterial("Materials/debug_text.mat");
	font = ResourceManager::Instance().LoadFont("Fonts/consola.ttf");
	textMaterial = std::make_shared<Material>(*textMaterial);
	textMaterial->SetProperty("colorMap", font->GetTexture(TextPointSize));
}


PhysicsDebugDrawer::~PhysicsDebugDrawer()
{
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
}

void PhysicsDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
	buffer.emplace_back(from, color);
	buffer.emplace_back(to, color);
}

void PhysicsDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor)
{
	buffer.emplace_back(from, fromColor);
	buffer.emplace_back(to, toColor);
}

void PhysicsDebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
{
	drawLine(PointOnB + btVector3(0.125f, 0, 0), PointOnB + btVector3(-0.125f, 0, 0), color);
	drawLine(PointOnB + btVector3(0, 0.125f, 0), PointOnB + btVector3(0, -0.125f, 0), color);
	drawLine(PointOnB + btVector3(0, 0, 0.125f), PointOnB + btVector3(0, 0, -0.125f), color);
	drawLine(PointOnB, PointOnB + normalOnB * 0.25f, color);

	std::stringstream text;
	text << "Dist: " << std::setprecision(3) << std::fixed << distance << "\nTime: " << lifeTime;
	draw3dText(PointOnB, text.str().c_str());
}

void PhysicsDebugDrawer::reportErrorWarning(const char* warningString)
{
	std::cout << "PhysicsDebugDrawer: " << warningString << std::endl;
}

void PhysicsDebugDrawer::draw3dText(const btVector3& location, const char* textString)
{
	Transform* camTransform = renderer.CurrentCamera().GetEntity()->GetComponent<Transform>();
	renderer.SetMaterial(textMaterial);
	glm::vec3 dir{ camTransform->WorldPosition() - BtToGlmVec3(location) };
	glm::vec3 forward{ -camTransform->Forward() };
	float planeDistance = dot(forward, dir);
	forward = normalize(forward);
	glm::vec3 up{ camTransform->Up() };
	glm::mat4 model{glm::vec4(cross(up, forward), 0), glm::vec4(up, 0), glm::vec4(forward, 0), glm::vec4(BtToGlmVec3(location), 1) };
	float scale = planeDistance * TextScale / TextPointSize;
	model = glm::scale(model, glm::vec3(scale, scale, scale));
	renderer.SetModel(model);
	renderer.DrawText(textString, *font, TextPointSize, -1, 1, -1);
}

void PhysicsDebugDrawer::setDebugMode(int debugMode)
{
	this->debugMode = debugMode;
}

int PhysicsDebugDrawer::getDebugMode() const
{
	return debugMode;
}

void PhysicsDebugDrawer::flushLines()
{
	if(buffer.size() > 0)
	{
		renderer.SetMaterial(lineMaterial);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * buffer.size(), buffer.data(), GL_STREAM_DRAW);
		glBindVertexArray(vao);
		glDrawArrays(GL_LINES, 0, buffer.size());

		buffer.clear();
	}
}
