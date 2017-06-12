#include "Wheel.h"

void Wheel::Deserialize(const Json& json)
{
	if (json.HasMember("index"))
		index = json["index"].GetInt();
}
