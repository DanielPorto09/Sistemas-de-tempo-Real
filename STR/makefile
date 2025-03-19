# Compilador
CXX = g++

# Flags de compilação
CXXFLAGS = -std=c++11 -Wall -Wextra -pthread

# Nome do executável
TARGET = jantar_filosofos[Daniel_Porto]

# Arquivos fonte
SRCS = jantar_filosofos[Daniel_Porto].cpp

# Arquivos objeto (gerados a partir dos arquivos fonte)
OBJS = $(SRCS:.cpp=.o)

# Regra padrão (primeira regra do Makefile)
all: $(TARGET)

# Regra para gerar o executável
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Regra para compilar cada arquivo fonte em um objeto
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Regra para limpar arquivos gerados
clean:
	rm -f $(OBJS) $(TARGET)

# Regra para rodar o programa
run: $(TARGET)
	./$(TARGET)

# Regra para rodar e limpar
rerun: clean run

# Indica que "clean" e "run" não são arquivos, mas sim comandos
.PHONY: clean run rerun