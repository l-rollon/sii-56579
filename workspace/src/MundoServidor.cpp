
#include "MundoServidor.h"
#include "glut.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define FIFO_COORDS "/tmp/fifo_coords"
#define FIFO_TECLAS "/tmp/fifo_teclas"
#define FIFO_LOGGER "/tmp/tuberia"

static void* hilo_comandos(void* d)
{
    MundoServidor* p = (MundoServidor*)d;
    p->RecibeComandosJugador();
    return nullptr;
}

MundoServidor::MundoServidor()
{
    fifo_logger_fd = -1;
    fifo_coords_fd = -1;
    fifo_teclas_fd = -1;
    Init();
}

MundoServidor::~MundoServidor()
{
    if (fifo_logger_fd >= 0) close(fifo_logger_fd);
    if (fifo_coords_fd >= 0) {
        close(fifo_coords_fd);
        unlink(FIFO_COORDS);
    }
    if (fifo_teclas_fd >= 0) {
        close(fifo_teclas_fd);
    }
}

void MundoServidor::InitGL()
{
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);

    glMatrixMode(GL_PROJECTION);
    gluPerspective(40.0, 800 / 600.0f, 0.1, 150);
}

static void print_texto(char *mensaje, int x, int y, float r, float g, float b)
{
    glDisable(GL_LIGHTING);

    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT));

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glColor3f(r, g, b);
    glRasterPos3f(x, glutGet(GLUT_WINDOW_HEIGHT) - 18 - y, 0);
    int len = strlen(mensaje);
    for (int i = 0; i < len; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, mensaje[i]);

    glMatrixMode(GL_TEXTURE);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glEnable(GL_DEPTH_TEST);
}

void MundoServidor::OnDraw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(0.0, 0, 17,
              0.0, 0.0, 0.0,
              0.0, 1.0, 0.0);

    char cad[100];
    sprintf(cad, "Jugador1: %d", puntos1);
    print_texto(cad, 10, 0, 1, 1, 1);
    sprintf(cad, "Jugador2: %d", puntos2);
    print_texto(cad, 650, 0, 1, 1, 1);

    for (size_t i = 0; i < paredes.size(); i++)
        paredes[i].Dibuja();

    fondo_izq.Dibuja();
    fondo_dcho.Dibuja();
    jugador1.Dibuja();
    jugador2.Dibuja();
    esfera.Dibuja();

    glutSwapBuffers();
}

void MundoServidor::RecibeComandosJugador()
{
    if (mkfifo(FIFO_TECLAS, 0666) == -1 && errno != EEXIST) {
        perror("Servidor: error creando FIFO_TECLAS");
    }

    fifo_teclas_fd = open(FIFO_TECLAS, O_RDONLY);
    if (fifo_teclas_fd < 0) {
        perror("Servidor: error abriendo FIFO_TECLAS");
        return;
    }

    while (1) {
        usleep(10);
        char cad[100];
        ssize_t n = read(fifo_teclas_fd, cad, sizeof(cad) - 1);
        if (n > 0) {
            cad[n] = '\0';
            unsigned char key = 0;
            if (sscanf(cad, "%c", &key) == 1) {
                if (key == 's') jugador1.velocidad.y = -4;
                else if (key == 'w') jugador1.velocidad.y = 4;
                else if (key == 'l') jugador2.velocidad.y = -4;
                else if (key == 'o') jugador2.velocidad.y = 4;
                else if (key == ' ') {
                    jugador1.velocidad.y = 0;
                    jugador2.velocidad.y = 0;
                }
            }
        }
    }
}

