@echo off
REM ============================================================================
REM Script de configuracion para pruebas de memoria en xv6
REM Prepara ambas versiones (original y modificada) para pruebas de memoria
REM ============================================================================

echo ================================================
echo   CONFIGURACION - Pruebas de Memoria xv6
echo ================================================
echo.

REM Verificar que estamos en el directorio correcto
if not exist "xv6-public" (
    echo [ERROR] No se encuentra la carpeta xv6-public
    echo Asegurate de ejecutar este script desde SOproyecto-main
    pause
    exit /b 1
)

if not exist "xv6-public-modificado" (
    echo [ERROR] No se encuentra la carpeta xv6-public-modificado
    echo Asegurate de ejecutar este script desde SOproyecto-main
    pause
    exit /b 1
)

echo [OK] Carpetas de xv6 encontradas
echo.

REM ============================================================================
echo [PASO 1/6] Verificando archivos necesarios...
echo ============================================================================
echo.

set archivos_faltantes=0

REM Verificar memtest.c
if not exist "archivos de prueba\memtest.c" (
    echo [FALTA] archivos de prueba\memtest.c
    echo Debes crear este archivo con el programa de pruebas
    set archivos_faltantes=1
) else (
    echo [OK] memtest.c encontrado
)

REM Verificar kalloc_bestfit.c (tu implementacion)
if not exist "archivos de prueba\kalloc_bestfit.c" (
    echo [ADVERTENCIA] archivos de prueba\kalloc_bestfit.c no encontrado
    echo Este debe ser tu implementacion Best Fit con BST
    echo.
    echo Opciones:
    echo   1. Crear kalloc_bestfit.c con tu implementacion
    echo   2. Copiar desde xv6-public-modificado/kalloc.c si ya lo modificaste
    echo.
) else (
    echo [OK] kalloc_bestfit.c encontrado
)

echo.

if %archivos_faltantes% equ 1 (
    echo [ERROR] Faltan archivos necesarios
    echo Por favor crea los archivos indicados arriba
    pause
    exit /b 1
)

REM ============================================================================
echo [PASO 2/6] Creando respaldo del kalloc.c original...
echo ============================================================================
echo.

if not exist "archivos de prueba\kalloc_original_backup.c" (
    if exist "xv6-public\kalloc.c" (
        copy "xv6-public\kalloc.c" "archivos de prueba\kalloc_original_backup.c" >nul
        echo [OK] Respaldo creado: kalloc_original_backup.c
    ) else (
        echo [ERROR] No se encuentra xv6-public\kalloc.c
        pause
        exit /b 1
    )
) else (
    echo [OK] Respaldo ya existe: kalloc_original_backup.c
)

echo.

REM ============================================================================
echo [PASO 3/6] Copiando memtest.c a ambas versiones de xv6...
echo ============================================================================
echo.

REM Copiar a xv6-public (original)
copy "archivos de prueba\memtest.c" "xv6-public\memtest.c" >nul
if %errorlevel% equ 0 (
    echo [OK] memtest.c copiado a xv6-public
) else (
    echo [ERROR] No se pudo copiar memtest.c a xv6-public
)

REM Copiar a xv6-public-modificado
copy "archivos de prueba\memtest.c" "xv6-public-modificado\memtest.c" >nul
if %errorlevel% equ 0 (
    echo [OK] memtest.c copiado a xv6-public-modificado
) else (
    echo [ERROR] No se pudo copiar memtest.c a xv6-public-modificado
)

echo.

REM ============================================================================
echo [PASO 4/6] Actualizando kalloc.c en xv6-public-modificado...
echo ============================================================================
echo.

if exist "archivos de prueba\kalloc_bestfit.c" (
    echo Reemplazando kalloc.c en xv6-public-modificado con Best Fit BST...
    copy /Y "archivos de prueba\kalloc_bestfit.c" "xv6-public-modificado\kalloc.c" >nul
    echo [OK] kalloc.c actualizado con Best Fit BST
) else (
    echo [ADVERTENCIA] No se encontro kalloc_bestfit.c
    echo xv6-public-modificado/kalloc.c NO fue actualizado
    echo Debes hacerlo manualmente antes de ejecutar las pruebas
)

