# Atari 2600 Emulator (Projeto Educacional)

Este repositório contém um **emulador didático do Atari 2600**, desenvolvido em C++ por mim, com o objetivo de aprender sobre:

- Arquitetura da CPU MOS 6507 / MOS 6502
- Emulação ciclo a ciclo
- Sistemas embarcados antigos
- Organização de memória
- Desenvolvimento de software de baixo nível
- Estrutura de um emulador real

O projeto atualmente está quase pronto, falta a implementação de áudio e correção de alguns bugs.

O jogo para testes: Space Invaders, já é quase jogável, é possível andar para o lado direito e esquerdo, porém o botão de atirar (Espaço) não está funcionando ainda.

---

## Como Rodar o Projeto

Este projeto foi configurado para ser facilmente executado no **VS Code**, nos seguintes sistemas operacionais:

✔ Windows  
✔ Linux  

### **Rodar com VS Code**
1. Instale a extensão **C/C++** (Microsoft)
2. Abra a pasta do projeto no VS Code
3. Caso esteja no Linux, rode o `./requirements.sh` (para instalar dependências).
Se estiver no Windows, procure um tutorial para baixar os compiladores de C++ via MSYS2.
4. Pressione: **Ctrl + Shift + B**
5. O VS Code irá compilar e gerar o executável:
   - `emulator.exe` no Windows  
   - `emulator` no Linux/Mac
6. Rode o executável pelo terminal do próprio VS Code.

---

## Status do Projeto

### Já implementado
- Núcleo da CPU 6502/6507 funcional (fetch/execute, modos de endereçamento, flags, stack e branches)
- Barramento/memória do Atari 2600: mapeamento de ROM 4KB, RAM/stack espelhados e registradores do TIA
- RIOT (6532): RAM, I/O básico (`SWCHA`/`SWCHB`) e timer (`INTIM` e presets)
- Sincronização de clock CPU <-> TIA (proporção 1:3) e suporte a `WSYNC`
- Vídeo (TIA): VSYNC/VBLANK simplificados, framebuffer por scanline e renderização 160x192
- Gráficos: `COLUBK`, Playfield (`PF0/PF1/PF2` + reflect), prioridade via `CTRLPF`, paleta NTSC/PAL (`TIA_PALETTE=NTSC|PAL`)
- Sprites e colisões (TIA): Players (`GRP0/GRP1`), Missiles/Ball, `RESPx`/`HMOVE`/`RESMx`, latches de colisão (`CX*`) e `CXCLR`
- Controles para teste: setas do teclado mapeadas para joystick e espaço como botão de disparo

### Pendente (Etapa 8 — Áudio)
- Emulação de áudio do TIA (`AUDC0/AUDC1`, `AUDF0/AUDF1`, `AUDV0/AUDV1`)

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
