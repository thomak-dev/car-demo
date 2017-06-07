@ECHO OFF
XCOPY %~dp0"External\PhysX-3.4\PhysX_3.4\Bin\vc14win32\nvToolsExt32_1.dll" %~dp0"Debug\" /Y /Q
XCOPY %~dp0"External\PhysX-3.4\PhysX_3.4\Bin\vc14win32\PhysX3CommonDEBUG_x86.dll" %~dp0"Debug\" /Y /Q
XCOPY %~dp0"External\PhysX-3.4\PhysX_3.4\Bin\vc14win32\PhysX3CookingDEBUG_x86.dll" %~dp0"Debug\" /Y /Q
XCOPY %~dp0"External\PhysX-3.4\PhysX_3.4\Bin\vc14win32\PhysX3DEBUG_x86.dll" %~dp0"Debug\" /Y /Q
XCOPY %~dp0"External\PhysX-3.4\PxShared\bin\vc14win32\PxFoundationDEBUG_x86.dll" %~dp0"Debug\" /Y /Q
XCOPY %~dp0"External\PhysX-3.4\PxShared\bin\vc14win32\PxPvdSDKDEBUG_x86.dll" %~dp0"Debug\" /Y /Q
XCOPY %~dp0"External\SDL\VisualC\Win32\Debug\SDL2.dll" %~dp0"Debug\" /Y /Q
XCOPY %~dp0"External\SDL_image\VisualC\Win32\Debug\libjpeg-9.dll" %~dp0"Debug\" /Y /Q
XCOPY %~dp0"External\SDL_image\VisualC\Win32\Debug\libpng16-16.dll" %~dp0"Debug\" /Y /Q
XCOPY %~dp0"External\SDL_image\VisualC\Win32\Debug\libtiff-5.dll" %~dp0"Debug\" /Y /Q
XCOPY %~dp0"External\SDL_image\VisualC\Win32\Debug\libwebp-4.dll" %~dp0"Debug\" /Y /Q
XCOPY %~dp0"External\SDL_image\VisualC\Win32\Debug\zlib1.dll" %~dp0"Debug\" /Y /Q
XCOPY %~dp0"External\SDL_image\VisualC\Win32\Debug\SDL2_image.dll" %~dp0"Debug\" /Y /Q
XCOPY %~dp0"External\SDL_ttf\VisualC\Win32\Debug\SDL2_ttf.dll" %~dp0"Debug\" /Y /Q
XCOPY %~dp0"External\SDL_ttf\VisualC\Win32\Debug\libfreetype-6.dll" %~dp0"Debug\" /Y /Q
XCOPY %~dp0"External\assimp\bin\code\Debug\assimp-vc140-mt.dll" %~dp0"Debug\" /Y /Q