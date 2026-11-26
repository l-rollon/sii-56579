#include "glut.h"
#include "MundoCliente.h"

MundoCliente cliente;

void OnDraw(void) {
    cliente.OnDraw();
}

void OnTimer(int value) {
    cliente.OnTimer(value);
    glutTimerFunc(25, OnTimer, 0);
    glutPostRedisplay();
}

void OnKeyboardDown(unsigned char key, int x, int y) {
    cliente.OnKeyboardDown(key, x, y);
    glutPostRedisplay();
}

int main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitWindowSize(800, 600);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutCreateWindow("Cliente - Tenis");

    glutDisplayFunc(OnDraw);
    glutTimerFunc(25, OnTimer, 0);
    glutKeyboardFunc(OnKeyboardDown);
    glutSetCursor(GLUT_CURSOR_FULL_CROSSHAIR);

    cliente.InitGL();
    glutMainLoop();

    return 0;
}
