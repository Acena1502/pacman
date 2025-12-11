// pacman.c
// Trabalho Pratico PROG2 (2025/2) - Pac-Man
// Compilar (MSYS2 UCRT64/w64devkit): gcc pacman.c -o pacman.exe -lraylib -lopengl32 -lgdi32 -lwinmm

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "raylib.h"


// Para a entrega final, coloque "//" na frente dela (Modo 1600x840 oficial).
#define MODO_NOTEBOOK 

#define LINHAS 20
#define COLUNAS 40
#define TAM_BLOCO 40   // Mantemos 40px (requisito oficial)
#define ALTURA_HUD 40

#define AREA_JOGAVEL (COLUNAS * TAM_BLOCO)
#define AREA_TOTAL   (LINHAS * TAM_BLOCO + ALTURA_HUD)

#define MAPA1_FILE "mapas/mapa1.txt"
#define MAPA2_FILE "mapas/mapa2.txt"
#define SAVE_FILE  "pacman_save.bin"

#define VEL_PAC 4.0f
#define STEP_PAC (1.0f / VEL_PAC)

#define VEL_FANT 4.0f
#define STEP_FANT (1.0f / VEL_FANT)

#define VEL_FANT_VULN 3.0f
#define STEP_FANT_VULN (1.0f / VEL_FANT_VULN)

#define DUR_POWER 8.0

typedef struct { int x,y; } Pos;

typedef struct {
    Pos pos;
    Pos inicio;
    int vidas;
    int pontuacao;
    
    // --- CORREÇÃO: Variáveis de Movimento ---
    int dir;      // Direção atual (Inércia)
    int prox_dir; // Buffer (Intenção do jogador - Requisito 1.1)
} Pacman;

typedef struct {
    Pos pos;
    int dir;        // 0 up,1 down,2 left,3 right
    bool vuln;
    double endVuln;
    double since;
    bool alive;
} Fantasma;

typedef enum { TELA_MENU, TELA_JOGO, TELA_PAUSA } Tela;

typedef struct {
    Pacman pac;
    char *mapa;         // LINHAS * COLUNAS
    Fantasma *fant;
    int nF;

    Tela tela;
    int nivel;
    int pellets;

    double lastPac;     // tempo do ultimo passo do Pacman
    bool power;
    double endPower;
} Estado;

// ---------------- helpers ----------------
static inline char map_get(char *m,int x,int y){ return m[y*COLUNAS + x]; }
static inline void map_set(char *m,int x,int y,char v){ m[y*COLUNAS + x] = v; }

bool pode_mover(char *m,int x,int y){
    if(x<0||x>=COLUNAS||y<0||y>=LINHAS) return false;
    return map_get(m,x,y) != '#';
}

int oposto(int d){ return d==0?1: d==1?0: d==2?3:2; }

// ---------------- leitura do mapa ----------------
char *ler_mapa(const char *arquivo, Pacman *p, Fantasma **outF, int *outN, int *outPel){
    FILE *f = fopen(arquivo, "r");
    if(!f) return NULL;

    char *m = malloc(LINHAS * COLUNAS);
    if(!m){ fclose(f); return NULL; }

    char linha[COLUNAS + 8];
    Fantasma *fv = NULL;
    int nf = 0;
    int pellets = 0;

    for(int i=0;i<LINHAS;i++){
        if(fgets(linha, sizeof(linha), f)){
            linha[strcspn(linha, "\r\n")] = '\0';
            int len = (int)strlen(linha);
            for(int j=0;j<COLUNAS;j++){
                char c = (j < len) ? linha[j] : ' ';
                if(c == '<' || c == '>'){
                    map_set(m,j,i,c);
                }
                else if(c == 'F' || c == 'G'){
                    Fantasma *tmp = realloc(fv, sizeof(Fantasma)*(nf+1));
                    if(!tmp){ free(fv); free(m); fclose(f); return NULL; }
                    fv = tmp;
                    fv[nf].pos.x = j; fv[nf].pos.y = i;
                    fv[nf].dir = GetRandomValue(0,3);
                    fv[nf].vuln = false;
                    fv[nf].endVuln = 0;
                    fv[nf].since = 0;
                    fv[nf].alive = true;
                    nf++;
                    map_set(m,j,i,' ');
                }
                else if(c == 'P'){
                    p->pos.x = j; p->pos.y = i;
                    p->inicio.x = j; p->inicio.y = i;
                    map_set(m,j,i,' ');
                }
                else {
                    map_set(m,j,i,c);
                    if(c == '.' || c == 'o') pellets++;
                }
            }
        } else {
            for(int j=0;j<COLUNAS;j++) map_set(m,j,i,' ');
        }
    }

    fclose(f);
    *outF = fv; *outN = nf; *outPel = pellets;
    return m;
}

