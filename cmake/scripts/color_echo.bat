@echo off
set "MESSAGE=%~1"
set "COLOR_NAME=%~2"

rem PowerShell is the best bet for color on modern Windows.
where powershell >nul 2>nul
if %errorlevel% == 0 (
    powershell -Command "switch ('%COLOR_NAME%') { 'GREEN' { $c = 'Green' } 'YELLOW' { $c = 'Yellow' } 'RED' { $c = 'Red' } 'CYAN' { $c = 'Cyan' } default { $c = 'White' } } Write-Host '[INFO]    -- %MESSAGE%' -NoNewline -ForegroundColor $c; Write-Host ''"
) else (
    rem Fallback for systems without PowerShell
    echo [INFO]    -- %MESSAGE%
)