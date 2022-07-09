:: Doxygenê∂ê¨Ç®ÇÊÇ—ï\é¶
@echo off

cd document
"%ProgramFiles%\doxygen\bin\doxygen" .\Doxyfile
start .\html\index.html

exit /b 0
