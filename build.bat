@echo off
REM Build script for PlanetVegeta — runs from native Windows CMD so GCC
REM gets correct TMP/TEMP env vars (avoids MSYS2's /tmp POSIX path issue).
set PATH=C:\msys64\ucrt64\bin;C:\msys64\usr\bin;%PATH%
set CMAKE=C:\Program Files\CMake\bin\cmake.exe

if not exist build mkdir build
cd build

"%CMAKE%" -G "MSYS Makefiles" ..
"%CMAKE%" --build . --config Release

cd ..
echo.
echo === Build complete ===
echo Executable: build\PlanetVegeta.exe