// ---------------- portais ----------------
void usar_portal(char *m, Pos *p, int dir){
    char c = map_get(m, p->x, p->y);
    if(c == '<'){
        for(int y=0;y<LINHAS;y++) for(int x=0;x<COLUNAS;x++) if(map_get(m,x,y) == '>'){ p->x = x; p->y = y; return; }
    }
    if(c == '>'){
        for(int y=0;y<LINHAS;y++) for(int x=0;x<COLUNAS;x++) if(map_get(m,x,y) == '<'){ p->x = x; p->y = y; return; }
    }
    if(c == 'T'){
        if(dir == 2 || dir == 3){
            for(int x=0;x<COLUNAS;x++) if(x != p->x && map_get(m,x,p->y) == 'T'){ p->x = x; return; }
        } else {
            for(int y=0;y<LINHAS;y++) if(y != p->y && map_get(m,p->x,y) == 'T'){ p->y = y; return; }
        }
    }
}

// ---------------- IA dos fantasmas ----------------
int escolher_dir(char *m,int x,int y,int atual){
    int op = oposto(atual);
    int cand[4]; int n=0;
    if(pode_mover(m,x,y-1) && 0!=op) cand[n++]=0;
    if(pode_mover(m,x,y+1) && 1!=op) cand[n++]=1;
    if(pode_mover(m,x-1,y) && 2!=op) cand[n++]=2;
    if(pode_mover(m,x+1,y) && 3!=op) cand[n++]=3;
    if(n==0){
        if(pode_mover(m,x,y-1)) cand[n++]=0;
        if(pode_mover(m,x,y+1)) cand[n++]=1;
        if(pode_mover(m,x-1,y)) cand[n++]=2;
        if(pode_mover(m,x+1,y)) cand[n++]=3;
    }
    if(n==0) return atual;
    return cand[GetRandomValue(0,n-1)];
}

void mover_fantasmas(Estado *E, float dt){
    for(int i=0;i<E->nF;i++){
        Fantasma *f = &E->fant[i];
        if(!f->alive) continue;
        float intervalo = f->vuln ? STEP_FANT_VULN : STEP_FANT;
        f->since += dt;
        if(f->since < intervalo) continue;
        f->since = 0;

        int dx=0, dy=0;
        if(f->dir==0) dy=-1;
        else if(f->dir==1) dy=1;
        else if(f->dir==2) dx=-1;
        else dx=1;

        int nx = f->pos.x + dx, ny = f->pos.y + dy;
        if(!pode_mover(E->mapa, nx, ny)){
            f->dir = escolher_dir(E->mapa, f->pos.x, f->pos.y, f->dir);
        }

        dx=dy=0;
        if(f->dir==0) dy=-1;
        else if(f->dir==1) dy=1;
        else if(f->dir==2) dx=-1;
        else dx=1;
        nx = f->pos.x + dx; ny = f->pos.y + dy;

        if(pode_mover(E->mapa, nx, ny)){
            f->pos.x = nx; f->pos.y = ny;
            usar_portal(E->mapa, &f->pos, f->dir);
        }

        if(f->vuln && GetTime() >= f->endVuln) f->vuln = false;
    }
}

// ---------------- interação com pellets ----------------
void ao_mover_pac(Estado *E){
    char c = map_get(E->mapa, E->pac.pos.x, E->pac.pos.y);
    if(c == '.'){
        E->pac.pontuacao += 10;
        E->pellets--;
        map_set(E->mapa, E->pac.pos.x, E->pac.pos.y, ' ');
    } else if(c == 'o'){
        E->pac.pontuacao += 50;
        E->pellets--;
        map_set(E->mapa, E->pac.pos.x, E->pac.pos.y, ' ');
        E->power = true;
        E->endPower = GetTime() + DUR_POWER;
        for(int i=0;i<E->nF;i++){
            E->fant[i].vuln = true;
            E->fant[i].endVuln = GetTime() + DUR_POWER;
        }
    }
}

