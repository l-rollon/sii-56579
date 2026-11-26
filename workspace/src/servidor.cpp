#include "glut.h"
#include "MundoServidor.h"

MundoServidor servidor;

void OnDraw(void) {
    servidor.OnDraw();
}

void OnTimer(int value) {
    servidor.OnTimer(value);
    glutTimerFunc(25, OnTimer, 0);
    glutPostRedisplay();
}

int main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitWindowSize(800, 600);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutCreateWindow("Servidor - Tenis");

    glutDisplayFunc(OnDraw);
    glutTimerFunc(25, OnTimer, 0);
    glutSetCursor(GLUT_CURSOR_FULL_CROSSHAIR);

    servidor.InitGL();
    glutMainLoop();

    return 0;
}
