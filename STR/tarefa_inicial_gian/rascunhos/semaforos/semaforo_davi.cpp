class MySemaphore {
    private:
        volatile bool ocupado;
        // Bloqueia a seção crítica
           void lock() {
    
               // Agora, a seção crítica pode ser acessada
               ocupado = true;
    
    
           }
           void unlock(){
               ocupado=false;
           }
    
    
    
    public:
        // Construtor
        MySemaphore() : ocupado(false) {}
    
        bool tryUnlock() {
             __disable_irq();
             if(isLocked()){
                unlock();
                 __enable_irq();
                             return true;
    
             }
             __enable_irq();
                         return false;
    
    
    
    
          }
        bool isLocked(){
            return ocupado;
        }
        bool isAvailable(){
            return !ocupado;
        }
    
    
    
        // Verifica se o MySemaphore está disponível (opcional)
        bool tryLock() {
            __disable_irq();// preciso usar o result pq se colocar ocupado dps de habilitar interupcao o valor pode mudar
            if(isAvailable()){
            lock();
            __enable_irq();
            return true;
            }
            __enable_irq();
            return false;
        }
    };