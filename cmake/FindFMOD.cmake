# Locates the FMOD Engine (Studio API) and defines imported targets.
#
# The SDK is a separate download from the FMOD Studio authoring tool and is not
# redistributable, so it is not vendored. Its version must match the authoring
# tool that built Audio/Audio.fspro (currently 2.03.x) because banks are refused
# by an older runtime.
#
# Defines:
#   FMOD::Core     fmod.dll   / fmodL.dll    (logging build in Debug)
#   FMOD::Studio   fmodstudio.dll / fmodstudioL.dll
#   FMOD_FOUND
#
# Override the search location with -DFMOD_ROOT=<path> if installed elsewhere.

if(NOT FMOD_ROOT)
    # Note: the installer places the SDK under the 32-bit Program Files even for
    # the x64 libraries, and it kept the legacy "FMOD Studio API Windows" folder
    # name after FMOD 2.x renamed the product to "FMOD Engine".
    set(FMOD_ROOT "$ENV{ProgramFiles\(x86\)}/FMOD SoundSystem/FMOD Studio API Windows"
        CACHE PATH "Root of the FMOD Engine (Studio API) installation")
endif()

find_path(FMOD_CORE_INCLUDE_DIR
    NAMES fmod.hpp
    PATHS "${FMOD_ROOT}/api/core/inc"
    NO_DEFAULT_PATH
)
find_path(FMOD_STUDIO_INCLUDE_DIR
    NAMES fmod_studio.hpp
    PATHS "${FMOD_ROOT}/api/studio/inc"
    NO_DEFAULT_PATH
)

set(_fmod_core_lib   "${FMOD_ROOT}/api/core/lib/x64")
set(_fmod_studio_lib "${FMOD_ROOT}/api/studio/lib/x64")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FMOD
    REQUIRED_VARS FMOD_CORE_INCLUDE_DIR FMOD_STUDIO_INCLUDE_DIR
    FAIL_MESSAGE
        "FMOD Engine not found under '${FMOD_ROOT}'. Download \"FMOD Engine\" for \
Windows (2.03.x, matching the FMOD Studio version that builds Audio/Audio.fspro) \
from https://www.fmod.com/download and install it, or pass -DFMOD_ROOT=<path>."
)

if(FMOD_FOUND AND NOT TARGET FMOD::Core)
    # 'L' suffix denotes the logging build, used for Debug.
    add_library(FMOD::Core SHARED IMPORTED)
    set_target_properties(FMOD::Core PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${FMOD_CORE_INCLUDE_DIR}"
        IMPORTED_CONFIGURATIONS "Debug;Release"
        IMPORTED_IMPLIB_DEBUG     "${_fmod_core_lib}/fmodL_vc.lib"
        IMPORTED_LOCATION_DEBUG   "${_fmod_core_lib}/fmodL.dll"
        IMPORTED_IMPLIB_RELEASE   "${_fmod_core_lib}/fmod_vc.lib"
        IMPORTED_LOCATION_RELEASE "${_fmod_core_lib}/fmod.dll"
    )

    add_library(FMOD::Studio SHARED IMPORTED)
    set_target_properties(FMOD::Studio PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${FMOD_STUDIO_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES FMOD::Core
        IMPORTED_CONFIGURATIONS "Debug;Release"
        IMPORTED_IMPLIB_DEBUG     "${_fmod_studio_lib}/fmodstudioL_vc.lib"
        IMPORTED_LOCATION_DEBUG   "${_fmod_studio_lib}/fmodstudioL.dll"
        IMPORTED_IMPLIB_RELEASE   "${_fmod_studio_lib}/fmodstudio_vc.lib"
        IMPORTED_LOCATION_RELEASE "${_fmod_studio_lib}/fmodstudio.dll"
    )
endif()

mark_as_advanced(FMOD_CORE_INCLUDE_DIR FMOD_STUDIO_INCLUDE_DIR)