// ---------------- colisões ----------------
void checar_colisoes(Estado *E){
    for(int i=0;i<E->nF;i++){
        Fantasma *f = &E->fant[i];
        if(!f->alive) continue;
        if(f->pos.x == E->pac.pos.x && f->pos.y == E->pac.pos.y){
            if(f->vuln){
                f->alive = false;
                E->pac.pontuacao += 100;
            } else {
                E->pac.vidas--;
                E->pac.pontuacao -= 200;
                if(E->pac.pontuacao < 0) E->pac.pontuacao = 0;

                // resetar posições
                E->pac.pos = E->pac.inicio; 
                
                // --- CORREÇÃO: Resetar movimento ---
                E->pac.dir = -1;      // Para parado
                E->pac.prox_dir = -1; // Limpa intenção
                E->lastPac = GetTime();

                E->power = false;

                if(E->pac.vidas <= 0){
                    free(E->mapa); free(E->fant);
                    Pacman ptmp = E->pac;
                    Fantasma *fv = NULL; int nf = 0, pel = 0;
                    char *m = ler_mapa(MAPA1_FILE, &ptmp, &fv, &nf, &pel);
                    if(m){
                        E->mapa = m; E->pac = ptmp; E->fant = fv; E->nF = nf; E->pellets = pel;
                        E->power = false; E->lastPac = GetTime(); E->nivel = 1;
                        E->pac.dir = -1; E->pac.prox_dir = -1;
                    } else {
                        // fallback mínimo
                        E->pac.vidas = 3; E->lastPac = GetTime();
                    }
                }
            }
        }
    }
}

// ---------------- desenho ----------------
void desenhar_mapa(Estado *E){
    for(int i=0;i<LINHAS;i++){
        for(int j=0;j<COLUNAS;j++){
            int x = j * TAM_BLOCO;
            int y = i * TAM_BLOCO;
            char c = map_get(E->mapa, j, i);
            switch(c){
                case '#': DrawRectangle(x,y,TAM_BLOCO,TAM_BLOCO, BLUE); break;
                case '.': DrawCircle(x+TAM_BLOCO/2, y+TAM_BLOCO/2, 4, WHITE); break;
                case 'o': DrawCircle(x+TAM_BLOCO/2, y+TAM_BLOCO/2, TAM_BLOCO/2 - 8, GREEN); break;
                case '<': case '>': DrawRectangle(x,y,TAM_BLOCO,TAM_BLOCO, PURPLE); break;
                case 'T': DrawRectangle(x,y,TAM_BLOCO,TAM_BLOCO, MAGENTA); break;
                default: DrawRectangleLines(x,y,TAM_BLOCO,TAM_BLOCO, Fade(BLACK, 0.04f)); break;
            }
        }
    }
}

void desenhar_HUD(Estado *E){
    DrawRectangle(0, LINHAS*TAM_BLOCO, AREA_JOGAVEL, ALTURA_HUD, BLACK);

    // --- REQUISITO 1.9: Menu no Rodapé ---
    if(E->tela == TELA_PAUSA){
        DrawText("MENU: (V)oltar  (S)alvar  (C)arregar  (N)ovo  (Q)Sair", 
                 20, LINHAS*TAM_BLOCO + 10, 20, GREEN);
    } 
    else {
        char buf[128];
        sprintf(buf, "Vidas: %d", E->pac.vidas);
        DrawText(buf, 8, LINHAS*TAM_BLOCO + 6, 20, WHITE);
        sprintf(buf, "Pts: %06d", E->pac.pontuacao);
        DrawText(buf, 200, LINHAS*TAM_BLOCO + 6, 20, WHITE);
        sprintf(buf, "Nv: %d", E->nivel);
        DrawText(buf, 450, LINHAS*TAM_BLOCO + 6, 20, WHITE);
        sprintf(buf, "Pellets: %d", E->pellets);
        DrawText(buf, 600, LINHAS*TAM_BLOCO + 6, 20, YELLOW);
        if(E->power) DrawText("POWER", 800, LINHAS*TAM_BLOCO + 6, 20, ORANGE);
    }
}

// ---------------- salvar / carregar ----------------
void salvar(Estado *E){
    FILE *f = fopen(SAVE_FILE, "wb");
    if(!f) return;
    fwrite(&E->nivel, sizeof(int), 1, f);
    fwrite(&E->pac, sizeof(Pacman), 1, f);
    fwrite(&E->pellets, sizeof(int), 1, f);
    fwrite(E->mapa, 1, LINHAS * COLUNAS, f);
    fwrite(&E->nF, sizeof(int), 1, f);
    for(int i=0;i<E->nF;i++) fwrite(&E->fant[i], sizeof(Fantasma), 1, f);
    fclose(f);
}

