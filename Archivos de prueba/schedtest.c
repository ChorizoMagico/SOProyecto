// ============================================================================
// PROGRAMA DE PRUEBA PARA MLFQ EN XV6
// ============================================================================
// Este programa crea tres tipos de procesos para demostrar las ventajas
// del scheduler MLFQ sobre el Round-Robin tradicional:
//
// 1. PROCESO CPU-BOUND: Hace cálculos intensivos (loops)
// 2. PROCESO I/O-BOUND: Hace operaciones de I/O frecuentes (sleep)
// 3. PROCESO MIXTO: Combina CPU y I/O
//
// MÉTRICAS:
// - Tiempo de inicio: Cuándo el proceso comienza
// - Tiempo de fin: Cuándo el proceso termina
// - Tiempo total: Duración completa de ejecución
//
// CÓMO USARLO:
// 1. Compilar: gcc -o schedtest schedtest.c
// 2. Agregar a xv6 Makefile en la sección UPROGS
// 3. Ejecutar en xv6: $ schedtest
// 4. Comparar resultados entre Round-Robin y MLFQ
// ============================================================================

#include "types.h"
#include "stat.h"
#include "user.h"

// Número de iteraciones para cada tipo de proceso
#define CPU_ITERATIONS 100000000   // Para proceso CPU-bound
#define IO_ITERATIONS 50            // Para proceso I/O-bound
#define MIXED_CPU_ITER 10000000     // CPU iterations en proceso mixto
#define MIXED_IO_ITER 10            // I/O iterations en proceso mixto

// ============================================================================
// FUNCIONES AUXILIARES
// ============================================================================

// Obtener el tiempo actual (en ticks del sistema)
int
gettime(void)
{
  return uptime();
}

// Realizar cálculo intensivo de CPU (factorial aproximado)
void
cpu_work(int iterations)
{
  int i, j;
  volatile int result = 1;  // volatile para evitar optimización del compilador
  
  for(i = 0; i < iterations; i++){
    for(j = 1; j < 10; j++){
      result = result * j;
    }
    result = 1;  // Reset para evitar overflow
  }
}

// ============================================================================
// TIPOS DE PROCESOS
// ============================================================================

// PROCESO CPU-BOUND: Consume mucho CPU, casi sin I/O
// En MLFQ: Debería ser degradado a cola baja rápidamente
// En Round-Robin: Compite igualmente con todos los procesos
void
cpu_bound_process(int id)
{
  int start_time, end_time;
  
  start_time = gettime();
  printf(1, "[CPU-%d] Iniciado en tick %d\n", id, start_time);
  
  // Trabajo intensivo de CPU
  cpu_work(CPU_ITERATIONS);
  
  end_time = gettime();
  printf(1, "[CPU-%d] Finalizado en tick %d (duración: %d ticks)\n", 
         id, end_time, end_time - start_time);
}

// PROCESO I/O-BOUND: Hace operaciones de I/O frecuentes
// En MLFQ: Debería mantenerse en cola alta (respuesta rápida)
// En Round-Robin: Puede ser bloqueado por procesos CPU-bound
void
io_bound_process(int id)
{
  int start_time, end_time, i;
  
  start_time = gettime();
  printf(1, "[I/O-%d] Iniciado en tick %d\n", id, start_time);
  
  // Simular I/O: alternar entre sleep y trabajo ligero
  for(i = 0; i < IO_ITERATIONS; i++){
    sleep(1);  // Simula espera de I/O
    cpu_work(100000);  // Trabajo ligero al recibir datos
  }
  
  end_time = gettime();
  printf(1, "[I/O-%d] Finalizado en tick %d (duración: %d ticks)\n", 
         id, end_time, end_time - start_time);
}

// PROCESO MIXTO: Combina CPU y I/O
// En MLFQ: Puede ser degradado y subir de cola según comportamiento
// En Round-Robin: Comportamiento intermedio
void
mixed_process(int id)
{
  int start_time, end_time, i;
  
  start_time = gettime();
  printf(1, "[MIX-%d] Iniciado en tick %d\n", id, start_time);
  
  // Alternar entre trabajo CPU e I/O
  for(i = 0; i < MIXED_IO_ITER; i++){
    cpu_work(MIXED_CPU_ITER);  // Ráfaga de CPU
    sleep(1);                   // Pequeña espera
  }
  
  end_time = gettime();
  printf(1, "[MIX-%d] Finalizado en tick %d (duración: %d ticks)\n", 
         id, end_time, end_time - start_time);
}

