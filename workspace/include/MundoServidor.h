
#include <vector>
#include "Plano.h"


#pragma once


#include <pthread.h>
#include "Esfera.h"
#include "Raqueta.h"

class MundoServidor
{
public:
    MundoServidor();
    virtual ~MundoServidor();

    void Init();
    void InitGL();
    void OnTimer(int value);
    void OnDraw();
    void RecibeComandosJugador();

    Esfera esfera;
    std::vector<Plano> paredes;
    Plano fondo_izq;
    Plano fondo_dcho;
    Raqueta jugador1;
    Raqueta jugador2;

    int puntos1;
    int puntos2;

    int fifo_logger_fd;
    int fifo_coords_fd;
    int fifo_teclas_fd;

    pthread_t thid1;
};







