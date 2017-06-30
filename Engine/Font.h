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
		int tileWidth{};
		int tileHeight{};
		int advance{};
		int height{};
		int columns{};
		int lineSkip{};
	};


	Font(SDL_RWops* fontFile);
	~Font();

	std::shared_ptr<Texture2D> GetTexture(int ptSize);
	Metrics GetMetrics(int ptSize);
	const std::pair<std::shared_ptr<Texture2D>, Metrics>& GetTextureAndMetrics(int ptSize);

private:
	std::unordered_map<int, std::pair<std::shared_ptr<Texture2D>, Metrics>> cache;
	SDL_RWops* fontFile{};
};

