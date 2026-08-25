cmake_minimum_required(VERSION 3.21)   # for $<TARGET_RUNTIME_DLLS:...>

project(Prometheus
    DESCRIPTION "A small 3D engine"
    LANGUAGES C CXX
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(NOT MSVC)
    message(FATAL_ERROR "This project currently builds with MSVC on Windows only.")
endif()

# --- Global settings -----------------------------------------------------------
# Static CRT throughout. This is dictated by PhysX: its static libraries
# (PhysX3Extensions, PhysX3Vehicle) are built /MT and cannot be reconfigured
# without rebuilding PhysX itself. A CRT mismatch across modules gives LNK2038
# at link time, or two heaps at runtime, so everything conforms to PhysX.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(CMAKE_CXX_STANDARD 14)             # matches the toolset default the code was written against
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

# CMake 4 removed compatibility with cmake_minimum_required below 3.5, which some
# vendored dependencies still declare (freetype under SDL_ttf, for one). This is
# the supported escape hatch and applies to subdirectories too.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# Keep the historical output layout: <root>/Debug and <root>/Release.
foreach(_cfg Debug Release)
    string(TOUPPER ${_cfg} _CFG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${_CFG} "${CMAKE_CURRENT_SOURCE_DIR}/${_cfg}")
endforeach()

# --- Dependencies --------------------------------------------------------------
# SDL must be added before SDL_image/SDL_ttf: their sdl_find_sdl2() helper starts
# with if(NOT TARGET SDL2::SDL2), so an in-tree target short-circuits the
# find_package() that would otherwise need an installed SDL2.
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
add_subdirectory(External/SDL EXCLUDE_FROM_ALL)

# 2.8.x / 2.24.x build their decoders, freetype and harfbuzz from the nested
# external/ submodules, so nothing needs to be supplied prebuilt.
set(SDL2IMAGE_VENDORED ON  CACHE BOOL "" FORCE)
set(SDL2IMAGE_DEPS_SHARED OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_SAMPLES OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_TESTS   OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_INSTALL OFF CACHE BOOL "" FORCE)
# AVIF is on by default and drags in libavif + dav1d, two large submodules. The
# assets only use PNG and JPG, both served by the built-in stb_image backend.
set(SDL2IMAGE_AVIF OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_JXL  OFF CACHE BOOL "" FORCE)
add_subdirectory(External/SDL_image EXCLUDE_FROM_ALL)

set(SDL2TTF_VENDORED ON  CACHE BOOL "" FORCE)
set(SDL2TTF_SAMPLES  OFF CACHE BOOL "" FORCE)
set(SDL2TTF_INSTALL  OFF CACHE BOOL "" FORCE)
set(SDL2TTF_HARFBUZZ OFF CACHE BOOL "" FORCE)
add_subdirectory(External/SDL_ttf EXCLUDE_FROM_ALL)

set(ASSIMP_BUILD_TESTS        OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL            OFF CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
add_subdirectory(External/assimp EXCLUDE_FROM_ALL)

# glew is a single translation unit, so it is compiled directly rather than
# driven through its own build. That also avoids its generated build/glew.rc
# (excluded by glew's .gitignore, so absent from a clean checkout) and its
# cmake_minimum_required(2.8.7), which CMake 4 rejects outright.
add_library(glew_s STATIC External/glew/src/glew.c)
target_include_directories(glew_s PUBLIC External/glew/include)
target_compile_definitions(glew_s PUBLIC GLEW_STATIC)
target_link_libraries(glew_s PUBLIC opengl32)

# Header-only.
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE External/glm)

add_library(rapidjson INTERFACE)
target_include_directories(rapidjson INTERFACE External/rapidjson/include)

include(PhysX)
find_package(FMOD REQUIRED)

# --- Engine --------------------------------------------------------------------
add_executable(Engine
    Engine/Audio.cpp            Engine/AudioListener.cpp    Engine/AudioSource.cpp
    Engine/CamControls.cpp      Engine/Camera.cpp           Engine/Component.cpp
    Engine/Drawable.cpp         Engine/Entity.cpp           Engine/Font.cpp
    Engine/ForwardSpinner.cpp   Engine/FpsCounter.cpp       Engine/GameWindow.cpp
    Engine/Light.cpp            Engine/Material.cpp         Engine/Mesh.cpp
    Engine/MeshInstance.cpp     Engine/OrbitingCam.cpp      Engine/Physics.cpp
    Engine/RandomColor.cpp      Engine/Renderer.cpp         Engine/ResourceManager.cpp
    Engine/RigidBody.cpp        Engine/Shader.cpp           Engine/ShaderElement.cpp
    Engine/ShaderProgram.cpp    Engine/Text.cpp             Engine/Texture2D.cpp
    Engine/Time.cpp             Engine/Transform.cpp        Engine/Updatable.cpp
    Engine/Vehicle.cpp          Engine/VehicleController.cpp Engine/Wheel.cpp
    Engine/core.cpp             Engine/input.cpp            Engine/json.cpp
    Engine/main.cpp             Engine/math_utility.cpp
)

# Deliberately no target_include_directories for Engine/ itself. The engine's own
# headers are included with quotes, which MSVC resolves relative to the including
# file, so the directory does not need to be on the include path -- and must not
# be: Engine/Time.h would shadow the CRT's <time.h> on a case-insensitive
# filesystem, breaking <chrono> and <ctime> deep inside the STL.

target_link_libraries(Engine PRIVATE
    SDL2::SDL2 SDL2::SDL2main
    SDL2_image::SDL2_image
    SDL2_ttf::SDL2_ttf
    assimp::assimp
    glew_s glm rapidjson
    PhysX::All
    FMOD::Studio
)

target_compile_definitions(Engine PRIVATE
    _CONSOLE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

# SDL2main provides WinMain, but this is a console application.
set_target_properties(Engine PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Engine>"
    WIN32_EXECUTABLE FALSE
)

# --- Runtime deployment --------------------------------------------------------
# Everything the exe loads has to sit beside it: shared libraries, the asset
# tree, and the FMOD banks.
add_custom_command(TARGET Engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:Engine> "$<TARGET_FILE_DIR:Engine>"
    COMMAND_EXPAND_LISTS
    COMMENT "Copying runtime DLLs"
)

add_custom_command(TARGET Engine POST_BUILD
    COMMAND ${CMAKE_COMMAND}
            -DSRC=${CMAKE_CURRENT_SOURCE_DIR}/Assets
            -DDST=$<TARGET_FILE_DIR:Engine>
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CopyAssets.cmake"
    COMMENT "Copying assets"
)

# FMOD Studio 2.x writes banks to Audio/Build/Desktop, but Audio.cpp loads them
# from <exe>/Audio, so they are built (if possible) and then copied across.
set(_bank_dir  "${CMAKE_CURRENT_SOURCE_DIR}/Audio/Build/Desktop")
set(_fspro     "${CMAKE_CURRENT_SOURCE_DIR}/Audio/Audio.fspro")
# Audio.cpp loads this one first and discovers the rest through it, which makes
# it a good stand-in for "the banks are built".
set(_bank_stamp "${_bank_dir}/Master Bank.strings.bank")

option(ENGINE_BUILD_FMOD_BANKS "Rebuild FMOD banks with the FMOD Studio CLI" ON)

if(ENGINE_BUILD_FMOD_BANKS)
    find_package(FMODStudio)
endif()

if(ENGINE_BUILD_FMOD_BANKS AND FMODStudio_FOUND)
    # The banks depend on the whole authoring project. Its Metadata tree is the
    # practical proxy: FMOD Studio rewrites it whenever anything changes, so
    # globbing it keeps the banks up to date without listing every .ogg.
    file(GLOB_RECURSE _bank_sources CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/Audio/Metadata/*.xml")

    add_custom_command(
        OUTPUT  "${_bank_stamp}"
        COMMAND "${FMODSTUDIO_CL}" -build -platforms Desktop "${_fspro}"
        DEPENDS "${_fspro}" ${_bank_sources}
        COMMENT "Building FMOD banks (fmodstudiocl)"
        VERBATIM
        USES_TERMINAL
    )
    add_custom_target(Banks DEPENDS "${_bank_stamp}")
    add_dependencies(Engine Banks)
elseif(NOT EXISTS "${_bank_stamp}")
    message(WARNING
        "No FMOD banks in Audio/Build/Desktop and none will be built -- the engine "
        "will link but abort on startup at the first loadBankFile. Build them from "
        "Audio/Audio.fspro in FMOD Studio.")
endif()

# Banks go under Resources/, not next to the exe: Audio.cpp asks for
# "Audio/Master Bank.strings.bank", but every resource path goes through
# MakeNativePath, which prefixes GetResourcePath() -- i.e. <exe dir>/Resources/.
# The Assets tree lands the same way, which is why Resources/ is the asset root.
#
# Copied unconditionally: with ENGINE_BUILD_FMOD_BANKS the Banks target has
# already run by this point, and otherwise this picks up banks built by hand.
add_custom_command(TARGET Engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:Engine>/Resources/Audio"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${_bank_dir}" "$<TARGET_FILE_DIR:Engine>/Resources/Audio"
    COMMENT "Copying FMOD banks"
)