bool carregar(Estado *E){
    FILE *f = fopen(SAVE_FILE, "rb");
    if(!f) return false;
    fread(&E->nivel, sizeof(int), 1, f);
    fread(&E->pac, sizeof(Pacman), 1, f);
    fread(&E->pellets, sizeof(int), 1, f);
    if(!E->mapa){
        E->mapa = malloc(LINHAS * COLUNAS);
        if(!E->mapa){ fclose(f); return false; }
    }
    fread(E->mapa, 1, LINHAS * COLUNAS, f);
    int nf = 0;
    fread(&nf, sizeof(int), 1, f);
    free(E->fant);
    E->fant = malloc(sizeof(Fantasma) * nf);
    E->nF = nf;
    for(int i=0;i<nf;i++) fread(&E->fant[i], sizeof(Fantasma), 1, f);
    fclose(f);
    E->lastPac = GetTime();
    return true;
}

// ---------------- novo jogo ----------------
void novo_jogo(Estado *E, const char *arquivo){
    E->nivel = 1;
    E->pac.vidas = 3;
    E->pac.pontuacao = 0;
    
    // --- CORREÇÃO: Inicializar movimento zerado
    E->pac.dir = -1; 
    E->pac.prox_dir = -1;

    E->pellets = 0;
    E->power = false;
    E->lastPac = GetTime();

    free(E->mapa); E->mapa = NULL;
    free(E->fant); E->fant = NULL;
    E->nF = 0;

    Fantasma *fv = NULL; int nf = 0, pel = 0;
    Pacman ptmp = E->pac;
    char *m = ler_mapa(arquivo, &ptmp, &fv, &nf, &pel);
    
    if(!m){
        // fallback (cria mapa simples se arquivo falhar)
        m = malloc(LINHAS * COLUNAS);
        for(int i=0;i<LINHAS;i++){
            for(int j=0;j<COLUNAS;j++){
                if(i==0||i==LINHAS-1||j==0||j==COLUNAS-1) map_set(m,j,i,'#');
                else map_set(m,j,i,'.');
            }
        }
        ptmp.pos.x = COLUNAS/2; ptmp.pos.y = LINHAS/2;
        fv = malloc(sizeof(Fantasma)*4);
        for(int k=0;k<4;k++){
            fv[k].pos.x = 1 + k*2; fv[k].pos.y = 1;
            fv[k].dir = GetRandomValue(0,3); fv[k].vuln = false; fv[k].endVuln = 0; fv[k].since = 0; fv[k].alive = true;
        }
        nf = 4; pel = 0;
        for(int i=0;i<LINHAS;i++) for(int j=0;j<COLUNAS;j++) if(map_get(m,j,i)=='.') pel++;
    }

    E->mapa = m;
    E->pac = ptmp;
    E->fant = fv;
    E->nF = nf;
    E->pellets = pel;
}

