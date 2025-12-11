# pacman
Releitura do clássico jogo PacMan em C. 

# ᗧ···ᗣ···ᗣ Pac-Man em C (Raylib)

Este é um projeto acadêmico da disciplina de Programação II (2025/2), implementando uma releitura do clássico **Pac-Man** utilizando a linguagem **C** e a biblioteca gráfica **Raylib**.

O projeto foi configurado para ser **portátil**, permitindo desenvolvimento tanto em Windows (local) quanto em Linux/GitHub Codespaces.

---
## ✨ Funcionalidades

### 🧠 Lógica e Física
- **Input Buffer System:** Sistema de fila de comandos que elimina o "input lag", permitindo curvas perfeitas (cornering) e responsividade de 60 FPS.
- **Colisão Pixel-Perfect:** Detecção precisa entre Pac-Man, paredes e fantasmas.
- **IA dos Fantasmas:** Comportamentos distintos (Perseguição e Fuga/Vulnerável).

### 💾 Gerenciamento de Dados
- **Sistema de Mapas Dinâmico:** Carregamento de níveis via arquivos `.txt` (`mapa1.txt`, `mapa2.txt`, etc.), suportando qualquer tamanho de grade.
- **Persistência Binária:** Salvar e Carregar jogo (`.bin`) preservando estado exato (posições, score, vidas, timers).
- **Alocação Dinâmica:** Uso de `malloc/realloc` para gerenciamento otimizado de entidades.

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
└── README.md          # Este arquivo

### Como Rodar o Jogo
##🖥️ Windows (Local)
Pré-requisito: VS Code + Compilador MinGW (w64devkit).

#Clone o repositório:
git clone [https://github.com/SEU_USUARIO/pacman.git](https://github.com/SEU_USUARIO/pacman.git)

#Compile:
Abra a pasta no VS Code.
Pressione Ctrl + Shift + B.
Selecione: "Compilar no Windows (Local)".

#Jogue:
O executável será criado na pasta output.

#Rode:
.\output\main.exe

###👥 Autores e Responsabilidades
Este projeto foi desenvolvido colaborativamente com divisão clara de módulos:

##Açucena Santos - Gestão de Dados & Memória
Estruturas (structs), Alocação Dinâmica, Leitura de Arquivos e Sistema de Save/Load Binário.

##Sara Mendes - Lógica & Física
Algoritmos de Movimentação, Colisões, IA dos Fantasmas e Input Buffer.

##Beatriz Pereira - Renderização & Interface
Integração com Raylib, Desenho do Mapa/Entidades, HUD e Estados de Tela.
