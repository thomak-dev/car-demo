#include "Texture2D.h"
#include <iostream>
#include "core.h"

std::unordered_map<std::string, Texture2D::Filtering> Texture2D::stringToFiltering{
	{ "Nearest", Texture2D::Filtering::Nearest },
	{ "Bilinear", Texture2D::Filtering::Bilinear },
	{ "Trilinear", Texture2D::Filtering::Trilinear },
};
std::unordered_map<std::string, Texture2D::Wrapping> Texture2D::stringToWrapping{
	{ "Clamp", Texture2D::Wrapping::Clamp },
	{ "Repeat", Texture2D::Wrapping::Repeat },
};

Texture2D::Texture2D(SDL_Surface* surface)
	:surface{surface}
{
}

Texture2D::Texture2D(const Texture2D& other)
	:surface{ SDL_ConvertSurfaceFormat(other.surface, other.surface->format->format, 0) }
{
	auto palette = other.surface->format->palette;
	if (palette)
		SDL_SetSurfacePalette(surface, palette);
}

Texture2D::Texture2D(Texture2D&& other) noexcept
{
	swap(*this, other);
}

Texture2D::~Texture2D()
{
	SDL_FreeSurface(surface);
	glDeleteTextures(1, &texture);
}

Texture2D::Filtering Texture2D::FilteringFromString(const std::string& str)
{
	const auto& found = stringToFiltering.find(str);
	PRO_ASSERT(found != stringToFiltering.end());
	return found->second;
}

Texture2D::Wrapping Texture2D::WrappingFromString(const std::string& str)
{
	const auto& found = stringToWrapping.find(str);
	PRO_ASSERT(found != stringToWrapping.end());
	return found->second;
}

Texture2D& Texture2D::operator=(const Texture2D& other)
{
	Texture2D tmp{ other };
	swap(*this, tmp);
	return *this;
}

Texture2D& Texture2D::operator=(Texture2D&& other) noexcept
{
	swap(*this, other);
	return *this;
}

void Texture2D::SetFiltering(Filtering filtering)
{
	this->filtering = filtering;
	MarkDirty();
}

void Texture2D::SetWrapping(Wrapping wrapping)
{
	this->wrapping = wrapping;
	MarkDirty();
}

void Texture2D::SetUseMipMaps(bool value)
{
	useMipMaps = value;
	MarkDirty();
}


void Texture2D::Bind()
{
	CreateTextureIfNeeded();
	glBindTexture(GL_TEXTURE_2D, texture);
	
	if (dirty)
		Upload();
}

void Texture2D::MarkDirty()
{
	dirty = true;
}

void Texture2D::CreateTextureIfNeeded()
{
	if (!texture)
		glGenTextures(1, &texture);
}

GLint FilteringForMagnification(Texture2D::Filtering filtering)
{
	switch (filtering)
	{
	case Texture2D::Filtering::Nearest:
		return GL_NEAREST;
	case Texture2D::Filtering::Bilinear:
		return GL_LINEAR;
	case Texture2D::Filtering::Trilinear:
		return GL_LINEAR;
	default:
		PRO_ASSERT(false);
	}
	return GL_NEAREST;
}

GLint Texture2D::FilteringForMinification(Texture2D::Filtering filtering) const
{
	switch (filtering)
	{
	case Texture2D::Filtering::Nearest:
		return useMipMaps? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
	case Texture2D::Filtering::Bilinear:
		return useMipMaps? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR;
	case Texture2D::Filtering::Trilinear:
		return useMipMaps? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
	default:
		PRO_ASSERT(false);
	}
	return GL_NEAREST;
}

void Texture2D::Upload()
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLint>(wrapping));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLint>(wrapping));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, FilteringForMinification(filtering));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, FilteringForMagnification(filtering));

	if (surface->format->format != SDL_PIXELFORMAT_RGBA8888)
	{
		auto tmp = surface;
		surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA8888, 0);
		SDL_FreeSurface(tmp);
		PRO_ASSERT(surface);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, nullptr);
	for (int i = 0; i < surface->h; ++i)
	{
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, surface->h - 1 - i, surface->w, 1, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, &static_cast<uint32_t*>(surface->pixels)[i * surface->w]);
	}
	if (useMipMaps)
		glGenerateMipmap(GL_TEXTURE_2D);
	else if (filtering == Filtering::Trilinear)
		std::cout << "Trilinear filtering is set but mipmapping is off. Using bilinear filtering." << std::endl;

	dirty = false;
}

void swap(Texture2D& first, Texture2D& second) noexcept
{
	using std::swap;
	swap(first.surface, second.surface);
	swap(first.dirty, second.dirty);
	swap(first.texture, second.texture);
	swap(first.filtering, second.filtering);
	swap(first.wrapping, second.wrapping);
	swap(first.useMipMaps, second.useMipMaps);
}
