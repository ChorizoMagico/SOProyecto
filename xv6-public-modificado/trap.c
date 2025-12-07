#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;

void
tvinit(void)
{
  int i;

  for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);

  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  lidt(idt, sizeof(idt));
}

//PAGEBREAK: 41
void
trap(struct trapframe *tf)
{
  if(tf->trapno == T_SYSCALL){
    if(myproc()->killed)
      exit();
    myproc()->tf = tf;
    syscall();
    if(myproc()->killed)
      exit();
    return;
  }

  switch(tf->trapno){
  case T_IRQ0 + IRQ_TIMER:
    if(cpuid() == 0){
      acquire(&tickslock);
      ticks++;
      wakeup(&ticks);
      release(&tickslock);
    }
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpuid(), tf->cs, tf->eip);
    lapiceoi();
    break;

  //PAGEBREAK: 13
  default:
    if(myproc() == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpuid(), tf->eip, rcr2());
      panic("trap");
    }
    // In user space, assume process misbehaved.
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            myproc()->pid, myproc()->name, tf->trapno,
            tf->err, cpuid(), tf->eip, rcr2());
    myproc()->killed = 1;
  }

  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running
  // until it gets to the regular system call return.)
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();

  // ========================================================================
  // IMPLEMENTACIÓN MLFQ (Multi-Level Feedback Queue) CON 2 COLAS
  // ========================================================================
  // Esta sección implementa un planificador de dos niveles de prioridad:
  //
  // COLA ALTA (priority = 0):
  //   - Procesos nuevos o interactivos (I/O bound)
  //   - Quantum de tiempo: 5 ticks (~50ms)
  //   - Si un proceso usa todo su quantum, se degrada a cola baja
  //
  // COLA BAJA (priority = 1):
  //   - Procesos que consumen mucho CPU (CPU bound)
  //   - Quantum ilimitado (Round-Robin indefinido)
  //   - Se ejecutan solo cuando no hay procesos en cola alta
  //
  // OBJETIVO: Dar mejor tiempo de respuesta a procesos interactivos
  // ========================================================================
  
  if(myproc() && myproc()->state == RUNNING &&
     tf->trapno == T_IRQ0+IRQ_TIMER){
    
    // Incrementar el contador de ticks usados por el proceso actual
    // Cada tick representa aproximadamente 10ms de tiempo de CPU
    myproc()->ticks_used++;
    
    // Verificar si el proceso debe ser degradado de prioridad:
    // - ¿Está en la cola de alta prioridad? (priority == 0)
    // - ¿Ha consumido su quantum completo? (ticks_used >= 5)
    if(myproc()->priority == 0 && myproc()->ticks_used >= 5){
      // DEGRADACIÓN: Mover el proceso a la cola de baja prioridad
      // Esto indica que el proceso es CPU-intensive y no debe bloquear
      // a procesos más interactivos
      myproc()->priority = 1;      // Cambiar a cola baja
      myproc()->ticks_used = 0;    // Reiniciar contador de ticks
      
      // Nota: El proceso NO se resetea a priority=0 automáticamente.
      // Para implementar "boost" periódico (todos vuelven a alta prioridad),
      // se necesitaría un contador global de ticks en el scheduler.
    }
    
    // Forzar al proceso a ceder la CPU en cada tick del reloj
    // Esto permite que el scheduler elija el siguiente proceso según prioridad
    yield();
  }

  // Check if the process has been killed since we yielded
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();
}
