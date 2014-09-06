@echo off
set hour=%time:~0,2%
if %hour% lss 10 (set hour=0%time:~1,1%)
echo #define BUILD_TIMESTAMP "%1/%date:~-4%%date:~4,2%%date:~7,2%%hour%%time:~3,2%%time:~6,2%" > "%2\timestamp.h"
