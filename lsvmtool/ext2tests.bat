@echo off
setlocal enabledelayedexpansion

nmake
set "target=ext2.exe --ext2fs \\.\Harddisk1Partition2"
set "testdir=ext2fs"
for /f %%i in ('echo %cd%') do set pwd=%%i
echo.

echo Testing lsr
copy nul lsrresult.txt > nul
%target% lsr / |  sort > lsrtmp.txt
for /f %%i in  ('type lsrtmp.txt') do (
	set t=%%i
	set t=!t:/=\!
	echo %pwd%\%testdir%!t! >> lsrresult.txt
)
del lsrtmp.txt

dir /s /b %testdir% | sort > dirresult.txt
fc /w dirresult.txt lsrresult.txt
del lsrresult.txt
del dirresult.txt
echo.

echo Testing get
%target% get /initrd.img-3.13.0-24-generic initrdcopy
fc /b initrdcopy %testdir%\initrd.img-3.13.0-24-generic
echo.

echo Testing mkdir
%target% mkdir /newdir
mkdir %testdir%\newdir
%target% ls / | findstr newdir
echo.

echo Testing trunc
%target% check
%target% trunc /initrd.img-3.13.0-24-generic
%target% get /initrd.img-3.13.0-24-generic initrdempty
copy nul trunctest > nul
fc /b trunctest initrdempty
del trunctest
del initrdempty
echo.

echo Testing update
%target% update initrdcopy /initrd.img-3.13.0-24-generic
%target% get /initrd.img-3.13.0-24-generic initrdcopy1
fc initrdcopy initrdcopy1
del initrdcopy1
del initrdcopy
echo.

echo Testing put
%target% put main.c /main.c
%target% put main.c /newdir/main.c
%target% get /main.c m1.c
%target% get /newdir/main.c m2.c
fc main.c m1.c
fc main.c m2.c
del m1.c
del m2.c
echo.

echo Testing rm
%target% rm /main.c
%target% rm /newdir/main.c
echo.

echo Final check
%target% check
echo.

echo All tests complete
