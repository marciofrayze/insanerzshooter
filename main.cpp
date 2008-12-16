/*  Copyright 2008 Insanerz
    This file is part of Insanerz Shooter.

    Insanerz Shooter is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Insanerz Shooter is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Insanerz Shooter.  If not, see <http://www.gnu.org/licenses/>. */


#include <stdlib.h>
#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_ttf.h"
#include <iostream>
#include <vector>
#include <string>
#include "SDL/SDL_opengl.h"
#include <time.h>
#include <sstream>
#include <string>
#include <math.h>
#include <SDL/SDL_mixer.h>
//#include "InsanersShooter.h"

using namespace std;
using std::vector;

// variaveis globais (sim, podre.. depois eu tiro isso)
int pontos = 0;


// Método criado para calcular o valor absoluto (ou seja, sem o sinal) de um número
// @param numero número a ser avaliado
double Abs(double numero) {
    if ( numero >= 0 )
        return numero;
    else
        return -numero;
}

// Classe utilizada para controle de tempo. É usada para controlar por exemplo o número de frames por segundo
class Timer {

private:
    //The clock time when the timer started
    int startTicks;

    //The ticks stored when the timer was paused
    int pausedTicks;

    //The timer status
    bool paused;
    bool started;

public:
    Timer() {
        //Initialize the variables
        startTicks = 0;
        pausedTicks = 0;
        paused = false;
        started = false;
    }

    ~Timer() {}

    void start() {
        //Start the timer
        started = true;

        //Unpause the timer
        paused = false;

        //Get the current clock time
        startTicks = SDL_GetTicks();
    }

    void stop() {
        //Stop the timer
        started = false;

        //Unpause the timer
        paused = false;
    }

    void pause() {
        //If the timer is running and isn't already paused
        if ( ( started == true ) && ( paused == false ) ) {
            //Pause the timer
            paused = true;

            //Calculate the paused ticks
            pausedTicks = SDL_GetTicks() - startTicks;
        }
    }

    void unpause() {
        //If the timer is paused
        if ( paused == true ) {
            //Unpause the timer
            paused = false;

            //Reset the starting ticks
            startTicks = SDL_GetTicks() - pausedTicks;

            //Reset the paused ticks
            pausedTicks = 0;
        }
    }

    int get_ticks() {
        //If the timer is running
        if ( started == true ) {
            //If the timer is paused
            if ( paused == true ) {
                //Return the number of ticks when the timer was paused
                return pausedTicks;
            } else {
                //Return the current time minus the start time
                return SDL_GetTicks() - startTicks;
            }
        }

        //If the timer isn't running
        return 0;
    }

    bool is_started() {
        return started;
    }

    bool is_paused() {
        return paused;
    }

};


// Representa uma posição X,Y.
class Posicao {

public:
    // A posição é tratada como float para podermos colocar velocidades não inteiras e esse tipo de coisa.
    // Mas na hora de exibir na tela, será feito um cast pra int.
    float x, y;

    Posicao() {}

    ~Posicao() {}

    Posicao(int novoX, int novoY) {
        x = novoX;
        y = novoY;
    }

};


// Representa um Sprite; ou seja, uma imagem/surface com posição X,Y e que no futuro suportará vários frames (animação)
class IL_Sprite {

public:

    // Quando utilizamos SDL, armazenamos todas as imagens em uma Surface.
    SDL_Surface *surface;
    // Representa a posição X,Y onde o Sprite se encontra na tela.
    Posicao position;
    // Construtor padrao
    IL_Sprite() { }

    ~IL_Sprite() {
    }

    // Construtor
    IL_Sprite(char nomeArquivoImagem[], int larguraFrameNovo) {

        // Carregando a imagem do sprite e colocando ela em um Surface
        //SDL_Surface *tempSurface = SDL_LoadBMP(nomeArquivoImagem);
        SDL_Surface* tempSurface = IMG_Load(nomeArquivoImagem);
        if (!tempSurface) {
            fprintf(stderr, "Não foi possível carregar a imagem %s %s:\n", nomeArquivoImagem, IMG_GetError());
            exit(-1);
        } else {
            // Este comando abaixo serve para otimizar a imagem
            surface = SDL_DisplayFormat( tempSurface );
            SDL_FreeSurface( tempSurface );
        }

        // Em todas as imagens a com a cor 255,0,255 ficará transparente
        Uint32 colorkey = SDL_MapRGB( surface->format, 0xFF, 0, 0xFF );
        SDL_SetColorKey( surface, SDL_SRCCOLORKEY, colorkey );

        // Não usado por enquanto... mais pra frente teremos animações ai essa variavel será usada.
        larguraFrame = larguraFrameNovo;

        // Iniciando o Sprite na posição 0,0
        position.x = 0;
        position.y = 0;
    }

