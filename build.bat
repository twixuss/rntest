@echo off
setlocal
pushd
if not exist bin mkdir bin
cd bin
set cflags=/nologo /I"../" /O2 /Oi /Ob2 /Zi
cl ../rngs/mulxor32.c %cflags%     || goto :error
cl ../rngs/mulxor64hi32.c %cflags% || goto :error
cl ../rngs/rand.c %cflags%         || goto :error
popd
exit /b 0
:error
popd
exit /b -1
