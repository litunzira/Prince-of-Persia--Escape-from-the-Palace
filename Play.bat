@echo off
rem The game opens its pictures and sounds by relative path, so it must be
rem started from the iGame folder. This launcher does that for you.
cd /d "%~dp0iGame"
if not exist "iGame.exe" (
    echo iGame.exe is not built yet.
    echo Open iGame.sln in Visual Studio 2013 and build the solution.
    pause
    exit /b 1
)
start "" "iGame.exe"
