@echo off
REM Orion Player Windows Build & Package Script
REM Copyright (C) 2026 Abhinav Santhosh (GitHub: @abhinavsanthoshpp)
REM All Rights Reserved.

echo ============================================================
echo   Building Orion Player for Windows 64-bit
echo   Copyright (C) 2026 Abhinav Santhosh. All Rights Reserved.
echo ============================================================

set BUILD_DIR=build-windows
set DIST_DIR=windows\dist

if not exist %BUILD_DIR% mkdir %BUILD_DIR%
if not exist %DIST_DIR% mkdir %DIST_DIR%

cd %BUILD_DIR%
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release -j%NUMBER_OF_PROCESSORS%
cd ..

echo Packaging binaries and Qt dependencies...
copy %BUILD_DIR%\Release\orionplayer.exe %DIST_DIR%\
windeployqt --release %DIST_DIR%\orionplayer.exe

echo Creating ZIP archive...
powershell Compress-Archive -Path %DIST_DIR%\* -DestinationPath OrionPlayer-v1.0.0-windows-x64.zip -Force

echo Done! Output: OrionPlayer-v1.0.0-windows-x64.zip
pause
