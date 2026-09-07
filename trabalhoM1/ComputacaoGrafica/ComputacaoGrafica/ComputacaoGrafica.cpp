#include <iostream>
#include <GL/freeglut.h>
#include <vector>
using vertice = std::pair<double, double>;
using lista_vertices = std::vector<vertice>;
using aresta = std::pair<int, int>;
using lista_arestas = std::vector<aresta>;

struct Poligono {
	double tamanhoLado;
	int numLados;
	vertice posicao;
	std::pair<double, double> escala;
	double rotacao;
	lista_vertices vertices;
	lista_arestas arestas;
};

Poligono criar_poligono(double posicao_x, double posicao_y, double tamanho_lado, int num_lados);
void desenhar(Poligono poligono);
void movimentar(Poligono& poligono, double distancia, double angulo);
void escalar(Poligono& poligono, double escala_x, double escala_y);
void rotacionar(Poligono& poligono, double angulo);

void display();
void redraw(int value);
void keyboard(unsigned char key, int x, int y);
void keyboard_special(int key, int x, int y);

Poligono pentagono;
float velocidadeRotacao = 0;
int delay = 10;

int main(int argc, char** argv) {

	pentagono = criar_poligono(128, 128, 50, 5);

	glutInit(&argc, argv);

	glutInitWindowSize(512, 512);

	glutCreateWindow("Desenhando um Pol�gono");

	glClearColor(1, 1, 1, 0);
	glOrtho(0, 255, 0, 255, -1, 1);

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(keyboard_special);
	glutTimerFunc(10, redraw, 0);

	glutMainLoop();

	return 0;
}


void display(void) {
	glClear(GL_COLOR_BUFFER_BIT);

	desenhar(pentagono);

	glFlush();
}

void keyboard(unsigned char key, int x, int y) {
	std::cout << key;
	switch (key) {
	case 27:
		exit(0);
		break;
	case 'o':
		escalar(pentagono, 0.9, 0.9);
		break;
	case 'p':
		escalar(pentagono, 1.1, 1.1);
		break;
	case 'k':
		velocidadeRotacao -= 0.01;
		break;
	case 'l':
		velocidadeRotacao += 0.01;
		break;
	}
}

void keyboard_special(int key, int x, int y) {
	std::cout << key;
	switch (key) {
	case GLUT_KEY_DOWN:
		movimentar(pentagono, 10, (270 / 180.0) * 3.1416);
		break;

	case GLUT_KEY_UP:
		movimentar(pentagono, 10, (90 / 180.0) * 3.1416);
		break;

	case GLUT_KEY_RIGHT:
		movimentar(pentagono, 10, (0 / 180.0) * 3.1416);
		break;

	case GLUT_KEY_LEFT:
		movimentar(pentagono, 10, (180 / 180.0) * 3.1416);
		break;

	}
}

void redraw(int value) {
	rotacionar(pentagono, velocidadeRotacao);

	glutPostRedisplay();
	glutTimerFunc(delay, redraw, 0);
}

Poligono criar_poligono(double posicao_x, double posicao_y, double tamanho_lado, int num_lados) {
	Poligono novo_poligono;
	novo_poligono.numLados = num_lados;
	novo_poligono.posicao.first = posicao_x;
	novo_poligono.posicao.second = posicao_y;
	novo_poligono.tamanhoLado = tamanho_lado;

	novo_poligono.escala.first = 1;
	novo_poligono.escala.second = 1;

	novo_poligono.rotacao = 0;

	float angulo = 0;
	float passo_angulo = ((360 / float(num_lados)) * 3.1415926536) / 180.0;

	float apothem = tamanho_lado / (2 * tan(3.1416 / float(num_lados)));
	posicao_x -= tamanho_lado / 2.0;
	posicao_y -= apothem;

	novo_poligono.vertices.push_back(vertice(posicao_x, posicao_y));
	std::cout << "Vertices:\n";
	std::cout << 0 << " - " << posicao_x << " - " << posicao_y << "\n";
	for (int i = 1; i < num_lados; i++) {
		posicao_x = posicao_x + tamanho_lado * cos(angulo);
		posicao_y = posicao_y + tamanho_lado * sin(angulo);
		novo_poligono.vertices.push_back(vertice(posicao_x, posicao_y));
		std::cout << i << " - " << posicao_x << " - " << posicao_y << "\n";
		angulo += passo_angulo;
	}

	std::cout << "Arestas:\n";
	for (int i = 0; i < num_lados; i++) {
		novo_poligono.arestas.push_back(aresta(i, (i + 1) % num_lados));
		std::cout << i << " - " << (i + 1) % num_lados << "\n";
	}


	return novo_poligono;
}

void movimentar(Poligono& poligono, double distancia, double angulo) {
	poligono.posicao.first = poligono.posicao.first + distancia * cos(angulo);
	poligono.posicao.second = poligono.posicao.second + distancia * sin(angulo);

	for (int i = 0; i < poligono.numLados; i++) {
		poligono.vertices[i].first = poligono.vertices[i].first + distancia * cos(angulo);
		poligono.vertices[i].second = poligono.vertices[i].second + distancia * sin(angulo);
	}

}

void escalar(Poligono& poligono, double escala_x, double escala_y) {
	poligono.escala.first *= escala_x;
	poligono.escala.second *= escala_y;

	poligono.posicao.first = poligono.posicao.first;
	poligono.posicao.second = poligono.posicao.second;

	for (int i = 0; i < poligono.numLados; i++) {
		poligono.vertices[i].first -= poligono.posicao.first;
		poligono.vertices[i].second -= poligono.posicao.second;

		poligono.vertices[i].first = poligono.vertices[i].first * escala_x;
		poligono.vertices[i].second = poligono.vertices[i].second * escala_y;

		poligono.vertices[i].first += poligono.posicao.first;
		poligono.vertices[i].second += poligono.posicao.second;
	}
}

void rotacionar(Poligono& poligono, double angulo) {

	poligono.rotacao += angulo;

	for (int i = 0; i < poligono.arestas.size(); i++) {
		poligono.vertices[i].first -= poligono.posicao.first;
		poligono.vertices[i].second -= poligono.posicao.second;

		float novoX = poligono.vertices[i].first * cos(angulo) - poligono.vertices[i].second * sin(angulo);
		float novoY = poligono.vertices[i].second = poligono.vertices[i].first * sin(angulo) + poligono.vertices[i].second * cos(angulo);

		poligono.vertices[i].first = novoX;
		poligono.vertices[i].second = novoY;

		poligono.vertices[i].first += poligono.posicao.first;
		poligono.vertices[i].second += poligono.posicao.second;
	}

}

void desenhar(Poligono poligono) {
	glColor3f(0, 0, 0);
	glBegin(GL_LINES);
	for (int i = 0; i < poligono.arestas.size(); i++) {
		float v_o = poligono.arestas[i].first;
		float v_d = poligono.arestas[i].second;

		glVertex2f(poligono.vertices[v_o].first, poligono.vertices[v_o].second);
		glVertex2f(poligono.vertices[v_d].first, poligono.vertices[v_d].second);
	}
	glEnd();
}