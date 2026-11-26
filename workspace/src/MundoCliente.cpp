#include "MundoCliente.h"
#include "glut.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define FIFO_COORDS "/tmp/fifo_coords"
#define FIFO_TECLAS "/tmp/fifo_teclas"

MundoCliente::MundoCliente()
{
    fifo_coords_fd = -1;
    fifo_teclas_fd = -1;
    shm_fd = -1;
    datos = nullptr;
    Init();
}

MundoCliente::~MundoCliente()
{
    if (fifo_coords_fd >= 0) {
        close(fifo_coords_fd);
    }
    if (fifo_teclas_fd >= 0) {
        close(fifo_teclas_fd);
        unlink(FIFO_TECLAS);
    }
    if (datos != nullptr && datos != MAP_FAILED) {
        munmap(datos, sizeof(DatosMemCompartida));
    }
    if (shm_fd >= 0) {
        close(shm_fd);
    }
}

void MundoCliente::InitGL()
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

void MundoCliente::OnDraw()
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

void MundoCliente::OnTimer(int value)
{
    if (fifo_coords_fd >= 0) {
        char cad[256];
        ssize_t n = read(fifo_coords_fd, cad, sizeof(cad) - 1);
        if (n > 0) {
            cad[n] = '\0';
            sscanf(cad, "%f %f %f %f %f %f %f %f %f %f",
                   &esfera.centro.x, &esfera.centro.y,
                   &jugador1.x1, &jugador1.y1, &jugador1.x2, &jugador1.y2,
                   &jugador2.x1, &jugador2.y1, &jugador2.x2, &jugador2.y2);
        }
    }

    if (datos != nullptr && datos != MAP_FAILED) {
        datos->esfera = esfera;
        datos->raqueta1 = jugador1;

        if (datos->accion == 1) {
            OnKeyboardDown('w', 0, 0);
        } else if (datos->accion == -1) {
            OnKeyboardDown('s', 0, 0);
        }
    }
    
}

void MundoCliente::OnKeyboardDown(unsigned char key, int x, int y)
{
    if (fifo_teclas_fd >= 0) {
        char cad[16];
        sprintf(cad, "%c\n", key);
        write(fifo_teclas_fd, cad, strlen(cad));
    }
}

void MundoCliente::Init()
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

    fifo_coords_fd = open(FIFO_COORDS, O_RDONLY);
    if (fifo_coords_fd == -1) {
        perror("Cliente: error al abrir FIFO_COORDS en modo lectura");
    }

    unlink(FIFO_TECLAS);
    if (mkfifo(FIFO_TECLAS, 0666) < 0) {
        perror("Cliente: error al crear FIFO_TECLAS");
    }
    fifo_teclas_fd = open(FIFO_TECLAS, O_WRONLY);
    if (fifo_teclas_fd == -1) {
        perror("Cliente: error al abrir FIFO_TECLAS en modo escritura");
    }

    shm_unlink("/shm_tenis");
    shm_fd = shm_open("/shm_tenis", O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Cliente: error al abrir memoria compartida");
    }
    if (ftruncate(shm_fd, sizeof(DatosMemCompartida)) == -1) {
        perror("Cliente: error al dar tamaño a memoria compartida");
    }
    datos = (DatosMemCompartida *)mmap(NULL, sizeof(DatosMemCompartida),
                                       PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (datos == MAP_FAILED) {
        perror("Cliente: error en mmap");
    }
    close(shm_fd);
    shm_fd = -1;

    if (datos != nullptr && datos != MAP_FAILED) {
        datos->esfera = esfera;
        datos->raqueta1 = jugador1;
        datos->accion = 0;
    }
}






        

