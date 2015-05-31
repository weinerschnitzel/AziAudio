@ECHO OFF
TITLE MinGW Compiler Suite Invocation

:Set this to your mingw bin directory (contains gcc.exe)
set MinGW=C:\MinGW\bin


set CFLAGS= -O2 -msse2 -DSSE2_SUPPORT -mstackrealign
set LDFLAGS= -static-libstdc++ -static-libgcc
set RESFLAGS=
set XA_FLAGS= -I"3rd Party/directx/include" -I"3rd Party"
set DS_FLAGS= -DXAUDIO_LIBRARIES_UNAVAILABLE
set SRCDIR=AziAudio

set OBJDIR=build/Release_mingw32
set BUILDDIR=bin/Release_mingw32

if not exist build\Release_mingw32 (
mkdir build\Release_mingw32
)
if not exist bin\Release_mingw32 (
mkdir bin\Release_mingw32
)
set PATH=%MinGW%;%PATH%

ECHO Compiling sources...
g++ %CFLAGS% %XA_FLAGS%     -o %OBJDIR%/XAudio2SoundDriver.o        -c %SRCDIR%/XAudio2SoundDriver.cpp
g++ %CFLAGS% %DS_FLAGS%     -o %OBJDIR%/DummyXAudio2SoundDriver.o   -c %SRCDIR%/XAudio2SoundDriver.cpp
g++ %CFLAGS% %XA_FLAGS%     -o %OBJDIR%/XA2main.o                   -c %SRCDIR%/main.cpp
g++ %CFLAGS% %DS_FLAGS%     -o %OBJDIR%/DS8main.o                   -c %SRCDIR%/main.cpp
g++ %CFLAGS% %XA_FLAGS%     -o %OBJDIR%/XA2HLEMain.o                -c %SRCDIR%/HLEMain.cpp
g++ %CFLAGS% %DS_FLAGS%     -o %OBJDIR%/DS8HLEMain.o                -c %SRCDIR%/HLEMain.cpp
g++ %CFLAGS% %XA_FLAGS%     -o %OBJDIR%/DummyDirectSoundDriver.o    -c %SRCDIR%/DirectSoundDriver.cpp
g++ %CFLAGS% %DS_FLAGS%     -o %OBJDIR%/DirectSoundDriver.o         -c %SRCDIR%/DirectSoundDriver.cpp
g++ %CFLAGS%                -o %OBJDIR%/WaveOut.o                   -c %SRCDIR%/WaveOut.cpp
g++ %CFLAGS%                -o %OBJDIR%/ABI_Resample.o              -c %SRCDIR%/ABI_Resample.cpp
g++ %CFLAGS%                -o %OBJDIR%/ABI_MixerInterleave.o       -c %SRCDIR%/ABI_MixerInterleave.cpp
g++ %CFLAGS%                -o %OBJDIR%/ABI_Filters.o               -c %SRCDIR%/ABI_Filters.cpp
g++ %CFLAGS%                -o %OBJDIR%/ABI_Envmixer.o              -c %SRCDIR%/ABI_Envmixer.cpp
g++ %CFLAGS%                -o %OBJDIR%/ABI_Buffers.o               -c %SRCDIR%/ABI_Buffers.cpp
g++ %CFLAGS%                -o %OBJDIR%/ABI_Adpcm.o                 -c %SRCDIR%/ABI_Adpcm.cpp
g++ %CFLAGS%                -o %OBJDIR%/ABI3mp3.o                   -c %SRCDIR%/ABI3mp3.cpp
g++ %CFLAGS%                -o %OBJDIR%/ABI3.o                      -c %SRCDIR%/ABI3.cpp
g++ %CFLAGS%                -o %OBJDIR%/ABI2.o                      -c %SRCDIR%/ABI2.cpp
g++ %CFLAGS%                -o %OBJDIR%/ABI1.o                      -c %SRCDIR%/ABI1.cpp
gcc %CFLAGS%                -o %OBJDIR%/musyx.o                     -c %SRCDIR%/Mupen64plusHLE/musyx.c
gcc %CFLAGS%                -o %OBJDIR%/Mupen64Support.o            -c %SRCDIR%/Mupen64plusHLE/Mupen64Support.c
gcc %CFLAGS%                -o %OBJDIR%/memory.o                    -c %SRCDIR%/Mupen64plusHLE/memory.c
gcc %CFLAGS%                -o %OBJDIR%/audio.o                     -c %SRCDIR%/Mupen64plusHLE/audio.c
windres %RESFLAGS% %SRCDIR%/resource.rc %OBJDIR%/resource.o

set COMMON_OBJS=^
%OBJDIR%/WaveOut.o ^
%OBJDIR%/ABI_Resample.o ^
%OBJDIR%/ABI_MixerInterleave.o ^
%OBJDIR%/ABI_Filters.o ^
%OBJDIR%/ABI_Envmixer.o ^
%OBJDIR%/ABI_Buffers.o ^
%OBJDIR%/ABI_Adpcm.o ^
%OBJDIR%/ABI3mp3.o ^
%OBJDIR%/ABI3.o ^
%OBJDIR%/ABI2.o ^
%OBJDIR%/ABI1.o ^
%OBJDIR%/musyx.o ^
%OBJDIR%/Mupen64Support.o ^
%OBJDIR%/memory.o ^
%OBJDIR%/audio.o ^
%OBJDIR%/resource.o

set XA_OBJS=^
%OBJDIR%/XAudio2SoundDriver.o ^
%OBJDIR%/XA2HLEMain.o ^
%OBJDIR%/XA2main.o ^
%OBJDIR%/DummyDirectSoundDriver.o

set DS_OBJS=^
%OBJDIR%/DummyXAudio2SoundDriver.o ^
%OBJDIR%/DS8HLEMain.o ^
%OBJDIR%/DS8main.o ^
%OBJDIR%/DirectSoundDriver.o

ECHO Linking assembled object files...
g++ -shared %CFLAGS% -o %obj%\AziAudioDS8_m.dll %DS_OBJS% %COMMON_OBJS% %LDFLAGS% -ldsound
g++ -shared %CFLAGS% -o %obj%\AziAudioXA2_m.dll %XA_OBJS% %COMMON_OBJS% %LDFLAGS% -lole32
PAUSE