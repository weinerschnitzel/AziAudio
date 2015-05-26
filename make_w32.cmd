@ECHO OFF
TITLE MinGW Compiler Suite Invocation

set MinGW=C:\MinGW
set src=%CD%\AziAudio
set obj=%CD%\gccbuild

if not exist gccbuild (
mkdir gccbuild
)

cd gccbuild

if not exist Mupen64plusHLE (
mkdir Mupen64plusHLE
)

set FLAGS_x86=^
 -I"%src%\..\3rd Party"^
 -I"%src%\..\3rd Party\directx\include"^
 -DSSE2_SUPPORT^
 -masm=intel^
 -msse2^
 -mstackrealign
set C_FLAGS=%FLAGS_X86%

cd %MinGW%\bin

ECHO Compiling sources...
gcc -o %obj%\Mupen64plusHLE\audio.asm          -x c %src%\Mupen64plusHLE\audio.c          -S %C_FLAGS% -O2
gcc -o %obj%\Mupen64plusHLE\memory.asm         -x c %src%\Mupen64plusHLE\memory.c         -S %C_FLAGS% -O2
gcc -o %obj%\Mupen64plusHLE\Mupen64Support.asm -x c %src%\Mupen64plusHLE\Mupen64Support.c -S %C_FLAGS% -Os
gcc -o %obj%\Mupen64plusHLE\musyx.asm          -x c %src%\Mupen64plusHLE\musyx.c          -S %C_FLAGS% -O2
g++ -o %obj%\ABI1.asm                   -x c++ %src%\ABI1.cpp                  -S %C_FLAGS% -O2
g++ -o %obj%\ABI2.asm                   -x c++ %src%\ABI2.cpp                  -S %C_FLAGS% -O2
g++ -o %obj%\ABI3.asm                   -x c++ %src%\ABI3.cpp                  -S %C_FLAGS% -O2
g++ -o %obj%\ABI3mp3.asm                -x c++ %src%\ABI3mp3.cpp               -S %C_FLAGS% -O2
g++ -o %obj%\ABI_Adpcm.asm              -x c++ %src%\ABI_Adpcm.cpp             -S %C_FLAGS% -O2
g++ -o %obj%\ABI_Buffers.asm            -x c++ %src%\ABI_Buffers.cpp           -S %C_FLAGS% -O2
g++ -o %obj%\ABI_Envmixer.asm           -x c++ %src%\ABI_Envmixer.cpp          -S %C_FLAGS% -O2
g++ -o %obj%\ABI_Filters.asm            -x c++ %src%\ABI_Filters.cpp           -S %C_FLAGS% -O2
g++ -o %obj%\ABI_MixerInterleave.asm    -x c++ %src%\ABI_MixerInterleave.cpp   -S %C_FLAGS% -O2
g++ -o %obj%\ABI_Resample.asm           -x c++ %src%\ABI_Resample.cpp          -S %C_FLAGS% -O2
g++ -o %obj%\DirectSoundDriver.asm      -x c++ %src%\DirectSoundDriver.cpp     -S %C_FLAGS% -Os
g++ -o %obj%\HLEMain.asm                -x c++ %src%\HLEMain.cpp               -S %C_FLAGS% -Os
g++ -o %obj%\main.asm                   -x c++ %src%\main.cpp                  -S %C_FLAGS% -Os
g++ -o %obj%\WaveOut.asm                -x c++ %src%\WaveOut.cpp               -S %C_FLAGS% -Os
g++ -o %obj%\XAudio2SoundDriver.asm     -x c++ %src%\XAudio2SoundDriver.cpp    -S %C_FLAGS% -Os

ECHO Assembling compiled sources...
as -o %obj%\ABI1.o                          %obj%\ABI1.asm
as -o %obj%\ABI2.o                          %obj%\ABI2.asm
as -o %obj%\ABI3.o                          %obj%\ABI3.asm
as -o %obj%\ABI3mp3.o                       %obj%\ABI3mp3.asm
as -o %obj%\ABI_Adpcm.o                     %obj%\ABI_Adpcm.asm
as -o %obj%\ABI_Buffers.o                   %obj%\ABI_Buffers.asm
as -o %obj%\ABI_Envmixer.o                  %obj%\ABI_Envmixer.asm
as -o %obj%\ABI_Filters.o                   %obj%\ABI_Filters.asm
as -o %obj%\ABI_MixerInterleave.o           %obj%\ABI_MixerInterleave.asm
as -o %obj%\ABI_Resample.o                  %obj%\ABI_Resample.asm
as -o %obj%\DirectSoundDriver.o             %obj%\DirectSoundDriver.asm
as -o %obj%\HLEMain.o                       %obj%\HLEMain.asm
as -o %obj%\main.o                          %obj%\main.asm
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
%obj%\ABI_Adpcm.o ^
%obj%\ABI_Buffers.o ^
%obj%\ABI_Envmixer.o ^
%obj%\ABI_Filters.o ^
%obj%\ABI_MixerInterleave.o ^
%obj%\ABI_Resample.o ^
%obj%\DirectSoundDriver.o ^
%obj%\HLEMain.o ^
%obj%\main.o ^
%obj%\WaveOut.o ^
%obj%\XAudio2SoundDriver.o ^
%obj%\Mupen64plusHLE\audio.o ^
%obj%\Mupen64plusHLE\memory.o ^
%obj%\Mupen64plusHLE\Mupen64Support.o ^
%obj%\Mupen64plusHLE\musyx.o ^
%obj%\res.res

ECHO Linking assembled object files...
windres -i %src%\resource.rc --input-format=rc -o %obj%\res.res -O coff
g++ -o %obj%\AziAudio.dll %OBJ_LIST% -ldsound -lole32 --shared -s -Wl,--subsystem,windows -shared -shared-libgcc
PAUSE
