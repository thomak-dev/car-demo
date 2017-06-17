#pragma once
#include <memory>
#include "Component.h"
#include "core.h"
#include "Drawable.h"

class Transform;
class Font;

class Text : public Drawable
{
	DELETE_COPY_MOVE(Text)
public:
	using Drawable::Drawable;
	virtual ~Text() = default;

	void Initialize() override;
	int Deserialize(const Json& json) override;
	void Draw() override;
	std::shared_ptr<Material> GetMaterial() const override { return material; }

	std::string content;
	int anchor[2]{};
	int alignment{0};
private:
	unsigned pointSize{ 11 };
	
	std::shared_ptr<Material> material;
	Transform* transform{};
	std::shared_ptr<Font> font;
	GLuint vbo{};
	GLuint vao{};
	
};

