@setlocal enableextensions enabledelayedexpansion
@ECHO off
ECHO ÕÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¸
ECHO ³ AziAudio MSYS-less MinGW build script v2.0                             ³
ECHO ÆÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¾


 REM Defaults
SET MinGW_PATH_DEFAULT=C:\MinGW





 REM Load all arguments into variables so we can easily parse them.
FOR %%a IN (%*) do (
    CALL:getargs "%%a" 
)

 REM Parse --help
IF DEFINED --help (
    ECHO ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ECHO ³ Use this script to build AziAudio without using MSYS or messing with   ³
    ECHO ³ your PATH. Both versions of the plugin will be built.                  ³
    ECHO ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´
    ECHO ³ This script offers some limited Configuration options.                 ³
    ECHO ³                                                                        ³
    ECHO ³ --mingwpath [directory]                                                ³
    ECHO ³   Location of your mingw installation. We look for the bin folder, so  ³
    ECHO ³   do not point this path directly at the location of GCC.              ³
    ECHO ³   DEFAULT: C:\MinGW                                                    ³
    ECHO ³                                                                        ³
    ECHO ³ --debug                                                                ³
    ECHO ³   Pass this argument to make debug builds.                             ³
    ECHO ³                                                                        ³
    ECHO ³ --arch [architecture]                                                  ³
    ECHO ³   Choose which architecture to build for. Valid options are x86 and    ³
    ECHO ³   x86_64.                                                              ³
    ECHO ³   DEFAULT: try to auto-detect                                          ³
    ECHO ³   NOTE: This option only affects the build locations.                  ³
    ECHO ³                                                                        ³
    ECHO ³ --help                                                                 ³
    ECHO ³   You're already here.                                                 ³
    ECHO ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
    GOTO done
)

 REM Parse --mingwpath
IF NOT DEFINED --mingwpath (
    SET --mingwpath=%MinGW_PATH_DEFAULT%
)
CALL:argtopath %--mingwpath% MinGW
SET PATH=%MinGW%\bin;%PATH%

ECHO ³ Verifying presence of needed tools...
CALL:existsinpath gcc
IF errorlevel 404 (
    exit /b 404
)
CALL:existsinpath g++
IF errorlevel 404 (
    exit /b 404
)
CALL:existsinpath ar
IF errorlevel 404 (
    exit /b 404
)
CALL:existsinpath windres
IF errorlevel 404 (
    exit /b 404
)

 REM Parse --debug
IF NOT DEFINED --debug_PASSED (
	SET CFLAGS_BUILD= -O2
	SET RESFLAGS_BUILD=
	SET BUILD_TYPE=Release
	SET XA_PLUGIN_FILE=AziAudioXA2_m.dll
	SET DS_PLUGIN_FILE=AziAudioDS8_m.dll
) ELSE (
	SET CFLAGS_BUILD= -g -D_DEBUG
	SET RESFLAGS_BUILD= -D_DEBUG
	SET BUILD_TYPE=Debug
	SET XA_PLUGIN_FILE=AziAudioXA2_md.dll
	SET DS_PLUGIN_FILE=AziAudioDS8_md.dll
)

 REM Parse --arch
CALL:detectarch
IF NOT DEFINED --arch (
    SET --arch=%ARCH_DETECTED%
) ELSE (
    ECHO --arch is %--arch%
)
IF "%--arch%" == "x86" (
    SET ARCH=x86
    SET OUTDIR=%BUILD_TYPE%_mingw32
) ELSE ( IF "%--arch%" == "x86_64" (
    SET ARCH=x86_64
    SET OUTDIR=%BUILD_TYPE%_mingw64
) ELSE (
    ECHO ³ FATAL:    Unsupported architecture specified.
    ECHO ³           Valid architectures are x86 and x86_64.
    ECHO À Stopping.
    exit /b 400
) )
IF NOT %ARCH% == %ARCH_DETECTED% (
    ECHO ³ WARNING:  Detected GCC target architecture does not match the architecture
    ECHO ³           specified.
    ECHO ³           Specified architecture: %ARCH%
    ECHO ³           GCC Target:             %GCC_TRIP%
    ECHO ³                                   (interpreted as %ARCH_DETECTED%^)
)

 REM Set variables
