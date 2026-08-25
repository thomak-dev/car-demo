# PhysX 3.4 has no CMake build of its own -- NVIDIA only added one in PhysX 4 --
# so it is driven through its shipped Visual Studio solution and then exposed as
# ordinary imported targets. Writing CMakeLists for its ~16 projects by hand
# would mean enumerating every source file for no practical gain.
#
# Defines: PhysX::PhysX, PhysX::Common, PhysX::Cooking, PhysX::Extensions,
#          PhysX::Vehicle, PhysX::Foundation, PhysX::PvdSDK, PhysX::nvToolsExt

include(ExternalProject)

set(PHYSX_DIR       "${CMAKE_CURRENT_SOURCE_DIR}/External/PhysX-3.4")
set(PHYSX_SLN_DIR   "${PHYSX_DIR}/PhysX_3.4/Source/compiler/vc14win64")
set(PHYSX_LIB_DIR   "${PHYSX_DIR}/PhysX_3.4/Lib/vc14win64")
set(PHYSX_BIN_DIR   "${PHYSX_DIR}/PhysX_3.4/Bin/vc14win64")
set(PXSHARED_LIB_DIR "${PHYSX_DIR}/PxShared/lib/vc14win64")
set(PXSHARED_BIN_DIR "${PHYSX_DIR}/PxShared/bin/vc14win64")

if(NOT EXISTS "${PHYSX_SLN_DIR}/PhysX.sln")
    message(FATAL_ERROR
        "PhysX sources not found. Run: git submodule update --init --recursive")
endif()

# Use the generator's own MSBuild rather than searching PATH: msbuild is only on
# PATH inside a Visual Studio developer shell, and requiring one just for this
# would couple the whole build to it. The VS generator always provides this.
if(NOT CMAKE_VS_MSBUILD_COMMAND)
    message(FATAL_ERROR
        "CMAKE_VS_MSBUILD_COMMAND is not set. PhysX 3.4 has no CMake build and is "
        "driven through its Visual Studio solution, so a Visual Studio generator "
        "is required (configure with -A x64).")
endif()
set(MSBUILD_EXECUTABLE "${CMAKE_VS_MSBUILD_COMMAND}")

# PhysX's configuration names are lower-case, unlike the CMake defaults.
set(_physx_config "$<LOWER_CASE:$<CONFIG>>")

# /WX- because this 2017-era code trips warnings current MSVC did not emit then,
# and the projects set /WX. TreatWarningAsError is item metadata in those
# vcxproj files, so -p: cannot override it; _CL_ is appended after the project's
# own flags and therefore wins.
ExternalProject_Add(PhysXBuild
    SOURCE_DIR        "${PHYSX_SLN_DIR}"
    CONFIGURE_COMMAND ""
    BUILD_IN_SOURCE   TRUE
    BUILD_COMMAND     ${CMAKE_COMMAND} -E env _CL_=/WX-
                      "${MSBUILD_EXECUTABLE}" "${PHYSX_SLN_DIR}/PhysX.sln"
                      -nologo -maxCpuCount -nodeReuse:false -verbosity:quiet
                      "-property:Configuration=${_physx_config}"
                      -property:Platform=x64
                      -property:PlatformToolset=${CMAKE_VS_PLATFORM_TOOLSET}
                      -property:WindowsTargetPlatformVersion=${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}
    INSTALL_COMMAND   ""
    BUILD_ALWAYS      FALSE
    USES_TERMINAL_BUILD TRUE
)

# PhysX decorates debug binaries with a DEBUG infix; the architecture suffix is
# present only on the shared libraries, not the static ones.
function(_physx_add_shared name basename)
    add_library(PhysX::${name} SHARED IMPORTED GLOBAL)
    set_target_properties(PhysX::${name} PROPERTIES
        IMPORTED_CONFIGURATIONS "Debug;Release"
        IMPORTED_IMPLIB_DEBUG     "${ARGV2}/${basename}DEBUG_x64.lib"
        IMPORTED_LOCATION_DEBUG   "${ARGV3}/${basename}DEBUG_x64.dll"
        IMPORTED_IMPLIB_RELEASE   "${ARGV2}/${basename}_x64.lib"
        IMPORTED_LOCATION_RELEASE "${ARGV3}/${basename}_x64.dll"
    )
    add_dependencies(PhysX::${name} PhysXBuild)
endfunction()

function(_physx_add_static name basename)
    add_library(PhysX::${name} STATIC IMPORTED GLOBAL)
    set_target_properties(PhysX::${name} PROPERTIES
        IMPORTED_CONFIGURATIONS "Debug;Release"
        IMPORTED_LOCATION_DEBUG   "${ARGV2}/${basename}DEBUG.lib"
        IMPORTED_LOCATION_RELEASE "${ARGV2}/${basename}.lib"
    )
    add_dependencies(PhysX::${name} PhysXBuild)
endfunction()

_physx_add_shared(PhysX      PhysX3        "${PHYSX_LIB_DIR}"    "${PHYSX_BIN_DIR}")
_physx_add_shared(Common     PhysX3Common  "${PHYSX_LIB_DIR}"    "${PHYSX_BIN_DIR}")
_physx_add_shared(Cooking    PhysX3Cooking "${PHYSX_LIB_DIR}"    "${PHYSX_BIN_DIR}")
_physx_add_shared(Foundation PxFoundation  "${PXSHARED_LIB_DIR}" "${PXSHARED_BIN_DIR}")
_physx_add_shared(PvdSDK     PxPvdSDK      "${PXSHARED_LIB_DIR}" "${PXSHARED_BIN_DIR}")

_physx_add_static(Extensions PhysX3Extensions "${PHYSX_LIB_DIR}")
_physx_add_static(Vehicle    PhysX3Vehicle    "${PHYSX_LIB_DIR}")

# NVIDIA Tools Extension: a prebuilt profiling library NVIDIA vendors as binaries
# inside the PhysX tree. PhysX3.dll links it, so it must sit beside the exe even
# though we never reference it directly.
add_library(PhysX::nvToolsExt SHARED IMPORTED GLOBAL)
set_target_properties(PhysX::nvToolsExt PROPERTIES
    IMPORTED_IMPLIB  "${PHYSX_DIR}/Externals/nvToolsExt/1/lib/x64/nvToolsExt64_1.lib"
    IMPORTED_LOCATION "${PHYSX_BIN_DIR}/nvToolsExt64_1.dll"
)

# Interface target carrying the include paths and the whole link set.
add_library(PhysX::All INTERFACE IMPORTED GLOBAL)
set_target_properties(PhysX::All PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES
        "${PHYSX_DIR}/PhysX_3.4/Include;${PHYSX_DIR}/PxShared/include"
    INTERFACE_LINK_LIBRARIES
        "PhysX::PhysX;PhysX::Common;PhysX::Cooking;PhysX::Extensions;PhysX::Vehicle;PhysX::Foundation;PhysX::PvdSDK;PhysX::nvToolsExt"
)
