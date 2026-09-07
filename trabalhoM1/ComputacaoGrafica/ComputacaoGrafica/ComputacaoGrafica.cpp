/*
 * Computacao Grafica - Cubo 3D em wireframe
 *
 * Adaptacao do trabalho 2D (poligono regular) para 3 dimensoes:
 *  - o cubo e descrito diretamente por 8 vertices e 12 arestas;
 *  - o desenho usa projecao em perspectiva feita "na mao" e apenas GL_LINES;
 *  - movimentar, rotacionar e escalar funcionam nos 3 eixos, sempre em torno
 *    do centro do cubo;
 *  - nao sao usados glTranslate, glRotate ou glScale.
 *
 * Controles:
 *  Setas				move o cubo nos eixos X e Y
 *  Page Up / Down		move o cubo no eixo Z (aproxima / afasta da camera)
 *  W / S				rotaciona em torno do eixo X
 *  A / D				rotaciona em torno do eixo Y
 *  Q / E				rotaciona em torno do eixo Z
 *  Mouse (arrastar)	rotaciona nos eixos X e Y
 *  O / P 				diminui / aumenta a escala
 *  R					liga / desliga o giro automatico
 *  K / L				ajusta a velocidade do giro automatico
 *  Espaco				volta o cubo ao estado inicial
 *  ESC					sai
 */

#include <iostream>
#include <cmath>
#include <vector>
#include <GL/freeglut.h>

 // Um ponto no espaco tridimensional
struct vertice {
	double x;
	double y;
	double z;
};

// Um ponto ja projetado para a tela (2D)
struct ponto2d {
	double x;
	double y;
};

// Uma aresta guarda os indices dos dois vertices que ela liga
using aresta = std::pair<int, int>;
using lista_vertices = std::vector<vertice>;
using lista_arestas = std::vector<aresta>;

// Forma desenhada na tela. Os vertices sao guardados ja no espaco do mundo e as
// transformacoes alteram esses valores diretamente, sempre usando "centro" como
// pivo para respeitar o centro da forma.
struct Poligono {
	double tamanhoLado;      // tamanho da aresta usado na criacao / reset
	vertice centro;          // centro do cubo, pivo das transformacoes
	vertice escala;          // escala acumulada em cada eixo (apenas informativo)
	vertice rotacao;         // rotacao acumulada em cada eixo, em radianos (informativo)
	lista_vertices vertices; // posicao atual de cada vertice
	lista_arestas arestas;   // ligacoes entre vertices (o wireframe)
};

// Parametros da camera virtual usada na projecao em perspectiva. A camera fica
// parada em (0, 0, CAMERA_Z) olhando para -Z; o ponto (0, 0) do mundo cai no
// centro da tela.
const double CAMERA_Z = 400.0;
const double DISTANCIA_FOCAL = 400.0;
const double CENTRO_TELA_X = 128.0;
const double CENTRO_TELA_Y = 128.0;

Poligono criar_cubo(double centro_x, double centro_y, double centro_z, double tamanho_lado);
ponto2d projetar(vertice ponto);
void desenhar(Poligono poligono);
void movimentar(Poligono& poligono, double dx, double dy, double dz);
void escalar(Poligono& poligono, double escala_x, double escala_y, double escala_z);
void rotacionar(Poligono& poligono, double anguloX, double anguloY, double anguloZ);

void display();
void redraw(int value);
void keyboard(unsigned char key, int x, int y);
void keyboard_special(int key, int x, int y);
void mouse(int botao, int estado, int x, int y);
void mouse_arrasto(int x, int y);

Poligono cubo;
double velocidadeRotacao = 0.01; // giro automatico em torno do eixo Y
int delay = 10;

// Estado do mouse, para poder rotacionar o cubo arrastando o cursor
bool arrastando = false;
int mouseX = 0;
int mouseY = 0;

int main(int argc, char** argv) {

	// Cubo centrado na origem do mundo (a projecao leva a origem ao meio da tela)
	cubo = criar_cubo(0, 0, 0, 100);

	// Leve inclinacao inicial so para o cubo ja aparecer em 3D
	rotacionar(cubo, -0.5, 0.6, 0);

	glutInit(&argc, argv);

	// Buffer duplo para o giro nao piscar
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

	glutInitWindowSize(512, 512);

	glutCreateWindow("Cubo 3D em wireframe");

	glClearColor(1, 1, 1, 0);
	glOrtho(0, 255, 0, 255, -1, 1);

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(keyboard_special);
	glutMouseFunc(mouse);
	glutMotionFunc(mouse_arrasto);
	glutTimerFunc(delay, redraw, 0);

	glutMainLoop();

	return 0;
}


