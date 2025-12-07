@echo off
REM ============================================================================
REM Prueba de memoria con algoritmo ORIGINAL (Freelist First Fit)
REM Usa: xv6-public
REM ============================================================================

echo ========================================
echo PRUEBA DE MEMORIA: ORIGINAL (Freelist)
echo ========================================
echo.
echo Sistema:    xv6-public
echo Scheduler:  Round-Robin (original)
echo Memoria:    Freelist First Fit (original)
echo Programa:   memtest
echo.
echo ========================================
echo INSTRUCCIONES:
echo ========================================
echo 1. Espera a que xv6 inicie
echo 2. En el prompt ($), escribe: memtest
echo 3. Observa los resultados de las 4 pruebas
echo 4. ANOTA o CAPTURA los tiempos mostrados
echo 5. Presiona Ctrl+A y luego X para salir
echo 6. Escribe 'exit' para salir del contenedor
echo ========================================
echo.
pause

docker run -it --rm -v "%cd%\xv6-public:/xv6" xv6-test bash -c "cd /xv6 && chmod +x sign.pl && sed -i 's|./sign.pl|perl sign.pl|g' Makefile && make clean && make && echo '' && echo '========================================' && echo '   COMPILACION EXITOSA - ALGORITMO ORIGINAL' && echo '========================================' && echo '' && echo 'xv6 iniciando...' && echo '' && echo 'EJECUTA: memtest' && echo '' && make qemu-nox"

echo.
echo ========================================
echo Prueba completada
echo ========================================
echo.
echo Guarda tus resultados en: resultados\memoria\original.txt
echo.
pause