    // Construtor onde você passa direto um ponteiro de uma surface.
    // É recomenado utiliza-lo quando você vai utilizar uma mesma imagem para varios sprites, evitando assim carregar uma mesma imagem várias vezes
    IL_Sprite(SDL_Surface *surfaceNova, int larguraFrameNovo) {

        surface = surfaceNova;

        // Colocando transparencia para a cor de fundo 255,0,255
        Uint32 colorkey = SDL_MapRGB( surface->format, 0xFF, 0, 0xFF );
        SDL_SetColorKey( surface, SDL_SRCCOLORKEY, colorkey );

        // Não é utilizado no momento, será usado no futuro para fazer animações
        larguraFrame = larguraFrameNovo;

        // Posicionando o sprite em 0,0,
        position.x = 0;
        position.y = 0;
    }

    // Método agir será sobreescrito por outras classes.
    // Representa a ação tomada pelos sprites, como sua movimentação e seu comportamento em geral.
    virtual void agir() { }

private:

    // No futuro, cada imagem/surface poderá conter vários frames para realizar animações. No momento esta variavel não é usada.
    int larguraFrame;

};


// Representa uma nave inimiga. Nada mais é do que um Sprite com o método agir implementado e alguns novos atributos.
class Inimigo : public IL_Sprite {

public:

    float velocidade;
    float seno; // usado para fazer movimentações loucas nas naves hehe (seria bom colocar um nome melhor pra essa var)
    int tipo;

    // Construtor padrao
    Inimigo() {
        position.y = -250; // posiciona o inimigo pouco acima do topo da tela
        velocidade = rand()%100; // numero aleatorio entre 0 e 100
        velocidade = (velocidade / 100) + 1; // velocidade será aleatória entre 1 e 2
        seno=(rand()%100 / 100) - 0.5; // iniciando seno com um valor aleatorio entre -0.5 e +0.5
        tipo = 0; // indica o tipo de nave. Depdendo do tipo ela vai ter um comportamento diferente no método agir()
    }

    ~Inimigo() { }

    // Construtor
    // Para evitar ficar criando um monte de imagens, deve-se carregar a imagem antes de criar um inimigo e passar apenas uma referencia para a surface
    // O tipo indica qual será o comportamento do inimigo no método agir()
    Inimigo(IL_Sprite novoSprite, int tipoNovo) {
        tipo = tipoNovo;
        surface = novoSprite.surface;
        position.x = rand() % 730;
        position.y = -250;
        velocidade = rand()%100;
        velocidade = (velocidade / 100) + 1; // velocidade será aleatória entre 1 e 2
        seno=(rand()%100 / 100) - 0.5; // iniciando seno com um valor aleatorio entre -0.5 e +0.5
    }

    // Método responsável pelo comportamento do inimigo.
    // Cada tipo de inimigo tem um comportamento.
    // Talvez fosse mais interessante usar polimorfismo e dividir em várias classes os inimigos, mas tive problemas para fazer isso em C++.
    void agir() {

        if (tipo == 0) {
            position.y = position.y + velocidade;

        } else if (tipo == 1) {
            position.y = position.y + velocidade;
            position.x = position.x + (sin(seno)) + 1;
            seno = seno + 0.1;

        } else if (tipo == 2) {
            position.y = position.y + velocidade;
            position.x = position.x + (sin(seno)) - 1;
            seno = seno + 0.1;

        } else if (tipo == 3) {
            position.x = position.x + (cos(seno) * 5);
            position.y = position.y + (sin(seno) * 5) + velocidade;
            seno = seno + 0.1;

        } else if (tipo == 4) {
            position.x = position.x - (cos(seno) * 5);
            position.y = position.y + (sin(seno) * 5) + velocidade;
            seno = seno + 0.1;

        } else if (tipo == 5) {
            if (position.y < 200) {
                position.x = position.x + (cos(seno) * 5);
                position.y = position.y + (sin(seno) * 5) + velocidade;
                seno = seno + 0.1;
            } else {
                position.y = position.y + velocidade;
            }

        } else if (tipo == 6) {
            position.x = position.x + (cos(seno) * 5);
            position.y = position.y + (sin(seno) * 5) + velocidade;
            seno = seno + 0.03;
        }
        // Se sair da tela, volta para o topo em uma posição X aleatória
        if (position.y > 700) {
            position.y = -250;
            position.x = rand() % 730;
            seno = 0;
        }

    }

};


