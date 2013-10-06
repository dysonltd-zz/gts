@echo OFF

if [%1]==[] goto usage
	call "C:\Program Files\Microsoft Visual Studio 10.0\VC\bin\vcvars32.bat"
	cd ../installer
	cmake.exe -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE=RELEASE -DOpenCV_ROOT_DIR="%1" .
	mingw32-make package
	cd ..
goto :eof
:usage
	@echo Call from scripts folder
	@echo Usage: %0 OpenCV Location:^<PATH-TO-OPENCV^>
	@echo 	set ^<param^> values
	@echo 	e.g. %0 C:/opencv/
exit /B 1
