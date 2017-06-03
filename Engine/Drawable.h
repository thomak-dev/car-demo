#pragma once
#include <memory>
#include "core.h"
#include "Component.h"

class Material;

class Drawable : public Component
{
	DELETE_COPY_MOVE(Drawable)
public:
	Drawable() = delete;
	explicit Drawable(Entity* entity);
	virtual ~Drawable();
	virtual void Draw() = 0;
	virtual std::shared_ptr<Material> GetMaterial() const = 0;
};