// Representa um tiro do jogador.
class Tiro {

public:
    int tipo;               // Tipo do tiro (por enquanto só tem um tipo)
    //IL_Sprite spriteTiro;   // Sprite contendo a surface do tiro e tudo mais
    Posicao position;
    Uint32 cor;
    Tiro(int x, int y, IL_Sprite  spriteTiroNovo, int tipoNovo) {
        position.x = x;
        position.y = y;
        tipo = tipoNovo;
        //spriteTiro = spriteTiroNovo;
        //spriteTiro.position.x = x;
        //spriteTiro.position.y = y;
    }

    ~Tiro() { }

    void desenhar(SDL_Surface *tela) {

        SDL_Rect tiroRect;
        tiroRect.x = position.x;
        tiroRect.y = position.y;
        tiroRect.w = 7;
        tiroRect.h = 12;
        //SDL_BlitSurface(spriteTiro.surface, NULL, tela, &rect);
        cor = SDL_MapRGB( tela->format, 0xD4, 0xC4, 0x39 );
        SDL_FillRect(tela, &tiroRect, cor);
    }

    void atualizarPosicao() {
        // tiro para frente centralizado
        if (tipo ==0) {
            position.y = position.y - 5;
        // tiro para frente esquerda
        } else if (tipo ==1) {
            position.x = position.x - 1;
            position.y = position.y - 5;
        // tiro para direita
        } else if (tipo ==2) {
            position.x = position.x + 1;
            position.y = position.y - 5;
        }
    }

};


// Particula
class Particula {

public:

    Posicao posicao;
    float somaX;
    float somaY;
    int corParaFadeR;
    int corParaFadeG;
    int corParaFadeB;
    Uint32 cor;
    int tipo;

    ~Particula() {}

    Particula() {
        corParaFadeR = 0;
        corParaFadeG = 0;
        corParaFadeB = 0;
    }

    Particula(int x, int y, int tipoNovo) {
        tipo = tipoNovo;
        posicao.x = x;
        posicao.y = y;
        somaX = ((float(rand()) / RAND_MAX) - 0.5) * (rand()%6 + 2);
        somaY = ((float(rand()) / RAND_MAX) - 0.5) * (rand()%6 + 2);
        if (Abs(somaX + somaY) < 0.01) {
            somaX = ((float(rand()) / RAND_MAX) - 0.5) * (rand()%6 + 2);
            somaY = ((float(rand()) / RAND_MAX) - 0.5) * (rand()%6 + 2);
        }

        if (tipo == 1 && somaY < 0) {
            somaY = -somaY;
        }

        if (tipo == 0) {
            corParaFadeR = 120 + (rand()%135);
            corParaFadeG = 0;
            corParaFadeB = 0;
        } else if (tipo == 1) {
            corParaFadeR = 229 + (rand()%21);
            corParaFadeG = 148 + (rand()%40);
            corParaFadeB = 6;
        }
    }

    void desenhar(SDL_Surface *tela) {
        SDL_Rect posicaoSDL = SDL_Rect();
        posicaoSDL.x = posicao.x;
        posicaoSDL.y = posicao.y;
        if (tipo == 0) {
            posicaoSDL.h = 5;
            posicaoSDL.w = 5;
        } else if (tipo == 1) {
            posicaoSDL.h = 3;
            posicaoSDL.w = 3;
        }
        cor = SDL_MapRGB( tela->format, corParaFadeR, corParaFadeG, corParaFadeB);
        SDL_FillRect(tela, &posicaoSDL, cor);
        if (corParaFadeR >0) { corParaFadeR--; }
        if (corParaFadeG >0) { corParaFadeG--; }
    }

    void atualizarPosicao() {
        if (tipo == 0) {
            posicao.x = posicao.x + (somaX / 3);
            posicao.y = posicao.y + (somaY / 3);
        } else if (tipo == 1) {
            posicao.y = posicao.y + somaY;
        }
    }

};

