# Locates the FMOD Studio authoring tool's command-line build driver.
#
# This is the GUI authoring application, not the FMOD Engine SDK (see FindFMOD).
# It ships fmodstudiocl.exe, which builds banks headlessly:
#
#   fmodstudiocl -build -platforms Desktop <project.fspro>
#
# Only needed to regenerate banks. The engine links fine without it; it just
# aborts at the first loadBankFile if no banks were ever built.
#
# Defines:
#   FMODStudio_FOUND
#   FMODSTUDIO_CL   path to fmodstudiocl.exe
#
# Override with -DFMODSTUDIO_CL=<path>.

if(NOT FMODSTUDIO_CL)
    # Installs land in versioned directories ("FMOD Studio 2.03.14"), so glob and
    # take the highest version rather than guessing one.
    file(GLOB _fmod_studio_dirs
        "$ENV{ProgramFiles}/FMOD SoundSystem/FMOD Studio *"
        "$ENV{ProgramFiles\(x86\)}/FMOD SoundSystem/FMOD Studio *"
    )
    list(SORT _fmod_studio_dirs COMPARE NATURAL ORDER DESCENDING)

    find_program(FMODSTUDIO_CL
        NAMES fmodstudiocl
        PATHS ${_fmod_studio_dirs}
        DOC "FMOD Studio command-line bank builder (fmodstudiocl.exe)"
        NO_DEFAULT_PATH
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FMODStudio
    REQUIRED_VARS FMODSTUDIO_CL
    # No semicolons in this message: it is re-expanded as a list inside
    # find_package_handle_standard_args and would be split into stray keywords.
    FAIL_MESSAGE
        "FMOD Studio authoring tool not found, so banks cannot be rebuilt \
automatically. Build them from Audio/Audio.fspro in the FMOD Studio GUI, or pass \
-DFMODSTUDIO_CL=<path to fmodstudiocl.exe>."
)

mark_as_advanced(FMODSTUDIO_CL)
