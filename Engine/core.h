#pragma once
#include <string>
#include <cmath>
#include <functional>
#include <glm/fwd.hpp>
#include <SDL.h>
#include "static_glew.h"

#ifdef WIN32
#define PATH_SEPARATOR_CHAR '\\'
#else
#define PATH_SEPARATOR_CHAR '/'
#endif

#define CATCH_GL() CatchGlError(__FILE__, __LINE__)

#define DELETE_COPY(ClassName) \
ClassName(const ClassName&) = delete;\
ClassName& operator=(const ClassName&) = delete;

#define DELETE_MOVE(ClassName) \
ClassName(ClassName&&) = delete;\
ClassName& operator=(ClassName&&) = delete;

#define DELETE_COPY_MOVE(ClassName) DELETE_COPY(ClassName) DELETE_MOVE(ClassName)

extern std::ios formatBackup;

void Initialize();
void ResetConsoleFormatting();
std::string ReadTextFile(const std::string& path);
std::string GetGLErrorString(GLenum error);
void CatchGlError(const char* file, int line);

std::ostream& operator<<(std::ostream& os, const glm::vec3& vec);
const std::string& GetExecutablePath();
const std::string& GetResourcePath();
std::string MakeNativePath(const std::string& shortPath);

template <typename T>
using Predicate = std::function<bool(const T& arg)>;

// Transform a predicate instance method (() -> bool) to a free function predicate
template<typename T>
Predicate<T> MemberToPredicate(bool (T::*member)() const)
{
	return [=](const T& arg) { return (arg.*member)(); };
}

bool TryParseInt(const std::string& str, int& parsedInt);