// SistemaDeParticulas
class SistemaDeParticulas {

public:

    vector<Particula> particulas;
    int tempoDeVida, tempoDeVidaMaximo;

    SistemaDeParticulas() {
        tempoDeVida = 0;
        tempoDeVidaMaximo = 0;
    }

    SistemaDeParticulas(int x, int y, int tipoNovo, int numParticulas, int tempoDeVidaMaximoNovo) {
        tempoDeVida = 0;
        tempoDeVidaMaximo = tempoDeVidaMaximoNovo;
        if (tipoNovo == 0) {
            for (int i = 0; i < numParticulas; i++) {
                Particula p = Particula(x,y, tipoNovo);
                particulas.push_back(p);
            }
        } else if (tipoNovo == 1) {

            Particula p = Particula(x + rand()%8 + 11,y + 28, tipoNovo);
            particulas.push_back(p);

        }

    }

    ~SistemaDeParticulas() {
        particulas.clear();
    }

    void desenhar(SDL_Surface *tela) {
        for (int i = 0; i < particulas.size(); i++) {
            particulas.at(i).atualizarPosicao();
            particulas.at(i).desenhar(tela);
        }
        tempoDeVida++;
    }

};

// GrupoDeParticulas
class GrupoDeParticulas {

public:

    vector<SistemaDeParticulas> grupo;

    GrupoDeParticulas() {}


    ~GrupoDeParticulas() {}

    void adicionarSistemaParticula(int x, int y, int tipo, int quantidadeDeParticulas, int tempoDeVidaMaximo) {
        SistemaDeParticulas s = SistemaDeParticulas(x, y, tipo, quantidadeDeParticulas, tempoDeVidaMaximo);
        grupo.push_back(s);
    }

    void desenhar(SDL_Surface *tela) {
        for (int i = 0; i < grupo.size(); i++) {
            grupo.at(i).desenhar(tela);
            if (grupo.at(i).tempoDeVida > grupo.at(i).tempoDeVidaMaximo) {
                grupo.erase(grupo.begin() + i);
            }
        }
    }

};

// Um conjunto de tiros, onde todos tem a mesma imagem/surface
class Tiros {

public:
    vector<Tiro> tiros;     // vetor onde os tiros serão armazenados
    IL_Sprite spriteTiro;   // Sprite representando os tiros

    // Construtor padrão
    Tiros() {
        spriteTiro = IL_Sprite("tiro.png", 2);
    }

    ~Tiros() {}

    // Cria um novo tiro e o armazena no vetor de tiros
    void novoTiro(int x, int y, int tipo) {
        Tiro tiro = Tiro(x,y, spriteTiro, tipo);
        tiros.push_back(tiro);
    }

    // Desenha os tiros na tela/surface
    void desenhar(SDL_Surface *tela) {
        for (int i = 0; i < tiros.size(); i++) {
            Tiro tiro = tiros.at(i);
            tiro.desenhar(tela);
        }
    }

    // Move todos os tiros para cima
    void atualizarPosicoes() {
        for (int i = 0; i < tiros.size(); i++) {
            tiros.at(i).atualizarPosicao();
            if (tiros.at(i).position.y  < 50) {
                tiros.erase(tiros.begin() + i);
            }
        }
    }

};


// Representa um conjunto de objetos da classe Inimigo
class GrupoDeInimigos {

public:

    vector<Inimigo> inimigos; // vetor onde são armazenas os inimigos

    // Construtor padrao
    GrupoDeInimigos() { }

    ~GrupoDeInimigos() { }

    // Adiciona um novo inimigo ao grupo
    void adicionar(Inimigo sprite) {
        inimigos.push_back(sprite);
    }

    // Desenha todos os inimigos na tela/surface
    // Este método normalmente não é usado
    void desenhar(SDL_Surface *tela) {
        for (int i = 0; i < inimigos.size(); i++) {
            SDL_Rect rect;
            rect.x = inimigos.at(i).position.x;
            rect.y = inimigos.at(i).position.y;
            SDL_BlitSurface(inimigos.at(i).surface, NULL, tela, &rect);
        }
    }


    // Executa o método agir() de todos os inimigos para que eles se movam, atirem, etc.
    // Este método normalmente não é usado
    void agir(SDL_Surface *tela) {
        for (int i = 0; i < inimigos.size(); i++) {
            inimigos.at(i).agir();
        }
    }


