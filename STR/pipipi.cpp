

#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <chrono>

const int NUM_FILOSOFOS = 5;
void filosofo(int id);
char estado[NUM_FILOSOFOS];
void think(int id);
void starving(int id);
bool teste_garfo(int id);
void valida_estados();

int tentativasFalhas[NUM_FILOSOFOS] = {0}; // Contador de tentativas de pegar garfos
const int LIMITE_STARVATION = 20;          // Número de tentativas antes de detectar starvation
std ::mutex saida;
std ::mutex EmTesteGarfo[NUM_FILOSOFOS];
int estadoGarfo[NUM_FILOSOFOS];

void mostra();

// Mutex para sincronizar o acesso aos garfos
std ::mutex garfos[NUM_FILOSOFOS];

void valida_estados()
{
    for (int i = 0; i < NUM_FILOSOFOS; i++)
    {
        std::cout << "Filósofo " << i << " (" << estado[i] << ") - Garfo " << i << ": " << estadoGarfo[i]
                  << " | Garfo " << (i + 1) % NUM_FILOSOFOS << ": " << estadoGarfo[(i + 1) % NUM_FILOSOFOS] << std::endl;
    }
    int garfos_ocupados = 0;
    int filosofos_comendo = 0;

    // Contar garfos ocupados e filósofos comendo
    for (int i = 0; i < NUM_FILOSOFOS; i++)
    {
        if (estadoGarfo[i] == 1) // Garfo ocupado
            garfos_ocupados++;

        if (estado[i] == 'C') // Filósofo comendo
            filosofos_comendo++;
    }
    if (filosofos_comendo > 1)
    {
        std::cout << "⚠️ Atenção: " << filosofos_comendo << " filósofos estão comendo ao mesmo tempo!" << std::endl;
    }

    // Verificar se o número de garfos ocupados é ímpar (o que seria inválido)
    if (garfos_ocupados != filosofos_comendo * 2)
    {
        if (garfos_ocupados == 0 && filosofos_comendo == 0)
        {
            std::cout << "Válido: Nenhum filósofo está comendo e nenhum garfo está ocupado!" << std::endl;
        }
        else
        {
            std::cout << "Inválido: O número de garfos ocupados não corresponde ao número de filósofos comendo!" << std::endl;
            std::abort();
        }
    }

    else
    {
        // Verificar se o número de garfos ocupados é igual a filósofos comendo * 2
        if (garfos_ocupados != filosofos_comendo * 2)
        {
            std::cout << "Inválido: O número de garfos ocupados não corresponde ao número de filósofos comendo!" << std::endl;
            std::abort(); // Abortando o programa
        }
        else
        {
            std::cout << "Válido: O número de garfos ocupados corresponde corretamente ao número de filósofos comendo!" << std::endl;
        }
    }
    for (int i = 0; i < NUM_FILOSOFOS; i++)
    {
        if (estado[i] == 'C')
        {
            int garfo_esquerda = i;
            int garfo_direita = (i + 1) % NUM_FILOSOFOS;
            if (!(estadoGarfo[garfo_esquerda] == 1 && estadoGarfo[garfo_direita] == 1))
            {
                std::cout << "Erro: Filósofo " << i << " está comendo sem os dois garfos corretos!" << std::endl;
                std::abort();
            }
        }
    }
    // Detectar starvation
    for (int i = 0; i < NUM_FILOSOFOS; i++)
    {
        if (estado[i] == 'F')
        {
            tentativasFalhas[i]++;
            std::cout << " 🍽️ Filósofo " << i
                      << " tentou comer " << tentativasFalhas[i] << " vez(es) sem sucesso."
                      << std::endl;

            if (tentativasFalhas[i] > LIMITE_STARVATION)
            {
                std::cerr << " ⚠️ STARVATION DETECTADO: Filósofo " << i
                          << " está faminto há muito tempo!"
                          << std::endl;
                std::abort();
            }
        }
        else if (estado[i] == 'C')
        {
            std::cout << " 🍗 🍗 🍗 Filósofo " << i
                      << " esta comendo depois de " << tentativasFalhas[i] << " vez(es) sem sucesso."
                      << std::endl;
        }
        else
        {
            tentativasFalhas[i] = 0;
        }
    }

    // Detalhar os estados dos filósofos e garfos para cada situação
}

void mostra()
{
    for (int i = 0; i < NUM_FILOSOFOS; i++)
        std::cout << estado[i] << " ";

    for (int i = 0; i < NUM_FILOSOFOS; i++)
        std::cout << estadoGarfo[i] << " ";

    std::cout << std::endl;

    // Chama a validação para conferir se os estados estão corretos
    valida_estados();
}
void filosofo(int id)
{
    while (true)
    {
        think(id);
        while (true)
        {
            if (estado[id] != 'F')
            {

                starving(id);
            }
            if (EmTesteGarfo[id].try_lock())
            {
                if (EmTesteGarfo[(id + 1) % NUM_FILOSOFOS].try_lock())
                {
                    estadoGarfo[id] = 1;
                    estadoGarfo[(id + 1) % NUM_FILOSOFOS] = 1;
                    estado[id] = 'C';

                    std ::this_thread::sleep_for(std ::chrono::milliseconds(1000));
                    /*if (teste_garfo(id))
                      {*/

                    while (true)
                    {
                        if (saida.try_lock())
                        {

                            mostra();
                            saida.unlock();
                            break;
                        }
                    }

                    garfos[(id + 1) % NUM_FILOSOFOS].unlock();
                    garfos[id].unlock();
                    estadoGarfo[id] = 0;
                    estadoGarfo[(id + 1) % NUM_FILOSOFOS] = 0;
                    EmTesteGarfo[id].unlock();
                    EmTesteGarfo[(id + 1) % NUM_FILOSOFOS].unlock();
                    estado[id] = 'P';
                    break;
                    //}
                }
                EmTesteGarfo[id].unlock();
            }
        }
    }
}

bool teste_garfo(int id)
{
    if (garfos[id].try_lock())
    {
        estadoGarfo[id] = 1;
        if (garfos[(id + 1) % NUM_FILOSOFOS].try_lock())
        {
            estadoGarfo[(id + 1) % NUM_FILOSOFOS] = 1;
            estado[id] = 'C';
            return true;
        }
        estadoGarfo[id] = 0;
        garfos[id].unlock();
    }

    return false;
}
void think(int id)
{

    while (true)
    {

        if (saida.try_lock())
        {
            estado[id] = 'P';
            mostra();
            saida.unlock();
            break;
        }
    }
    std ::this_thread::sleep_for(std ::chrono::milliseconds(1000));
}
void starving(int id)
{

    while (true)
    {
        if (saida.try_lock())
        {
            estado[id] = 'F';
            mostra();
            saida.unlock();
            break;
        }
    }
    std ::this_thread::sleep_for(std ::chrono::milliseconds(1000));
}

int main()
{
    std ::vector<std ::thread> filosofos;
    for (int i = 0; i < NUM_FILOSOFOS; i++)
    {
        estado[i] = 'P';
        estadoGarfo[i] = 0;
    }

    // Criar threads para os filósofos
    for (int i = 0; i < NUM_FILOSOFOS; i++)
    {
        filosofos.emplace_back(filosofo, i);
    }

    // Aguardar a execução (nunca termina)
    for (auto &f : filosofos)
    {
        f.join();
    }

    return 0;
}