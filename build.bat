@echo off
echo Building bootloader...
nasm -f bin bootloader.asm -o bootloader.bin
if errorlevel 1 goto error

echo Building game code...
gcc -m32 -ffreestanding -fno-pie -fno-ident -c game.cpp -o game.o
if errorlevel 1 goto error

echo Linking...
ld -m i386pe -T linker.ld --image-base 0x1000 -e _start game.o -o game.pe
if errorlevel 1 goto error

echo Extracting raw binary...
objcopy -O binary game.pe game.bin
if errorlevel 1 goto error

del game.pe 2>nul

echo Creating disk image...
copy /b bootloader.bin + game.bin disk.img
@REM fsutil file createnew padding.bin 1474560
@REM copy /b disk.img + padding.bin disk.img
if errorlevel 1 goto error

echo.
echo ========================================
echo Build successful!
echo ========================================
echo Run with: qemu-system-i386 -fda disk.img
echo.
goto end

:error
echo.
echo ========================================
echo Build failed! Check the error above.
echo ========================================
pause

:end