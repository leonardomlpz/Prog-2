EXEC = game
BUILD_DIR = build
SRC_DIR = src
CC = gcc

# ATUALIZADO: Adicionamos allegro_image-5 e allegro_font-5
ALLEGRO_CFLAGS = $(shell pkg-config --cflags allegro-5 allegro_primitives-5 allegro_image-5 allegro_font-5 allegro_ttf-5)
ALLEGRO_LIBS = $(shell pkg-config --libs allegro-5 allegro_primitives-5 allegro_image-5 allegro_font-5 allegro_ttf-5)

CFLAGS = -Wall -g $(ALLEGRO_CFLAGS)
LDFLAGS = $(ALLEGRO_LIBS)

# Nao precisa mexer daqui para baixo
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

all: $(BUILD_DIR)/$(EXEC)

$(BUILD_DIR)/$(EXEC): $(OBJ)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)
	@echo "Jogo compilado! Execute com: ./$(BUILD_DIR)/$(EXEC)"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -rf $(BUILD_DIR)
	@echo "Arquivos de build removidos."

.PHONY: all clean