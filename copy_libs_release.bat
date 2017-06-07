@ECHO OFF
XCOPY %~dp0"External\PhysX-3.4\PhysX_3.4\Bin\vc14win32\nvToolsExt32_1.dll" %~dp0"Release\" /Y /Q
XCOPY %~dp0"External\PhysX-3.4\PhysX_3.4\Bin\vc14win32\PhysX3Common_x86.dll" %~dp0"Release\" /Y /Q
XCOPY %~dp0"External\PhysX-3.4\PhysX_3.4\Bin\vc14win32\PhysX3Cooking_x86.dll" %~dp0"Release\" /Y /Q
XCOPY %~dp0"External\PhysX-3.4\PhysX_3.4\Bin\vc14win32\PhysX3_x86.dll" %~dp0"Release\" /Y /Q
XCOPY %~dp0"External\PhysX-3.4\PxShared\bin\vc14win32\PxFoundation_x86.dll" %~dp0"Release\" /Y /Q
XCOPY %~dp0"External\PhysX-3.4\PxShared\bin\vc14win32\PxPvdSDK_x86.dll" %~dp0"Release\" /Y /Q
XCOPY %~dp0"External\SDL\VisualC\Win32\Release\SDL2.dll" %~dp0"Release\" /Y /Q
XCOPY %~dp0"External\SDL_image\VisualC\Win32\Release\libjpeg-9.dll" %~dp0"Release\" /Y /Q
XCOPY %~dp0"External\SDL_image\VisualC\Win32\Release\libpng16-16.dll" %~dp0"Release\" /Y /Q
XCOPY %~dp0"External\SDL_image\VisualC\Win32\Release\libtiff-5.dll" %~dp0"Release\" /Y /Q
XCOPY %~dp0"External\SDL_image\VisualC\Win32\Release\libwebp-4.dll" %~dp0"Release\" /Y /Q
XCOPY %~dp0"External\SDL_image\VisualC\Win32\Release\zlib1.dll" %~dp0"Release\" /Y /Q
XCOPY %~dp0"External\SDL_image\VisualC\Win32\Release\SDL2_image.dll" %~dp0"Release\" /Y /Q
XCOPY %~dp0"External\SDL_ttf\VisualC\Win32\Release\SDL2_ttf.dll" %~dp0"Release\" /Y /Q
XCOPY %~dp0"External\SDL_ttf\VisualC\Win32\Release\libfreetype-6.dll" %~dp0"Release\" /Y /Q
XCOPY %~dp0"External\assimp\bin\code\Release\assimp-vc140-mt.dll" %~dp0"Release\" /Y /Q