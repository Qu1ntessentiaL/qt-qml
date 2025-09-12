@echo off
chcp 65001 > nul
setlocal

REM Пути к Qt и MinGW — настрой под себя
set QT_BIN=C:\Qt\6.7.2\mingw_64\bin
set MINGW_BIN=C:\Qt\Tools\mingw1120_64\bin

REM Исходный exe и папки
set TARGET=cmake-build-debug\appqt-qml.exe
set OUT_DIR=cmake-build-debug
set DEPLOY_DIR=%OUT_DIR%\deploy

REM Создаём папку deploy, если нет
if not exist "%DEPLOY_DIR%" mkdir "%DEPLOY_DIR%"

echo [*] Копируем exe в deploy...
copy /Y "%TARGET%" "%DEPLOY_DIR%"

echo [*] Запускаем windeployqt (лог будет в deploy_log.txt)...
"%QT_BIN%\windeployqt.exe" --no-translations --dir "%DEPLOY_DIR%" --verbose 1 "%DEPLOY_DIR%\appqt-qml.exe" > "%DEPLOY_DIR%\deploy_log.txt" 2>&1

echo [*] Копируем MinGW DLLs в deploy...
copy /Y "%MINGW_BIN%\libstdc++-6.dll" "%DEPLOY_DIR%"
copy /Y "%MINGW_BIN%\libgcc_s_seh-1.dll" "%DEPLOY_DIR%"
copy /Y "%MINGW_BIN%\libwinpthread-1.dll" "%DEPLOY_DIR%"

echo [*] Деплой завершён. Смотрите логи в "%DEPLOY_DIR%\deploy_log.txt"
pause
