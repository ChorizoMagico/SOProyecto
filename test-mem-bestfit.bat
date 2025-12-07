@echo off
REM ============================================================================
REM Prueba de memoria con algoritmo MODIFICADO (Best Fit BST)
REM Usa: xv6-public-modificado
REM ============================================================================

echo ========================================
echo PRUEBA DE MEMORIA: BEST FIT BST
echo ========================================
echo.
echo Sistema:    xv6-public-modificado
echo Scheduler:  MLFQ (modificado)
echo Memoria:    Best Fit con BST (MODIFICADO)
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

docker run -it --rm -v "%cd%\xv6-public-modificado:/xv6" xv6-test bash -c "cd /xv6 && chmod +x sign.pl && sed -i 's|./sign.pl|perl sign.pl|g' Makefile && make clean && make && echo '' && echo '========================================' && echo '   COMPILACION EXITOSA - BEST FIT BST' && echo '========================================' && echo '' && echo 'xv6 iniciando...' && echo '' && echo 'EJECUTA: memtest' && echo '' && make qemu-nox"

echo.
echo ========================================
echo Prueba completada
echo ========================================
echo.
echo Guarda tus resultados en: resultados\memoria\bestfit.txt
echo.
pause