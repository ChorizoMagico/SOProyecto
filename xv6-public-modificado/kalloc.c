// Asignador de memoria física con algoritmo Best Fit usando Árbol Binario de Búsqueda
// Diseñado para asignar memoria para procesos de usuario, stacks del kernel,
// tablas de páginas y buffers de pipes. Asigna páginas de 4096 bytes.
//
// MODIFICACIÓN: Implementa Best Fit con BST para minimizar fragmentación externa
// Autor del cambio: [Tu Nombre]
// Fecha: Diciembre 2025

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"

void freerange(void *vstart, void *vend);
extern char end[]; // Primera dirección después del kernel cargado desde el archivo ELF

// ============================================================================
// ESTRUCTURAS DE DATOS PARA EL ÁRBOL BINARIO DE BÚSQUEDA (BST)
// ============================================================================

// Nodo del árbol binario que representa un bloque de memoria libre
struct bst_node {
  char *addr;              // Dirección física del bloque de memoria
  uint size;               // Tamaño del bloque en bytes (múltiplo de PGSIZE)
  struct bst_node *left;   // Subárbol izquierdo (bloques más pequeños)
  struct bst_node *right;  // Subárbol derecho (bloques más grandes)
  struct bst_node *parent; // Nodo padre (para facilitar eliminación)
};

// Estructura global que mantiene el estado del asignador de memoria
struct {
  struct spinlock lock;      // Lock para sincronización en multiprocesador
  int use_lock;              // Flag: 1 = usar lock, 0 = no usar (inicialización)
  struct bst_node *root;     // Raíz del árbol binario de búsqueda
  uint total_free;           // Total de memoria libre en bytes (estadística)
  uint num_blocks;           // Número de bloques libres (estadística)
  struct bst_node nodes[128]; // Pool de nodos preasignados para el árbol
  int next_node_idx;         // Índice del siguiente nodo disponible
} kmem;

// ============================================================================
// FUNCIONES AUXILIARES PARA MANEJO DEL POOL DE NODOS
// ============================================================================

// Obtiene un nodo del pool preasignado
// Retorna: puntero al nodo o 0 si no hay nodos disponibles
struct bst_node*
get_node(void)
{
  if(kmem.next_node_idx >= 128)
    return 0;  // Pool agotado
  return &kmem.nodes[kmem.next_node_idx++];
}

// Libera un nodo regresándolo al pool (implementación simplificada)
// En una implementación más compleja, esto mantendría una freelist de nodos
void
free_node(struct bst_node *node)
{
  // Simplificación: en xv6 no reutilizamos nodos para evitar complejidad
  // En producción, esto debería mantener una lista de nodos libres
  (void)node;
}

// ============================================================================
// FUNCIONES DEL ÁRBOL BINARIO DE BÚSQUEDA
// ============================================================================

// Inserta un bloque de memoria en el árbol BST ordenado por tamaño
// Parámetros:
//   root: puntero al puntero de la raíz (puede modificar la raíz)
//   addr: dirección del bloque a insertar
//   size: tamaño del bloque en bytes
// Retorna: puntero al nodo insertado o 0 si falla
struct bst_node*
bst_insert(struct bst_node **root, char *addr, uint size)
{
  struct bst_node *new_node;
  struct bst_node *current;
  struct bst_node *parent;

  // Obtener un nodo del pool
  new_node = get_node();
  if(new_node == 0)
    return 0;

  // Inicializar el nuevo nodo
  new_node->addr = addr;
  new_node->size = size;
  new_node->left = 0;
  new_node->right = 0;
  new_node->parent = 0;

  // Si el árbol está vacío, este nodo se convierte en la raíz
  if(*root == 0) {
    *root = new_node;
    return new_node;
  }

  // Buscar la posición correcta en el árbol
  current = *root;
  parent = 0;

  while(current != 0) {
    parent = current;

    // Criterio de ordenamiento: por tamaño, si son iguales por dirección
    if(size < current->size || (size == current->size && addr < current->addr)) {
      current = current->left;  // Ir al subárbol izquierdo
    } else {
      current = current->right; // Ir al subárbol derecho
    }
  }

  // Insertar el nodo como hijo del padre encontrado
  new_node->parent = parent;
  if(size < parent->size || (size == parent->size && addr < parent->addr)) {
    parent->left = new_node;
  } else {
    parent->right = new_node;
  }

  return new_node;
}

