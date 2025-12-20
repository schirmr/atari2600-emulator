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

### ** Etapa 1 — CPU Básica (concluída)**
- [x] Estrutura mínima do projeto (`main`, CPU, memória)
- [x] 64KB de memória simulada
- [x] Program Counter (PC)
- [x] Registrador A (Acumulador)
- [x] Fetch de instrução (Busca)
- [x] Execução do opcode **0xA9 — LDA imediato**
- [x] Loop básico de clock

### ** Etapa 2 — CPU Basica/Intermediária (concluída)**
- [x] Sistema completo de flags (N, Z, C, V, D, I, B, U)
- [x] Controle de flags (CLC, SEC, CLI, SEI, CLV, CLD, SED)
- [x] Flags corretas em ADC (binário e decimal)
- [x] Flags corretas em CMP / CPX / CPY
- [x] Flags corretas em BIT
- [x] Flags corretas em shifts e rotates
- [x] BRK / RTI (stack e status corretos)
- [x] Modos de endereçamento completos
- [x] Branches relativos funcionais

### ** Etapa 3 — CPU Intermediária (concluiída)**
- [x] NOP, INC/DEC (mem), INX/DEX, INY/DEY, transfers (TAX/TXA/TAY/TYA)
- [x] `JMP` absoluto e indireto (com bug de página $xxFF preservado)
- [x] BRK / RTI (stack e status corretos)
- [x] Flags e controle: CLC/SEC, CLI/SEI, CLD/SED, CLV
- [x] Stack: PHA/PLA, PHP/PLP, TSX/TXS
- [x] `SBC` implementado
- [x] Controle básico de ciclos e page crossing (+1)
- [x] IRQ e NMI (estrutura base)
- [x] Passa ROMs de teste e homebrew simples

### ** Etapa 4 — Bus, RIOT e Mapeamento (atual)**
- [x] Espelhamento de RAM (Mirroring $00xx <-> $01xx para Stack)
- [ ] Espelhamento de Registradores TIA ($00-$0D repetidos a cada 64 bytes)
- [x] Chip RIOT (6532): Implementação básica
- [x] RIOT Timers: `INTIM` e escrita em `TIM64T` (decremento por clock)
- [x] Leitura de Inputs básicos via RIOT (`SWCHA` / `SWCHB`)

### ** Etapa 5 — TIA e Sincronização (atual)**
- [ ] Sincronização de Clock CPU <-> TIA (Proporção 1:3)
- [ ] Implementação do Registrador `WSYNC` (Halt CPU até fim da scanline)
- [ ] Lógica de `VSYNC` e `VBLANK` (Controle de frame)
- [ ] Estrutura básica de renderização (Scanline Loop)

### ** Etapa 6 — Gráficos Básicos**
- [ ] Janela de saída (SDL2 / SFML / OpenGL)
- [ ] Renderização de Cor de Fundo (`COLUBK`)
- [ ] Renderização de Playfield (`PF0`, `PF1`, `PF2`) com espelhamento/reflexão
- [ ] Paleta de Cores NTSC/PAL

### ** Etapa 7 — Sprites e Colisões (TIA Avançado)**
- [ ] Renderização de Players (`GRP0`, `GRP1`) e Missiles
- [ ] Posicionamento Fino (`HMOVE`, `RESPx`)
- [ ] Registradores de Colisão de Hardware (`CxMx`, `CxP0`, etc.)
- [ ] Lógica de prioridade de camadas (Playfield vs Player)

### ** Etapa 8 — Áudio e Refinamentos**
- [ ] Geração de Áudio (Canais `AUDC0`, `AUDF0`, `AUDV0`)
- [ ] Suporte a Cartuchos com Bankswitching (>4KB)
- [ ] Mapeamento de Teclado para Joystick

---

## Instruções já implementadas

Aqui estão as instruções de CPU que já implementei no núcleo (`mos6502r`), com os opcodes suportados:

- **LDA (Load Accumulator)**: `A9` (Immediate), `A5` (Zero Page), `B5` (Zero Page,X), `AD` (Absolute), `BD` (Absolute,X), `B9` (Absolute,Y), `A1` (Indirect,X), `B1` (Indirect,Y)
- **STA (Store Accumulator)**: `85` (Zero Page), `95` (Zero Page,X), `8D` (Absolute), `9D` (Absolute,X), `99` (Absolute,Y), `81` (Indirect,X), `91` (Indirect,Y)
- **LDX (Load X)**: `A2` (Immediate), `A6` (Zero Page), `B6` (Zero Page,Y), `AE` (Absolute), `BE` (Absolute,Y)
- **LDY (Load Y)**: `A0` (Immediate), `A4` (Zero Page), `B4` (Zero Page,X), `AC` (Absolute), `BC` (Absolute,X)
 - **STX (Store X)**: `86` (Zero Page), `96` (Zero Page,Y), `8E` (Absolute)
 - **STY (Store Y)**: `84` (Zero Page), `94` (Zero Page,X), `8C` (Absolute)

- **ADC (Add with Carry)**: `69` (Immediate), `65` (Zero Page), `75` (Zero Page,X), `6D` (Absolute), `7D` (Absolute,X), `79` (Absolute,Y), `61` (Indirect,X), `71` (Indirect,Y).  
   - Suporta modo decimal (BCD) quando a flag `D` está setada (`SED`/`CLD` disponíveis).
 - **SBC (Subtract with Carry)**: `E9` (Immediate), `E5` (Zero Page), `F5` (Zero Page,X), `ED` (Absolute), `FD` (Absolute,X), `F9` (Absolute,Y), `E1` (Indirect,X), `F1` (Indirect,Y).  
   - Suporta modo decimal (BCD): `C=1` indica sem empréstimo; `V` baseado na operação binária.

- **AND (Bitwise AND with A)**: `29` (Immediate), `25` (Zero Page), `35` (Zero Page,X), `2D` (Absolute), `3D` (Absolute,X), `39` (Absolute,Y), `21` (Indirect,X), `31` (Indirect,Y)
- **EOR (Exclusive-OR)**: `49` (Immediate), `45` (Zero Page), `55` (Zero Page,X), `4D` (Absolute), `5D` (Absolute,X), `59` (Absolute,Y), `41` (Indirect,X), `51` (Indirect,Y)
 - **ORA (OR with A)**: `09` (Immediate), `05` (Zero Page), `15` (Zero Page,X), `0D` (Absolute), `1D` (Absolute,X), `19` (Absolute,Y), `01` (Indirect,X), `11` (Indirect,Y)

- **ASL (Arithmetic Shift Left)**: `0A` (Accumulator), `06` (Zero Page), `16` (Zero Page,X), `0E` (Absolute), `1E` (Absolute,X)
- **LSR (Logical Shift Right)**: `4A` (Accumulator), `46` (Zero Page), `56` (Zero Page,X), `4E` (Absolute), `5E` (Absolute,X)
 - **ROL (Rotate Left)**: `2A` (Accumulator), `26` (Zero Page), `36` (Zero Page,X), `2E` (Absolute), `3E` (Absolute,X)
 - **ROR (Rotate Right)**: `6A` (Accumulator), `66` (Zero Page), `76` (Zero Page,X), `6E` (Absolute), `7E` (Absolute,X)
- **BIT (Test Bits)**: `24` (Zero Page), `2C` (Absolute)

- **CMP (Compare A)**: `C9` (Immediate), `C5` (Zero Page), `D5` (Zero Page,X), `CD` (Absolute), `DD` (Absolute,X), `D9` (Absolute,Y), `C1` (Indirect,X), `D1` (Indirect,Y)
- **CPX (Compare X)**: `E0` (Immediate), `E4` (Zero Page), `EC` (Absolute)
- **CPY (Compare Y)**: `C0` (Immediate), `C4` (Zero Page), `CC` (Absolute)
- **DEC (Decrement Memory)**: `C6` (Zero Page), `D6` (Zero Page,X), `CE` (Absolute), `DE` (Absolute,X)
- **INC (Increment Memory)**: `E6` (Zero Page), `F6` (Zero Page,X), `EE` (Absolute), `FE` (Absolute,X)

