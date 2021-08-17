@echo off

setlocal

if exist "%~dp0user_env.cmd" (
  call "%~dp0user_env.cmd"
) else (
  call "%~dp0user_env.default.cmd"
)

set ConEmuHttp=http://conemu.github.io/

if "%~1" == "" (
set /P BUILD_NO="Deploy build number: "
) else (
set BUILD_NO=%~1
)
if "%BUILD_NO%" == "" (
Echo Build no was not specified!
exit /B 1
)

cd /d "%~dp0..\"

rem Location of "installer sources template"
set AppTempl=%~dp0..\PortableApps
rem Temporary directory to build "installer sources" from template and ConEmuPack...7z
set AppSetup=%~dp0..\PortableApps_Installer
rem Full path to ConEmuPack...7z, it will be unpacked to %AppSetup% folder
set "AppPack7z=%CONEMU_DEPLOY%Pack\ConEmuPack.%BUILD_NO%.7z"
if NOT exist "%AppPack7z%" (
echo File not found!
echo %AppPack7z%
exit /B 1
)

call :do_del
if exist "%AppSetup%" (
call cecho "Failed to erase: %AppSetup%"
goto err
)

call cecho /yellow "Copying template files"
echo\  Src: %AppTempl%
echo\  Dst: %AppSetup%
xcopy.exe /E "%AppTempl%\*.*" "%AppSetup%\"
if errorlevel 1 goto err

call cecho /yellow "Extracting package"
echo\  Creating folder "%AppSetup%\App\ConEmu"
cd /d "%AppSetup%\App"
if errorlevel 1 goto err
if not exist ConEmu mkdir ConEmu
echo\  cd "%AppSetup%\App\ConEmu"
cd /d "%AppSetup%\App\ConEmu"
if errorlevel 1 goto err
echo\  "%ZIP7%" x "%AppPack7z%"
"%ZIP7%" x "%AppPack7z%"
if errorlevel 1 goto err

cd /d "%~dp0..\"

set AppInfoDir=%AppSetup%\App\AppInfo

rem if exist "%AppInfoDir%\installer.ini" del "%AppInfoDir%\installer.ini"
if exist "%AppSetup%\App\ConEmu\ConEmu64.exe" del "%AppSetup%\App\ConEmu\ConEmu64.exe"

rem set inst_name=ConEmu_%BUILD_NO%_English_online.paf.exe
set inst_name=ConEmu_%BUILD_NO%_English.paf.exe

if exist "%inst_name%" del "%inst_name%"

set SetIniCmd=ver "%AppInfoDir%\appinfo.ini" Version PackageVersion %BUILD_NO%
set SetIniCmd=%SetIniCmd% set "%AppInfoDir%\appinfo.ini" Version DisplayVersion %BUILD_NO%

call cecho /yellow "Updating ini files with version info"
"%~dp0..\src\tools\SetIni.exe" %SetIniCmd%
if errorlevel 1 goto err

call cecho /yellow "Creating PortableApps.com installer"
start "Portable" /MIN /WAIT "%~dp0..\..\Tools\Portable\PortableApps\PortableApps.comInstaller\PortableApps.comInstaller.exe" "%AppSetup%"
if errorlevel 1 goto err
timeout /t 3 1> nul
echo PortableApps.com installer created successfully

if exist "%~dp0sign_any.cmd" (
call cecho /green "Signing..."
call "%~dp0sign_any.cmd" /d "ConEmu %BUILD_NO% & PortableApps.com" /du %ConEmuHttp% "%~dp0..\%inst_name%"
if errorlevel 1 goto err
)

call cecho /yellow "Moving %inst_name% to %CONEMU_DEPLOY%Setup\"
if not exist "%CONEMU_DEPLOY%Setup" mkdir "%CONEMU_DEPLOY%Setup"
:do_move
cd /d "%~dp0..\"
call cecho /green "Waiting for file: %inst_name%"
timeout /t 2 1> nul
move "%inst_name%" "%CONEMU_DEPLOY%Setup\"
if errorlevel 1 (
call cecho "Failed to move file, will retry: %inst_name%"
rem pause
timeout /t 2
goto do_move
)

:do_del
if NOT exist "%AppSetup%" goto :EOF
call cecho /yellow "Erasing temp folder: %AppSetup%"
rd /S /Q "%AppSetup%"
if exist "%AppSetup%" (
call csleep 1000
goto do_del
)
echo Temp folder "%AppSetup%" was cleaned
goto :EOF

:err
call cecho "Failed to create PortableApps.com installer"
pause