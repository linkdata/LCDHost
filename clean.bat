@echo off
setlocal
if exist Debug rmdir /s /q Debug >NUL
if exist Release rmdir /s /q Release >NUL
if exist *.pro.user del *.pro.user
for /F "tokens=*" %%i in ('cd') do set BASENAME=%%~nx%i
for /D %%i in (..\build-%BASENAME%-*) do rmdir /s /q %%i
endlocal
hg status --quiet --subrepos
