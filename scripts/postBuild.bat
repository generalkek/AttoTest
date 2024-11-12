echo off
if exist "testApps" (
    echo dir exists
) else (
    mkdir "testApps"
)

set SOURCE_DIR=%1
xcopy /S /F /Y "%SOURCE_DIR%" "testApps/"