    // Execura o método agir() de todos os inimigos e os desenha na tela/surface
    void agirEdesenhar(SDL_Surface *tela) {
        for (int i = 0; i < inimigos.size(); i++) {
            inimigos.at(i).agir();
            if (inimigos.at(i).position.y > 0) {
                SDL_Rect rect;
                rect.x = inimigos.at(i).position.x;
                rect.y = inimigos.at(i).position.y;
                SDL_BlitSurface(inimigos.at(i).surface, NULL, tela, &rect);
            }
        }
    }

    // Cria um novo inimigo com tipo aleatório e posição X aleatória e o adiciona no vetor de inimigos
    void criarNovoInimigo(IL_Sprite spriteInimigo) {
        spriteInimigo.position.x = rand() % 800;
        spriteInimigo.position.y= -250;
        Inimigo inimigo = Inimigo(spriteInimigo, rand()%7);
        adicionar(inimigo);
    }

    // Verifica se algum dos inimigos foi atingido por um tiro
    // @param tiros objeto do tipo Tiros contendo todos os tiros que serão analisados
    void verificaColisao(Tiros *tiros, GrupoDeParticulas *grupo) {
        // para cada inimigo
        for (int i = 0; i < inimigos.size(); i++) {

            // verifica colisao com cada tiro
            for (int j = 0; j < tiros->tiros.size(); j++) {

                // Para tornar o método mais rápido é feita uma verificação simples de colisão:
                // se os objetos estiverem a um raio menos do que 20, é por que houve colisão
                if ( Abs(tiros->tiros.at(j).position.x - inimigos.at(i).position.x) < 20 && Abs(tiros->tiros.at(j).position.y - inimigos.at(i).position.y) < 20) {
                    grupo->adicionarSistemaParticula(tiros->tiros.at(j).position.x, tiros->tiros.at(j).position.y, 0, 50, 200);
                    if (pontos < 99999) { // atualiza contador de pontos
                        pontos++;
                    }
                    inimigos.erase(inimigos.begin() + i);
                    tiros->tiros.erase(tiros->tiros.begin() + j);
                    break;
                }
            }

        }
    }

    // Esse metodo verifica colisao entre o jogador e um grupo de inimigos.
    // Não deveria estar aqui... num refactory ele deve mudar de lugar no futuro.
    bool verificaColisao(IL_Sprite sprite) {
        for (int i=0; i < inimigos.size(); i++) {
            if (Abs(sprite.position.x - inimigos.at(i).position.x) < 20 && Abs(sprite.position.y - inimigos.at(i).position.y) < 20) {
                return true;
            }
        }

        return false;
    }


};



// Classe responsável pela entrada de comandos do usuário
class IL_Keyboard {

public:
    // Variaveis para o controle das teclas do teclado. Será true quando a tecla respectiva estiver pressionada.
    bool leftPressed, rightPressed, upPressed, downPressed, spacePressed;
    Tiros *tiros;       // Referencia para que seja possível criar um novo tiro
    Timer *shootTimer;  // Timer para colocar um "delay" entre os intervalos da criação de um novo tiro


    // Construtor - passe como parametro o(s) objetos que serão maniputaldos pelo teclado e crie um atributo novo para cada um deles
    IL_Keyboard(IL_Sprite *spriteJogadorNovo, Tiros *tirosNovo) {
        spriteJogador = spriteJogadorNovo;
        tiros = tirosNovo;
        leftPressed = false;
        rightPressed = false;
        upPressed = false;
        downPressed = false;
        spacePressed = false;
        shootTimer = new Timer();
        shootTimer->start();
    }

    ~IL_Keyboard() {
        delete tiros;
        delete shootTimer;
    }

