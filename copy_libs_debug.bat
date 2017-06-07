@ECHO OFF
XCOPY %~dp0"External\PhysX-3.4\PhysX_3.4\Bin\vc14win32\nvToolsExt32_1.dll" %~dp0"Debug\" /E /I /Y /Q
XCOPY %~dp0"External\PhysX-3.4\PhysX_3.4\Bin\vc14win32\PhysX3CommonDEBUG_x86.dll" %~dp0"Debug\" /E /I /Y /Q
XCOPY %~dp0"External\PhysX-3.4\PhysX_3.4\Bin\vc14win32\PhysX3CookingDEBUG_x86.dll" %~dp0"Debug\" /E /I /Y /Q
XCOPY %~dp0"External\PhysX-3.4\PhysX_3.4\Bin\vc14win32\PhysX3DEBUG_x86.dll" %~dp0"Debug\" /E /I /Y /Q
XCOPY %~dp0"External\PhysX-3.4\PxShared\bin\vc14win32\PxFoundationDEBUG_x86.dll" %~dp0"Debug\" /E /I /Y /Q
XCOPY %~dp0"External\PhysX-3.4\PxShared\bin\vc14win32\PxPvdSDKDEBUG_x86.dll" %~dp0"Debug\" /E /I /Y /Q