# Atari 2600 Emulator (Projeto Educacional)

Este repositório contém um **emulador didático do Atari 2600**, desenvolvido em C++ por um estudante de Engenharia de Computação com o objetivo de aprender sobre:

- Arquitetura da CPU MOS 6507 / MOS 6502
- Emulação ciclo a ciclo
- Sistemas embarcados antigos
- Organização de memória
- Desenvolvimento de software de baixo nível
- Estrutura de um emulador real

O projeto está em construção.

---

## 🚀 Como Rodar o Projeto

Este projeto foi configurado para ser facilmente executado no **VS Code**, em qualquer sistema operacional:

✔ Windows  
✔ Linux  
✔ macOS  

### **Rodar com VS Code**
1. Instale a extensão **C/C++** (Microsoft)
2. Abra a pasta do projeto no VS Code
3. Pressione: **Ctrl + Shift + B**
4. O VS Code irá compilar e gerar o executável:
   - `emulator.exe` no Windows  
   - `emulator` no Linux/Mac
5. Rode o executável pelo terminal do próprio VS Code.

---

## 📌 Sobre o Projeto

Atualmente, estamos construindo um **emulador modularizado**, começando pelo núcleo mais importante:  
**a CPU 6507**, que é praticamente um MOS 6502 simplificado (usado no Atari).

O primeiro objetivo é:

- Executar instruções simples (como `LDA #imediato`)
- Ler bytes da memória
- Avançar o Program Counter (PC)
- Criar um ciclo de clock básico da CPU

Depois iremos expandir para:

- Instruções completas da CPU
- TIA (Television Interface Adaptor)
- RIOT (I/O + timers)
- Carregamento de ROMs
- Debugger simples

Este é um projeto de aprendizado, mas que pode evoluir para algo grande.

---

## 📅 Checklist do Progresso

### **✔ Etapa 1 — CPU Básica (concluído / iniciada)**
- [x] Estrutura mínima do projeto (`main`, CPU, memória)
- [x] 64KB de memória simulada
- [x] Program Counter (PC)
- [x] Registrador A (Acumulador)
- [x] Fetch de instrução (Busca)
- [x] Execução do opcode **0xA9 — LDA imediato**
- [x] Loop básico de clock

### **🔄 Etapa 2 — CPU Intermediária (em andamento)**
- [ ] Flags da CPU (Zero, Negativo, Carry, Overflow…)
- [ ] Modos de endereçamento
- [ ] OpCodes essenciais do 6502
- [ ] Testes unitários para instruções

---

## 💡 Objetivo Educacional

O propósito deste repositório é servir como:

- um **estudo sistemático** de emuladores,
- um **projeto de portfólio de alto impacto**,  
- e um caminho para entender como consoles antigos realmente funcionavam.

Esse tipo de projeto é extremamente valorizado e raro em currículos de estudantes, porque exige:

- conhecimento de arquitetura de computadores  
- redes de barramento  
- compilação/montagem  
- software de baixo nível  
- engenharia reversa  
- processamento de sinais (fase TIA)

---

## 🤝 Contribuindo

Pull requests são bem-vindos.  
Abra uma issue caso queira sugerir melhorias ou reportar bugs.

---

## 📜 Licença

MIT — livre para usar, estudar e modificar.
