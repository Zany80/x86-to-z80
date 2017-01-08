@echo on
cd %~dp0
:: appearently clang needs visual studio headers to compiler, which doesn't make much sense to me, but it works.
call "c:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\Tools\vsvars32.bat"
C:\project\LLVM\bin\clang++.exe %1 -O3 -S
::C:\project\LLVM\bin\clang++.exe %1 -O3 -S -o-

:: http://clang.llvm.org/get_started.html, consider using:
:: -fomit-frame-pointer
:: -emit-llvm  (maybe less optimized?)
::
