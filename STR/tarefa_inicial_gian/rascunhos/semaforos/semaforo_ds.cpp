/// aqui vai no miros.h
typedef struct {
    uint32_t count;          // Contador do semáforo
    OSThread *waitingQueue;  // Threads bloqueadas no semáforo (opcional, para suporte a bloqueio)
} OSSemaphore;


////////////////////////////////////////////////////////////////////
void OS_sem_init(OSSemaphore *sem, uint32_t initial_count) {
    sem->count = initial_count;
    sem->waitingQueue = NULL; // Inicializa fila de espera vazia
}

void OS_sem_wait(OSSemaphore *sem) {
    __disable_irq(); // Entra em seção crítica
    if (sem->count > 0) {
        sem->count--; // Consome o semáforo
    } else {
        // Bloqueia a thread atual (adiciona à fila de espera)
        OS_curr->blocked = sem;
        OS_readySet &= ~(1U << (OS_currIdx - 1U)); // Remove do conjunto de prontas
        OS_sched(); // Chama o escalonador
    }
    __enable_irq();
}

void OS_sem_signal(OSSemaphore *sem) {
    __disable_irq();
    if (sem->waitingQueue != NULL) {
        // Se houver threads bloqueadas, acorda a primeira (FIFO simples)
        OSThread *thread = sem->waitingQueue;
        sem->waitingQueue = thread->next;
        OS_readySet |= (1U << (thread->idx - 1U)); // Marca como pronta
    } else {
        sem->count++; // Libera o semáforo
    }
    __enable_irq();
}