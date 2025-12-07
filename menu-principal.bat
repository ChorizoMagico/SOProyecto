@echo off
chcp 65001 >nul
:menu
cls
echo ╔════════════════════════════════════════════════════════╗
echo ║           PROYECTO xv6 - SISTEMAS OPERATIVOS          ║
echo ╚════════════════════════════════════════════════════════╝
echo.
echo ┌────────────────────────────────────────────────────────┐
echo │  MODO INTERACTIVO (Explorar xv6)                      │
echo └────────────────────────────────────────────────────────┘
echo   1. Iniciar xv6 NORMAL (Round-Robin)
echo   2. Iniciar xv6 MODIFICADO (MLFQ)
echo.
echo ┌────────────────────────────────────────────────────────┐
echo │  MODO PRUEBAS (Ejecutar schedtest automático)         │
echo └────────────────────────────────────────────────────────┘
echo   3. Prueba xv6 NORMAL (ejecuta schedtest)
echo   4. Prueba xv6 MODIFICADO (ejecuta schedtest)
echo.
echo ┌────────────────────────────────────────────────────────┐
echo │  UTILIDADES                                            │
echo └────────────────────────────────────────────────────────┘
echo   5. Solo compilar xv6 NORMAL
echo   6. Solo compilar xv6 MODIFICADO
echo   7. Ver diferencias entre versiones
echo   8. Limpiar compilaciones
echo.
echo   0. Salir
echo.
echo ════════════════════════════════════════════════════════
set /p opcion="Selecciona una opcion: "

if "%opcion%"=="1" goto normal_interactive
if "%opcion%"=="2" goto mlfq_interactive
if "%opcion%"=="3" goto normal_test
if "%opcion%"=="4" goto mlfq_test
if "%opcion%"=="5" goto compile_normal
if "%opcion%"=="6" goto compile_mlfq
if "%opcion%"=="7" goto show_diff
if "%opcion%"=="8" goto clean_all
if "%opcion%"=="0" goto end
echo Opcion invalida. Presiona cualquier tecla...
pause >nul
goto menu

:normal_interactive
cls
echo ════════════════════════════════════════════════════════
echo  MODO INTERACTIVO - xv6 NORMAL (Round-Robin)
echo ════════════════════════════════════════════════════════
echo.
echo Compilando y iniciando xv6...
echo.
echo Dentro de xv6 puedes:
echo   - ls              Ver archivos
echo   - cat archivo     Leer archivo
echo   - schedtest       Ejecutar prueba de scheduler
echo   - cualquier otro programa de usuario
echo.
echo Para salir: Ctrl+A, luego X
echo ════════════════════════════════════════════════════════
echo.
pause
docker run --rm -it -v G:\Documents\SOProyecto\xv6-public:/xv6 xv6-test bash -c "cd /xv6 && chmod +x sign.pl && sed -i 's|./sign.pl|perl sign.pl|g' Makefile && make clean 2>&1 | grep -v 'rm -f' && make 2>&1 | tail -5 && echo '' && echo '✓ Compilacion exitosa' && echo '' && make qemu-nox"
pause
goto menu

:mlfq_interactive
cls
echo ════════════════════════════════════════════════════════
echo  MODO INTERACTIVO - xv6 MODIFICADO (MLFQ)
echo ════════════════════════════════════════════════════════
echo.
echo Compilando y iniciando xv6...
echo.
echo Dentro de xv6 puedes:
echo   - ls              Ver archivos
echo   - cat archivo     Leer archivo
echo   - schedtest       Ejecutar prueba de scheduler
echo   - cualquier otro programa de usuario
echo.
echo Para salir: Ctrl+A, luego X
echo ════════════════════════════════════════════════════════
echo.
pause
docker run --rm -it -v G:\Documents\SOProyecto\xv6-public-modificado:/xv6 xv6-test bash -c "cd /xv6 && chmod +x sign.pl && sed -i 's|./sign.pl|perl sign.pl|g' Makefile && make clean 2>&1 | grep -v 'rm -f' && make 2>&1 | tail -5 && echo '' && echo '✓ Compilacion exitosa' && echo '' && make qemu-nox"
pause
goto menu

:normal_test
cls
echo ════════════════════════════════════════════════════════
echo  PRUEBA AUTOMATICA - xv6 NORMAL (Round-Robin)
echo ════════════════════════════════════════════════════════
echo.
echo Esta prueba:
echo  1. Compila xv6
echo  2. Inicia el sistema
echo  3. DEBES ejecutar manualmente: schedtest
echo  4. Captura los resultados para tu reporte
echo.
echo ════════════════════════════════════════════════════════
pause
docker run --rm -it -v G:\Documents\SOProyecto\xv6-public:/xv6 xv6-test bash -c "cd /xv6 && chmod +x sign.pl && sed -i 's|./sign.pl|perl sign.pl|g' Makefile && make clean 2>&1 | grep -v 'rm -f' && make 2>&1 | tail -5 && echo '' && echo '✓ Compilacion exitosa - Ejecuta: schedtest' && echo '' && make qemu-nox"
pause
goto menu

