// **********************************************************************
//	DesenhaPontos-Linhas.cpp
//  Programa de testes com OpenGL 3D
//	Marcio Sarroglia Pinho
//  pinho@pucrs.br
// **********************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef WIN32
#include <windows.h>    // includes only in MSWindows not in UNIX
#include "gl/glut.h"
#endif

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#define TAM_MAX 20
#define BUFFER_SIZE 200

typedef struct{
    float x,y,z;
} Ponto;

typedef struct{
    Ponto ponto;
    int identificador;
    bool visivel;
} Marcador;

typedef struct{
    Marcador marcadores[10];
    int identificador;
} Frame;

typedef struct {
    int verticeInicial, verticeFinal;
} Segmento;

typedef struct {
    Frame frames[30000];
} Filme;

Filme filme;
int total_de_marcadores, frame_atual, total_de_frames, quantidade_de_segmentos;
Segmento ligacoes_visiveis_ou_nao[TAM_MAX];

GLfloat ratio;
GLfloat angY, angX;

Ponto Marcadores3D[TAM_MAX];
Segmento Ligacoes[TAM_MAX];
Ponto Obs, Alvo;
Ponto ObsIni;

int width,height;
float deltax=0, deltay=0;
GLfloat angle=60, fAspect=1.0;
GLfloat rotX=0, rotY=0, rotX_ini=0, rotY_ini=0;
int x_ini=0,y_ini=0,bot=0;

// **********************************************************************
//  Gera um conjunto de pontos aleatorio e armazena este pontos
//  em um vetor
// **********************************************************************
void GeraPontosAleatorios()
{
    frame_atual = 0;

    char buffer[BUFFER_SIZE], str1[TAM_MAX], str2[TAM_MAX];
    char *nome_arquivo = "4e.spd";
    FILE *fp = fopen(nome_arquivo, "r");
    if(fp == NULL)
        printf("Erro! Não foi possível abrir o arquivo %s.", nome_arquivo);
    else {
        //Ignora as primeiras quatro linhas do arquivo
        fgets(buffer, BUFFER_SIZE, fp);
        fgets(buffer, BUFFER_SIZE, fp);
        fgets(buffer, BUFFER_SIZE, fp);
        fgets(buffer, BUFFER_SIZE, fp);

        //le o numero de marcadores esperados em cada frame
        fscanf(fp, "%s %d\n", str1, &total_de_marcadores);

        //le o numero de frames
        fscanf(fp, "%s %s %d\n", str1, str2, &total_de_frames);

        //le o numero de frames esperado
        int i = 0;
        for (i = 0; i < total_de_frames; i++) {
            int identificador_frame, constante_zero, marcadores_neste_frame;
            float tempo;
            fscanf(fp, "%s %d %f %d %d", str1, &identificador_frame, &tempo, &constante_zero, &marcadores_neste_frame);
            filme.frames[i].identificador = identificador_frame;
            if (marcadores_neste_frame != total_de_marcadores)
                printf("Warning: frame %d possui %d marcadores. Esperava %d.\n", identificador_frame, marcadores_neste_frame, total_de_marcadores);

            int j, identificador_marcador;
            float x, y, z;
            for (j = 0; j < marcadores_neste_frame; j++) {
                fscanf(fp, "%f %f %f %d\n", &x, &y, &z, &identificador_marcador);
                filme.frames[i].marcadores[identificador_marcador].identificador = identificador_marcador;
                filme.frames[i].marcadores[identificador_marcador].ponto.x = x;
                filme.frames[i].marcadores[identificador_marcador].ponto.y = y;
                filme.frames[i].marcadores[identificador_marcador].ponto.z = z;
                filme.frames[i].marcadores[identificador_marcador].visivel = true;
            }
        }

        //le a quantidade de segmentos
        fscanf(fp, "%s %d\n", str1, &quantidade_de_segmentos);

        // Armazena as ligacoes (visiveis ou nao)
        int j, marcador1, marcador2;
        for (j = 0; j < quantidade_de_segmentos; j++) {
            fscanf(fp, "%s %d %d\n", str1, &marcador1, &marcador2);
            ligacoes_visiveis_ou_nao[j].verticeInicial = marcador1;
            ligacoes_visiveis_ou_nao[j].verticeFinal = marcador2;
        }
    }

    atualiza_marcadores();
    atualiza_ligacoes();

    // Define a posição para Alvo e Observador em função dos pontos gerados
    Obs.x = 0;
    Obs.y = 0.5;
    Obs.z = -2.7;
    rotX = 39;
    rotY = 110;

}

