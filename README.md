# ᗧ···ᗣ···ᗣ Pac-Man em C (Raylib)

Este é um projeto acadêmico da disciplina de **Programação II (2025/2)**, implementando uma releitura do clássico **Pac-Man** utilizando a linguagem **C** e a biblioteca gráfica **Raylib**.

---

## ✨ Funcionalidades

### 🧠 Lógica e Física
- **Input Buffer System:** Sistema de fila de comandos que elimina o "input lag", permitindo curvas perfeitas (*cornering*) e responsividade de 60 FPS.
- **Colisão Pixel-Perfect:** Detecção precisa entre Pac-Man, paredes e fantasmas.
- **IA dos Fantasmas:** Comportamentos distintos (Perseguição e Fuga/Vulnerável).

### 💾 Gerenciamento de Dados
- **Sistema de Mapas Dinâmico:** Carregamento de níveis via arquivos `.txt` (`mapa1.txt`, `mapa2.txt`, etc.), suportando qualquer tamanho de grade.
- **Persistência Binária:** Salvar e Carregar jogo (`.bin`) preservando estado exato (posições, score, vidas, timers).
- **Alocação Dinâmica:** Uso de `malloc/realloc` para gerenciamento otimizado de memória.

### 🎨 Renderização
- **Gráficos via Raylib:** Interface limpa com renderização de formas geométricas.
- **HUD Informativo:** Exibição em tempo real de Vidas, Pontuação, Nível e Pellets restantes.
- **Feedback Visual:** Mudança de cor dos fantasmas (Vulnerável = Branco) e animações de movimento.

---

## 📂 Estrutura do Projeto

```text
PACMAN/
├── .devcontainer/     # Configuração automática para Codespaces (Linux)
├── .vscode/           # Configuração de compilação (tasks.json)
├── mapas/             # Arquivos de texto dos níveis (mapa1.txt, mapa2.txt...)
├── output/            # Onde o executável (.exe) é gerado
├── vendor/            # Bibliotecas Raylib (include/lib) para Windows
├── main.c             # Código fonte principal
└── README.md          # Documentação do projeto
```
---
## 🚀 Como Rodar o Jogo

### 🖥️Windows (Local)
*Pré-requisito: VS Code + Compilador MinGW (w64devkit).*

1. **Clone o repositório:**
   ```bash
   git clone https://github.com/SEU_USUARIO/pacman.git

2. **Compile**:
Abra a pasta do projeto no VS Code.
Pressione Ctrl + Shift + B.
Selecione: "Compilar no Windows (Local)".

3. **Jogue**:
O executável será criado na pasta output.
Rode no terminal: 
   ```bash
   .\output\main.exe
   ```
---
## 🕹️ Controles

| Tecla | Ação |
| :---: | :--- |
| **W / ↑** | Mover para Cima |
| **S / ↓** | Mover para Baixo |
| **A / ←** | Mover para Esquerda |
| **D / →** | Mover para Direita |
| **TAB** | Pausar Jogo / Menu |
| **S** | Salvar Jogo (No Menu) |
| **C** | Carregar Jogo (No Menu) |
---
