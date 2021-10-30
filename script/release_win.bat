@REM launch this script by double-click on the icon

@REM go back to parent directory
cd ..

@REM create Makefile from .pro file
qmake
@if errorlevel 1 goto end:

@REM compilation
make
@if errorlevel 1 goto end:

@REM compress the binary
upx --best ./release/glop.exe

@REM pause to read errors
:end
pause
