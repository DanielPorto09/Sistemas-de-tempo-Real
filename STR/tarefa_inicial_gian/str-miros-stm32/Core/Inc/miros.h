/*
 * miros.h
 *
 *  Created on: Feb 6, 2025
 *      Author: guilh
 */

#ifndef INC_MIROS_H_
#define INC_MIROS_H_

namespace rtos {
/* Thread Control Block (TCB) */
typedef struct {
    void *sp; /* stack pointer */
    uint32_t timeout; /* timeout delay down-counter */
    struct OSSemaforos *blocked;
    struct OSThread *next;
} OSThread;

const uint16_t TICKS_PER_SEC = 100U;

typedef void (*OSThreadHandler)();

/* Estrutura do Semáforo */
typedef struct OSSemaforos { //corrigido - movido para dentro do namespace
    uint32_t count; // contador do semáforo //corrigido de "uint32_t contador"
    OSThread *filaDeEspera; //lista de threads bloqueadas //corrigido de "OSThread **filaDeEspera"
} OSSemaforos;

void OS_init(void *stkSto, uint32_t stkSize);

/* callback to handle the idle condition */
void OS_onIdle(void);

/* this function must be called with interrupts DISABLED */
void OS_sched(void);

/* transfer control to the RTOS to run the threads */
void OS_run(void);

/* blocking delay */
void OS_delay(uint32_t ticks);

/* process all timeouts */
void OS_tick(void);

/* callback to configure and start interrupts */
void OS_onStartup(void);

void OSThread_start(
    OSThread *me,
    OSThreadHandler threadHandler,
    void *stkSto, uint32_t stkSize);

/* Funções do Semáforo */
void OS_initialize_sem(OSSemaforos *sem, uint32_t contador_inicial); //corrigido - removido parâmetro lista_externa
void OS_lock_sem(OSSemaforos *sem);
void OS_decrementa_sem(OSSemaforos *sem);
void OS_incrementa_sem(OSSemaforos *sem);
bool OS_verifica_integridade(OSSemaforos *sem); //corrigido - adicionado retorno bool

} // fim namespace rtos

#endif /* INC_MIROS_H_ */