echo.

REM ============================================================================
echo [PASO 5/6] Verificando Makefiles...
echo ============================================================================
echo.

REM Verificar Makefile de xv6-public
findstr /C:"_memtest" "xv6-public\Makefile" >nul
if %errorlevel% neq 0 (
    echo [ACCION REQUERIDA] Debes agregar memtest al Makefile de xv6-public
    echo.
    echo Abre: xv6-public\Makefile
    echo Busca la seccion UPROGS y agrega:
    echo    _memtest\
    echo.
    echo Ejemplo:
    echo    UPROGS=\
    echo        _cat\
    echo        _echo\
    echo        ...
    echo        _memtest\     ^<-- AGREGAR ESTA LINEA
    echo        _zombie\
    echo.
) else (
    echo [OK] _memtest ya esta en xv6-public\Makefile
)

REM Verificar Makefile de xv6-public-modificado
findstr /C:"_memtest" "xv6-public-modificado\Makefile" >nul
if %errorlevel% neq 0 (
    echo [ACCION REQUERIDA] Debes agregar memtest al Makefile de xv6-public-modificado
    echo.
    echo Abre: xv6-public-modificado\Makefile
    echo Busca la seccion UPROGS y agrega:
    echo    _memtest\
    echo.
    echo Ejemplo:
    echo    UPROGS=\
    echo        _cat\
    echo        _echo\
    echo        ...
    echo        _memtest\     ^<-- AGREGAR ESTA LINEA
    echo        _zombie\
    echo.
) else (
    echo [OK] _memtest ya esta en xv6-public-modificado\Makefile
)

echo.
echo IMPORTANTE: Si viste mensajes de [ACCION REQUERIDA] arriba,
echo edita los Makefiles antes de continuar.
echo.
pause

REM ============================================================================
echo [PASO 6/6] Creando directorios de resultados...
echo ============================================================================
echo.

if not exist "resultados" mkdir "resultados"
if not exist "resultados\memoria" mkdir "resultados\memoria"
if not exist "resultados\scheduler" mkdir "resultados\scheduler"

echo [OK] Directorios creados:
echo   - resultados\memoria\
echo   - resultados\scheduler\
echo.

REM ============================================================================
echo [PASO FINAL] Construyendo imagen de Docker...
echo ============================================================================
echo.
echo Este proceso puede tomar varios minutos...
pause

docker build -t xv6-test .

if %errorlevel% neq 0 (
    echo.
    echo [ERROR] La construccion de Docker fallo
    pause
    exit /b 1
)

echo.
echo [OK] Imagen de Docker construida
echo.

REM ============================================================================
echo ================================================
echo   CONFIGURACION COMPLETADA
echo ================================================
echo.
echo Tu proyecto esta listo para ejecutar pruebas de memoria.
echo.
echo ESTRUCTURA FINAL:
echo   xv6-public/           : Scheduler RR + Memoria Freelist (ORIGINAL)
echo   xv6-public-modificado/: Scheduler MLFQ + Memoria Best Fit (MODIFICADO)
echo.
echo SCRIPTS DISPONIBLES:
echo   Scheduler:
echo     - test-normal.bat      (RR en xv6-public)
echo     - test-mlfq.bat        (MLFQ en xv6-public-modificado)
echo.
echo   Memoria:
echo     - test-mem-normal.bat  (Freelist en xv6-public)
echo     - test-mem-bestfit.bat (Best Fit en xv6-public-modificado)
echo     - compare-memory.bat   (Compara ambos)
echo.
echo PROXIMOS PASOS:
echo   1. Verifica que _memtest\ este en ambos Makefiles (UPROGS)
echo   2. Ejecuta: test-mem-normal.bat
echo   3. Dentro de xv6, ejecuta: memtest
echo   4. Anota los resultados
echo   5. Ejecuta: test-mem-bestfit.bat
echo   6. Dentro de xv6, ejecuta: memtest
echo   7. Compara los resultados
echo.
echo ================================================
pause