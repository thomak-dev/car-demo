@ECHO OFF
XCOPY %~dp0"External\PhysX-3.4\PhysX_3.4\Bin\vc14win32\nvToolsExt32_1.dll" %~dp0"Release\" /E /I /Y /Q
XCOPY %~dp0"External\PhysX-3.4\PhysX_3.4\Bin\vc14win32\PhysX3Common_x86.dll" %~dp0"Release\" /E /I /Y /Q
XCOPY %~dp0"External\PhysX-3.4\PhysX_3.4\Bin\vc14win32\PhysX3Cooking_x86.dll" %~dp0"Release\" /E /I /Y /Q
XCOPY %~dp0"External\PhysX-3.4\PhysX_3.4\Bin\vc14win32\PhysX3_x86.dll" %~dp0"Release\" /E /I /Y /Q
XCOPY %~dp0"External\PhysX-3.4\PxShared\bin\vc14win32\PxFoundation_x86.dll" %~dp0"Release\" /E /I /Y /Q
XCOPY %~dp0"External\PhysX-3.4\PxShared\bin\vc14win32\PxPvdSDK_x86.dll" %~dp0"Release\" /E /I /Y /Q