#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "DatosMemCompartida.h"

int main(int argc, char **argv) {
    //puntero a DatosMemCompartida
    DatosMemCompartida *datos = nullptr;

    //fichero proyectado en memoria creado anteriormente en el tenis (open) y proyectarlo en memoria (mmap)
    int shm_fd = shm_open("/shm_tenis", O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Bot: Error al abrir el objeto de memoria compartida");
        return 1;
    }

    datos = (DatosMemCompartida *)mmap(NULL, sizeof(DatosMemCompartida), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (datos == MAP_FAILED) {
        perror("Bot: Error en mmap");
        return 1;
    }
    close(shm_fd);
    while (true) {
        usleep(25000);

        //comparar la coordenadas
        if (datos->raqueta1.y1 < datos->esfera.centro.y) {
            datos->accion = 1;  // Mover hacia arriba ('w')
        } else if (datos->raqueta1.y1 > datos->esfera.centro.y) {
            datos->accion = -1; // Mover hacia abajo ('s')
        } else {
            datos->accion = 0;  // Quieto / nada
        }
    }
    munmap(datos, sizeof(DatosMemCompartida));
    return 0;
}
