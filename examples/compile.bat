@echo on
cd %~dp0
:: appearently clang needs visual studio headers to compiler, which doesn't make much sense to me, but it works.
call "c:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\Tools\vsvars32.bat"
bin\clang++.exe main.cpp -O3 -S
bin\clang++.exe main.cpp -O3 -S -o-
