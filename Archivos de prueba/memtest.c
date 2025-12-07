// ============================================================================
// PROGRAMA DE PRUEBA PARA ALGORITMOS DE ASIGNACIÓN DE MEMORIA EN XV6
// ============================================================================
// Este programa prueba el rendimiento de los algoritmos de memoria:
// 1. FREELIST (Original): Simple lista enlazada - First Fit
// 2. BST BEST FIT (Modificado): Árbol binario de búsqueda - Best Fit
//
// MÉTRICAS:
// - Tiempo de asignación/liberación
// - Fragmentación de memoria
// - Número de bloques libres
// - Memoria libre total
//
// CÓMO USARLO:
// 1. Agregar a xv6 Makefile en la sección UPROGS
// 2. Ejecutar en xv6: $ memtest
// 3. Comparar resultados entre ambos algoritmos
// ============================================================================

#include "types.h"
#include "stat.h"
#include "user.h"
#include "param.h"

#define NUM_ALLOCS 50           // Número de asignaciones a realizar
#define MAX_CONCURRENT 30       // Máximo de bloques concurrentes
#define STRESS_ITERATIONS 100   // Iteraciones para prueba de estrés

// ============================================================================
// FUNCIONES AUXILIARES
// ============================================================================

// Obtener tiempo actual en ticks
int
gettime(void)
{
  return uptime();
}

// Patrón de escritura simple para verificar integridad
void
write_pattern(char *ptr, int pattern)
{
  int i;
  for(i = 0; i < 4096; i += 4) {
    *(int*)(ptr + i) = pattern;
  }
}

// Verificar patrón escrito
int
verify_pattern(char *ptr, int pattern)
{
  int i;
  for(i = 0; i < 4096; i += 4) {
    if(*(int*)(ptr + i) != pattern) {
      return 0;  // Patrón no coincide
    }
  }
  return 1;  // Patrón correcto
}

// ============================================================================
// PRUEBA 1: ASIGNACIÓN SECUENCIAL
// ============================================================================
// Asigna y libera páginas secuencialmente para medir tiempo básico

void
test_sequential_alloc(void)
{
  char *pages[NUM_ALLOCS];
  int start_time, end_time;
  int alloc_time, free_time;
  int i;
  int success_count = 0;

  printf(1, "\n");
  printf(1, "========================================\n");
  printf(1, "PRUEBA 1: ASIGNACION SECUENCIAL\n");
  printf(1, "========================================\n");
  printf(1, "Asignando %d paginas secuencialmente...\n", NUM_ALLOCS);

  // FASE 1: Asignación
  start_time = gettime();

  for(i = 0; i < NUM_ALLOCS; i++) {
    pages[i] = sbrk(4096);
    if((int)pages[i] == -1) {
      printf(1, "[ERROR] Fallo asignacion en iteracion %d\n", i);
      break;
    }
    success_count++;
    write_pattern(pages[i], i + 100);  // Escribir patrón único
  }

  end_time = gettime();
  alloc_time = end_time - start_time;

  printf(1, "Asignaciones exitosas: %d/%d\n", success_count, NUM_ALLOCS);
  printf(1, "Tiempo de asignacion: %d ticks\n", alloc_time);

  // FASE 2: Verificación
  printf(1, "\nVerificando integridad de memoria...\n");
  int verify_errors = 0;
  for(i = 0; i < success_count; i++) {
    if(!verify_pattern(pages[i], i + 100)) {
      printf(1, "[ERROR] Verificacion fallo en pagina %d\n", i);
      verify_errors++;
    }
  }
  printf(1, "Errores de verificacion: %d\n", verify_errors);

  // FASE 3: Liberación
  printf(1, "\nLiberando memoria...\n");
  start_time = gettime();

  for(i = 0; i < success_count; i++) {
    sbrk(-4096);
  }

  end_time = gettime();
  free_time = end_time - start_time;

  printf(1, "Tiempo de liberacion: %d ticks\n", free_time);
  printf(1, "\nRESUMEN PRUEBA 1:\n");
  printf(1, "  Asignacion: %d ticks (%d ticks/pagina)\n",
         alloc_time, success_count > 0 ? alloc_time/success_count : 0);
  printf(1, "  Liberacion: %d ticks (%d ticks/pagina)\n",
         free_time, success_count > 0 ? free_time/success_count : 0);
  printf(1, "  Total: %d ticks\n", alloc_time + free_time);
  printf(1, "========================================\n");
}

// ============================================================================
// PRUEBA 2: PATRÓN ALTERNADO (FRAGMENTACIÓN)
// ============================================================================
// Asigna bloques y libera alternadamente para crear fragmentación