set OBJDIR=build/%OUTDIR%
set BUILDDIR=bin/%OUTDIR%
set CFLAGS= %CFLAGS_BUILD% -msse2 -DSSE2_SUPPORT -mstackrealign
set LDFLAGS= -static-libstdc++ -static-libgcc
set RESFLAGS= %RESFLAGS_BUILD%
set XA_FLAGS= -I"3rd Party/directx/include" -I"3rd Party" -Wno-attributes
set DS_FLAGS= -DXAUDIO_LIBRARIES_UNAVAILABLE -Wno-conversion-null
set SRCDIR=AziAudio

ECHO ÆÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¸
ECHO ³ Configuration:                                                         ³
ECHO ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
ECHO ³ MinGW location:       %MinGW%
ECHO ³   (Don't worry if this isn't accurate)
ECHO ³ Build architecture:  %ARCH%
ECHO ³ Build type:          %BUILD_TYPE%
ECHO À Starting build...

 REM Make directories
CALL:mkdir %OBJDIR%
CALL:mkdir %BUILDDIR%

ECHO Compiling Sources...
ECHO     XAudio2SoundDriver.cpp (for XA2 plugin)...
g++ %CFLAGS% %XA_FLAGS%     -o %OBJDIR%/XAudio2SoundDriver.o        -c %SRCDIR%/XAudio2SoundDriver.cpp
ECHO     XAudio2SoundDriver.cpp (for DS8 plugin)...
g++ %CFLAGS% %DS_FLAGS%     -o %OBJDIR%/DummyXAudio2SoundDriver.o   -c %SRCDIR%/XAudio2SoundDriver.cpp
ECHO     main.cpp (for XA2 plugin)...
g++ %CFLAGS% %XA_FLAGS%     -o %OBJDIR%/XA2main.o                   -c %SRCDIR%/main.cpp
ECHO     main.cpp (for DS8 plugin)...
g++ %CFLAGS% %DS_FLAGS%     -o %OBJDIR%/DS8main.o                   -c %SRCDIR%/main.cpp
ECHO     HLEMain.cpp (for XA2 plugin)...
g++ %CFLAGS% %XA_FLAGS%     -o %OBJDIR%/XA2HLEMain.o                -c %SRCDIR%/HLEMain.cpp
ECHO     HLEMain.cpp (for DS8 plugin)...
g++ %CFLAGS% %DS_FLAGS%     -o %OBJDIR%/DS8HLEMain.o                -c %SRCDIR%/HLEMain.cpp
ECHO     DirectSoundDriver.cpp (for XA2 plugin)...
g++ %CFLAGS% %XA_FLAGS%     -o %OBJDIR%/DummyDirectSoundDriver.o    -c %SRCDIR%/DirectSoundDriver.cpp
ECHO     DirectSoundDriver.cpp (for DS8 plugin)...
g++ %CFLAGS% %DS_FLAGS%     -o %OBJDIR%/DirectSoundDriver.o         -c %SRCDIR%/DirectSoundDriver.cpp
ECHO     WaveOut.cpp...
g++ %CFLAGS%                -o %OBJDIR%/WaveOut.o                   -c %SRCDIR%/WaveOut.cpp
ECHO     ABI_Resample.cpp...
g++ %CFLAGS%                -o %OBJDIR%/ABI_Resample.o              -c %SRCDIR%/ABI_Resample.cpp
ECHO     ABI_MixerInterleave.cpp...
g++ %CFLAGS%                -o %OBJDIR%/ABI_MixerInterleave.o       -c %SRCDIR%/ABI_MixerInterleave.cpp
ECHO     ABI_Filters.cpp...
g++ %CFLAGS%                -o %OBJDIR%/ABI_Filters.o               -c %SRCDIR%/ABI_Filters.cpp
ECHO     ABI_Envmixer.cpp...
g++ %CFLAGS%                -o %OBJDIR%/ABI_Envmixer.o              -c %SRCDIR%/ABI_Envmixer.cpp
ECHO     ABI_Buffers.cpp...
g++ %CFLAGS%                -o %OBJDIR%/ABI_Buffers.o               -c %SRCDIR%/ABI_Buffers.cpp
ECHO     ABI_Adpcm.cpp...
g++ %CFLAGS%                -o %OBJDIR%/ABI_Adpcm.o                 -c %SRCDIR%/ABI_Adpcm.cpp
ECHO     ABI3mp3.cpp...
g++ %CFLAGS%                -o %OBJDIR%/ABI3mp3.o                   -c %SRCDIR%/ABI3mp3.cpp
ECHO     ABI3.cpp...
g++ %CFLAGS%                -o %OBJDIR%/ABI3.o                      -c %SRCDIR%/ABI3.cpp
ECHO     ABI2.cpp...
g++ %CFLAGS%                -o %OBJDIR%/ABI2.o                      -c %SRCDIR%/ABI2.cpp
ECHO     ABI1.cpp...
g++ %CFLAGS%                -o %OBJDIR%/ABI1.o                      -c %SRCDIR%/ABI1.cpp
ECHO     Mupen64plusHLE/musyx.c...
gcc %CFLAGS%                -o %OBJDIR%/musyx.o                     -c %SRCDIR%/Mupen64plusHLE/musyx.c
ECHO     Mupen64plusHLE/Mupen64Support.c...
gcc %CFLAGS%                -o %OBJDIR%/Mupen64Support.o            -c %SRCDIR%/Mupen64plusHLE/Mupen64Support.c
ECHO     Mupen64plusHLE/memory.c...
gcc %CFLAGS%                -o %OBJDIR%/memory.o                    -c %SRCDIR%/Mupen64plusHLE/memory.c
ECHO     Mupen64plusHLE/audio.c...
gcc %CFLAGS%                -o %OBJDIR%/audio.o                     -c %SRCDIR%/Mupen64plusHLE/audio.c
ECHO Compiling resources...
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

