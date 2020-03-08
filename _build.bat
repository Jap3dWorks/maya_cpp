rmdir /s build
mkdir build
cd build
cmake -G "Visual Studio 14 Win64" -DMAYA_VERSION=2018 ..\
cmake --build . --config Release --target Install
cd ..
pause
