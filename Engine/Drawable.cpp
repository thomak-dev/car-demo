#include "Drawable.h"
#include "Renderer.h"

Drawable::Drawable(Entity& entity)
	: Component{entity}
{
	Renderer::Instance().RegisterDrawable(this);
}

Drawable::~Drawable()
{
	Renderer::Instance().UnregisterDrawable(this);
}
