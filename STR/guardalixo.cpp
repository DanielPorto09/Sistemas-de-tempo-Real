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
    int idFilosofo;
    mutex mtx; // Mutex para proteger o acesso ao garfo

public:
    Garfo() : emUso(false), idFilosofo(-1) {}

    bool estaSendoUsado() const {
        return emUso;
    }

    bool podePegar(int idFilosofoSolicitante, int idGarfo) {
        int garfoEsquerda = idFilosofoSolicitante;
        int garfoDireita = (idFilosofoSolicitante + 1) % 5;
        return (idGarfo == garfoEsquerda || idGarfo == garfoDireita);
    }

    bool pegar(int idFilosofoSolicitante, int idGarfo) {
        lock_guard<mutex> lock(mtx);
        if (!emUso && podePegar(idFilosofoSolicitante, idGarfo)) {
            emUso = true;
            idFilosofo = idFilosofoSolicitante;
            return true;
        }
        return false;
    }

    void soltar() {
        lock_guard<mutex> lock(mtx);
        emUso = false;
        idFilosofo = -1;
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
    
        // Filósofos pares pegam garfo da esquerda primeiro, e ímpares pegam garfo da direita primeiro
        if (id % 2 == 0) {
            if (garfos[garfoEsquerda].pegar(id, garfoEsquerda) &&
                garfos[garfoDireita].pegar(id, garfoDireita)) {
                setStatus(1); // Começando a comer
                return true;
            }
        } else {
            if (garfos[garfoDireita].pegar(id, garfoDireita) &&
                garfos[garfoEsquerda].pegar(id, garfoEsquerda)) {
                setStatus(1); // Começando a comer
                return true;
            }
        }
        return false;
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
    while (true) {
        filosofo.setStatus(2); // Pensando
        mostrarTodosFilosofosEGarfos(filosofos, garfos); // Exibe o estado atual

        this_thread::sleep_for(chrono::milliseconds(1000)); // Pensando

        filosofo.setStatus(3); // Com fome
        mostrarTodosFilosofosEGarfos(filosofos, garfos); // Exibe o estado atual

        if (filosofo.pegarGarfos(garfos)) { // Tenta pegar os garfos
            filosofo.setStatus(1); // Comendo
            mostrarTodosFilosofosEGarfos(filosofos, garfos); // Exibe o estado atual
            this_thread::sleep_for(chrono::milliseconds(2000)); // Comendo
            filosofo.soltarGarfos(garfos); // Solta os garfos
            mostrarTodosFilosofosEGarfos(filosofos, garfos); // Exibe o estado atual
        }
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