:mlfq_test
cls
echo ════════════════════════════════════════════════════════
echo  PRUEBA AUTOMATICA - xv6 MODIFICADO (MLFQ)
echo ════════════════════════════════════════════════════════
echo.
echo Esta prueba:
echo  1. Compila xv6
echo  2. Inicia el sistema
echo  3. DEBES ejecutar manualmente: schedtest
echo  4. Captura los resultados para tu reporte
echo.
echo ════════════════════════════════════════════════════════
pause
docker run --rm -it -v G:\Documents\SOProyecto\xv6-public-modificado:/xv6 xv6-test bash -c "cd /xv6 && chmod +x sign.pl && sed -i 's|./sign.pl|perl sign.pl|g' Makefile && make clean 2>&1 | grep -v 'rm -f' && make 2>&1 | tail -5 && echo '' && echo '✓ Compilacion exitosa - Ejecuta: schedtest' && echo '' && make qemu-nox"
pause
goto menu

:compile_normal
cls
echo ════════════════════════════════════════════════════════
echo  COMPILANDO xv6 NORMAL
echo ════════════════════════════════════════════════════════
echo.
docker run --rm -v G:\Documents\SOProyecto\xv6-public:/xv6 xv6-test bash -c "cd /xv6 && chmod +x sign.pl && sed -i 's|./sign.pl|perl sign.pl|g' Makefile && make clean && make"
echo.
echo ════════════════════════════════════════════════════════
echo  ✓ Compilacion completada
echo ════════════════════════════════════════════════════════
pause
goto menu

:compile_mlfq
cls
echo ════════════════════════════════════════════════════════
echo  COMPILANDO xv6 MODIFICADO (MLFQ)
echo ════════════════════════════════════════════════════════
echo.
docker run --rm -v G:\Documents\SOProyecto\xv6-public-modificado:/xv6 xv6-test bash -c "cd /xv6 && chmod +x sign.pl && sed -i 's|./sign.pl|perl sign.pl|g' Makefile && make clean && make"
echo.
echo ════════════════════════════════════════════════════════
echo  ✓ Compilacion completada
echo ════════════════════════════════════════════════════════
pause
goto menu

:show_diff
cls
echo ════════════════════════════════════════════════════════
echo  DIFERENCIAS ENTRE VERSIONES
echo ════════════════════════════════════════════════════════
echo.
echo ARCHIVOS MODIFICADOS EN MLFQ:
echo ────────────────────────────────────────────────────────
echo.
echo 1. proc.h
echo    └─ Agregados campos:
echo       • int priority      (0=alta, 1=baja)
echo       • int ticks_used    (contador de ticks)
echo.
echo 2. proc.c
echo    └─ Modificada funcion scheduler():
echo       • Busca primero en cola alta (priority==0)
echo       • Si vacia, busca en cola baja (priority==1)
echo       • Inicializa procesos nuevos en cola alta
echo.
echo 3. trap.c
echo    └─ Agregada logica de degradacion:
echo       • Cuenta ticks usados por proceso
echo       • Degrada a cola baja tras 5 ticks
echo       • Fuerza yield() al degradar
echo.
echo 4. schedtest.c
echo    └─ Programa de prueba (NUEVO):
echo       • 2 procesos CPU-bound
echo       • 3 procesos I/O-bound
echo       • 2 procesos mixtos
echo.
echo ════════════════════════════════════════════════════════
pause
goto menu

:clean_all
cls
echo ════════════════════════════════════════════════════════
echo  LIMPIANDO COMPILACIONES
echo ════════════════════════════════════════════════════════
echo.
echo Limpiando xv6 NORMAL...
docker run --rm -v G:\Documents\SOProyecto\xv6-public:/xv6 xv6-test bash -c "cd /xv6 && make clean"
echo.
echo Limpiando xv6 MODIFICADO...
docker run --rm -v G:\Documents\SOProyecto\xv6-public-modificado:/xv6 xv6-test bash -c "cd /xv6 && make clean"
echo.
echo ════════════════════════════════════════════════════════
echo  ✓ Limpieza completada
echo ════════════════════════════════════════════════════════
pause
goto menu

:end
cls
echo.
echo Gracias por usar el sistema de pruebas xv6!
echo.
timeout /t 2 >nul
exit