void display(void) {
	glClear(GL_COLOR_BUFFER_BIT);

	desenhar(cubo);

	glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 27: // ESC
		exit(0);
		break;

		// Escala em torno do centro (uniforme nos tres eixos)
	case 'o':
		escalar(cubo, 0.9, 0.9, 0.9);
		break;
	case 'p':
		escalar(cubo, 1.1, 1.1, 1.1);
		break;

		// Rotacao em torno do eixo X
	case 'w':
		rotacionar(cubo, -0.1, 0, 0);
		break;
	case 's':
		rotacionar(cubo, 0.1, 0, 0);
		break;

		// Rotacao em torno do eixo Y
	case 'a':
		rotacionar(cubo, 0, -0.1, 0);
		break;
	case 'd':
		rotacionar(cubo, 0, 0.1, 0);
		break;

		// Rotacao em torno do eixo Z
	case 'q':
		rotacionar(cubo, 0, 0, 0.1);
		break;
	case 'e':
		rotacionar(cubo, 0, 0, -0.1);
		break;

		// Giro automatico em torno do eixo Y
	case 'r':
		velocidadeRotacao = (fabs(velocidadeRotacao) < 0.0001) ? 0.01 : 0.0;
		break;
	case 'k':
		velocidadeRotacao -= 0.01;
		break;
	case 'l':
		velocidadeRotacao += 0.01;
		break;

		// Volta o cubo ao estado inicial
	case ' ':
		cubo = criar_cubo(0, 0, 0, 100);
		rotacionar(cubo, -0.5, 0.6, 0);
		break;
	}
}

void keyboard_special(int key, int x, int y) {
	switch (key) {
		// Translacao no plano da tela (eixos X e Y)
	case GLUT_KEY_DOWN:
		movimentar(cubo, 0, -10, 0);
		break;
	case GLUT_KEY_UP:
		movimentar(cubo, 0, 10, 0);
		break;
	case GLUT_KEY_RIGHT:
		movimentar(cubo, 10, 0, 0);
		break;
	case GLUT_KEY_LEFT:
		movimentar(cubo, -10, 0, 0);
		break;

		// Translacao em profundidade (eixo Z): Page Up aproxima, Page Down afasta
	case GLUT_KEY_PAGE_UP:
		movimentar(cubo, 0, 0, 10);
		break;
	case GLUT_KEY_PAGE_DOWN:
		movimentar(cubo, 0, 0, -10);
		break;
	}
}

// Guarda o botao e a posicao do cursor quando o usuario comeca a arrastar
void mouse(int botao, int estado, int x, int y) {
	if (botao == GLUT_LEFT_BUTTON) {
		arrastando = (estado == GLUT_DOWN);
		mouseX = x;
		mouseY = y;
	}
}

// Enquanto o botao esquerdo esta pressionado, transforma o movimento do mouse
// em rotacao: arrasto horizontal gira em torno de Y, arrasto vertical gira em
// torno de X.
void mouse_arrasto(int x, int y) {
	if (!arrastando) return;

	int dx = x - mouseX;
	int dy = y - mouseY;

	rotacionar(cubo, dy * 0.01, dx * 0.01, 0);

	mouseX = x;
	mouseY = y;

	glutPostRedisplay();
}

void redraw(int value) {
	// Giro automatico continuo em torno do eixo Y
	rotacionar(cubo, 0, velocidadeRotacao, 0);

	glutPostRedisplay();
	glutTimerFunc(delay, redraw, 0);
}

// Cria o cubo pela descricao direta dos vertices e arestas. Nao ha calculo de
// posicao: cada canto e apenas o centro somado ou subtraido de meio lado.
Poligono criar_cubo(double centro_x, double centro_y, double centro_z, double tamanho_lado) {
	Poligono novo_cubo;
	novo_cubo.tamanhoLado = tamanho_lado;
	novo_cubo.centro = { centro_x, centro_y, centro_z };
	novo_cubo.escala = { 1, 1, 1 };
	novo_cubo.rotacao = { 0, 0, 0 };

	double m = tamanho_lado / 2.0; // distancia do centro ate cada face

	// Descricao direta dos 8 vertices. O numero ao lado e o indice usado nas arestas.
	novo_cubo.vertices = {
		{ centro_x - m, centro_y - m, centro_z - m }, // 0 - tras,   baixo, esquerda
		{ centro_x + m, centro_y - m, centro_z - m }, // 1 - tras,   baixo, direita
		{ centro_x + m, centro_y + m, centro_z - m }, // 2 - tras,   cima,  direita
		{ centro_x - m, centro_y + m, centro_z - m }, // 3 - tras,   cima,  esquerda
		{ centro_x - m, centro_y - m, centro_z + m }, // 4 - frente, baixo, esquerda
		{ centro_x + m, centro_y - m, centro_z + m }, // 5 - frente, baixo, direita
		{ centro_x + m, centro_y + m, centro_z + m }, // 6 - frente, cima,  direita
		{ centro_x - m, centro_y + m, centro_z + m }, // 7 - frente, cima,  esquerda
	};

	// Descricao direta das 12 arestas do cubo
	novo_cubo.arestas = {
		{ 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 }, // face de tras
		{ 4, 5 }, { 5, 6 }, { 6, 7 }, { 7, 4 }, // face da frente
		{ 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 }, // arestas que ligam as duas faces
	};

	// Mostra a descricao no console, igual a versao 2D fazia
	std::cout << "Vertices:\n";
	for (int i = 0; i < novo_cubo.vertices.size(); i++) {
		std::cout << i << " - " << novo_cubo.vertices[i].x << " - "
			<< novo_cubo.vertices[i].y << " - " << novo_cubo.vertices[i].z << "\n";
	}

	std::cout << "Arestas:\n";
	for (int i = 0; i < novo_cubo.arestas.size(); i++) {
		std::cout << novo_cubo.arestas[i].first << " - " << novo_cubo.arestas[i].second << "\n";
	}

	return novo_cubo;
}