void MundoServidor::OnTimer(int value)
{
    jugador1.Mueve(0.025f);
    jugador2.Mueve(0.025f);
    esfera.Mueve(0.025f);

    for (size_t i = 0; i < paredes.size(); i++) {
        paredes[i].Rebota(esfera);
        paredes[i].Rebota(jugador1);
        paredes[i].Rebota(jugador2);
    }

    if (jugador1.Rebota(esfera)) {
       if (esfera.velocidad.x < 0) esfera.velocidad.x = -esfera.velocidad.x;
        esfera.centro.x = jugador1.x1 + esfera.radio + 0.05f;
    }

    if (jugador2.Rebota(esfera)) {
       if (esfera.velocidad.x > 0) esfera.velocidad.x = -esfera.velocidad.x;
        esfera.centro.x = jugador2.x1 - esfera.radio - 0.05f;
    }

    if (fondo_izq.Rebota(esfera)) {
        esfera.centro.x = 0;
        esfera.centro.y = rand() / (float)RAND_MAX;
        esfera.velocidad.x = 2 + 2 * rand() / (float)RAND_MAX;
        esfera.velocidad.y = 2 + 2 * rand() / (float)RAND_MAX;
        puntos2++;

        if (fifo_logger_fd >= 0) {
            char mensaje[256];
            sprintf(mensaje, "Jugador 2 marca 1 punto, lleva un total de %d puntos.\n", puntos2);
            write(fifo_logger_fd, mensaje, strlen(mensaje));
        }
    }

    if (fondo_dcho.Rebota(esfera)) {
        esfera.centro.x = 0;
        esfera.centro.y = rand() / (float)RAND_MAX;
        esfera.velocidad.x = -2 - 2 * rand() / (float)RAND_MAX;
        esfera.velocidad.y = -2 - 2 * rand() / (float)RAND_MAX;
        puntos1++;

        if (fifo_logger_fd >= 0) {
            char mensaje[256];
            sprintf(mensaje, "Jugador 1 marca 1 punto, lleva un total de %d puntos.\n", puntos1);
            write(fifo_logger_fd, mensaje, strlen(mensaje));
        }
    }

    if (puntos1 >= 3 || puntos2 >= 3) {
        if (fifo_logger_fd >= 0) {
            char mensaje_fin[256];
            sprintf(mensaje_fin, "Fin de la partida\n");
            write(fifo_logger_fd, mensaje_fin, strlen(mensaje_fin));
        }
        if (fifo_logger_fd >= 0) close(fifo_logger_fd);
        if (fifo_coords_fd >= 0) {
            close(fifo_coords_fd);
            unlink(FIFO_COORDS);
        }
        if (fifo_teclas_fd >= 0) close(fifo_teclas_fd);
        exit(0);
    }

    if (fifo_coords_fd >= 0) {
        char cad[256];
        sprintf(cad, "%f %f %f %f %f %f %f %f %f %f\n",
                esfera.centro.x, esfera.centro.y,
                jugador1.x1, jugador1.y1, jugador1.x2, jugador1.y2,
                jugador2.x1, jugador2.y1, jugador2.x2, jugador2.y2);
        write(fifo_coords_fd, cad, strlen(cad));
    }
   
}

void MundoServidor::Init()
{
    Plano p;
    p.x1 = -7; p.y1 = -5;
    p.x2 = 7;  p.y2 = -5;
    paredes.push_back(p);

    p.x1 = -7; p.y1 = 5;
    p.x2 = 7;  p.y2 = 5;
    paredes.push_back(p);

    fondo_izq.r = 0;
    fondo_izq.x1 = -7; fondo_izq.y1 = -5;
    fondo_izq.x2 = -7; fondo_izq.y2 = 5;

    fondo_dcho.r = 0;
    fondo_dcho.x1 = 7; fondo_dcho.y1 = -5;
    fondo_dcho.x2 = 7; fondo_dcho.y2 = 5;

    jugador1.g = 0;
    jugador1.x1 = -6; jugador1.y1 = -1;
    jugador1.x2 = -6; jugador1.y2 = 1;

    jugador2.g = 0;
    jugador2.x1 = 6; jugador2.y1 = -1;
    jugador2.x2 = 6; jugador2.y2 = 1;

    puntos1 = 0;
    puntos2 = 0;

    fifo_logger_fd = open(FIFO_LOGGER, O_WRONLY);
    if (fifo_logger_fd == -1) {
        perror("Servidor: error al abrir FIFO_LOGGER en modo escritura");
    }

    unlink(FIFO_COORDS);
    if (mkfifo(FIFO_COORDS, 0666) < 0) {
        perror("Servidor: error al crear FIFO_COORDS");
    }
    fifo_coords_fd = open(FIFO_COORDS, O_WRONLY);
    if (fifo_coords_fd == -1) {
        perror("Servidor: error al abrir FIFO_COORDS en modo escritura");
    }

    if (pthread_create(&thid1, NULL, hilo_comandos, this) != 0) {
        perror("Servidor: error al crear hilo de comandos");
    }
}


