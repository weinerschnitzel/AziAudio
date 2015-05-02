@ECHO OFF
TITLE MinGW Compiler Suite Invocation

set MinGW=C:\MinGW
set src=%CD%\AziAudio
set obj=%CD%\gccbuild

if not exist gccbuild (
mkdir gccbuild
cd gccbuild
mkdir Mupen64plusHLE
)

set FLAGS_x86=-O3^
 -I%obj%\dx_mingw^
 -DXAUDIO_LIBRARIES_UNAVAILABLE^
 -masm=intel^
 -msse2^
 -mstackrealign
set C_FLAGS=%FLAGS_X86%

cd %MinGW%\bin

ECHO Compiling sources...
gcc -o %obj%\Mupen64plusHLE\audio.asm           %src%\Mupen64plusHLE\audio.c          -S %C_FLAGS%
gcc -o %obj%\Mupen64plusHLE\memory.asm          %src%\Mupen64plusHLE\memory.c         -S %C_FLAGS%
gcc -o %obj%\Mupen64plusHLE\Mupen64Support.asm  %src%\Mupen64plusHLE\Mupen64Support.c -S %C_FLAGS%
gcc -o %obj%\Mupen64plusHLE\musyx.asm           %src%\Mupen64plusHLE\musyx.c          -S %C_FLAGS%
g++ -o %obj%\ABI1.asm                   %src%\ABI1.cpp                  -S %C_FLAGS%
g++ -o %obj%\ABI2.asm                   %src%\ABI2.cpp                  -S %C_FLAGS%
g++ -o %obj%\ABI3.asm                   %src%\ABI3.cpp                  -S %C_FLAGS%
g++ -o %obj%\ABI3mp3.asm                %src%\ABI3mp3.cpp               -S %C_FLAGS%
g++ -o %obj%\DirectSoundDriver.asm      %src%\DirectSoundDriver.cpp     -S %C_FLAGS%
g++ -o %obj%\HLEMain.asm                %src%\HLEMain.cpp               -S %C_FLAGS%
g++ -o %obj%\main.asm                   %src%\main.cpp                  -S %C_FLAGS%
g++ -o %obj%\SafeABI.asm                %src%\SafeABI.cpp               -S %C_FLAGS%
g++ -o %obj%\WaveOut.asm                %src%\WaveOut.cpp               -S %C_FLAGS%
g++ -o %obj%\XAudio2SoundDriver.asm     %src%\XAudio2SoundDriver.cpp    -S %C_FLAGS%

ECHO Assembling compiled sources...
as -o %obj%\ABI1.o                          %obj%\ABI1.asm
as -o %obj%\ABI2.o                          %obj%\ABI2.asm
as -o %obj%\ABI3.o                          %obj%\ABI3.asm
as -o %obj%\ABI3mp3.o                       %obj%\ABI3mp3.asm
as -o %obj%\DirectSoundDriver.o             %obj%\DirectSoundDriver.asm
as -o %obj%\HLEMain.o                       %obj%\HLEMain.asm
as -o %obj%\main.o                          %obj%\main.asm
as -o %obj%\SafeABI.o                       %obj%\SafeABI.asm
as -o %obj%\WaveOut.o                       %obj%\WaveOut.asm
as -o %obj%\XAudio2SoundDriver.o            %obj%\XAudio2SoundDriver.asm

as -o %obj%\Mupen64plusHLE\audio.o          %obj%\Mupen64plusHLE\audio.asm
as -o %obj%\Mupen64plusHLE\memory.o         %obj%\Mupen64plusHLE\memory.asm
as -o %obj%\Mupen64plusHLE\Mupen64Support.o %obj%\Mupen64plusHLE\Mupen64Support.asm
as -o %obj%\Mupen64plusHLE\musyx.o          %obj%\Mupen64plusHLE\musyx.asm
ECHO.

set OBJ_LIST=^
%obj%\ABI1.o ^
%obj%\ABI2.o ^
%obj%\ABI3.o ^
%obj%\ABI3mp3.o ^
%obj%\DirectSoundDriver.o ^
%obj%\HLEMain.o ^
%obj%\main.o ^
%obj%\SafeABI.o ^
%obj%\WaveOut.o ^
%obj%\XAudio2SoundDriver.o ^
%obj%\Mupen64plusHLE\audio.o ^
%obj%\Mupen64plusHLE\memory.o ^
%obj%\Mupen64plusHLE\Mupen64Support.o ^
%obj%\Mupen64plusHLE\musyx.o

ECHO Linking assembled object files...
g++ -o %obj%\AziAudio.dll %OBJ_LIST% -ldsound --shared -s
PAUSE
