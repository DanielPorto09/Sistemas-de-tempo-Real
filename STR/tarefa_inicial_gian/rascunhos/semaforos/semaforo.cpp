

void OS_initialize_sem(OSSemaforos *sem, uint32_t contador_inicial, OSThread** lista_externa) {
    sem->count = contador_inicial;
    if (lista_externa != NULL) {
        sem->filaDeEspera = lista_externa;  // Usa lista externa se fornecida
    } else {
        static OSThread* lista_interna = NULL;  // Cria lista interna própria
        sem->filaDeEspera = &lista_interna;
    }
    *sem->filaDeEspera = NULL;  // Garante lista vazia inicial
}

void OS_lock_sem(OSSemaforos *sem) {
    __disable_irq();
    
    // Configura thread atual como bloqueada
    OS_curr->blocked = sem;
    OS_curr->next = NULL;
    
    // Adiciona ao final da fila de espera
    if (*sem->filaDeEspera == NULL) {
        *sem->filaDeEspera = OS_curr;
    } else {
        OSThread* t = *sem->filaDeEspera;
        while (t->next != NULL) t = t->next;
        t->next = OS_curr;
    }
    
    // Remove do conjunto de prontas
    uint32_t bit_position = OS_currIdx - 1U;
    uint32_t thread_bitmask = 1U << bit_position;
    OS_readySet &= ~thread_bitmask;
    __enable_irq();
    OS_sched();  // Chama o escalonador
}

void OS_decrementa_sem(OSSemaforos *sem) {
    __disable_irq();
    if (sem->count > 0) {
        sem->count--;  // Consome um recurso
    } else {
        OS_lock_sem(sem);  // Bloqueia a thread
    }
    __enable_irq();
}

void OS_trata_fila_sem(OSSemaforos *sem) {
    if (*sem->filaDeEspera != NULL) {
        // Remove a primeira thread da fila
        OSThread* thread = *sem->filaDeEspera;
        *sem->filaDeEspera = thread->next;
        
        // Atualiza estado para pronto
        thread->blocked = NULL;
        OS_readySet |= (1U << (thread->idx - 1U));
    }
}

void OS_incrementa_sem(OSSemaforos *sem) {
    __disable_irq();
    if (*sem->filaDeEspera != NULL) {
        OS_trata_fila_sem(sem);  // Libera uma thread bloqueada
    } else {
        sem->count++;  // Aumenta contador se não há threads esperando
    }
    __enable_irq();
}

void OS_verifica_integridade(OSSemaforos *sem) {
    __disable_irq();
    OSThread* t = *sem->filaDeEspera;
    while (t != NULL) {
        if (t->blocked != sem) {
            t->blocked = sem;  // Corrige inconsistências
        }
        t = t->next;
    }
    __enable_irq();
}

    
    
    
    

    
    