void atualiza_marcadores();
void atualiza_marcadores()
{
    int i;
    for (i=0; i< total_de_marcadores; i++)
    {
        Marcadores3D[i].x = filme.frames[frame_atual].marcadores[i].ponto.x;
        Marcadores3D[i].y = filme.frames[frame_atual].marcadores[i].ponto.y;
        Marcadores3D[i].z = filme.frames[frame_atual].marcadores[i].ponto.z;
    }
}

void atualiza_ligacoes();
void atualiza_ligacoes()
{
    //Limpa a lista de ligacoes que serao renderizadas
    int j;
    for (j = 0; j < quantidade_de_segmentos; j++) {
        Ligacoes[j].verticeInicial = 0;
        Ligacoes[j].verticeFinal = 0;
    }

    //armazena apenas os segmentos que devem ser renderizados (estao visiveis)
    for (j = 0; j < quantidade_de_segmentos; j++) {
        int inicio = ligacoes_visiveis_ou_nao[j].verticeInicial;
        int fim = ligacoes_visiveis_ou_nao[j].verticeFinal;

        if (filme.frames[frame_atual].marcadores[inicio].visivel && filme.frames[frame_atual].marcadores[fim].visivel) {
            Ligacoes[j].verticeInicial = ligacoes_visiveis_ou_nao[j].verticeInicial;
            Ligacoes[j].verticeFinal = ligacoes_visiveis_ou_nao[j].verticeFinal;
        }
    }
}

// **********************************************************************
//  Desenha um ponto em 3D
// **********************************************************************
void DesenhaPonto(Ponto P)
{
    glPointSize(3);
    glBegin(GL_POINTS);
        glVertex3f(P.x, P.y, P.z);
    glEnd();
}
// **********************************************************************
//  Desenha um vetor de pontos
// **********************************************************************
void DesenhaVetorPontos(Ponto V[], int qtd)
{
    int i;
    for (i=0; i<qtd; i++)
    {
        DesenhaPonto(V[i]);
    }
}
// **********************************************************************
//  Desenha linha entre dois pontos
// **********************************************************************
void DesenhaSegmento(Ponto P1, Ponto P2)
{
    glBegin(GL_LINES);
        glVertex3f(P1.x, P1.y, P1.z);
        glVertex3f(P2.x, P2.y, P2.z);
    glEnd();

}
// **********************************************************************
//  Desenha um vetor de pontos
// **********************************************************************
void DesenhaVetorSegmentos(Segmento V[], int qtd)
{
    //printf("=========\n");
    Ponto P1, P2;
    int inicio, fim;
    int i;
    for (i=0; i<qtd; i++)
    {
        inicio = Ligacoes[i].verticeInicial;
        fim = Ligacoes[i].verticeFinal;
        P1 = Marcadores3D[inicio];
        P2 = Marcadores3D[fim];
        DesenhaSegmento(P1, P2);
        //printf("%d %d\n", inicio, fim);
    }
}