// Projeta um ponto 3D para a tela aplicando perspectiva. Quanto mais perto da
// camera (z maior), maior o "fator" e mais o ponto se afasta do centro; quanto
// mais longe, mais o ponto tende ao centro da tela.
ponto2d projetar(vertice ponto) {
	double profundidade = CAMERA_Z - ponto.z; // distancia entre o ponto e a camera
	if (profundidade < 1.0) {
		profundidade = 1.0; // trava para nao dividir por zero nem inverter a imagem
	}

	double fator = DISTANCIA_FOCAL / profundidade;

	ponto2d resultado;
	resultado.x = CENTRO_TELA_X + ponto.x * fator;
	resultado.y = CENTRO_TELA_Y + ponto.y * fator;
	return resultado;
}

// Move o cubo somando o deslocamento ao centro e a todos os vertices, para que
// a forma inteira ande junta.
void movimentar(Poligono& poligono, double dx, double dy, double dz) {
	poligono.centro.x += dx;
	poligono.centro.y += dy;
	poligono.centro.z += dz;

	for (int i = 0; i < poligono.vertices.size(); i++) {
		poligono.vertices[i].x += dx;
		poligono.vertices[i].y += dy;
		poligono.vertices[i].z += dz;
	}
}

// Escala o cubo em cada eixo. Cada vertice e levado para a origem (subtraindo o
// centro), multiplicado pela escala e devolvido, de modo que a escala aconteca
// em torno do centro do cubo e nao do canto da tela.
void escalar(Poligono& poligono, double escala_x, double escala_y, double escala_z) {
	poligono.escala.x *= escala_x;
	poligono.escala.y *= escala_y;
	poligono.escala.z *= escala_z;

	for (int i = 0; i < poligono.vertices.size(); i++) {
		double x = poligono.vertices[i].x - poligono.centro.x;
		double y = poligono.vertices[i].y - poligono.centro.y;
		double z = poligono.vertices[i].z - poligono.centro.z;

		x *= escala_x;
		y *= escala_y;
		z *= escala_z;

		poligono.vertices[i].x = x + poligono.centro.x;
		poligono.vertices[i].y = y + poligono.centro.y;
		poligono.vertices[i].z = z + poligono.centro.z;
	}
}

// Rotaciona o cubo em torno do seu centro nos tres eixos (angulos em radianos).
// Mesma ideia da versao 2D: leva o vertice para a origem, aplica as rotacoes de
// X, Y e Z em sequencia e devolve o vertice para junto do centro.
void rotacionar(Poligono& poligono, double anguloX, double anguloY, double anguloZ) {
	poligono.rotacao.x += anguloX;
	poligono.rotacao.y += anguloY;
	poligono.rotacao.z += anguloZ;

	for (int i = 0; i < poligono.vertices.size(); i++) {
		double x = poligono.vertices[i].x - poligono.centro.x;
		double y = poligono.vertices[i].y - poligono.centro.y;
		double z = poligono.vertices[i].z - poligono.centro.z;

		// Rotacao em torno de X: altera Y e Z
		double ny = y * cos(anguloX) - z * sin(anguloX);
		double nz = y * sin(anguloX) + z * cos(anguloX);
		y = ny;
		z = nz;

		// Rotacao em torno de Y: altera X e Z
		double nx = x * cos(anguloY) + z * sin(anguloY);
		nz = -x * sin(anguloY) + z * cos(anguloY);
		x = nx;
		z = nz;

		// Rotacao em torno de Z: altera X e Y
		nx = x * cos(anguloZ) - y * sin(anguloZ);
		ny = x * sin(anguloZ) + y * cos(anguloZ);
		x = nx;
		y = ny;

		poligono.vertices[i].x = x + poligono.centro.x;
		poligono.vertices[i].y = y + poligono.centro.y;
		poligono.vertices[i].z = z + poligono.centro.z;
	}
}

// Desenha o cubo em wireframe: para cada aresta, projeta os dois vertices de 3D
// para 2D e traca uma linha entre eles usando GL_LINES.
void desenhar(Poligono poligono) {
	glColor3f(0, 0, 0);
	glBegin(GL_LINES);
	for (int i = 0; i < poligono.arestas.size(); i++) {
		int v_o = poligono.arestas[i].first;  // indice do vertice de origem
		int v_d = poligono.arestas[i].second; // indice do vertice de destino

		ponto2d origem = projetar(poligono.vertices[v_o]);
		ponto2d destino = projetar(poligono.vertices[v_d]);

		glVertex2d(origem.x, origem.y);
		glVertex2d(destino.x, destino.y);
	}
	glEnd();
}
