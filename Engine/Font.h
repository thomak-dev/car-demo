#pragma once
#include <memory>
#include <unordered_map>
#include <SDL.h>

class Texture2D;

class Font
{
public:
	struct Metrics
	{
		unsigned tileWidth{};
		unsigned tileHeight{};
		unsigned advance{};
		unsigned height{};
		unsigned columns{};
		unsigned lineSkip{};
	};


	Font(SDL_RWops* fontFile);
	~Font();

	std::shared_ptr<Texture2D> GetTexture(unsigned ptSize);
	Metrics GetMetrics(unsigned ptSize);
	const std::pair<std::shared_ptr<Texture2D>, Metrics>& GetTextureAndMetrics(unsigned ptSize);

private:
	std::unordered_map<unsigned, std::pair<std::shared_ptr<Texture2D>, Metrics>> cache;
	SDL_RWops* fontFile{};
};

