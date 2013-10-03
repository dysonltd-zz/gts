@echo OFF

if [%1]==[] goto usage
	call "C:\Program Files\Microsoft Visual Studio 10.0\VC\bin\vcvars32.bat"
	cd ../installer
	cmake.exe -G"NMake Makefiles" -DCMAKE_BUILD_TYPE=RELEASE -DOpenCV_ROOT_DIR="%1" .
	nmake package
goto :eof
:usage
	@echo Call from scripts folder
	@echo Usage: %0 ^<PATH-TO-OPENCV^>
exit /B 1