    // Verifica se algum tecla foi pressionada ou solta.
    // Caso usuario tenha pressionado uma tecla, sua variavel respectiva torna-se true. Ficará como false caso o usuário solte a tecla.
    void verificaTeclasPressionadas() {

        if ( SDL_PollEvent(&event) != 0 ) {

            //Verificamos o tipo do evento
            switch (event.type) {
                // Caso tenha fechado a janela
            case SDL_QUIT:
                exit(0); //Fechamos a aplicação
                break;

                // Caso tenha pressionado uma tecla qualquer
            case SDL_KEYDOWN:

                switch (event.key.keysym.sym) {
                case SDLK_LEFT:
                    leftPressed = true;
                    break;
                case SDLK_RIGHT:
                    rightPressed = true;
                    break;
                case SDLK_DOWN:
                    downPressed = true;
                    break;
                case SDLK_UP:
                    upPressed = true;
                    break;
                case SDLK_ESCAPE:
                    exit(0);
                    break;
                case SDLK_SPACE:
                    spacePressed = true;
                    break;

                }
                break; // NÃO apague este break!

                // Caso tenha soltado uma tecla qualquer
            case SDL_KEYUP:
                switch (event.key.keysym.sym) {
                case SDLK_LEFT:
                    leftPressed = false;
                    break;
                case SDLK_RIGHT:
                    rightPressed = false;
                    break;
                case SDLK_DOWN:
                    downPressed = false;
                    break;
                case SDLK_UP:
                    upPressed = false;
                    break;
                case SDLK_SPACE:
                    spacePressed = false;
                    break;
                }
                break;

            }
        }

    }

    // O método verificaTeclasPressionadas() apenas seta as variaveis para true ou false.
    // O método acoesDoTeclado é o responsável por executar de fato as ações
    void acoesDoTeclado() {
        if (leftPressed == true && spriteJogador->position.x > 10) {
            spriteJogador->position.x = spriteJogador->position.x - 3;
        }
        if (rightPressed == true && spriteJogador->position.x < 768) {
            spriteJogador->position.x = spriteJogador->position.x + 3;
        }
        if (upPressed == true && spriteJogador->position.y > 150) {
            spriteJogador->position.y = spriteJogador->position.y - 3;
        }
        if (downPressed == true && spriteJogador->position.y < 560) {
            spriteJogador->position.y = spriteJogador->position.y + 3;
        }
        if (spacePressed == true) {
            // verifica se ja deu tempo para atirar denovo
            if (shootTimer->get_ticks() > 130) {
                shootTimer->start();
                // Se já tiver mais de 300 pontos, liberamos o tiro sextuplo! :D
                if (pontos > 4000) {
                    tiros->novoTiro(spriteJogador->position.x + 42, spriteJogador->position.y - 10, 2);
                    tiros->novoTiro(spriteJogador->position.x + 32, spriteJogador->position.y - 10, 2);
                    tiros->novoTiro(spriteJogador->position.x - 20, spriteJogador->position.y - 10, 1);
                    tiros->novoTiro(spriteJogador->position.x - 10, spriteJogador->position.y - 10, 1);
                    tiros->novoTiro(spriteJogador->position.x + 3, spriteJogador->position.y - 15, 0);
                    tiros->novoTiro(spriteJogador->position.x + 23, spriteJogador->position.y - 15, 0);
                }
                // Se já tiver mais de 200 pontos, liberamos o tiro triplo! :D
                if (pontos > 2000) {
                    tiros->novoTiro(spriteJogador->position.x + 32, spriteJogador->position.y - 10, 2);
                    tiros->novoTiro(spriteJogador->position.x - 10, spriteJogador->position.y - 10, 1);
                    tiros->novoTiro(spriteJogador->position.x + 13, spriteJogador->position.y - 15, 0);
                // Se já tiver mais de 100 pontos, liberamos o tiro duplo! :D
                } else if (pontos > 500) {
                    tiros->novoTiro(spriteJogador->position.x + 3, spriteJogador->position.y - 15, 0);
                    tiros->novoTiro(spriteJogador->position.x + 23, spriteJogador->position.y - 15, 0);
                // Se tiver menos que 100 pontos, terá apenas o tiro normal
                } else {
                    tiros->novoTiro(spriteJogador->position.x + 13, spriteJogador->position.y - 15, 0);
                }

            }
        }

    }

private:

    IL_Sprite *spriteJogador; // Referencia para o sprite do jogador, para que possamos movimenta-lo pelo teclado
    SDL_Event event;          // Objeto usado para capturar os eventos. Coloquei como atributo para não ficar instanciando toda hora.

};


// Classe responvável por exibir as coisas na tela.
class IL_Screen {
public:

    // surface que será exibida ao usuário
    SDL_Surface *surface;

    // quantidade máxima de frames por segundo; estabiliza a velocidade do jogo em computadores muito rápidos
    static const int FRAMES_PER_SECOND = 180;

