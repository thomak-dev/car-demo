#include "RandomColor.h"
#include <glm/gtc/random.hpp>
#include "Entity.h"
#include "MeshInstance.h"
#include "Material.h"

void RandomColor::Initialize()
{
	std::shared_ptr<Material> mat{ new Material{ *entity->GetComponent<MeshInstance>()->GetMaterial() } };
	mat->SetProperty("color", linearRand(glm::vec4(0), glm::vec4(1)));
	entity->GetComponent<MeshInstance>()->SetMaterial(mat);
}