// ---------------- main ----------------
int main(void){
    // --- CONFIGURAÇÃO AUTOMÁTICA DE TELA ---
    #ifdef MODO_NOTEBOOK
        // Sua configuração (Zoom in)
        int larguraJanela = 1280;
        int alturaJanela = 700;
        InitWindow(larguraJanela, alturaJanela, "Pac-Man (Modo Notebook)");
    #else
        // Configuração Oficial do Trabalho
        InitWindow(AREA_JOGAVEL, AREA_TOTAL, "Pac-Man - TP2");
    #endif

    SetTargetFPS(60);
    srand((unsigned)time(NULL));

    Estado E;
    memset(&E,0,sizeof(Estado));
    E.tela = TELA_MENU;
    E.nivel = 1;
    E.lastPac = GetTime();

    novo_jogo(&E, MAPA1_FILE);

    // --- CÂMERA (ZOOM) ---
    Camera2D cam = { 0 };
    cam.zoom = 1.0f;
    #ifdef MODO_NOTEBOOK
        float scaleX = (float)larguraJanela / AREA_JOGAVEL;
        float scaleY = (float)alturaJanela / AREA_TOTAL;
        cam.zoom = (scaleX < scaleY) ? scaleX : scaleY;
    #endif

    double last = GetTime();
    while(!WindowShouldClose()){
        double now = GetTime();
        float dt = (float)(now - last);
        last = now;

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // ATIVA MODO CÂMERA (Zoom)
        BeginMode2D(cam);

        if(E.tela == TELA_MENU){
            DrawText("Pac-Man - Menu", 260, 220, 40, BLACK);
            DrawText("Pressione:", 260, 280, 20, DARKGRAY);
            DrawText("N: Novo Jogo", 260, 310, 20, DARKGRAY);
            DrawText("C: Carregar", 260, 340, 20, DARKGRAY);
            DrawText("Q: Sair", 260, 370, 20, DARKGRAY);

            if(IsKeyPressed(KEY_N)){ novo_jogo(&E, MAPA1_FILE); E.tela = TELA_JOGO; }
            if(IsKeyPressed(KEY_C)){ if(carregar(&E)) E.tela = TELA_JOGO; }
            if(IsKeyPressed(KEY_Q)) break;
        }
        else if(E.tela == TELA_JOGO){
            // --- REQUISITO 1.1: INPUT NO BUFFER (60 FPS) ---
            if(IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))    E.pac.prox_dir = 0;
            else if(IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) E.pac.prox_dir = 1;
            else if(IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) E.pac.prox_dir = 2;
            else if(IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) E.pac.prox_dir = 3;

            // MOVIMENTAÇÃO CONTROLADA POR TEMPO
            double pac_now = GetTime();
            if(pac_now - E.lastPac >= STEP_PAC){
                
                // 1. Tenta virar (Buffer)
                int dx = 0, dy = 0;
                if(E.pac.prox_dir == 0) dy = -1; else if(E.pac.prox_dir == 1) dy = 1;
                else if(E.pac.prox_dir == 2) dx = -1; else if(E.pac.prox_dir == 3) dx = 1;

                if(E.pac.prox_dir != -1 && pode_mover(E.mapa, E.pac.pos.x + dx, E.pac.pos.y + dy)){
                    E.pac.dir = E.pac.prox_dir;
                }
                
                // 2. Tenta seguir reto (Inércia)
                dx = 0; dy = 0;
                if(E.pac.dir == 0) dy = -1; else if(E.pac.dir == 1) dy = 1;
                else if(E.pac.dir == 2) dx = -1; else if(E.pac.dir == 3) dx = 1;

                if(E.pac.dir != -1 && pode_mover(E.mapa, E.pac.pos.x + dx, E.pac.pos.y + dy)){
                    E.pac.pos.x += dx; E.pac.pos.y += dy;
                    usar_portal(E.mapa, &E.pac.pos, E.pac.dir);
                    ao_mover_pac(&E);
                }
                E.lastPac = pac_now;
            }

            mover_fantasmas(&E, dt);

            if(E.power && GetTime() >= E.endPower){
                E.power = false;
                for(int i=0;i<E.nF;i++) if(E.fant[i].vuln && GetTime() >= E.fant[i].endVuln) E.fant[i].vuln = false;
            }

            checar_colisoes(&E);

            if(E.pellets <= 0){
                E.nivel++;
                if(E.nivel == 2){ // Tenta mapa 2
                    free(E.mapa); free(E.fant);
                    Pacman ptmp = E.pac; Fantasma *fv = NULL; int nf=0, pel=0;
                    char *m = ler_mapa(MAPA2_FILE, &ptmp, &fv, &nf, &pel);
                    if(m){ 
                        E.mapa = m; E.pac = ptmp; E.fant = fv; E.nF = nf; E.pellets = pel; 
                        E.lastPac = GetTime(); E.pac.dir = -1; E.pac.prox_dir = -1;
                    } 
                    else { novo_jogo(&E, MAPA1_FILE); }
                } else { 
                    novo_jogo(&E, MAPA1_FILE); 
                }
            }

            desenhar_mapa(&E);
            
            for(int i=0;i<E.nF;i++){
                if(!E.fant[i].alive) continue;
                Color cor = E.fant[i].vuln ? SKYBLUE : RED;
                DrawCircle(E.fant[i].pos.x*TAM_BLOCO + TAM_BLOCO/2, E.fant[i].pos.y*TAM_BLOCO + TAM_BLOCO/2, TAM_BLOCO/2 - 4, cor);
            }

            DrawCircle(E.pac.pos.x*TAM_BLOCO + TAM_BLOCO/2, E.pac.pos.y*TAM_BLOCO + TAM_BLOCO/2, TAM_BLOCO/2 - 2, YELLOW);
            
            desenhar_HUD(&E);

            if(IsKeyPressed(KEY_TAB)) E.tela = TELA_PAUSA;
        }
        else if(E.tela == TELA_PAUSA){
            desenhar_HUD(&E); // Mostra menu no rodapé

            // --- REQUISITO 1.9: Opções do Menu ---
            if(IsKeyPressed(KEY_V) || IsKeyPressed(KEY_TAB)) E.tela = TELA_JOGO;
            if(IsKeyPressed(KEY_S)) { salvar(&E); E.tela = TELA_JOGO; }
            if(IsKeyPressed(KEY_C)) { if(carregar(&E)) E.tela = TELA_JOGO; }
            if(IsKeyPressed(KEY_N)) { novo_jogo(&E, MAPA1_FILE); E.tela = TELA_JOGO; }
            if(IsKeyPressed(KEY_Q)) break;
        }

        EndMode2D(); // Fecha modo Câmera

        EndDrawing();
    }

    free(E.mapa);
    free(E.fant);
    CloseWindow();
    return 0;
}
