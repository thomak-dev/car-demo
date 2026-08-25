# Copies the asset tree next to the executable, skipping authoring-only files.
#
# Invoked at build time with -P, because file(COPY)'s PATTERN ... EXCLUDE is what
# gives us the filtering that cmake -E copy_directory cannot do. The exclusions
# mirror the old exclude_assets.txt: source art is large and never loaded at
# runtime.
#
# Expects -DSRC=<asset dir> -DDST=<output dir>

if(NOT DEFINED SRC OR NOT DEFINED DST)
    message(FATAL_ERROR "CopyAssets.cmake requires -DSRC=<dir> -DDST=<dir>")
endif()

file(COPY "${SRC}/"
    DESTINATION "${DST}"
    PATTERN "*.psd"               EXCLUDE
    PATTERN "*.blend"             EXCLUDE
    PATTERN "*.blend1"            EXCLUDE
)
