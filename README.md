# Atari 2600 Emulator (Projeto Educacional)

Este repositório contém um **emulador didático do Atari 2600**, desenvolvido em C++ por mim, com o objetivo de aprender sobre:

- Arquitetura da CPU MOS 6507 / MOS 6502
- Emulação ciclo a ciclo
- Sistemas embarcados antigos
- Organização de memória
- Desenvolvimento de software de baixo nível
- Estrutura de um emulador real

O projeto está em construção.

---

## Como Rodar o Projeto

Este projeto foi configurado para ser facilmente executado no **VS Code**, em qualquer sistema operacional:

✔ Windows  
✔ Linux  
✔ macOS  

### **Rodar com VS Code**
1. Instale a extensão **C/C++** (Microsoft)
2. Abra a pasta do projeto no VS Code
3. Caso esteja no Linux, rode o `./requirements.sh` (para instalar dependências)
4. Pressione: **Ctrl + Shift + B**
5. O VS Code irá compilar e gerar o executável:
   - `emulator.exe` no Windows  
   - `emulator` no Linux/Mac
6. Rode o executável pelo terminal do próprio VS Code.

---

## Sobre o Projeto

Atualmente, estou construindo um **emulador modularizado**, começando pelo núcleo mais importante:  
**a CPU 6507**, que é praticamente um MOS 6502 simplificado (usado no Atari).

Meu primeiro objetivo é:

- Executar instruções simples (como `LDA #imediato`)
- Ler bytes da memória
- Avançar o Program Counter (PC)
- Criar um ciclo de clock básico da CPU

Depois, irei expandir para:

- Instruções completas da CPU
- TIA (Television Interface Adaptor)
- RIOT (I/O + timers)
- Carregamento de ROMs
- Debugger simples

Este é um projeto de aprendizado meu, mas que pode evoluir para algo maior.

---

## Progresso

### **✔ Etapa 1 — CPU Básica (concluída / iniciada)**
- [x] Estrutura mínima do projeto (`main`, CPU, memória)
- [x] 64KB de memória simulada
- [x] Program Counter (PC)
- [x] Registrador A (Acumulador)
- [x] Fetch de instrução (Busca)
- [x] Execução do opcode **0xA9 — LDA imediato**
- [x] Loop básico de clock

### **Etapa 2 — CPU Intermediária (em andamento)**
- [x] Flags da CPU (Zero, Negativo, Carry, Overflow)
- [ ] Flags da CPU (Break, Decimal, Unused, Interrupt)
- [ ] Modos de endereçamento
- [ ] OpCodes essenciais do 6502
- [ ] Testes unitários para instruções

---

## Instruções já implementadas

Aqui estão as instruções de CPU que já implementei no núcleo (`mos6502r`), com os opcodes suportados:

- **LDA (Load Accumulator)**: `0xA9` (Immediate), `0xA5` (Zero Page), `0xAD` (Absolute)
- **STA (Store Accumulator)**: `0x85` (Zero Page), `0x8D` (Absolute)
- **LDX (Load X)**: `0xA2` (Immediate), `0xA6` (Zero Page), `0xAE` (Absolute)
- **LDY (Load Y)**: `0xA0` (Immediate), `0xA4` (Zero Page), `0xAC` (Absolute)

- **ADC (Add with Carry)**: `0x69` (Immediate), `0x65` (Zero Page), `0x75` (Zero Page,X), `0x6D` (Absolute), `0x7D` (Absolute,X), `0x79` (Absolute,Y), `0x61` (Indirect,X), `0x71` (Indirect,Y).  
   - Suporta modo decimal (BCD) quando a flag `D` está setada (`SED`/`CLD` disponíveis).

- **AND (Bitwise AND with A)**: `0x29` (Immediate), `0x25` (Zero Page), `0x35` (Zero Page,X), `0x2D` (Absolute), `0x3D` (Absolute,X), `0x39` (Absolute,Y), `0x21` (Indirect,X), `0x31` (Indirect,Y)

- **ASL (Arithmetic Shift Left)**: `0x0A` (Accumulator), `0x06` (Zero Page), `0x16` (Zero Page,X), `0x0E` (Absolute), `0x1E` (Absolute,X)
- **BIT (Test Bits)**: `0x24` (Zero Page), `0x2C` (Absolute)

- **CMP (Compare A)**: `0xC9` (Immediate), `0xC5` (Zero Page), `0xD5` (Zero Page,X), `0xCD` (Absolute), `0xDD` (Absolute,X), `0xD9` (Absolute,Y), `0xC1` (Indirect,X), `0xD1` (Indirect,Y)
- **CPX (Compare X)**: `0xE0` (Immediate), `0xE4` (Zero Page), `0xEC` (Absolute)
- **CPY (Compare Y)**: `0xC0` (Immediate), `0xC4` (Zero Page), `0xCC` (Absolute)

- **Branch instructions (relative)**: `BPL` `0x10`, `BMI` `0x30`, `BVC` `0x50`, `BVS` `0x70`, `BCC` `0x90`, `BCS` `0xB0`, `BNE` `0xD0`, `BEQ` `0xF0`

- **Status / control**: `SED` (`0xF8`) and `CLD` (`0xD8`) to set/clear Decimal Mode; `BRK` (`0x00`) implemented (pushes PC+1 and P, sets I, loads vector $FFFE/$FFFF).

Observações:
- Flags atualizadas apropriadamente onde aplicável (Z, N, C, V).  
- Não estou contabilizando ciclos em runtime (a maior parte do comportamento funcional está implementadas).

---

## Objetivo Educacional

O propósito deste repositório é servir como:

- meu **estudo sistemático** de emuladores,
- meu **projeto de portfólio de alto impacto**,  
- e um caminho para eu entender como consoles antigos realmente funcionavam.

Esse tipo de projeto é valorizado e raro em currículos de estudantes, porque exige:

- conhecimento de arquitetura de computadores  
- redes de barramento  
- compilação/montagem  
- software de baixo nível  
- engenharia reversa  
- processamento de sinais (fase TIA)

---

## Contribuindo

Pull requests são bem-vindos.  
Abra uma issue se quiser sugerir melhorias ou reportar bugs.

---

## Licença

MIT — livre para usar, estudar e modificar.