// **********************************************************************
//  Desenha um quadriculado para representar um piso
// **********************************************************************
void DesenhaPiso()
{
    const float LARG = 4.0;
    int qtd = 40;
    float delta = (2*LARG)/(qtd-1);
    float z = -LARG;

    int i;
    for (i=0; i<qtd; i++)
    {
        glBegin(GL_LINES);
        glVertex3f(-LARG,0,z);
        glVertex3f(LARG,0,z);
        glEnd();
        glBegin(GL_LINES);
        glVertex3f(z,0,-LARG);
        glVertex3f(z,0,LARG);
        glEnd();

        z += delta;
    }

}
// **********************************************************************
//  Desenha os eixos coordenados
// **********************************************************************
void DesenhaEixos()
{
    glBegin(GL_LINES);
    glColor3f(1,0,0); // vermelho

    glVertex3f(0,0,0); // Eixo X
    glVertex3f(5,0,0);

    glVertex3f(0,0,0); // Eixo Y
    glVertex3f(0,5,0);

    glVertex3f(0,0,0); // Eixo X
    glVertex3f(0,0,5);
    glEnd();

}

// Função callback para eventos de botões do mouse
void mouse(int button, int state, int x, int y)
{
	if(state==GLUT_DOWN)
	{
		// Salva os parâmetros atuais
		x_ini = x;
		y_ini = y;
		ObsIni = Obs;
		rotX_ini = rotX;
		rotY_ini = rotY;
		bot = button;
	}
	else bot = -1;
}

// Função callback para eventos de movimento do mouse
#define SENS_ROT	5.0
#define SENS_OBS	20.0
void move(int x, int y)
{
	// Botão esquerdo ?
	if(bot==GLUT_LEFT_BUTTON)
	{
		// Calcula diferenças
		int deltax = x_ini - x;
		int deltay = y_ini - y;
		// E modifica ângulos
		rotY = rotY_ini - deltax/SENS_ROT;
		rotX = rotX_ini - deltay/SENS_ROT;
	}
	// Botão direito ?
	else if(bot==GLUT_RIGHT_BUTTON)
	{
		// Calcula diferença
		int deltaz = y_ini - y;
		// E modifica distância do observador
		//Obs.x = x;
		//Obs.y = y;
		Obs.z = ObsIni.z - deltaz/SENS_OBS;
	}
	//PosicionaObservador();
	glutPostRedisplay();
}

// **********************************************************************
//  void PosicUser()
//
// **********************************************************************
void PosicUser()
{
    // Set the clipping volume
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90,ratio,0.01,20);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Especifica posição do observador e do alvo
	glTranslatef(Obs.x, Obs.y, Obs.z);
	glRotatef(rotX,1,0,0);
	glRotatef(rotY,0,1,0);

}
// **********************************************************************
//  void reshape( int w, int h )
//		trata o redimensionamento da janela OpenGL
//
// **********************************************************************
void reshape( int w, int h )
{
    // Prevent a divide by zero, when window is too short
    // (you cant make a window of zero width).
    if(h == 0)
        h = 1;

    ratio = 1.0f * w / h;
    // Reset the coordinate system before modifying
    glMatrixMode(GL_PROJECTION);
    //glLoadIdentity();
    // Set the viewport to be the entire window
    glViewport(0, 0, w, h);

    PosicUser();

}

// **********************************************************************
//  void display( void )
//
//
// **********************************************************************
void display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    PosicUser();

    glMatrixMode(GL_MODELVIEW);

    glRotatef(30,0,1,0); // Gira o cenario todo
    DesenhaEixos();
    glColor3f(0.0f,0.0,1.0f); // azul
    DesenhaPiso();

    glPushMatrix();
    //glRotatef(angX,1,0,0);
    glRotatef(angY,0,1,0);
    glColor3f(1.0f,1.0,0.0f); // amarelo
    DesenhaVetorPontos(Marcadores3D,TAM_MAX);
    DesenhaVetorSegmentos(Ligacoes, TAM_MAX);
    glPopMatrix();

    glutSwapBuffers();
}

