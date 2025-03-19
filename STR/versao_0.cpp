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
    mutex mtx; // Mutex para proteger o acesso ao garfo

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
            case 1: return 'C'; // Comendo
            case 2: return 'P'; // Pensando
            case 3: return 'F'; // Com fome
            default: return '?';
        }
    }

    bool pegarGarfos(vector<Garfo>& garfos) {
        int garfoEsquerda = id;
        int garfoDireita = (id + 1) % 5;

        bool pegouEsquerdo = garfos[garfoEsquerda].pegar();
        bool pegouDireito = garfos[garfoDireita].pegar();

        if (pegouEsquerdo && pegouDireito) {
            setStatus(1); // Começando a comer
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
        int garfoEsquerda = id;
        int garfoDireita = (id + 1) % 5;
        garfos[garfoEsquerda].soltar();
        garfos[garfoDireita].soltar();
        setStatus(2); // Voltando a pensar
    }

    int getId() const {
        return id;
    }
};

void mostrarTodosFilosofosEGarfos(const vector<Filosofo>& filosofos, const vector<Garfo>& garfos) {
    lock_guard<mutex> lock(printMutex); // Garante que apenas uma thread imprime por vez
    // Mostrar filósofos
    for (const auto& f : filosofos) {
        cout << f.getEstadoChar() << ",";
    }
    cout << "";
    // Mostrar garfos
    for (const auto& g : garfos) {
        cout << g.getEstadoChar() << ",";
    }
    cout << endl;
}

void filosofoComendo(Filosofo& filosofo, vector<Garfo>& garfos, vector<Filosofo>& filosofos) {
    filosofo.setStatus(2); // Pensando
    mostrarTodosFilosofosEGarfos(filosofos, garfos); // Exibe o estado atual
    while (true) {
        this_thread::sleep_for(chrono::milliseconds(1000)); // Pensando
        filosofo.setStatus(3); // Com fome
        mostrarTodosFilosofosEGarfos(filosofos, garfos); // Exibe o estado atual

        bool pegouGarfos = false;
        while (!pegouGarfos) {
            pegouGarfos = filosofo.pegarGarfos(garfos); // Tenta pegar os garfos
            if (!pegouGarfos) {
                this_thread::sleep_for(chrono::milliseconds(500)); // Espera um pouco antes de tentar de novo
                mostrarTodosFilosofosEGarfos(filosofos, garfos); // Exibe o estado atual
            }
        }

        filosofo.setStatus(1); // Comendo
        mostrarTodosFilosofosEGarfos(filosofos, garfos); // Exibe o estado atual
        this_thread::sleep_for(chrono::milliseconds(2000)); // Comendo
        filosofo.soltarGarfos(garfos); // Solta os garfos
        filosofo.setStatus(2); // Pensando
        mostrarTodosFilosofosEGarfos(filosofos, garfos); // Exibe o estado atual
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



