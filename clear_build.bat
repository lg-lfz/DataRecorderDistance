@echo off
echo DELETE AND CLEANUP THE BUILD FOLDER CONTENT...
del /s /f /q .\build\*
for /f %%f in ('dir /ad /b .\build\') do rd /s /q .\build\%%f
echo DONE
