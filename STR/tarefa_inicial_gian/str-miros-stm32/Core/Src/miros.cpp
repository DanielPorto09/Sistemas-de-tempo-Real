/****************************************************************************
* MInimal Real-time Operating System (MiROS), GNU-ARM port.
*
* This software is a teaching aid to illustrate the concepts underlying
* a Real-Time Operating System (RTOS). The main goal of the software is
* simplicity and clear presentation of the concepts, but without dealing
* with various corner cases, portability, or error handling. For these
* reasons, the software is generally NOT intended or recommended for use
* in commercial applications.
*
* Copyright (C) 2018 Miro Samek. All Rights Reserved.
*
* SPDX-License-Identifier: GPL-3.0-or-later
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <https://www.gnu.org/licenses/>.
*
* Git repo:
* https://github.com/QuantumLeaps/MiROS
* @author
****************************************************************************/
#include <cstdint>
#include "miros.h"
#include "qassert.h"
#include "stm32g4xx.h"

Q_DEFINE_THIS_FILE

namespace rtos {

OSThread * volatile OS_curr; /* pointer to the current thread */
OSThread * volatile OS_next; /* pointer to the next thread to run */

OSThread *OS_thread[32 + 1]; /* array of threads started so far */
uint32_t OS_readySet; /* bitmask of threads that are ready to run */

uint8_t OS_threadNum; /* number of threads started */
uint8_t OS_currIdx; /* current thread index for the circular array */


OSThread idleThread;
void main_idleThread() {
    while (1) {
        OS_onIdle();
    }
}

void OS_init(void *stkSto, uint32_t stkSize) {
    /* set the PendSV interrupt priority to the lowest level 0xFF */
    *(uint32_t volatile *)0xE000ED20 |= (0xFFU << 16);

    /* start idleThread thread */
    OSThread_start(&idleThread,
                   &main_idleThread,
                   stkSto, stkSize);
}

void OS_sched(void) {
    if (OS_readySet == 0U) { /* idle condition? */
    	OS_currIdx = 0U; /* the idle thread */
    } else {
    	do{ /* find the next ready thread*/
            OS_currIdx++;
            if(OS_currIdx == OS_threadNum){
            	OS_currIdx = 1;
            }
            OS_next = OS_thread[OS_currIdx];
    	}while((OS_readySet & (1U <<(OS_currIdx - 1U))) == 0 );
    }
    OS_next = OS_thread[OS_currIdx];

    /* trigger PendSV, if needed */
    if(OS_next != OS_curr){
    	*(uint32_t volatile *)0xE000ED04 = (1U << 28);
    }
}

void OS_run(void) {
    /* callback to configure and start interrupts */
    OS_onStartup();

    __disable_irq();
    OS_sched();
    __enable_irq();

    /* the following code should never execute */
    Q_ERROR();
}

void OS_tick(void) {
	uint8_t n = 0;
	for(n=1U;n<OS_threadNum; n++){ 				/* cycle through every thread but the idle */
		if(OS_thread[n]->timeout != 0U){
			OS_thread[n]->timeout--;			/* decrease the timeout */
			if(OS_thread[n]->timeout == 0U){
				OS_readySet |= (1U << (n-1U));	/* if the thread is ready mask the corresponding bit */
			}
		}
	}
}

void OS_delay(uint32_t ticks) {
    __asm volatile ("cpsid i");

    /* never call OS_delay from the idleThread */
    Q_REQUIRE(OS_curr != OS_thread[0]);

    OS_curr->timeout = ticks;
    OS_readySet &= ~(1U << (OS_currIdx - 1U));
    OS_sched();
    __asm volatile ("cpsie i");
 }

void OSThread_start(
    OSThread *me,
    OSThreadHandler threadHandler,
    void *stkSto, uint32_t stkSize)
{
    /* round down the stack top to the 8-byte boundary
    * NOTE: ARM Cortex-M stack grows down from hi -> low memory
    */
    uint32_t *sp = (uint32_t *)((((uint32_t)stkSto + stkSize) / 8) * 8);
    uint32_t *stk_limit;

    /* thread number must be in ragne
    * and must be unused
    */
    Q_REQUIRE((OS_threadNum < Q_DIM(OS_thread)) && (OS_thread[OS_threadNum] == (OSThread *)0));

    *(--sp) = (1U << 24);  /* xPSR */
    *(--sp) = (uint32_t)threadHandler; /* PC */
    *(--sp) = 0x0000000EU; /* LR  */
    *(--sp) = 0x0000000CU; /* R12 */
    *(--sp) = 0x00000003U; /* R3  */
    *(--sp) = 0x00000002U; /* R2  */
    *(--sp) = 0x00000001U; /* R1  */
    *(--sp) = 0x00000000U; /* R0  */
    /* additionally, fake registers R4-R11 */
    *(--sp) = 0x0000000BU; /* R11 */
    *(--sp) = 0x0000000AU; /* R10 */
    *(--sp) = 0x00000009U; /* R9 */
    *(--sp) = 0x00000008U; /* R8 */
    *(--sp) = 0x00000007U; /* R7 */
    *(--sp) = 0x00000006U; /* R6 */
    *(--sp) = 0x00000005U; /* R5 */
    *(--sp) = 0x00000004U; /* R4 */

    /* save the top of the stack in the thread's attibute */
    me->sp = sp;

    /* round up the bottom of the stack to the 8-byte boundary */
    stk_limit = (uint32_t *)(((((uint32_t)stkSto - 1U) / 8) + 1U) * 8);

    /* pre-fill the unused part of the stack with 0xDEADBEEF */
    for (sp = sp - 1U; sp >= stk_limit; --sp) {
        *sp = 0xDEADBEEFU;
    }

    /* register the thread with the OS */
    OS_thread[OS_threadNum] = me;
    /* make the thread ready to run */
    if (OS_threadNum > 0U) {
        OS_readySet |= (1U << (OS_threadNum - 1U));
    }
    OS_threadNum++;
}
/***********************************************/
void OS_onStartup(void) {
    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / TICKS_PER_SEC);