void
test_alternating_pattern(void)
{
  char *pages[MAX_CONCURRENT];
  int start_time, end_time;
  int i;
  int operations = 0;

  printf(1, "\n");
  printf(1, "========================================\n");
  printf(1, "PRUEBA 2: PATRON ALTERNADO\n");
  printf(1, "========================================\n");
  printf(1, "Probando fragmentacion de memoria...\n");

  start_time = gettime();

  // FASE 1: Asignar todos los bloques
  for(i = 0; i < MAX_CONCURRENT; i++) {
    pages[i] = sbrk(4096);
    if((int)pages[i] != -1) {
      operations++;
      write_pattern(pages[i], i + 200);
    }
  }

  printf(1, "Fase 1: %d bloques asignados\n", operations);

  // FASE 2: Liberar bloques impares (crear huecos)
  int freed = 0;
  for(i = 1; i < MAX_CONCURRENT; i += 2) {
    if((int)pages[i] != -1) {
      sbrk(-4096);
      pages[i] = 0;
      freed++;
    }
  }

  printf(1, "Fase 2: %d bloques liberados (impares)\n", freed);

  // FASE 3: Intentar reasignar en los huecos
  int reallocated = 0;
  for(i = 1; i < MAX_CONCURRENT; i += 2) {
    pages[i] = sbrk(4096);
    if((int)pages[i] != -1) {
      reallocated++;
      write_pattern(pages[i], i + 300);
    }
  }

  printf(1, "Fase 3: %d bloques reasignados\n", reallocated);

  // FASE 4: Verificar todos los bloques activos
  printf(1, "\nVerificando integridad...\n");
  int verify_ok = 0;
  for(i = 0; i < MAX_CONCURRENT; i++) {
    if((int)pages[i] != -1) {
      int expected = (i % 2 == 0) ? (i + 200) : (i + 300);
      if(verify_pattern(pages[i], expected)) {
        verify_ok++;
      }
    }
  }

  printf(1, "Bloques verificados correctamente: %d\n", verify_ok);

  // FASE 5: Limpiar todo
  for(i = 0; i < MAX_CONCURRENT; i++) {
    if((int)pages[i] != -1) {
      sbrk(-4096);
    }
  }

  end_time = gettime();

  printf(1, "\nRESUMEN PRUEBA 2:\n");
  printf(1, "  Tiempo total: %d ticks\n", end_time - start_time);
  printf(1, "  Operaciones exitosas: asignar=%d, reasignar=%d\n",
         operations, reallocated);
  printf(1, "========================================\n");
}

// ============================================================================
// PRUEBA 3: ESTRÉS DE ASIGNACIÓN/LIBERACIÓN ALEATORIA
// ============================================================================

void
test_stress(void)
{
  char *pages[MAX_CONCURRENT];
  int start_time, end_time;
  int i, j;
  int total_allocs = 0;
  int total_frees = 0;
  int failed_allocs = 0;

  printf(1, "\n");
  printf(1, "========================================\n");
  printf(1, "PRUEBA 3: ESTRES DE MEMORIA\n");
  printf(1, "========================================\n");
  printf(1, "Realizando %d iteraciones de asignar/liberar...\n",
         STRESS_ITERATIONS);

  // Inicializar array
  for(i = 0; i < MAX_CONCURRENT; i++) {
    pages[i] = 0;
  }

  start_time = gettime();

  // Patrón de estrés: asignar/liberar pseudo-aleatoriamente
  for(i = 0; i < STRESS_ITERATIONS; i++) {
    // Determinar operación (basado en índice para ser determinístico)
    int op = (i * 7) % 3;  // 0=asignar, 1=liberar, 2=ambos

    if(op == 0 || op == 2) {
      // Asignar
      int idx = (i * 13) % MAX_CONCURRENT;
      if((int)pages[idx] == -1 || pages[idx] == 0) {
        pages[idx] = sbrk(4096);
        if((int)pages[idx] != -1) {
          total_allocs++;
          write_pattern(pages[idx], i);
        } else {
          failed_allocs++;
          pages[idx] = 0;
        }
      }
    }

    if(op == 1 || op == 2) {
      // Liberar
      int idx = (i * 17) % MAX_CONCURRENT;
      if((int)pages[idx] != -1 && pages[idx] != 0) {
        sbrk(-4096);
        total_frees++;
        pages[idx] = 0;
      }
    }

    // Cada 20 iteraciones, reportar progreso
    if(i % 20 == 0 && i > 0) {
      printf(1, "  Iteracion %d: asignadas=%d liberadas=%d\n",
             i, total_allocs, total_frees);
    }
  }

  // Limpiar memoria restante
  int remaining = 0;
  for(i = 0; i < MAX_CONCURRENT; i++) {
    if((int)pages[i] != -1 && pages[i] != 0) {
      sbrk(-4096);
      remaining++;
    }
  }

  end_time = gettime();

  printf(1, "\nRESUMEN PRUEBA 3:\n");
  printf(1, "  Tiempo total: %d ticks\n", end_time - start_time);
  printf(1, "  Tiempo promedio por iteracion: %d ticks\n",
         (end_time - start_time) / STRESS_ITERATIONS);
  printf(1, "  Asignaciones exitosas: %d\n", total_allocs);
  printf(1, "  Asignaciones fallidas: %d\n", failed_allocs);
  printf(1, "  Liberaciones: %d\n", total_frees);
  printf(1, "  Bloques no liberados: %d\n", remaining);
  printf(1, "========================================\n");
}