- **Branch instructions (relative)**: `BPL` `10`, `BMI` `30`, `BVC` `50`, `BVS` `70`, `BCC` `90`, `BCS` `B0`, `BNE` `D0`, `BEQ` `F0`

- **JMP/JSR/RTS/RTI**: `JMP` `4C` (Absolute), `6C` (Indirect c/ wrap), `JSR` `20`, `RTS` `60`, `RTI` `40`
- **Status / control**: `CLC` `18`, `SEC` `38`, `CLI` `58`, `SEI` `78`, `CLV` `B8`, `CLD` `D8`, `SED` `F8`, `BRK` `00` (empilha PC+1 e P|B, seta I, carrega vetor $FFFE/$FFFF)
 - **NOP**: `EA`
 - **Register / Transfer / Inc-Dec**: `TAX` `AA`, `TXA` `8A`, `TAY` `A8`, `TYA` `98`, `INX` `E8`, `DEX` `CA`, `INY` `C8`, `DEY` `88`

---

## Flags da CPU (Status Register P)

- N (Negativo): atualizado por `updateZN()` (ex.: `LDA`, `LDX`, `LDY`, `AND`, `ADC`, `ASL`) e por `CMP/CPX/CPY` (via resultado). Em `BIT`, reflete o bit 7 da memória.
- V (Overflow): definido em `ADC` (soma binária) e em `BIT` (bit 6 da memória). `CLV` implementado.
- B (Break): em `BRK`, B é setado apenas no valor de `P` empilhado (o status em runtime não mantém B setado).
- D (Decimal Mode): controlado por `SED`/`CLD`. Quando ativo, `ADC` realiza soma em BCD; `V` continua baseado na soma binária.
- I (Interrupt Disable): `SEI/CLI` e `BRK` alteram I; `RTI` restaura o status anterior.
- Z (Zero): atualizado por `updateZN()`, por `CMP/CPX/CPY` (igualdade) e por `BIT` quando `(A & M) == 0`.
- C (Carry): atualizado por `ADC` (binário/BCD), `ASL` (bit 7 original) e `CMP/CPX/CPY` (set se registrador >= M). Branches não alteram flags.
- Bit 5 (Unused): inicializado como 1 (`0x20`) e preservado ao empilhar `P`.

Observações:
- Flags são atualizadas onde aplicável; `RTI` limpa `B` e mantém o bit 5 (`UNUSED`) ligado ao restaurar `P`.
- Ciclos/penalidades por crossing de página ainda não são contabilizados em runtime.

---

## Comportamentos Importantes (6502)

- Zero Page wrap-around: endereçamento `($zz,X)`/`$zz,X`/`$zz,Y` em zero page faz wrap de 8 bits. Ex.: `LDA $80,X` com `X=$FF` acessa `$7F` (não `$017F`). Implementado em `zpx()`/`zpy()`.
- `JMP ($addr)` bug de página: ao apontar para `$xxFF`, o byte alto é lido de `$xx00` (wrap). Implementado no `JMP` indireto.
- Program Counter: `busca()` avança `PC` ao ler o opcode; os modos de endereçamento avançam `PC` pelos operandos. Branches usam deslocamento com sinal relativo ao endereço seguinte.
- Pilha e falsos retornos: `JSR` empilha `PC-1` (alto depois baixo); `RTS` soma `+1` ao retornar. Ao forjar retornos (tabelas com `RTS`), empilhe endereço-1 e sempre empurre MSB primeiro.
- Tempos de execução: este núcleo não contabiliza ciclos; instruções marcadas com `+` costumam custar +1 ciclo quando há crossing de página (não modelado aqui).

---

## Testes

- Teste funcional 6502: o arquivo `6502_functional_test.bin` foi obtido do repositório de Klaus – https://github.com/Klaus2m5/6502_65C02_functional_tests – e será utilizado para validação mais ampla (créditos ao autor).

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
