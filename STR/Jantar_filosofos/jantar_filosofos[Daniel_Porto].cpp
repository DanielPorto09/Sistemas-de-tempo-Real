#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>

using namespace std;

mutex printMutex;

class Garfo {
private:
    bool emUso;
    mutex mtx; 

public:
    Garfo() : emUso(false) {}

    bool pegar() {
        lock_guard<mutex> lock(mtx);
        if (!emUso) {
            emUso = true;
            return true;
        }
        return false;
    }

    void soltar() {
        lock_guard<mutex> lock(mtx);
        emUso = false;
    }

    char getEstadoChar() const {
        return emUso ? 'O' : 'L';
    }

    bool estaEmUso() const {
        return emUso;
    }
};

class Filosofo {
private:
    int id;
    int status; // 1: comendo, 2: pensando, 3: com fome

public:
    Filosofo(int id) : id(id), status(2) {}

    void setStatus(int new_status) {
        status = new_status;
    }

    int getStatus() const {
        return status;
    }

    char getEstadoChar() const {
        switch (status) {
            case 1: return 'C'; 
            case 2: return 'P'; 
            case 3: return 'F'; 
            default: return '?';
        }
    }

    bool pegarGarfos(vector<Garfo>& garfos) {
        int garfoEsquerda = (id - 1 + 5) % 5;
        int garfoDireita = id;

        bool pegouEsquerdo = garfos[garfoEsquerda].pegar();
        bool pegouDireito = garfos[garfoDireita].pegar();

        if (pegouEsquerdo && pegouDireito) {
            setStatus(1); 
            return true;
        } else {
            if (pegouEsquerdo) {
                garfos[garfoEsquerda].soltar();
            }
            if (pegouDireito) {
                garfos[garfoDireita].soltar();
            }
            return false;
        }
    }

    void soltarGarfos(vector<Garfo>& garfos) {
        int garfoEsquerda = (id - 1 + 5) % 5;
        int garfoDireita = id;
        garfos[garfoEsquerda].soltar();
        garfos[garfoDireita].soltar();
        setStatus(2); 
    }

    int getId() const {
        return id;
    }
};

void valida_estados(const vector<Filosofo>& filosofos, const vector<Garfo>& garfos) {
    const int NUM_FILOSOFOS = filosofos.size();
    const int LIMITE_STARVATION = 5; 
    vector<int> tentativasFalhas(NUM_FILOSOFOS, 0); 

    for (int i = 0; i < NUM_FILOSOFOS; i++) {
        cout << "Filósofo " << i << " (" << filosofos[i].getEstadoChar() << ") - Garfo " << i << ": " << garfos[i].getEstadoChar()
             << " | Garfo " << (i -1 + NUM_FILOSOFOS) % NUM_FILOSOFOS << ": " << garfos[(i -1 + NUM_FILOSOFOS) % NUM_FILOSOFOS].getEstadoChar() << endl;
    }

    int garfos_ocupados = 0;
    int filosofos_comendo = 0;

    for (int i = 0; i < NUM_FILOSOFOS; i++) {
        if (garfos[i].estaEmUso()) 
            garfos_ocupados++;

        if (filosofos[i].getEstadoChar() == 'C') 
            filosofos_comendo++;
    }

    if (filosofos_comendo > 1) {
        cout << "⚠️ Atenção: " << filosofos_comendo << " filósofos estão comendo ao mesmo tempo!" << endl;
    }

    // Verificar se o número de garfos ocupados é igual a filósofos comendo * 2
    if (garfos_ocupados != filosofos_comendo * 2) {
        if (garfos_ocupados == 0 && filosofos_comendo == 0) {
            cout << "Válido: Nenhum filósofo está comendo e nenhum garfo está ocupado!" << endl;
        } else {
            cout << "Inválido: O número de garfos ocupados não corresponde ao número de filósofos comendo!" << endl;
            abort();
        }
    } else {
        cout << "Válido: O número de garfos ocupados corresponde corretamente ao número de filósofos comendo!" << endl;
    }

    // Verificar se filósofos comendo têm os dois garfos
    for (int i = 0; i < NUM_FILOSOFOS; i++) {
        if (filosofos[i].getEstadoChar() == 'C') {
            int garfo_esquerda = (i -1 +NUM_FILOSOFOS) % NUM_FILOSOFOS;
            int garfo_direita = i;
            if (!(garfos[garfo_esquerda].estaEmUso() && garfos[garfo_direita].estaEmUso())) {
                cout << "Erro: Filósofo " << i << " está comendo sem os dois garfos corretos!" << endl;
                abort();
            }
        }
    }

    // Detectar starvation
    for (int i = 0; i < NUM_FILOSOFOS; i++) {
        if (filosofos[i].getEstadoChar() == 'F') {
            tentativasFalhas[i]++;
            cout << " 🍽️ Filósofo " << i
                 << " tentou comer " << tentativasFalhas[i] << " vez(es) sem sucesso."
                 << endl;

            if (tentativasFalhas[i] > LIMITE_STARVATION) {
                cerr << " ⚠️ STARVATION DETECTADO: Filósofo " << i
                     << " está faminto há muito tempo!"
                     << endl;
                abort();
            }
        } else if (filosofos[i].getEstadoChar() == 'C') {
            cout << " 🍗 Filósofo " << i
                 << " está comendo depois de " << tentativasFalhas[i] << " vez(es) sem sucesso."
                 << endl;
        } else {
            tentativasFalhas[i] = 0;
        }
    }
}

void mostrarTodosFilosofosEGarfos(const vector<Filosofo>& filosofos, const vector<Garfo>& garfos) {
    lock_guard<mutex> lock(printMutex); 
    for (const auto& f : filosofos) {
        cout << f.getEstadoChar() << ",";
    }
    cout << " ";
    for (const auto& g : garfos) {
        cout << g.getEstadoChar() << ",";
    }
    cout << endl;

    //valida_estados(filosofos, garfos);
}

void filosofoComendo(Filosofo& filosofo, vector<Garfo>& garfos, vector<Filosofo>& filosofos) {
    filosofo.setStatus(2); 
    mostrarTodosFilosofosEGarfos(filosofos, garfos); 
    while (true) {
        this_thread::sleep_for(chrono::milliseconds(1000)); 
        filosofo.setStatus(3); 
        mostrarTodosFilosofosEGarfos(filosofos, garfos); 

        bool pegouGarfos = false;
        while (!pegouGarfos) {
            pegouGarfos = filosofo.pegarGarfos(garfos); 
            if (!pegouGarfos) {
                this_thread::sleep_for(chrono::milliseconds(500)); 
                mostrarTodosFilosofosEGarfos(filosofos, garfos); 
            }
        }

        filosofo.setStatus(1); 
        mostrarTodosFilosofosEGarfos(filosofos, garfos); 
        this_thread::sleep_for(chrono::milliseconds(2000)); 
        filosofo.soltarGarfos(garfos); 
        mostrarTodosFilosofosEGarfos(filosofos, garfos); 
        filosofo.setStatus(2); 
        mostrarTodosFilosofosEGarfos(filosofos, garfos); 
    }
}

int main() {
    vector<Garfo> garfos(5);
    vector<Filosofo> filosofos;
    for (int i = 0; i < 5; i++) {
        filosofos.emplace_back(i);
    }

    vector<thread> threads;
    for (int i = 0; i < 5; i++) {
        threads.emplace_back(filosofoComendo, ref(filosofos[i]), ref(garfos), ref(filosofos));
    }

    for (auto& t : threads) {
        t.join();
    }

    return 0;
}