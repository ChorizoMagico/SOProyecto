@echo off
REM ============================================================================
REM Comparacion de algoritmos de memoria: Original vs Best Fit BST
REM Ejecuta ambas pruebas consecutivamente
REM ============================================================================

setlocal enabledelayedexpansion

echo ================================================
echo   COMPARACION DE ALGORITMOS DE MEMORIA
echo ================================================
echo.
echo Este script ejecutara:
echo   1. Prueba con algoritmo ORIGINAL (Freelist)
echo   2. Prueba con algoritmo BEST FIT (BST)
echo.
echo Debes ejecutar 'memtest' en cada una y anotar los resultados.
echo.
echo ================================================
pause

REM Obtener timestamp para los archivos
for /f "tokens=2-4 delims=/ " %%a in ('date /t') do (set fecha=%%c-%%a-%%b)
for /f "tokens=1-2 delims=: " %%a in ('time /t') do (set hora=%%a-%%b)
set timestamp=%fecha%_%hora%

echo.
echo Los resultados se guardaran en: resultados\memoria\
echo Archivo sugerido: comparacion_%timestamp%.txt
echo.

REM ============================================================================
echo.
echo ================================================
echo   PARTE 1/2: Algoritmo ORIGINAL (Freelist)
echo ================================================
echo.
echo Sistema:   xv6-public
echo Memoria:   Freelist First Fit
echo.
echo IMPORTANTE:
echo   1. Ejecuta: memtest
echo   2. ANOTA todos los tiempos mostrados
echo   3. Sal con: Ctrl+A luego X
echo.
pause

docker run -it --rm -v "%cd%\xv6-public:/xv6" xv6-test bash -c "cd /xv6 && chmod +x sign.pl && sed -i 's|./sign.pl|perl sign.pl|g' Makefile && make clean && make && echo '' && echo '========================================' && echo '   ALGORITMO ORIGINAL (FREELIST)' && echo '========================================' && echo '' && echo 'EJECUTA: memtest' && echo '' && make qemu-nox"

echo.
echo [OK] Prueba con algoritmo original completada
echo.
timeout /t 3 /nobreak >nul

REM ============================================================================
echo.
echo ================================================
echo   PARTE 2/2: Algoritmo MODIFICADO (Best Fit BST)
echo ================================================
echo.
echo Sistema:   xv6-public-modificado
echo Memoria:   Best Fit con BST
echo.
echo IMPORTANTE:
echo   1. Ejecuta: memtest
echo   2. ANOTA todos los tiempos mostrados
echo   3. Sal con: Ctrl+A luego X
echo.
pause

docker run -it --rm -v "%cd%\xv6-public-modificado:/xv6" xv6-test bash -c "cd /xv6 && chmod +x sign.pl && sed -i 's|./sign.pl|perl sign.pl|g' Makefile && make clean && make && echo '' && echo '========================================' && echo '   ALGORITMO BEST FIT (BST)' && echo '========================================' && echo '' && echo 'EJECUTA: memtest' && echo '' && make qemu-nox"

echo.
echo [OK] Prueba con algoritmo modificado completada
echo.

REM ============================================================================
echo.
echo ================================================
echo   COMPARACION COMPLETADA
echo ================================================
echo.
echo Debes haber anotado los resultados de ambas pruebas.
echo.
echo METRICAS CLAVE A COMPARAR:
echo ================================================
echo.
echo PRUEBA 1 - Asignacion Secuencial:
echo   - Tiempo de asignacion (ticks)
echo   - Tiempo de liberacion (ticks)
echo   - Tiempo total (ticks)
echo.
echo PRUEBA 2 - Patron Alternado (Fragmentacion):
echo   - Tiempo total (ticks)
echo   - Bloques reasignados exitosamente
echo.
echo PRUEBA 3 - Estres:
echo   - Tiempo promedio por iteracion (ticks) ***CLAVE***
echo   - Asignaciones fallidas
echo.
echo PRUEBA 4 - Multiples Procesos:
echo   - Tiempo total (ticks)
echo.
echo ================================================
echo   ANALISIS ESPERADO
echo ================================================
echo.
echo ORIGINAL (Freelist First Fit):
echo   + Mas rapido en asignaciones simples
echo   - Alta fragmentacion externa
echo   - Se degrada con fragmentacion
echo   - Busqueda O(n)
echo.
echo MODIFICADO (Best Fit BST):
echo   + Reduce fragmentacion externa
echo   + Busqueda O(log n)
echo   + Mejor con memoria fragmentada
echo   + Coalescencia automatica
echo   - Overhead del arbol binario
echo.
echo ================================================
echo.
echo Crea una tabla comparativa con tus resultados.
echo Guarda en: resultados\memoria\comparacion_%timestamp%.txt
echo.
echo ================================================
pause