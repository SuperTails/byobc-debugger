@echo off

curl -o dasm.zip --location https://github.com/dasm-assembler/dasm/releases/download/2.20.14.1/dasm-2.20.14.1-win-x64.zip
rmdir dasm /s /q
mkdir dasm
tar -xf dasm.zip -C dasm
del dasm.zip

dasm/dasm

echo.
echo If you see "fatal assembly error" above, that means dasm was successfully installed!
echo.

echo Installing pyserial

python -m pip install pyserial

echo.
echo Install finished

pause