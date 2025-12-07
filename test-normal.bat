@echo off
echo ========================================
echo PRUEBA 1: xv6 NORMAL (Round-Robin)
echo ========================================
echo.
echo Presiona Ctrl+A y luego X para salir de xv6
echo Luego escribe 'exit' para salir del contenedor
echo.
pause

docker run -it --rm -v G:\Documents\SOProyecto\xv6-public:/xv6 xv6-test bash -c "cd /xv6 && chmod +x sign.pl && sed -i 's|./sign.pl|perl sign.pl|g' Makefile && make clean && make && echo '=== Compilacion exitosa ===' && echo 'Iniciando xv6...' && echo 'Ejecuta: schedtest' && make qemu-nox"