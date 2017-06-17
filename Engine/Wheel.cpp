#include "Wheel.h"

int Wheel::Deserialize(const Json& json)
{
	int count = 0;
	if (json.HasMember("index") && ++count)
		index = json["index"].GetInt();

	return count;
}