    // contador de frames
    int frame;

    // Timer controlador do frames por segundo
    Timer *fps;

    // Construtor
    // @param telaCheia caso true, o jogo será executado em fullscreen.
    // @param resolucaoWidth da tela (padrao = 800)
    // @param resolucaoHeight da tela (padrao = 600)
    // @param bits de resolução (padrao = 32)

    IL_Screen() {
        delete surface;
        delete fps;
    }

    IL_Screen(bool telaCheia, int resolucaoWidth = 800, int resolucaoHeight = 600, int bits = 32) {

        frame = 0;
        fps = new Timer();
        fps->start();

        if ( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0 ) {
            fprintf(stderr,"Não foi possível inicializar a SDL: %s\n", SDL_GetError());
            exit(-1);
        }

        /* Setando resolução */
        if (telaCheia == true) {
            surface = SDL_SetVideoMode(resolucaoWidth, resolucaoHeight, bits, SDL_DOUBLEBUF | SDL_FULLSCREEN);
        } else {
            surface = SDL_SetVideoMode(resolucaoWidth, resolucaoHeight, bits, SDL_DOUBLEBUF | SDL_HWSURFACE);
        }


        if ( !surface ) {
            fprintf(stderr, "Não foi possível inicializar modo de vídeo %s\n", SDL_GetError());
            exit(-1);
        }

    }

    void desenhar(IL_Sprite &sprite) {
        //SDL_BlitSurface(sprite.surface, NULL, screen, sprite.position);
        SDL_Rect rect;
        rect.x = sprite.position.x;
        rect.y = sprite.position.y;
        SDL_BlitSurface(sprite.surface, NULL, surface, &rect);
    }


    void atualizar() {
        fps->start();
        frame++;
        if ( fps->get_ticks() < 1000 / FRAMES_PER_SECOND ) {
            //Sleep the remaining frame time
            SDL_Delay( ( 1000 / FRAMES_PER_SECOND ) - fps->get_ticks() );
        }
    }

    void limpar() {
        SDL_FillRect(surface, NULL, 0);
    }

    void desenharHeader() {
        SDL_Rect posicao = SDL_Rect();
        posicao.x = 0;
        posicao.y = 0;
        posicao.h = 40;
        posicao.w = 800;
        SDL_FillRect(surface, &posicao, 100);
    }

};


