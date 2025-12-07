// Best Fit SIMPLE con lista enlazada
#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"

void freerange(void *vstart, void *vend);
extern char end[];

struct run {
  uint size;           // Tamaño del bloque
  struct run *next;
};

struct {
  struct spinlock lock;
  int use_lock;
  struct run *freelist;
} kmem;

void
kinit1(void *vstart, void *vend)
{
  initlock(&kmem.lock, "kmem");
  kmem.use_lock = 0;
  kmem.freelist = 0;
  freerange(vstart, vend);
}

void
kinit2(void *vstart, void *vend)
{
  freerange(vstart, vend);
  kmem.use_lock = 1;
}

void
freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uint)vstart);
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
    kfree(p);
}

void
kfree(char *v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
    panic("kfree");

  memset(v, 1, PGSIZE);

  if(kmem.use_lock)
    acquire(&kmem.lock);

  r = (struct run*)v;
  r->size = PGSIZE;
  r->next = kmem.freelist;
  kmem.freelist = r;

  if(kmem.use_lock)
    release(&kmem.lock);
}

// BEST FIT: Busca el bloque MÁS PEQUEÑO que satisface el tamaño
char*
kalloc(void)
{
  struct run *r, *prev, *best, *best_prev;

  if(kmem.use_lock)
    acquire(&kmem.lock);

  // Buscar el mejor bloque (más pequeño >= PGSIZE)
  best = 0;
  best_prev = 0;
  prev = 0;
  r = kmem.freelist;

  while(r) {
    if(r->size >= PGSIZE) {
      if(best == 0 || r->size < best->size) {
        best = r;
        best_prev = prev;
      }
    }
    prev = r;
    r = r->next;
  }

  // Si no hay bloques disponibles
  if(best == 0) {
    if(kmem.use_lock)
      release(&kmem.lock);
    return 0;
  }

  // Remover el bloque de la lista
  if(best_prev == 0) {
    kmem.freelist = best->next;
  } else {
    best_prev->next = best->next;
  }

  if(kmem.use_lock)
    release(&kmem.lock);

  return (char*)best;
}