// Encuentra el nodo con el valor mínimo en un subárbol
// Usado para encontrar el sucesor en la eliminación
// Parámetros:
//   node: raíz del subárbol
// Retorna: nodo con el valor mínimo
struct bst_node*
bst_min_node(struct bst_node *node)
{
  while(node->left != 0)
    node = node->left;
  return node;
}

// Elimina un nodo del árbol BST
// Parámetros:
//   root: puntero al puntero de la raíz
//   node: nodo a eliminar
void
bst_delete(struct bst_node **root, struct bst_node *node)
{
  struct bst_node *replacement;
  struct bst_node *child;

  if(node == 0)
    return;

  // CASO 1: Nodo hoja (sin hijos)
  if(node->left == 0 && node->right == 0) {
    if(node->parent == 0) {
      *root = 0;  // Era la raíz y única nodo
    } else if(node->parent->left == node) {
      node->parent->left = 0;
    } else {
      node->parent->right = 0;
    }
    free_node(node);
    return;
  }

  // CASO 2: Nodo con un solo hijo
  if(node->left == 0 || node->right == 0) {
    child = (node->left != 0) ? node->left : node->right;

    if(node->parent == 0) {
      *root = child;  // El hijo se convierte en la nueva raíz
    } else if(node->parent->left == node) {
      node->parent->left = child;
    } else {
      node->parent->right = child;
    }

    child->parent = node->parent;
    free_node(node);
    return;
  }

  // CASO 3: Nodo con dos hijos
  // Encontrar el sucesor (mínimo del subárbol derecho)
  replacement = bst_min_node(node->right);

  // Copiar los datos del sucesor al nodo actual
  node->addr = replacement->addr;
  node->size = replacement->size;

  // Eliminar el sucesor (que tiene a lo más un hijo derecho)
  bst_delete(root, replacement);
}

// Busca el mejor bloque (Best Fit) que satisface el tamaño solicitado
// Best Fit: el bloque más pequeño que es >= size
// Parámetros:
//   node: nodo actual en la búsqueda
//   size: tamaño requerido
//   best: puntero al mejor nodo encontrado hasta ahora
// Retorna: mejor nodo encontrado o 0 si no hay bloques suficientemente grandes
struct bst_node*
bst_find_best_fit(struct bst_node *node, uint size, struct bst_node *best)
{
  if(node == 0)
    return best;

  // Si este nodo es suficientemente grande
  if(node->size >= size) {
    // Actualizar best si este nodo es mejor (más pequeño que el best actual)
    if(best == 0 || node->size < best->size) {
      best = node;
    }
    // Buscar en el subárbol izquierdo (bloques más pequeños)
    // Puede haber un bloque que se ajuste mejor
    best = bst_find_best_fit(node->left, size, best);
  } else {
    // Si este nodo es muy pequeño, buscar en el subárbol derecho
    best = bst_find_best_fit(node->right, size, best);
  }

  return best;
}

// ============================================================================
// FUNCIONES DE COALESCENCIA (FUSIÓN DE BLOQUES ADYACENTES)
// ============================================================================

// Intenta fusionar un bloque con sus vecinos físicamente adyacentes
// Esto reduce la fragmentación externa
// Parámetros:
//   addr: dirección del bloque
//   size: tamaño del bloque
// Retorna: tamaño del bloque después de la coalescencia
uint
try_coalesce(char *addr, uint size)
{
  struct bst_node *current;
  char *next_addr;
  uint new_size;

  // Intentar fusionar con el siguiente bloque
  next_addr = addr + size;
  current = kmem.root;

  // Buscar si el siguiente bloque está libre
  while(current != 0) {
    if(current->addr == next_addr) {
      // Bloque adyacente encontrado, fusionar
      new_size = size + current->size;

      // Eliminar el bloque adyacente del árbol
      bst_delete(&kmem.root, current);
      kmem.num_blocks--;

      // Reintentar coalescencia con el nuevo tamaño
      return try_coalesce(addr, new_size);
    }

    if(next_addr < current->addr)
      current = current->left;
    else
      current = current->right;
  }

  // No se encontró bloque adyacente, retornar el tamaño actual
  return size;
}

// ============================================================================
// FUNCIONES DE INICIALIZACIÓN
// ============================================================================