int main(int argc, char *argv[]) {


    atexit(SDL_Quit);   // Avisa o sistema que antes de sair da aplicação o método SDL_QUIT deve ser executado
    srand(time(NULL));  // Melhora o sistema de geração de números randômicos (caso contrario a sequencia dos numeros gerados é sempre a mesma)

    IL_Screen *screen = new IL_Screen(false);       // Cria uma nova tela
    SDL_ShowCursor(false);                          // Esconde o ponteiro do mouse
    SDL_WM_SetCaption("Insanerz Shooter", NULL);    // Muda o título da janela

    // Criando e posicionando o sprite do jogador (ignore o ultimo parametro, por enquanto não é usado)
    IL_Sprite spriteJogador("nave.bmp", 2);
    spriteJogador.position.x = 456;
    spriteJogador.position.y = 500;

    // Criando e sprite do jogador (ignore o ultimo parametro, por enquanto não é usado)
    IL_Sprite spriteInimigo("nave_inimigo.png", 2);

    // Cria um grupo inicial de inimigos
    GrupoDeInimigos grupoDeInimigos = GrupoDeInimigos();

    Tiros *tiros = new Tiros(); // Cria objeto responsável por controlar os tiros

    IL_Keyboard teclado = IL_Keyboard(&spriteJogador, tiros); // Cria objeto responsável por controlar o teclado

    // Cria uma fonte usado para exibir o nome do jogo e pontuação
    TTF_Font *font = NULL;
    TTF_Init();
    font = TTF_OpenFont( "ariblk.ttf", 28 );
    if (!font) {
        printf("Erro ao carregar a fonte. Está faltando o arquivo ariblk.ttf?");
        exit(-1);
    }

    // Cria uma surface usada para exibir nome do jogo
    SDL_Surface *message = NULL;
    SDL_Rect messageRec;
    messageRec.x = 10;
    messageRec.y = 0;
    SDL_Color textColor = { 255, 255, 255 };
    message = TTF_RenderText_Solid( font, "Insanerz Shooter", textColor );

    // Cria uma surface e um SDL_Rect (posicao) usados para exibir a pontuação do jogador
    SDL_Surface *pontosSurface = NULL;
    pontosSurface = new SDL_Surface;
    pontosSurface = TTF_RenderText_Solid( font, "0", textColor );
    SDL_Rect *posicaoPontos = new SDL_Rect();
    posicaoPontos->x = 500;
    posicaoPontos->y = 0;

    int probDeCriarInimigo; // usado para calcular probabilidade de criar um novo inimigo
    bool jogadorEstaVivo = true; // no futuro ficará em um lugar menos podre. Apenas para testes por enquanto

    GrupoDeParticulas *grupo = new GrupoDeParticulas();

    char pontuacao[5];
    char pontuacao2[13] = "Score: ";

    // iniciando sistema de som
    int frequencia = 22050;
    int canais = 2; // 1 para mono e 2 para stereo.
    int buffer = 4096;
    Uint16 formato = AUDIO_S16; //16 bits stereo
    Mix_OpenAudio ( frequencia, formato, canais, buffer );
    Mix_AllocateChannels(canais);
    Mix_Music *musica = NULL;
    musica = Mix_LoadMUS ( "musica.ogg" );
    Mix_PlayMusic ( musica, -1 );

    while (true) {

        screen->limpar();   // Pinta tudo de preto
        grupoDeInimigos.verificaColisao(tiros, grupo);         // Verifica se jogador matou algum inimigo
        grupoDeInimigos.agirEdesenhar(screen->surface); // Move os inimigos e desenha eles na tela
        tiros->atualizarPosicoes();                     // Move os tiros
        tiros->desenhar(screen->surface);                // Desenha tiros na tela
        teclado.verificaTeclasPressionadas();           // Verifica quais teclas estão sendo pressionadas
        if (jogadorEstaVivo) {
            screen->desenhar(spriteJogador);            // Desenha jogador na tela
            teclado.acoesDoTeclado();                   // Realiza ações do teclado (move a nave, atira, etc)
            // probabilidade de criar particula de "fogo" da nave
            if (rand()%2 == 1) {
                grupo->adicionarSistemaParticula(spriteJogador.position.x, spriteJogador.position.y, 1, 0, 40);
            }
        }

        grupo->desenhar(screen->surface);
        screen->desenharHeader();                       // Desenha a parte superior da tela

        SDL_BlitSurface(message, NULL, screen->surface, &messageRec);   // Exibe nome do jogo


        if (jogadorEstaVivo == true && grupoDeInimigos.verificaColisao(spriteJogador)) {
            jogadorEstaVivo = false;
            grupo->adicionarSistemaParticula(spriteJogador.position.x, spriteJogador.position.y, 0, 500, 200);
        }


        // Esta parte precisa de um refactory... Para exibir a pontuação do usuário, só consegui fazer desta maneira:
        strcpy(pontuacao2, "Pontos: ");
        sprintf(pontuacao,"%i",pontos);
        strcat(pontuacao2, pontuacao);
        pontosSurface = TTF_RenderText_Solid( font, pontuacao2, textColor );
        SDL_BlitSurface(pontosSurface, NULL, screen->surface, posicaoPontos);
        SDL_FreeSurface(pontosSurface);

        // Se necessário, aguarda alguns milisegundos para manter o FPS constante
        screen->atualizar();
        // Exibe o novo cenário ao usuário
        SDL_Flip(screen->surface);

        // se tiver menos do que 30 inimigos, tem certa probabilidade de criar um novo inimigo
        if (grupoDeInimigos.inimigos.size() < 50) {
            if (pontos < 500 && grupoDeInimigos.inimigos.size() < 30) {
                probDeCriarInimigo = rand() % 130;
            } else if (pontos < 2000 && grupoDeInimigos.inimigos.size() < 35) {
                probDeCriarInimigo = rand() % 30;
            } else if (grupoDeInimigos.inimigos.size() < 50) {
                probDeCriarInimigo = rand() % 10;
            }

            if (probDeCriarInimigo == 1) {
                grupoDeInimigos.criarNovoInimigo(spriteInimigo);
            }
        }

    }

    Mix_HaltMusic ( );
    Mix_FreeMusic ( musica );
    Mix_CloseAudio( );

    return 0;

}
