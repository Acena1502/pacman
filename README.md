# pacman
Releitura do clássico jogo PacMan em C. 

# ᗧ···ᗣ···ᗣ Pac-Man em C (Raylib)

Este é um projeto acadêmico da disciplina de Programação II (2025/2), implementando uma releitura do clássico **Pac-Man** utilizando a linguagem **C** e a biblioteca gráfica **Raylib**.

O projeto foi configurado para ser **portátil**, permitindo desenvolvimento tanto em Windows (local) quanto em Linux/GitHub Codespaces.

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