    /* set the SysTick interrupt priority (highest) */
    NVIC_SetPriority(SysTick_IRQn, 0U);
}

void OS_onIdle(void) {
#ifdef NDBEBUG
    __WFI(); /* stop the CPU and Wait for Interrupt */
#endif
}

}//fim namespace

void Q_onAssert(char const *module, int loc) {
    /* TBD: damage control */
    (void)module; /* avoid the "unused parameter" compiler warning */
    (void)loc;    /* avoid the "unused parameter" compiler warning */
    NVIC_SystemReset();
}

/***********************************************/
__attribute__ ((naked, optimize("-fno-stack-protector")))
void PendSV_Handler(void) {
__asm volatile (

    /* __disable_irq(); */
    "  CPSID         I                 \n"

    /* if (OS_curr != (OSThread *)0) { */
    "  LDR           r1,=_ZN4rtos7OS_currE       \n"
    "  LDR           r1,[r1,#0x00]     \n"
    "  CBZ           r1,PendSV_restore \n"

    /*     push registers r4-r11 on the stack */
    "  PUSH          {r4-r11}          \n"

    /*     OS_curr->sp = sp; */
    "  LDR           r1,=_ZN4rtos7OS_currE       \n"
    "  LDR           r1,[r1,#0x00]     \n"
    "  STR           sp,[r1,#0x00]     \n"
    /* } */

    "PendSV_restore:                   \n"
    /* sp = OS_next->sp; */
    "  LDR           r1,=_ZN4rtos7OS_nextE       \n"
    "  LDR           r1,[r1,#0x00]     \n"
    "  LDR           sp,[r1,#0x00]     \n"

    /* OS_curr = OS_next; */
    "  LDR           r1,=_ZN4rtos7OS_nextE       \n"
    "  LDR           r1,[r1,#0x00]     \n"
    "  LDR           r2,=_ZN4rtos7OS_currE       \n"
    "  STR           r1,[r2,#0x00]     \n"

    /* pop registers r4-r11 */
    "  POP           {r4-r11}          \n"

    /* __enable_irq(); */
    "  CPSIE         I                 \n"

    /* return to the next thread */
    "  BX            lr                \n"
    );
}

/*

IMPLEMENTAÇÃO DO SEMAFORO

*/

void OS_initialize_sem(OSSemaforos *sem, uint32_t contador_inicial) {
    Q_REQUIRE(sem != NULL);
    sem->count = contador_inicial;
    sem->filaDeEspera = NULL;
}

void OS_lock_sem(OSSemaforos *sem) {
    Q_REQUIRE(sem != NULL);
    __disable_irq();
    
    // Verifica se thread já está bloqueada
    Q_REQUIRE(OS_curr->blocked == NULL);

    // Adiciona à fila de espera
    OS_curr->blocked = sem;
    OS_curr->next = NULL;
    
    if (sem->filaDeEspera == NULL) {
        sem->filaDeEspera = OS_curr;
    } else {
        OSThread* t = sem->filaDeEspera;
        while (t->next != NULL) t = t->next;
        t->next = OS_curr;
    }

    // Encontra o índice da thread atual
    uint8_t idx;
    for (idx = 1; idx < OS_threadNum; idx++) {
        if (OS_thread[idx] == OS_curr) {
            uint32_t bit_position = OS_currIdx - 1U;
            uint32_t thread_bitmask = 1U << bit_position;
            OS_readySet &= ~thread_bitmask;
            break;
        }
    }
    
    OS_sched();
    __enable_irq();
}


void OS_decrementa_sem(OSSemaforos *sem) {
    Q_REQUIRE(sem != NULL);
    __disable_irq();
    if (sem->count > 0) {
        sem->count--;
    } else {
        OS_lock_sem(sem);
    }
    __enable_irq();
}

static void OS_trata_fila_sem(OSSemaforos *sem) {
    if (sem->filaDeEspera != NULL) {
        OSThread* thread = sem->filaDeEspera;
        sem->filaDeEspera = thread->next;
        thread->blocked = NULL;
        thread->next = NULL;
        
        // Encontra o índice da thread no array OS_thread
        uint8_t idx;
        for (idx = 1; idx < OS_threadNum; idx++) {
            if (OS_thread[idx] == thread) {
                OS_readySet |= (1U << (idx - 1U)); // Marca como ready
                break;
            }
        }
    }
}

void OS_incrementa_sem(OSSemaforos *sem) {
    Q_REQUIRE(sem != NULL);
    __disable_irq();
    if (sem->filaDeEspera != NULL) {
        OS_trata_fila_sem(sem);
    } else {
        if (sem->count < UINT32_MAX) {
            sem->count++;
        }
    }
    __enable_irq();
}

bool OS_verifica_integridade(OSSemaforos *sem) {
    bool integro = true;
    __disable_irq();
    OSThread* t = sem->filaDeEspera;
    while (t != NULL) {
        if (t->blocked != sem) {
            t->blocked = sem;
            integro = false;
        }
        if (t->next == t) {
            integro = false;
            t->next = NULL;
        }
        t = t->next;
    }
    __enable_irq();
    return integro;
}
