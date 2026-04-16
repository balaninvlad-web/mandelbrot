@echo off
REM ===============^^^^ отключаю вывод в командную строку===
REM запуск теста производительности высокий приоритет привязка ко 2 ядру
REM ========================================================

set AFFINITY_MASK = 4  
REM привязка ко 2 ядру

start "" /b /wait /high /affinity %AFFINITY_MASK% perf_quad_o2.exe data/Quad_after_two_fixes/res_quad_o2.csv

pause