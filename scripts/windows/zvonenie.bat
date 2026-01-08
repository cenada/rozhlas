@echo off
setlocal enabledelayedexpansion

rem ====== Configuration ======
set "START_TIMES=07:40 08:30 09:25 10:20 11:25 12:15 13:05 14:00"
set "END_TIMES=08:25 09:15 10:10 11:05 12:10 13:00 13:45 14:45"

rem Use the folder where this BAT lives as base (so it works from anywhere).
set "BASE=%~dp0"

echo.
echo Skolsky zvon bezi. Casy startu: %START_TIMES%
echo Casy konca:  %END_TIMES%
echo (Zavri okno pre ukoncenie.)
echo.

:LOOP
    call :GET_NOW
    rem Check start times
    for %%T in (%START_TIMES%) do (
        if "%%T"=="!NOW!" (
            echo [!NOW!] ZACIATOK hodiny -> prehravam...
            call :PLAY "zaciatok\z"
            goto NEXT_TICK
        )
    )

    rem Check end times
    for %%T in (%END_TIMES%) do (
        if "%%T"=="!NOW!" (
            echo [!NOW!] KONIEC hodiny -> prehravam...
            copy /y koniec\01.mp3 koniec\k
	    call :PLAY "koniec\k"
	    copy /y koniec\02.mp3 koniec\01.mp3
	    copy /y koniec\03.mp3 koniec\02.mp3
	    copy /y koniec\04.mp3 koniec\03.mp3
	    copy /y koniec\05.mp3 koniec\04.mp3
	    copy /y koniec\06.mp3 koniec\05.mp3
	    copy /y koniec\07.mp3 koniec\06.mp3
	    copy /y koniec\08.mp3 koniec\07.mp3
            goto NEXT_TICK
        )
    )

:NEXT_TICK
    rem Sleep 1 second between checks (quick reaction, low CPU)
    >nul timeout /t 1 /nobreak
goto LOOP


:GET_NOW
    rem Build HH:MM from %TIME% and pad hour if needed (leading space at 0..9h)
    set "HH=%time:~0,2%"
    if "!HH:~0,1!"==" " set "HH=0!HH:~1,1!"
    set "MM=%time:~3,2%"
    set "NOW=!HH!:!MM!"
    goto :eof

:PLAY
    rem %1 is "zaciatok" or "koniec"
    rozhlas com3 on
    >nul timeout /t 4 /nobreak
    pushd "%BASE%%~1"
    rem Call your player
    python ..\..\hraj.py
    popd
    rozhlas com3 off

    rem Prevent multiple plays within the same minute (wait slightly over 60s)
    >nul timeout /t 61 /nobreak
    goto :eof

