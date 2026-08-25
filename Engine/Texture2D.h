#pragma once
#include <string>
#include <type_traits>
#include <unordered_map>
#include <SDL.h>
#include "static_glew.h"

class Texture2D
{

public:
	enum class Filtering
	{
		Nearest,
		Bilinear,
		Trilinear
	};
	enum class Wrapping : GLint
	{
		Clamp = GL_CLAMP_TO_EDGE,
		Repeat = GL_REPEAT
	};

	enum class PixelFormat : uint32_t
	{
		Rgba8888 = SDL_PIXELFORMAT_RGBA8888,
		Default = Rgba8888
	};

	Texture2D() = delete;
	Texture2D(SDL_Surface* surface);
	Texture2D(const Texture2D&);
	Texture2D(Texture2D&&) noexcept;
	virtual ~Texture2D();

	static Filtering FilteringFromString(const std::string& str);
	static Wrapping WrappingFromString(const std::string& str);
	static int GetBitsPerPixel(PixelFormat format) { return SDL_BITSPERPIXEL(static_cast<std::underlying_type<PixelFormat>::type>(format)); }

	Texture2D& operator=(const Texture2D&);
	Texture2D& operator=(Texture2D&&) noexcept;

	Filtering GetFiltering() const { return filtering; }
	void SetFiltering(Filtering filtering);
	Wrapping GetWrapping() const { return wrapping; }
	void SetWrapping(Wrapping wrapping);
	void SetUseMipMaps(bool value);
	bool UsesMipMaps() const { return useMipMaps; }
	
	int Width() const { return surface->w; }
	int Height() const { return surface->h; }

	void Bind();
	void MarkDirty();

	friend void swap(Texture2D& first, Texture2D& second) noexcept;
private:
	SDL_Surface* surface{};
	bool dirty{ true };
	GLuint texture{};
	Filtering filtering{Filtering::Trilinear};
	Wrapping wrapping{Wrapping::Clamp};
	static std::unordered_map<std::string, Filtering> stringToFiltering;
	static std::unordered_map<std::string, Wrapping> stringToWrapping;
	bool useMipMaps{ true };

	void CreateTextureIfNeeded();
	void Upload();
	GLint Texture2D::FilteringForMinification(Texture2D::Filtering filtering) const;
};

