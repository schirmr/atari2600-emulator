# ===== Configurações =====
CXX      := g++
TARGET   := emulator_app

# Diretórios
SRC_DIRS := . emulator memory cpu tia graphics

# Flags
CXXFLAGS := -std=c++17 -Wall -Wextra -O2
SDL_CFLAGS := $(shell pkg-config --cflags sdl2)
SDL_LIBS   := $(shell pkg-config --libs sdl2)

# Arquivos fonte
SRCS := \
	main.cpp \
	emulator/emulator.cpp \
	memory/memory.cpp \
	cpu/mos6502r.cpp \
	memory/riot.cpp \
	tia/tia.cpp \
	graphics/tia_palette.cpp \
	graphics/sdl2_renderer.cpp

# ===== Regras =====
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) $^ -o $@ $(SDL_LIBS)

clean:
	rm -f $(TARGET)

.PHONY: all clean