ECHO Linking...
ECHO     %XA_PLUGIN_FILE%
g++ -shared %CFLAGS% -o %obj%\%XA_PLUGIN_FILE% %XA_OBJS% %COMMON_OBJS% %LDFLAGS% -lole32
ECHO     %DS_PLUGIN_FILE%
g++ -shared %CFLAGS% -o %obj%\%DS_PLUGIN_FILE% %DS_OBJS% %COMMON_OBJS% %LDFLAGS% -ldsound

GOTO done

:getargs
ECHO.%~1 | findstr /C:"--" 1>nul

 REM Next we check the errorlevel return to see if it contains a key or a value
 REM and set the appropriate action.

IF NOT errorlevel 1 (
  SET KEY=%~1
  SET %~1_PASSED=TRUE
) ELSE (
  SET VALUE=%~1
)
IF DEFINED VALUE (
    SET %KEY%=%~1
     REM It's important to clear the definitions for the the key and value in order to
     REM search for the next key value set.
    SET KEY=
    SET VALUE=
)
GOTO:EOF

:argtopath
SET %~2=%~f1
GOTO:EOF

:existsinpath
CALL:existsinpath2 %~1 %~1.exe
IF errorlevel 404 (
    exit /b 404
) ELSE (
    ECHO ³   %~1 found!
)
GOTO:EOF

:existsinpath2
IF "%~$PATH:1" == "" ( IF "%~$PATH:2" == "" (
	ECHO ³ FATAL:    Could not find required tool %~1.
    ECHO ³           Did you supply the correct --mingwpath?
    ECHO À Stopping.
	exit /b 404
) )
GOTO:EOF

:detectarch
 REM We get target triplicate from GCC and use that to try and figure out the
 REM target architecture.
FOR /F "delims=" %%i IN ('gcc -dumpmachine') DO set GCC_TRIP=%%i
FOR /F "delims=-" %%i IN ('gcc -dumpmachine') DO set ARCH_GCC=%%i
IF "%ARCH_GCC%" == "x86_64" (
	SET ARCH_DETECTED=x86_64
) ELSE (
	SET ARCH_DETECTED=x86
)
GOTO:EOF

:mkdir
IF NOT EXIST "%~f1" (
    mkdir "%~f1"
)
GOTO:EOF

:done
endlocal
