#include "core.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <SDL.h>
#include <glm/glm.hpp>


std::ios formatBackup{ nullptr };

void Initialize()
{
	formatBackup.copyfmt(std::cout);
}

void ResetConsoleFormatting()
{
	std::cout.copyfmt(formatBackup);
}

std::string ReadTextFile(const std::string& path)
{
	auto file = SDL_RWFromFile(path.c_str(), "r");
	if (file)
	{
		std::stringstream strstream;	
		char ch;
		while (SDL_RWread(file, &ch, 1, 1) == 1)
			strstream << ch;

		SDL_RWclose(file);
		return strstream.str();
	}
	std::cout << "Could not read text file: " << SDL_GetError() << std::endl;
	return "";
}

std::string GetGLErrorString(GLenum error)
{
	switch (error)
	{
	case GL_NO_ERROR:
		return "GL_NO_ERROR";
	case GL_INVALID_ENUM:
		return
			"GL_INVALID_ENUM: An unacceptable value is specified for an enumerated argument. "
			"The offending command is ignored and has no other side effect than to set the error flag.";
	case GL_INVALID_VALUE:
		return
			"GL_INVALID_VALUE: A numeric argument is out of range. "
			"The offending command is ignored and has no other side effect than to set the error flag.";
	case GL_INVALID_OPERATION:
		return
			"GL_INVALID_OPERATION: The specified operation is not allowed in the current state. "
			"The offending command is ignored and has no other side effect than to set the error flag.";
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		return
			"GL_INVALID_FRAMEBUFFER_OPERATION: The framebuffer object is not complete. "
			"The offending command is ignored and has no other side effect than to set the error flag.";
	case GL_OUT_OF_MEMORY:
		return
			"GL_OUT_OF_MEMORY: There is not enough memory left to execute the command. "
			"The state of the GL is undefined, except for the state of the error flags, after this error is recorded.";
	default:
		return "Unknown error.";
	}
}

void CatchGlError(const char* file, int line)
{
	auto error = glGetError();
	if (error)
	{
		std::cout << file << '(' << line << "): " << GetGLErrorString(error) << std::endl;
		SDL_assert(false);
	}
}



std::ostream& operator<<(std::ostream& os, const glm::vec3& vec)
{
	auto w = os.width();
	os << vec.x << ' ' << std::setw(w) << vec.y << ' ' << std::setw(w) << vec.z;
	return os;
}

std::string GetExecutablePathInternal()
{
	char* path = SDL_GetBasePath();
	std::string result{ path };
	SDL_free(path);
	return result;
}

std::string GetResourcePathInternal()
{
	std::string result{ GetExecutablePath() };
	result += "Resources\\";
	return result;
}

const std::string& GetExecutablePath()
{
	static std::string executablePath{ GetExecutablePathInternal()};
	return executablePath;
}

const std::string& GetResourcePath()
{
	static std::string resourcePath{ GetResourcePathInternal() };
	return resourcePath;
}

std::string MakeNativePath(const std::string& shortPath)
{
	std::string nativePath{ GetResourcePath() };
	nativePath += shortPath;
	for (size_t i = 0; i < nativePath.size(); ++i)
		if (nativePath[i] == '/' || nativePath[i] == '\\')
			nativePath[i] = PATH_SEPARATOR_CHAR;
	return nativePath;
}

bool TryParseInt(const std::string& str, int& parsedInt)
{
	try
	{
		size_t parsedChars;
		parsedInt = std::stoi(str, &parsedChars, 10);
		return parsedChars == str.size();
	}
	catch (std::invalid_argument&)
	{
		return false;
	}
	catch (std::out_of_range&)
	{
		return false;
	}
}