// ============================================================================
// FUNCIÓN PRINCIPAL: COORDINA LA PRUEBA
// ============================================================================

int
main(int argc, char *argv[])
{
  int test_start, test_end;
  int pid;
  int i;
  
  printf(1, "\n");
  printf(1, "========================================\n");
  printf(1, "PRUEBA DE SCHEDULER - xv6\n");
  printf(1, "========================================\n");
  printf(1, "Esta prueba crea multiples procesos:\n");
  printf(1, "- 2 procesos CPU-bound (intensivos)\n");
  printf(1, "- 3 procesos I/O-bound (interactivos)\n");
  printf(1, "- 2 procesos mixtos\n");
  printf(1, "\n");
  printf(1, "EXPECTATIVAS:\n");
  printf(1, "Round-Robin: Todos completan ~mismo tiempo\n");
  printf(1, "MLFQ: I/O-bound completan MUCHO mas rapido\n");
  printf(1, "========================================\n");
  printf(1, "\n");
  
  test_start = gettime();
  printf(1, "Iniciando prueba en tick %d\n\n", test_start);
  
  // ========================================================================
  // CREAR PROCESOS CPU-BOUND (2 procesos)
  // ========================================================================
  // Estos procesos deberían ser degradados a cola baja en MLFQ
  for(i = 0; i < 2; i++){
    pid = fork();
    if(pid < 0){
      printf(1, "Error: fork fallo\n");
      exit();
    }
    if(pid == 0){
      // Proceso hijo: ejecutar CPU-bound
      cpu_bound_process(i + 1);
      exit();
    }
    // Proceso padre: continúa creando más procesos
  }
  
  // ========================================================================
  // CREAR PROCESOS I/O-BOUND (3 procesos)
  // ========================================================================
  // Estos procesos deberían mantenerse en cola alta en MLFQ
  // y completar mucho más rápido que en Round-Robin
  for(i = 0; i < 3; i++){
    pid = fork();
    if(pid < 0){
      printf(1, "Error: fork fallo\n");
      exit();
    }
    if(pid == 0){
      // Proceso hijo: ejecutar I/O-bound
      io_bound_process(i + 1);
      exit();
    }
    // Proceso padre: continúa creando más procesos
  }
  
  // ========================================================================
  // CREAR PROCESOS MIXTOS (2 procesos)
  // ========================================================================
  // Comportamiento intermedio: pueden cambiar de cola
  for(i = 0; i < 2; i++){
    pid = fork();
    if(pid < 0){
      printf(1, "Error: fork fallo\n");
      exit();
    }
    if(pid == 0){
      // Proceso hijo: ejecutar mixto
      mixed_process(i + 1);
      exit();
    }
    // Proceso padre: continúa
  }
  
  // ========================================================================
  // ESPERAR A QUE TODOS LOS PROCESOS TERMINEN
  // ========================================================================
  printf(1, "\nEsperando a que todos los procesos terminen...\n\n");
  
  // Esperar por los 7 procesos (2 CPU + 3 I/O + 2 MIX)
  for(i = 0; i < 7; i++){
    wait();
  }
  
  test_end = gettime();
  
  // ========================================================================
  // MOSTRAR RESULTADOS FINALES
  // ========================================================================
  printf(1, "\n");
  printf(1, "========================================\n");
  printf(1, "RESULTADOS DE LA PRUEBA\n");
  printf(1, "========================================\n");
  printf(1, "Tiempo total de prueba: %d ticks\n", test_end - test_start);
  printf(1, "\n");
  printf(1, "INTERPRETACION:\n");
  printf(1, "----------------------------------------\n");
  printf(1, "Round-Robin:\n");
  printf(1, "  - Todos los procesos tardan similar\n");
  printf(1, "  - I/O-bound sufren latencia alta\n");
  printf(1, "  - Tiempo total: MAYOR\n");
  printf(1, "\n");
  printf(1, "MLFQ (2 colas):\n");
  printf(1, "  - I/O-bound completan RAPIDO\n");
  printf(1, "  - CPU-bound completan despues\n");
  printf(1, "  - Tiempo total: MENOR\n");
  printf(1, "  - Mejor respuesta interactiva\n");
  printf(1, "========================================\n");
  printf(1, "\n");
  
  exit();
}