// Inicialización fase 1: configura el asignador antes de tener paginación completa
// Parámetros:
//   vstart: dirección inicial del rango de memoria
//   vend: dirección final del rango de memoria
void
kinit1(void *vstart, void *vend)
{
  initlock(&kmem.lock, "kmem");
  kmem.use_lock = 0;           // No usar lock durante inicialización
  kmem.root = 0;               // Árbol vacío inicialmente
  kmem.total_free = 0;         // Sin memoria libre inicialmente
  kmem.num_blocks = 0;         // Sin bloques inicialmente
  kmem.next_node_idx = 0;      // Pool de nodos vacío
  freerange(vstart, vend);     // Agregar el rango de memoria al árbol
}

// Inicialización fase 2: agrega el resto de la memoria física
// Parámetros:
//   vstart: dirección inicial del rango de memoria
//   vend: dirección final del rango de memoria
void
kinit2(void *vstart, void *vend)
{
  freerange(vstart, vend);
  kmem.use_lock = 1;  // Activar locks para operación normal multiprocesador
}

// Libera un rango de memoria agregándola al árbol BST
// Parámetros:
//   vstart: dirección inicial
//   vend: dirección final
void
freerange(void *vstart, void *vend)
{
  char *p;
  // Redondear hacia arriba al límite de página
  p = (char*)PGROUNDUP((uint)vstart);

  // Liberar cada página en el rango
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
    kfree(p);
}

// ============================================================================
// FUNCIONES PRINCIPALES: KFREE Y KALLOC
// ============================================================================

// Libera una página de memoria física y la agrega al árbol BST
// Parámetros:
//   v: puntero a la página a liberar
// Nota: Normalmente v debería haber sido retornado por kalloc()
void
kfree(char *v)
{
  uint size;

  // Validar que la dirección sea válida
  if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
    panic("kfree");

  // Llenar con basura para detectar referencias colgantes (debugging)
  memset(v, 1, PGSIZE);

  // Adquirir lock si estamos en modo multiprocesador
  if(kmem.use_lock)
    acquire(&kmem.lock);

  // Intentar fusionar con bloques adyacentes
  size = try_coalesce(v, PGSIZE);

  // Insertar el bloque (posiblemente fusionado) en el árbol
  if(bst_insert(&kmem.root, v, size) != 0) {
    // Actualizar estadísticas
    kmem.total_free += size;
    kmem.num_blocks++;
  }

  // Liberar lock
  if(kmem.use_lock)
    release(&kmem.lock);
}

// Asigna una página de 4096 bytes de memoria física usando Best Fit
// Retorna: puntero que el kernel puede usar, o 0 si no hay memoria disponible
char*
kalloc(void)
{
  struct bst_node *best;
  char *allocated_addr;
  uint remaining_size;
  char *remaining_addr;

  // Adquirir lock si estamos en modo multiprocesador
  if(kmem.use_lock)
    acquire(&kmem.lock);

  // Buscar el mejor bloque (Best Fit) para una página
  best = bst_find_best_fit(kmem.root, PGSIZE, 0);

  if(best == 0) {
    // No hay bloques disponibles
    if(kmem.use_lock)
      release(&kmem.lock);
    return 0;
  }

  // Guardar la dirección a retornar
  allocated_addr = best->addr;

  // Si el bloque es más grande que una página, dividirlo (splitting)
  if(best->size > PGSIZE) {
    remaining_size = best->size - PGSIZE;
    remaining_addr = best->addr + PGSIZE;

    // Actualizar el nodo existente con el bloque restante
    best->addr = remaining_addr;
    best->size = remaining_size;

    // Actualizar estadísticas
    kmem.total_free -= PGSIZE;
  } else {
    // El bloque es exactamente del tamaño necesario, eliminarlo
    bst_delete(&kmem.root, best);
    kmem.num_blocks--;
    kmem.total_free -= PGSIZE;
  }

  // Liberar lock
  if(kmem.use_lock)
    release(&kmem.lock);

  return allocated_addr;
}

// ============================================================================
// FUNCIONES DE ESTADÍSTICAS (OPCIONAL - PARA DEMOSTRACIÓN)
// ============================================================================

// Retorna la cantidad total de memoria libre en bytes
uint
kfreemem(void)
{
  uint free;

  if(kmem.use_lock)
    acquire(&kmem.lock);

  free = kmem.total_free;

  if(kmem.use_lock)
    release(&kmem.lock);

  return free;
}

// Retorna el número de bloques libres en el árbol
uint
knumblocks(void)
{
  uint blocks;

  if(kmem.use_lock)
    acquire(&kmem.lock);

  blocks = kmem.num_blocks;

  if(kmem.use_lock)
    release(&kmem.lock);

  return blocks;
}
