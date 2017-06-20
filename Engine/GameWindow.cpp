#include "GameWindow.h"
#include <iostream>
#ifdef _WIN32
#include <Windows.h>
#elif
#define APIENTRY
#endif
#include "static_glew.h"

void APIENTRY OnGLError(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	std::string msgString{ message };
	
	bool filtered = type == GL_DEBUG_TYPE_PUSH_GROUP || type == GL_DEBUG_TYPE_POP_GROUP || (source == GL_DEBUG_SOURCE_API && id == 131185);
	if(!filtered)
		std::cout << msgString << std::endl;
	SDL_assert(severity == GL_DEBUG_SEVERITY_NOTIFICATION);
}

GameWindow::GameWindow(const std::string& title, int width, int height)
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#	ifdef _DEBUG
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#	endif
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	SDL_SetRelativeMouseMode(SDL_TRUE);

	window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	context = SDL_GL_CreateContext(window);
	SDL_GL_SetSwapInterval(0);
	glewExperimental = GL_TRUE;
	glewInit();

	GLint extensionCount;
	glGetIntegerv(GL_NUM_EXTENSIONS, &extensionCount);
	for (int i = 0; i < extensionCount; ++i)
	{
		auto str = glGetStringi(GL_EXTENSIONS, i);
		std::string ext{ reinterpret_cast<const char*>(str) };
		extensions.push_back(ext);
	}

	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(OnGLError, nullptr);
}

GameWindow::~GameWindow()
{
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
}


void GameWindow::Swap()
{
	SDL_GL_SwapWindow(window);
}