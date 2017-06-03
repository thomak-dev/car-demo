#include "Font.h"
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <glm/glm.hpp>
#include "Texture2D.h"
#include "math_utility.h"

// " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", // length: 95 + 1(null)

Font::Font(SDL_RWops* fontFile)
	:fontFile{fontFile}
{

}


Font::~Font()
{
	SDL_RWclose(fontFile);
}

std::shared_ptr<Texture2D> Font::GetTexture(unsigned ptSize)
{
	return GetTextureAndMetrics(ptSize).first;
}

Font::Metrics Font::GetMetrics(unsigned ptSize)
{
	return GetTextureAndMetrics(ptSize).second;
}

const std::pair<std::shared_ptr<Texture2D>, Font::Metrics>& Font::GetTextureAndMetrics(unsigned ptSize)
{
	const auto& found = cache.find(ptSize);
	if (found == cache.end())
	{
		SDL_RWseek(fontFile, 0, RW_SEEK_SET);
		TTF_Font* ttf = TTF_OpenFontRW(fontFile, 0, ptSize);
		int height = TTF_FontHeight(ttf);
		int maxAdvance = 0;
		for (char ch = ' '; ch <= '~'; ++ch)
		{
			int minX;
			int minY;
			int maxX;
			int maxY;
			int advance;
			TTF_GlyphMetrics(ttf, ch, &minX, &maxX, &minY, &maxY, &advance);
			if (advance > maxAdvance)
				maxAdvance = advance;
		}

		struct { int x, y; } padding{ 2, 2 };
		unsigned tileWidth = maxAdvance + padding.x;
		unsigned tileHeight = height + padding.y;
		unsigned tileCount = ('~' - ' ' + 1);
		float targetWidth = glm::sqrt(tileWidth * tileHeight * tileCount);
		unsigned columns = glm::ceil(targetWidth / tileWidth);
		unsigned actualWidth = columns * tileWidth;
		unsigned actualHeight = glm::ceil(static_cast<float>(tileCount) / columns) * tileHeight;

		SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, actualWidth, actualHeight, 32, SDL_PIXELFORMAT_ARGB32);
		SDL_FillRect(surface, nullptr, SDL_MapRGBA(surface->format, 255, 255, 255, 0));

		for (char ch = ' '; ch <= '~'; ++ch)
		{
			SDL_Surface* glyph = TTF_RenderGlyph_Blended(ttf, ch, SDL_Color{255, 255, 255, 255});
			int i = ch - ' ';
			SDL_Rect targetRect{ (i % columns) * tileWidth, (i / columns) * tileHeight, maxAdvance, height };
			SDL_BlitSurface(glyph, nullptr, surface, &targetRect);
			SDL_FreeSurface(glyph);
		}

		//SDL_Surface* converted = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
		//IMG_SavePNG(converted, R"(C:\Users\thomak\Desktop\fonttestimage32415.png)");
		//SDL_FreeSurface(converted);

		std::shared_ptr<Texture2D> newTex{ new Texture2D(surface) };
		// mipmapping doesnt work well with atlasses
		newTex->SetUseMipMaps(false);
		newTex->SetFiltering(Texture2D::Filtering::Bilinear);
		Metrics metrics{ tileWidth, tileHeight, maxAdvance, height, columns, TTF_FontLineSkip(ttf) };
		const auto& result = cache.insert(found, std::make_pair(ptSize, std::make_pair(newTex, metrics)))->second;
		TTF_CloseFont(ttf);
		return result;
	}
	else
		return found->second;
}
