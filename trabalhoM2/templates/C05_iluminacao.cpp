#include <iostream>
#include <fstream>
#include <GL/freeglut.h>
#include <string>
using namespace std;

void Draw() {
	glClear(GL_COLOR_BUFFER_BIT);

	// determina as propriedades do material
	GLfloat cor_verde[] = { 0.0, 1.0, 0.0, 1.0 };
	GLfloat cor_branco[] = { 1.0, 1.0, 1.0, 1.0 };
	glMaterialfv(GL_FRONT, GL_DIFFUSE, cor_verde);
	glMaterialfv(GL_FRONT, GL_SPECULAR, cor_branco);
	glMaterialf(GL_FRONT, GL_SHININESS, 100);

	// desenha um quadrado a partir de uma malha de quadrados menores
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, 1.0);
	const GLfloat kqDelta = .01;
	for (int i = -90; i < 90; ++i) {
		for (int j = -90; j < 90; ++j) {
			glVertex3f(j * kqDelta, i * kqDelta, -.2);
			glVertex3f((j + 1) * kqDelta, i * kqDelta, -.2);
			glVertex3f((j + 1) * kqDelta, (i + 1) * kqDelta, -.2);
			glVertex3f(j * kqDelta, (i + 1) * kqDelta, -.2);
		}
	}
	glEnd();

	glFlush();
}

void Initialize() {
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);

	//Ativa o uso de luz
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// determina a intensidade e cor da luz
	GLfloat luz_ambiente[] = { 0.2, 0.2, 0.2, 1.0 };
	GLfloat luz_difusa[] = { 0.8, 0.8, 0.8, 1.0 };
	GLfloat luz_especular[] = { 1.0, 1.0, 1.0, 1.0 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, luz_ambiente);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, luz_difusa);
	glLightfv(GL_LIGHT0, GL_SPECULAR, luz_especular);

	//determina a posiзгo da luz
	GLfloat posicao_luz[] = { .5, .5, 0.0, 1.0 };
	glLightfv(GL_LIGHT0, GL_POSITION, posicao_luz);
}

int main(int iArgc, char** cppArgv) {
	glutInit(&iArgc, cppArgv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(250, 250);
	glutInitWindowPosition(200, 200);
	glutCreateWindow("Iluminaзгo");
	Initialize();
	glutDisplayFunc(Draw);
	glutMainLoop();
	return 0;
}