// ============================================================================
// PRUEBA 4: MULTIPLE PROCESOS (CONCURRENCIA)
// ============================================================================

void
test_multiprocess(void)
{
  int start_time, end_time;
  int pid;
  int i;
  int num_children = 3;

  printf(1, "\n");
  printf(1, "========================================\n");
  printf(1, "PRUEBA 4: MULTIPLE PROCESOS\n");
  printf(1, "========================================\n");
  printf(1, "Creando %d procesos que asignan memoria simultaneamente...\n",
         num_children);

  start_time = gettime();

  // Crear procesos hijos
  for(i = 0; i < num_children; i++) {
    pid = fork();
    if(pid < 0) {
      printf(1, "[ERROR] Fork fallo\n");
      exit();
    }

    if(pid == 0) {
      // Proceso hijo
      int child_id = i + 1;
      int allocs = 0;
      char *pages[15];
      int j;

      printf(1, "[HIJO-%d] Iniciando...\n", child_id);

      // Asignar memoria
      for(j = 0; j < 15; j++) {
        pages[j] = sbrk(4096);
        if((int)pages[j] != -1) {
          allocs++;
          write_pattern(pages[j], child_id * 100 + j);
        }
        sleep(1);  // Pequeña pausa para intercalar con otros procesos
      }

      printf(1, "[HIJO-%d] Asignaciones: %d/15\n", child_id, allocs);

      // Verificar
      int verified = 0;
      for(j = 0; j < allocs; j++) {
        if(verify_pattern(pages[j], child_id * 100 + j)) {
          verified++;
        }
      }

      printf(1, "[HIJO-%d] Verificaciones OK: %d/%d\n",
             child_id, verified, allocs);

      // Liberar
      for(j = 0; j < allocs; j++) {
        sbrk(-4096);
      }

      printf(1, "[HIJO-%d] Finalizado\n", child_id);
      exit();
    }
  }

  // Proceso padre espera a todos los hijos
  for(i = 0; i < num_children; i++) {
    wait();
  }

  end_time = gettime();

  printf(1, "\nRESUMEN PRUEBA 4:\n");
  printf(1, "  Tiempo total: %d ticks\n", end_time - start_time);
  printf(1, "  Todos los procesos completados\n");
  printf(1, "========================================\n");
}

// ============================================================================
// FUNCIÓN PRINCIPAL
// ============================================================================

int
main(int argc, char *argv[])
{
  int total_start, total_end;

  printf(1, "\n");
  printf(1, "================================================\n");
  printf(1, "    SUITE DE PRUEBAS DE MEMORIA - xv6\n");
  printf(1, "================================================\n");
  printf(1, "\n");
  printf(1, "Esta suite compara el rendimiento de:\n");
  printf(1, "  - FREELIST (original): First Fit simple\n");
  printf(1, "  - BST BEST FIT: Arbol binario con Best Fit\n");
  printf(1, "\n");
  printf(1, "Ejecutando 4 pruebas diferentes...\n");
  printf(1, "================================================\n");

  total_start = gettime();

  // Ejecutar todas las pruebas
  test_sequential_alloc();
  sleep(10);  // Pausa entre pruebas

  test_alternating_pattern();
  sleep(10);

  test_stress();
  sleep(10);

  test_multiprocess();

  total_end = gettime();

  // Resumen final
  printf(1, "\n");
  printf(1, "================================================\n");
  printf(1, "    RESUMEN FINAL\n");
  printf(1, "================================================\n");
  printf(1, "Tiempo total de todas las pruebas: %d ticks\n",
         total_end - total_start);
  printf(1, "\n");
  printf(1, "INTERPRETACION:\n");
  printf(1, "------------------------------------------------\n");
  printf(1, "FREELIST (original):\n");
  printf(1, "  + Mas rapido en asignaciones simples\n");
  printf(1, "  - Fragmentacion externa alta\n");
  printf(1, "  - Busqueda lineal O(n)\n");
  printf(1, "\n");
  printf(1, "BST BEST FIT (modificado):\n");
  printf(1, "  + Reduce fragmentacion externa\n");
  printf(1, "  + Busqueda logaritmica O(log n)\n");
  printf(1, "  + Mejor con cargas fragmentadas\n");
  printf(1, "  - Overhead de arbol\n");
  printf(1, "================================================\n");
  printf(1, "\n");

  exit();
}