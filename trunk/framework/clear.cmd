rd /s /q debug-x64
rd /s /q release-x64
rd /s /q debug-win32
rd /s /q release-win32

cd win32
del *.user
del *.ncb
rem del *.suo
rd /s /q debug-x64
rd /s /q release-x64
rd /s /q debug-win32
rd /s /q release-win32
cd ..

cd tools
del *.lex.c
del *.tab.c
del *.tab.h
cd ..