// **********************************************************************
//  void keyboard ( unsigned char key, int x, int y )
//
//
// **********************************************************************
void keyboard ( unsigned char key, int x, int y )
{
    switch ( key )
    {
    case 27:        // Termina o programa qdo
        exit ( 0 );   // a tecla ESC for pressionada
        break;
    case 112: //tecla P
        if (frame_atual < total_de_frames - 1) {
            frame_atual++;
            atualiza_marcadores();
            atualiza_ligacoes();
            display();
        }
        break;
    case 111: //tecla O
        if (frame_atual > 0) {
            frame_atual--;
            atualiza_marcadores();
            atualiza_ligacoes();
            display();
        }
        break;
    default:
        break;
    }

    printf("Frame atual: %d\n", frame_atual);
    int i;
    for (i = 0; i < total_de_marcadores; i++) {
        float x = filme.frames[frame_atual].marcadores[i].ponto.x;
        float y = filme.frames[frame_atual].marcadores[i].ponto.y;
        float z = filme.frames[frame_atual].marcadores[i].ponto.z;
        printf("Marcador %d: \t%f \t%f \t%f \n", i, x, y, z);
    }
    printf("\n");
}

// **********************************************************************
//  void arrow_keys ( int a_keys, int x, int y )
//
//
// **********************************************************************
void arrow_keys ( int a_keys, int x, int y )
{
    float passo = 3.0;
    switch ( a_keys )
    {
    case GLUT_KEY_RIGHT:
        angY += passo;
        if (angY > 360)
            angY = 0;
        glutPostRedisplay();
        break;
    case GLUT_KEY_LEFT:
        angY -= passo;
        if (angY < 0)
            angY = 360;
        glutPostRedisplay();
        break;
    default:
        break;
    }
}
// **********************************************************************
//  void init(void)
//		Inicializa os parâmetros globais de OpenGL
//
// **********************************************************************
void init(void)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Fundo de tela preto

    glShadeModel(GL_SMOOTH);
    glColorMaterial ( GL_FRONT, GL_AMBIENT_AND_DIFFUSE );
    glEnable(GL_DEPTH_TEST);
    glEnable ( GL_CULL_FACE );

    angX = 0.0;
    angY = 0.0;

}
// **********************************************************************
//  void main ( int argc, char** argv )
//
//
// **********************************************************************
int main ( int argc, char** argv )
{
    glutInit            ( &argc, argv );
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB );
    glutInitWindowPosition (0,0);

    // Define o tamanho inicial da janela grafica do programa
    glutInitWindowSize  ( 650, 500);

    // Cria a janela na tela, definindo o nome da
    // que aparecera na barra de título da janela.
    glutCreateWindow    ("Point Loader" );

    // executa algumas inicializações
    init ();

    GeraPontosAleatorios();

    // Define que o tratador de evento para
    // o redesenho da tela. A funcao "display"
    // será chamada automaticamente quando
    // for necessário redesenhar a janela
    glutDisplayFunc ( display );

    // Define que a função que irá rodar a
    // continuamente. Esta função é usada para fazer animações
    // A funcao "display" será chamada automaticamente
    // sempre que for possível
    //glutIdleFunc ( display );

    // Define que o tratador de evento para
    // o redimensionamento da janela. A funcao "reshape"
    // será chamada automaticamente quando
    // o usuário alterar o tamanho da janela
    glutReshapeFunc ( reshape );

    // Define que o tratador de evento para
    // as teclas. A funcao "keyboard"
    // será chamada automaticamente sempre
    // o usuário pressionar uma tecla comum
    glutKeyboardFunc ( keyboard );

    // Define que o tratador de evento para
    // as teclas especiais(F1, F2,... ALT-A,
    // ALT-B, Teclas de Seta, ...).
    // A funcao "arrow_keys" será chamada
    // automaticamente sempre o usuário
    // pressionar uma tecla especial
    glutSpecialFunc ( arrow_keys );

    // Registra a função callback para eventos de botões do mouse
	glutMouseFunc(mouse);

	// Registra a função callback para eventos de movimento do mouse
	glutMotionFunc(move);

    // inicia o tratamento dos eventos
    glutMainLoop ( );
}



