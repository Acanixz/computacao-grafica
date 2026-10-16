/*
 * M2 Computacao Grafica - Objetos 3D e iluminação
 * Alunos: Hérick Vitor Vieira Bittencourt, Paola Zanella, Lorina Zondervan, Luiz Augusto Inthurn
 *
 * Le modelos .obj (vertices, coordenadas de textura, normais e faces) e desenha
 * suas faces como triangulos, iluminados por 3 luzes pontuais e com texturas
 * lidas de arquivos BMP.
 *
 * A lista de controles é exibida no console quando a aplicacao inicia
 * (ver a funcao imprimir_controles).
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <cstdlib>
#include <cctype>
#include <cmath>
#include <GL/freeglut.h>

// Um ponto ou uma direcao no espaco tridimensional (usado para posicoes e normais)
struct vertice {
	double x;
	double y;
	double z;
};

// Coordenada de textura: ponto (u, v) da imagem, de 0 a 1 em cada eixo
struct coord_textura {
	double u;
	double v;
};

// Um canto de face do .obj: indices (comecando em 0) da posicao, da coordenada
// de textura e da normal usadas naquele canto. -1 indica que o dado nao existe
struct canto {
	int v;
	int vt;
	int vn;
};

// Face de 3 cantos. Faces com mais cantos sao divididas em triangulos na leitura
struct triangulo {
	canto cantos[3];
};

// Listas de dados do modelo
using lista_vertices = std::vector<vertice>;
using lista_coord_textura = std::vector<coord_textura>;
using lista_triangulos = std::vector<triangulo>;

// Modelo lido de um arquivo .obj e as transformacoes aplicadas a ele
struct Modelo {
	std::string nome;                // nome do arquivo
	lista_vertices vertices;         // linhas "v"
	lista_coord_textura texturas;    // linhas "vt"
	lista_vertices normais;          // linhas "vn" (mais as calculadas, se faltarem)
	lista_triangulos triangulos;     // linhas "f", ja divididas em triangulos
	int faces_arquivo = 0;           // quantidade de linhas "f" do arquivo
	bool normais_calculadas = false; // true se o arquivo nao trazia todas as normais

	vertice posicao;                 // translacao, aplicada com glTranslated
	vertice rotacao;                 // angulos em graus em X, Y e Z, aplicados com glRotated
	vertice escala;                  // escala em cada eixo, aplicada com glScaled

	GLuint lista_desenho = 0;        // display list com os triangulos
	GLuint lista_normais = 0;        // display list com as normais (tecla N)
};

// Imagem lida de um arquivo BMP
struct imagem_bmp {
	int largura;
	int altura;
	std::vector<unsigned char> pixels; // RGB, 3 bytes por pixel, linha de baixo primeiro
};

// Textura ja enviada para o OpenGL
struct Textura {
	std::string nome;
	GLuint id;
};

// Luz pontual com as tres componentes do modelo de iluminacao do OpenGL
struct Luz {
	GLenum id;            // GL_LIGHT0, GL_LIGHT1 ou GL_LIGHT2
	std::string nome;
	GLfloat posicao[4];   // w = 1: luz pontual, que fica numa posicao da cena
	GLfloat ambiente[4];  // luz espalhada, ilumina todas as faces por igual
	GLfloat difusa[4];    // depende do angulo entre a normal e a direcao da luz
	GLfloat especular[4]; // reflexo brilhante, depende tambem da posicao da camera
	bool ligada;
};

const double PI = 3.14159265358979;

// Camera fixa em (0, 0, DISTANCIA_CAMERA) olhando para a origem
const double DISTANCIA_CAMERA = 5.0;
const double CAMPO_DE_VISAO = 40.0; // abertura vertical da perspectiva, em graus

// Pastas lidas ao iniciar. O Visual Studio executa o programa na pasta do
// projeto, onde ficam as duas
const std::string PASTA_MODELOS = "modelos";
const std::string PASTA_TEXTURAS = "texturas";

// Modelos carregados. Apenas o modelo_atual e exibido e transformado
std::vector<Modelo> modelos;
int modelo_atual = 0;

// Texturas carregadas e a textura aplicada ao modelo
std::vector<Textura> texturas;
int textura_atual = 0;
bool usar_textura = true;

// Tres luzes pontuais em volta do modelo, cada uma de uma cor para que o
// efeito de ligar e desligar cada uma seja facil de perceber
Luz luzes[3] = {
	// id, nome, posicao, ambiente, difusa, especular, ligada
	{ GL_LIGHT0, "principal (branca)", { 1.0f, 1.0f, 1.5f, 1.0f },
		{ 0.15f, 0.15f, 0.15f, 1.0f }, { 0.75f, 0.75f, 0.75f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, true },
	{ GL_LIGHT1, "lateral (laranja)", { -1.6f, 0.2f, 0.8f, 1.0f },
		{ 0.10f, 0.05f, 0.01f, 1.0f }, { 0.6f, 0.3f, 0.05f, 1.0f }, { 0.6f, 0.4f, 0.2f, 1.0f }, true },
	{ GL_LIGHT2, "inferior (azul)", { 0.3f, -1.2f, 1.0f, 1.0f },
		{ 0.01f, 0.04f, 0.10f, 1.0f }, { 0.1f, 0.25f, 0.65f, 1.0f }, { 0.2f, 0.4f, 0.8f, 1.0f }, true },
};

// Componentes ativas em todas as luzes (teclas Z, X e C)
bool usar_ambiente = true;
bool usar_difusa = true;
bool usar_especular = true;

// Opcoes de desenho (teclas M e N)
bool modo_wireframe = false;
bool mostrar_normais = false;

double velocidadeRotacao = 0.5; // giro automatico em torno do eixo Y, em graus por quadro
int delay = 10;

// Tamanho atual da janela, atualizado em reshape
int largura_janela = 800;
int altura_janela = 600;

// Estado do mouse
int botao_arrastando = -1; // botao pressionado durante o arrasto (-1: nenhum)
int mouseX = 0;
int mouseY = 0;

// ===== LISTA DE FUNÇÕES =====
//	LEITURA DE ARQUIVOS

std::vector<std::string> listar_arquivos(const std::string& pasta, const std::string& extensao);
void carregar_modelos(int argc, char** argv);
bool carregar_obj(const std::string& caminho, Modelo& modelo);
canto ler_canto(const std::string& texto, const Modelo& modelo);
int converter_indice(const std::string& texto, int quantidade);
void calcular_normais(Modelo& modelo);
void centralizar(Modelo& modelo);
void carregar_texturas();
bool carregar_bmp(const std::string& caminho, imagem_bmp& imagem);

//	COMANDOS

void movimentar(Modelo& modelo, double dx, double dy, double dz);
void escalar(Modelo& modelo, double escala_x, double escala_y, double escala_z);
void rotacionar(Modelo& modelo, double angulo_x, double angulo_y, double angulo_z);
void resetar(Modelo& modelo);

//	RENDERIZAÇÃO & CONTROLE

void configurar_opengl();
void aplicar_luzes();
GLuint criar_textura(int largura, int altura, const unsigned char* pixels);
void compilar(Modelo& modelo);
void desenhar(const Modelo& modelo);
void desenhar_luzes();
void desenhar_hud();
void cor_da_luz(const Luz& luz);
void cor_opcao(bool ativa);
int escrever(int x, int y, const std::string& texto);
void display();
void reshape(int largura, int altura);
void redraw(int value);
void keyboard(unsigned char key, int x, int y);
void keyboard_special(int key, int x, int y);
void mouse(int botao, int estado, int x, int y);
void mouse_arrasto(int x, int y);
void mouse_roda(int roda, int direcao, int x, int y);
void imprimir_controles();

//	OPERACOES COM VETORES

vertice subtrair(vertice a, vertice b);
vertice produto_vetorial(vertice a, vertice b);
vertice normalizar(vertice v);

// Mostra a lista de controles no console enquanto a aplicacao esta rodando
void imprimir_controles() {
	std::cout << "===== CONTROLES =====\n";
	std::cout << "Setas                  move o modelo nos eixos X e Y\n";
	std::cout << "Page Up / Down         move o modelo no eixo Z (aproxima / afasta da camera)\n";
	std::cout << "Mouse Dir. (arrastar)  move o modelo nos eixos X e Y\n";
	std::cout << "W / S                  rotaciona em torno do eixo X\n";
	std::cout << "A / D                  rotaciona em torno do eixo Y\n";
	std::cout << "Q / E                  rotaciona em torno do eixo Z\n";
	std::cout << "Mouse Esq. (arrastar)  rotaciona nos eixos X e Y\n";
	std::cout << "O / P                  diminui / aumenta a escala\n";
	std::cout << "Roda do mouse          diminui / aumenta a escala\n";
	std::cout << "R                      liga / desliga o giro automatico\n";
	std::cout << "K / L                  ajusta a velocidade do giro automatico\n";
	std::cout << "Espaco                 volta o modelo ao estado inicial\n";
	std::cout << "Tab                    troca o modelo exibido\n";
	std::cout << "1 / 2 / 3              liga / desliga cada uma das tres luzes\n";
	std::cout << "Z / X / C              liga / desliga a componente ambiente / difusa / especular\n";
	std::cout << "T                      liga / desliga a textura\n";
	std::cout << "Y                      troca a textura\n";
	std::cout << "M                      alterna entre faces preenchidas e wireframe\n";
	std::cout << "N                      mostra / esconde as normais dos vertices\n";
	std::cout << "ESC                    fecha a aplicacao\n";
	std::cout << "=====================\n\n";
}

int main(int argc, char** argv) {

	imprimir_controles();

	glutInit(&argc, argv);

	// Buffer duplo para o giro nao piscar e buffer de profundidade para que as
	// faces mais proximas da camera escondam as que estao atras delas
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(largura_janela, altura_janela);

	glutCreateWindow("Modelos .obj com iluminacao");

	// Texturas e display lists so podem ser criadas depois que a janela (e o
	// contexto do OpenGL) existe
	configurar_opengl();
	carregar_texturas();
	carregar_modelos(argc, argv);

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(keyboard_special);
	glutMouseFunc(mouse);
	glutMotionFunc(mouse_arrasto);
	glutMouseWheelFunc(mouse_roda);
	glutTimerFunc(delay, redraw, 0);

	glutMainLoop();

	return 0;
}


void display(void) {
	// Limpa as cores e as profundidades do quadro anterior
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Camera em (0, 0, DISTANCIA_CAMERA) olhando para a origem, com Y para cima
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 0, DISTANCIA_CAMERA, 0, 0, 0, 0, 1, 0);

	// A posicao da luz passa pela matriz atual. Enviada depois da camera e antes
	// das transformacoes do modelo, a luz fica parada na cena enquanto o modelo
	// se move
	for (const Luz& luz : luzes) {
		glLightfv(luz.id, GL_POSITION, luz.posicao);
	}
	desenhar_luzes();

	if (!modelos.empty()) {
		desenhar(modelos[modelo_atual]);
	}

	desenhar_hud();

	// Apresenta o resultado da renderização
	glutSwapBuffers();
}

// Ajusta a area de desenho e a perspectiva quando a janela muda de tamanho
void reshape(int largura, int altura) {
	largura_janela = largura;
	altura_janela = (altura > 0) ? altura : 1;

	glViewport(0, 0, largura_janela, altura_janela);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(CAMPO_DE_VISAO, (double)largura_janela / altura_janela, 0.1, 100.0);
	glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y) {
	// Com Caps Lock ligado as teclas funcionam do mesmo jeito
	key = (unsigned char)tolower(key);

	// Opcoes da cena
	switch (key) {
		// ESC
	case 27:
		exit(0);
		break;

		// Liga / desliga cada luz
	case '1':
	case '2':
	case '3':
		luzes[key - '1'].ligada = !luzes[key - '1'].ligada;
		aplicar_luzes();
		break;

		// Liga / desliga as componentes de todas as luzes
	case 'z':
		usar_ambiente = !usar_ambiente;
		aplicar_luzes();
		break;
	case 'x':
		usar_difusa = !usar_difusa;
		aplicar_luzes();
		break;
	case 'c':
		usar_especular = !usar_especular;
		aplicar_luzes();
		break;

		// Textura: liga / desliga e troca a imagem
	case 't':
		usar_textura = !usar_textura;
		break;
	case 'y':
		textura_atual = (textura_atual + 1) % (int)texturas.size();
		break;

		// Modo de desenho
	case 'm':
		modo_wireframe = !modo_wireframe;
		break;
	case 'n':
		mostrar_normais = !mostrar_normais;
		break;

		// Giro automatico em torno do eixo Y
	case 'r':
		velocidadeRotacao = (fabs(velocidadeRotacao) < 0.0001) ? 0.5 : 0.0;
		break;
	case 'k':
		velocidadeRotacao -= 0.25;
		break;
	case 'l':
		velocidadeRotacao += 0.25;
		break;

		// Troca o modelo exibido
	case '\t':
		if (!modelos.empty()) {
			modelo_atual = (modelo_atual + 1) % (int)modelos.size();
		}
		break;
	}

	// Transformacoes do modelo exibido
	if (modelos.empty()) {
		return;
	}
	Modelo& modelo = modelos[modelo_atual];

	switch (key) {
		// Escala em torno do centro (uniforme nos tres eixos)
	case 'o':
		escalar(modelo, 0.9, 0.9, 0.9);
		break;
	case 'p':
		escalar(modelo, 1.1, 1.1, 1.1);
		break;

		// Rotacao em torno do eixo X
	case 'w':
		rotacionar(modelo, -5, 0, 0);
		break;
	case 's':
		rotacionar(modelo, 5, 0, 0);
		break;

		// Rotacao em torno do eixo Y
	case 'a':
		rotacionar(modelo, 0, -5, 0);
		break;
	case 'd':
		rotacionar(modelo, 0, 5, 0);
		break;

		// Rotacao em torno do eixo Z
	case 'q':
		rotacionar(modelo, 0, 0, 5);
		break;
	case 'e':
		rotacionar(modelo, 0, 0, -5);
		break;

		// Volta o modelo ao estado inicial
	case ' ':
		resetar(modelo);
		break;
	}
}

void keyboard_special(int key, int x, int y) {
	if (modelos.empty()) {
		return;
	}
	Modelo& modelo = modelos[modelo_atual];

	switch (key) {
		// Translacao no plano da tela (eixos X e Y)
	case GLUT_KEY_DOWN:
		movimentar(modelo, 0, -0.1, 0);
		break;
	case GLUT_KEY_UP:
		movimentar(modelo, 0, 0.1, 0);
		break;
	case GLUT_KEY_RIGHT:
		movimentar(modelo, 0.1, 0, 0);
		break;
	case GLUT_KEY_LEFT:
		movimentar(modelo, -0.1, 0, 0);
		break;

		// Translacao em profundidade (eixo Z): Page Up aproxima, Page Down afasta
	case GLUT_KEY_PAGE_UP:
		movimentar(modelo, 0, 0, 0.1);
		break;
	case GLUT_KEY_PAGE_DOWN:
		movimentar(modelo, 0, 0, -0.1);
		break;
	}
}

// Guarda o botao e a posicao do cursor quando o usuario comeca a arrastar
void mouse(int botao, int estado, int x, int y) {
	if (botao == GLUT_LEFT_BUTTON || botao == GLUT_RIGHT_BUTTON) {
		botao_arrastando = (estado == GLUT_DOWN) ? botao : -1;
		mouseX = x;
		mouseY = y;
	}
}

// Arrasto com o botao esquerdo rotaciona: horizontal gira em torno de Y e
// vertical em torno de X. Arrasto com o botao direito move o modelo no plano
// da tela; o deslocamento em pixels e convertido para unidades da cena na
// profundidade do modelo, para que ele acompanhe o cursor.
void mouse_arrasto(int x, int y) {
	if (botao_arrastando < 0 || modelos.empty()) {
		return;
	}

	Modelo& modelo = modelos[modelo_atual];
	int dx = x - mouseX;
	int dy = y - mouseY;

	if (botao_arrastando == GLUT_LEFT_BUTTON) {
		rotacionar(modelo, dy * 0.5, dx * 0.5, 0);
	}
	else {
		// Altura visivel da cena na profundidade do modelo dividida pela altura da janela
		double profundidade = DISTANCIA_CAMERA - modelo.posicao.z;
		double unidades_por_pixel = 2 * profundidade * tan(CAMPO_DE_VISAO / 2 * PI / 180) / altura_janela;
		movimentar(modelo, dx * unidades_por_pixel, -dy * unidades_por_pixel, 0);
	}

	mouseX = x;
	mouseY = y;

	glutPostRedisplay();
}

// Roda do mouse: para cima aumenta a escala, para baixo diminui
void mouse_roda(int roda, int direcao, int x, int y) {
	if (modelos.empty()) {
		return;
	}
	double fator = (direcao > 0) ? 1.1 : 0.9;
	escalar(modelos[modelo_atual], fator, fator, fator);
}

void redraw(int value) {
	// Giro automatico continuo em torno do eixo Y
	if (!modelos.empty()) {
		rotacionar(modelos[modelo_atual], 0, velocidadeRotacao, 0);
	}

	glutPostRedisplay();
	glutTimerFunc(delay, redraw, 0);
}

// Lista, em ordem alfabetica, os arquivos de uma pasta com a extensao informada
std::vector<std::string> listar_arquivos(const std::string& pasta, const std::string& extensao) {
	std::vector<std::string> caminhos;
	std::error_code erro;

	for (const auto& entrada : std::filesystem::directory_iterator(pasta, erro)) {
		std::string ext = entrada.path().extension().string();
		for (char& c : ext) {
			c = (char)tolower((unsigned char)c);
		}
		if (ext == extensao) {
			caminhos.push_back(entrada.path().string());
		}
	}

	if (erro) {
		std::cout << "Pasta '" << pasta << "' nao encontrada (pasta atual: "
			<< std::filesystem::current_path().string() << ")\n";
	}

	std::sort(caminhos.begin(), caminhos.end());
	return caminhos;
}

// Carrega os .obj passados como argumento do programa ou, sem argumentos,
// todos os .obj da pasta de modelos. Depois da leitura, cada modelo recebe as
// normais que faltarem, e centralizado e gravado em display lists.
void carregar_modelos(int argc, char** argv) {
	std::vector<std::string> caminhos;
	for (int i = 1; i < argc; i++) {
		caminhos.push_back(argv[i]);
	}
	if (caminhos.empty()) {
		caminhos = listar_arquivos(PASTA_MODELOS, ".obj");
	}

	std::cout << "===== MODELOS =====\n";
	for (const std::string& caminho : caminhos) {
		Modelo modelo;
		if (!carregar_obj(caminho, modelo)) {
			continue;
		}
		calcular_normais(modelo);
		centralizar(modelo);
		resetar(modelo);
		compilar(modelo);
		modelos.push_back(modelo);
	}

	if (modelos.empty()) {
		std::cout << "Nenhum modelo carregado. Coloque arquivos .obj na pasta '" << PASTA_MODELOS << "'\n";
	}
	std::cout << "\n";
}

// Le um arquivo .obj. Cada linha comeca com o tipo do dado:
//   v  x y z       posicao de um vertice
//   vt u v         coordenada de textura
//   vn x y z       normal
//   f  a b c ...   face; cada canto e "v", "v/vt", "v//vn" ou "v/vt/vn"
// As demais linhas (comentarios com #, o, g, s, mtllib, usemtl...) sao
// ignoradas. Faces com mais de 3 cantos sao divididas em triangulos em leque,
// todos partindo do primeiro canto: (0, 1, 2), (0, 2, 3), (0, 3, 4)...
bool carregar_obj(const std::string& caminho, Modelo& modelo) {
	std::ifstream arquivo(caminho);
	if (!arquivo.is_open()) {
		std::cout << "Nao foi possivel abrir " << caminho << "\n";
		return false;
	}

	modelo.nome = std::filesystem::path(caminho).filename().string();

	// O leitor da linha e a lista de cantos sao reaproveitados de uma linha para
	// a outra: cria-los a cada linha deixa a leitura de arquivos grandes lenta
	std::istringstream leitor;
	std::vector<canto> cantos;
	std::string linha;
	std::string tipo;
	std::string texto;

	while (std::getline(arquivo, linha)) {
		leitor.clear();
		leitor.str(linha);
		tipo.clear();
		leitor >> tipo;

		if (tipo == "v") {
			vertice v = { 0, 0, 0 };
			leitor >> v.x >> v.y >> v.z;
			modelo.vertices.push_back(v);
		}
		else if (tipo == "vt") {
			coord_textura t = { 0, 0 };
			leitor >> t.u >> t.v;
			modelo.texturas.push_back(t);
		}
		else if (tipo == "vn") {
			vertice n = { 0, 0, 0 };
			leitor >> n.x >> n.y >> n.z;
			modelo.normais.push_back(n);
		}
		else if (tipo == "f") {
			cantos.clear();
			while (leitor >> texto) {
				cantos.push_back(ler_canto(texto, modelo));
			}

			for (size_t i = 1; i + 1 < cantos.size(); i++) {
				triangulo t;
				t.cantos[0] = cantos[0];
				t.cantos[1] = cantos[i];
				t.cantos[2] = cantos[i + 1];
				modelo.triangulos.push_back(t);
			}
			modelo.faces_arquivo++;
		}
	}

	// Descarta triangulos que apontam para posicoes que nao existem. Indices de
	// textura ou de normal fora das listas sao tratados como ausentes
	lista_triangulos validos;
	for (triangulo t : modelo.triangulos) {
		bool valido = true;
		for (canto& c : t.cantos) {
			if (c.v < 0 || c.v >= (int)modelo.vertices.size()) {
				valido = false;
			}
			if (c.vt >= (int)modelo.texturas.size()) {
				c.vt = -1;
			}
			if (c.vn >= (int)modelo.normais.size()) {
				c.vn = -1;
			}
		}
		if (valido) {
			validos.push_back(t);
		}
	}
	size_t descartados = modelo.triangulos.size() - validos.size();
	modelo.triangulos = validos;

	std::cout << modelo.nome << ": " << modelo.vertices.size() << " vertices, "
		<< modelo.texturas.size() << " coordenadas de textura, "
		<< modelo.normais.size() << " normais, "
		<< modelo.faces_arquivo << " faces (" << modelo.triangulos.size() << " triangulos)\n";
	if (descartados > 0) {
		std::cout << "  " << descartados << " triangulos ignorados por usar indices invalidos\n";
	}

	if (modelo.triangulos.empty()) {
		std::cout << "  nenhuma face valida, modelo ignorado\n";
		return false;
	}
	return true;
}

// Le um canto de face ("7", "7/3", "7//2" ou "7/3/2") e converte os numeros
// em indices das listas do modelo
canto ler_canto(const std::string& texto, const Modelo& modelo) {
	// Separa o texto nas partes entre as barras: v, vt e vn
	std::string partes[3];
	int parte = 0;
	for (char c : texto) {
		if (c != '/') {
			partes[parte] += c;
		}
		else if (parte < 2) {
			parte++;
		}
		else {
			break; // mais de duas barras: ignora o resto
		}
	}

	canto resultado;
	resultado.v = converter_indice(partes[0], (int)modelo.vertices.size());
	resultado.vt = converter_indice(partes[1], (int)modelo.texturas.size());
	resultado.vn = converter_indice(partes[2], (int)modelo.normais.size());
	return resultado;
}

// Converte o texto de um indice do .obj em indice de vetor (comecando em 0).
// No .obj os indices comecam em 1, e indices negativos contam a partir do fim
// da lista lida ate aquele ponto do arquivo (-1 e o ultimo). Devolve -1 se o
// texto estiver vazio ou o indice for invalido.
int converter_indice(const std::string& texto, int quantidade) {
	if (texto.empty()) {
		return -1;
	}

	int indice = atoi(texto.c_str());
	if (indice > 0) {
		return indice - 1;
	}
	if (indice < 0 && quantidade + indice >= 0) {
		return quantidade + indice;
	}
	return -1;
}

// Calcula as normais que faltam no arquivo (cantos sem "vn"). A normal de cada
// triangulo vem do produto vetorial de duas de suas arestas. Cada vertice soma
// as normais dos triangulos que o usam (triangulos maiores pesam mais) e o
// resultado e normalizado, deixando a iluminacao suave entre faces vizinhas.
void calcular_normais(Modelo& modelo) {
	for (const triangulo& t : modelo.triangulos) {
		for (const canto& c : t.cantos) {
			if (c.vn < 0) {
				modelo.normais_calculadas = true;
			}
		}
	}
	if (!modelo.normais_calculadas) {
		return;
	}

	vertice zero = { 0, 0, 0 };
	lista_vertices soma(modelo.vertices.size(), zero);
	for (const triangulo& t : modelo.triangulos) {
		vertice a = modelo.vertices[t.cantos[0].v];
		vertice b = modelo.vertices[t.cantos[1].v];
		vertice c = modelo.vertices[t.cantos[2].v];
		vertice normal = produto_vetorial(subtrair(b, a), subtrair(c, a));

		for (const canto& k : t.cantos) {
			soma[k.v].x += normal.x;
			soma[k.v].y += normal.y;
			soma[k.v].z += normal.z;
		}
	}

	// As normais calculadas vao para o fim da lista, uma para cada vertice
	int inicio = (int)modelo.normais.size();
	for (const vertice& n : soma) {
		modelo.normais.push_back(normalizar(n));
	}
	for (triangulo& t : modelo.triangulos) {
		for (canto& c : t.cantos) {
			if (c.vn < 0) {
				c.vn = inicio + c.v;
			}
		}
	}

	std::cout << "  normais calculadas a partir das faces (faltavam linhas vn)\n";
}

// Os arquivos .obj usam escalas muito diferentes (ha modelos com 0.1 unidade e
// outros com 1000). Para qualquer modelo aparecer inteiro na tela, os vertices
// sao deslocados para que o centro da caixa que envolve o modelo fique na
// origem e escalados para que o vertice mais distante fique a 1 unidade do
// centro. As normais nao mudam: deslocar e escalar igualmente nos tres eixos
// nao altera a direcao das faces.
void centralizar(Modelo& modelo) {
	vertice minimo = modelo.vertices[0];
	vertice maximo = modelo.vertices[0];
	for (const vertice& v : modelo.vertices) {
		minimo.x = std::min(minimo.x, v.x);
		minimo.y = std::min(minimo.y, v.y);
		minimo.z = std::min(minimo.z, v.z);
		maximo.x = std::max(maximo.x, v.x);
		maximo.y = std::max(maximo.y, v.y);
		maximo.z = std::max(maximo.z, v.z);
	}
	vertice centro = { (minimo.x + maximo.x) / 2, (minimo.y + maximo.y) / 2, (minimo.z + maximo.z) / 2 };

	double raio = 0;
	for (const vertice& v : modelo.vertices) {
		vertice d = subtrair(v, centro);
		raio = std::max(raio, sqrt(d.x * d.x + d.y * d.y + d.z * d.z));
	}
	if (raio == 0) {
		raio = 1;
	}

	for (vertice& v : modelo.vertices) {
		v.x = (v.x - centro.x) / raio;
		v.y = (v.y - centro.y) / raio;
		v.z = (v.z - centro.z) / raio;
	}
}

// Carrega todas as imagens BMP da pasta de texturas. Por ultimo cria, como no
// exemplo de texturas, um tabuleiro de xadrez gerado no codigo, que deixa facil
// ver como as coordenadas de textura estao distribuidas pelo modelo.
void carregar_texturas() {
	std::cout << "===== TEXTURAS =====\n";
	for (const std::string& caminho : listar_arquivos(PASTA_TEXTURAS, ".bmp")) {
		imagem_bmp imagem;
		if (carregar_bmp(caminho, imagem)) {
			Textura textura;
			textura.nome = std::filesystem::path(caminho).filename().string();
			textura.id = criar_textura(imagem.largura, imagem.altura, imagem.pixels.data());
			texturas.push_back(textura);
			std::cout << textura.nome << ": " << imagem.largura << " x " << imagem.altura << " pixels\n";
		}
	}

	// Xadrez de 64 x 64 pixels com casas de 8 x 8
	std::vector<unsigned char> xadrez(64 * 64 * 3);
	for (int i = 0; i < 64; i++) {
		for (int j = 0; j < 64; j++) {
			unsigned char cor = ((i / 8 + j / 8) % 2 == 0) ? 255 : 40;
			for (int k = 0; k < 3; k++) {
				xadrez[(i * 64 + j) * 3 + k] = cor;
			}
		}
	}
	Textura textura;
	textura.nome = "xadrez (gerada no codigo)";
	textura.id = criar_textura(64, 64, xadrez.data());
	texturas.push_back(textura);
	std::cout << "\n";
}

// Le um arquivo BMP de 24 bits sem compressao, o formato das imagens da pasta de
// texturas. Segue a funcao getBMPData do exemplo de texturas, com dois cuidados a
// mais: no arquivo cada linha da imagem e completada com bytes extras ate um
// multiplo de 4, e as cores estao na ordem BGR, aqui convertida para RGB.
bool carregar_bmp(const std::string& caminho, imagem_bmp& imagem) {
	std::ifstream arquivo(caminho, std::ios::binary);
	if (!arquivo.is_open()) {
		std::cout << "Nao foi possivel abrir " << caminho << "\n";
		return false;
	}

	// Cabecalho: assinatura "BM" no inicio, posicao dos pixels no byte 10,
	// largura e altura no byte 18, bits por pixel no 28 e compressao no 30
	char assinatura[2] = { 0, 0 };
	unsigned int inicio_pixels = 0;
	int largura = 0;
	int altura = 0;
	unsigned short bits_por_pixel = 0;
	unsigned int compressao = 0;

	arquivo.read(assinatura, 2);
	arquivo.seekg(10);
	arquivo.read((char*)&inicio_pixels, 4);
	arquivo.seekg(18);
	arquivo.read((char*)&largura, 4);
	arquivo.read((char*)&altura, 4);
	arquivo.seekg(28);
	arquivo.read((char*)&bits_por_pixel, 2);
	arquivo.read((char*)&compressao, 4);

	if (!arquivo || assinatura[0] != 'B' || assinatura[1] != 'M' || bits_por_pixel != 24
		|| compressao != 0 || largura <= 0 || altura == 0) {
		std::cout << caminho << ": formato nao suportado (use BMP de 24 bits sem compressao)\n";
		return false;
	}

	// Altura negativa indica linhas gravadas de cima para baixo. O OpenGL espera
	// a linha de baixo da imagem primeiro
	bool de_cima_para_baixo = altura < 0;
	altura = abs(altura);

	int bytes_linha = largura * 3;
	int bytes_linha_arquivo = (bytes_linha + 3) / 4 * 4;
	std::vector<unsigned char> linha(bytes_linha_arquivo);

	imagem.largura = largura;
	imagem.altura = altura;
	imagem.pixels.resize((size_t)bytes_linha * altura);

	arquivo.seekg(inicio_pixels);
	for (int i = 0; i < altura; i++) {
		arquivo.read((char*)linha.data(), bytes_linha_arquivo);

		int destino = de_cima_para_baixo ? altura - 1 - i : i;
		unsigned char* pixel = &imagem.pixels[(size_t)destino * bytes_linha];
		for (int j = 0; j < largura; j++) {
			pixel[j * 3 + 0] = linha[j * 3 + 2]; // vermelho
			pixel[j * 3 + 1] = linha[j * 3 + 1]; // verde
			pixel[j * 3 + 2] = linha[j * 3 + 0]; // azul
		}
	}

	if (!arquivo) {
		std::cout << caminho << ": arquivo incompleto\n";
		return false;
	}
	return true;
}

// Translacao: acumula o deslocamento, aplicado no desenho com glTranslated
void movimentar(Modelo& modelo, double dx, double dy, double dz) {
	modelo.posicao.x += dx;
	modelo.posicao.y += dy;
	modelo.posicao.z += dz;
}

// Escala: multiplica a escala atual de cada eixo, aplicada com glScaled
void escalar(Modelo& modelo, double escala_x, double escala_y, double escala_z) {
	modelo.escala.x *= escala_x;
	modelo.escala.y *= escala_y;
	modelo.escala.z *= escala_z;
}

// Rotacao: soma os angulos (em graus) de cada eixo, aplicados com glRotated.
// O resto da divisao por 360 evita que os angulos crescam sem limite
void rotacionar(Modelo& modelo, double angulo_x, double angulo_y, double angulo_z) {
	modelo.rotacao.x = fmod(modelo.rotacao.x + angulo_x, 360.0);
	modelo.rotacao.y = fmod(modelo.rotacao.y + angulo_y, 360.0);
	modelo.rotacao.z = fmod(modelo.rotacao.z + angulo_z, 360.0);
}

// Volta o modelo ao estado inicial, com uma leve inclinacao para aparecer em 3D
void resetar(Modelo& modelo) {
	modelo.posicao = { 0, 0, 0 };
	modelo.rotacao = { 20, -30, 0 };
	modelo.escala = { 1, 1, 1 };
}

// Estado do OpenGL que nao muda durante a execucao
void configurar_opengl() {
	glClearColor(0.12f, 0.12f, 0.15f, 1.0f);

	// Teste de profundidade: cada pixel mostra a face mais proxima da camera
	glEnable(GL_DEPTH_TEST);

	// glScale tambem muda o tamanho das normais; GL_NORMALIZE devolve as normais
	// ao comprimento 1 antes do calculo da iluminacao
	glEnable(GL_NORMALIZE);

	// Iluminacao. Sem luz ambiente global: toda a luz da cena vem das 3 luzes
	GLfloat ambiente_global[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambiente_global);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
	glEnable(GL_LIGHTING);
	aplicar_luzes();

	// Material dos modelos: cinza claro com reflexo especular
	GLfloat material_ambiente[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	GLfloat material_difusa[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	GLfloat material_especular[] = { 0.6f, 0.6f, 0.6f, 1.0f };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material_ambiente);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material_difusa);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material_especular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 40.0f);

	// GL_MODULATE multiplica a cor da textura pela cor calculada na iluminacao
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// As linhas das imagens RGB nao tem bytes extras de alinhamento
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

// Envia ao OpenGL as componentes e o estado (ligada / desligada) de cada luz.
// As componentes desativadas com Z, X e C sao enviadas zeradas, sem perder os
// valores guardados em "luzes"
void aplicar_luzes() {
	GLfloat zero[] = { 0.0f, 0.0f, 0.0f, 1.0f };

	for (const Luz& luz : luzes) {
		glLightfv(luz.id, GL_AMBIENT, usar_ambiente ? luz.ambiente : zero);
		glLightfv(luz.id, GL_DIFFUSE, usar_difusa ? luz.difusa : zero);
		glLightfv(luz.id, GL_SPECULAR, usar_especular ? luz.especular : zero);

		if (luz.ligada) {
			glEnable(luz.id);
		}
		else {
			glDisable(luz.id);
		}
	}
}

// Envia uma imagem RGB para o OpenGL e devolve o identificador da textura.
// gluBuild2DMipmaps tambem gera versoes reduzidas da imagem (mipmaps), usadas
// quando o modelo aparece pequeno na tela, e aceita tamanhos que nao sao
// potencia de 2.
GLuint criar_textura(int largura, int altura, const unsigned char* pixels) {
	GLuint id;
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

	// Coordenadas fora do intervalo 0..1 repetem a imagem
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, largura, altura, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	return id;
}

// Grava os triangulos do modelo em uma display list: os comandos sao enviados
// ao OpenGL uma unica vez e depois apenas repetidos a cada quadro com
// glCallList, o que deixa leve desenhar modelos grandes.
// Cada vertice leva a sua normal (usada na iluminacao), a sua coordenada de
// textura e a sua posicao.
void compilar(Modelo& modelo) {
	modelo.lista_desenho = glGenLists(1);
	glNewList(modelo.lista_desenho, GL_COMPILE);
	glBegin(GL_TRIANGLES);
	for (const triangulo& t : modelo.triangulos) {
		for (const canto& c : t.cantos) {
			const vertice& normal = modelo.normais[c.vn];
			glNormal3d(normal.x, normal.y, normal.z);

			if (c.vt >= 0) {
				const coord_textura& textura = modelo.texturas[c.vt];
				glTexCoord2d(textura.u, textura.v);
			}
			else {
				glTexCoord2d(0, 0);
			}

			const vertice& posicao = modelo.vertices[c.v];
			glVertex3d(posicao.x, posicao.y, posicao.z);
		}
	}
	glEnd();
	glEndList();

	// Segunda lista: um segmento saindo de cada vertice na direcao da sua normal
	const double COMPRIMENTO_NORMAL = 0.08;
	modelo.lista_normais = glGenLists(1);
	glNewList(modelo.lista_normais, GL_COMPILE);
	glBegin(GL_LINES);
	for (const triangulo& t : modelo.triangulos) {
		for (const canto& c : t.cantos) {
			vertice p = modelo.vertices[c.v];
			vertice n = normalizar(modelo.normais[c.vn]);
			glVertex3d(p.x, p.y, p.z);
			glVertex3d(p.x + n.x * COMPRIMENTO_NORMAL, p.y + n.y * COMPRIMENTO_NORMAL, p.z + n.z * COMPRIMENTO_NORMAL);
		}
	}
	glEnd();
	glEndList();
}

// Desenha o modelo aplicando as transformacoes com as funcoes do OpenGL. As
// chamadas aparecem na ordem inversa da aplicacao: cada vertice e escalado,
// depois rotacionado (Z, Y e X) e por ultimo transladado. Como o modelo foi
// centralizado na origem ao ser carregado, a escala e a rotacao acontecem em
// torno do seu centro.
void desenhar(const Modelo& modelo) {
	glPushMatrix();

	glTranslated(modelo.posicao.x, modelo.posicao.y, modelo.posicao.z);
	glRotated(modelo.rotacao.x, 1, 0, 0);
	glRotated(modelo.rotacao.y, 0, 1, 0);
	glRotated(modelo.rotacao.z, 0, 0, 1);
	glScaled(modelo.escala.x, modelo.escala.y, modelo.escala.z);

	// A textura so pode ser aplicada se o arquivo trouxe coordenadas de textura (vt)
	if (usar_textura && !modelo.texturas.empty()) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texturas[textura_atual].id);
	}

	glPolygonMode(GL_FRONT_AND_BACK, modo_wireframe ? GL_LINE : GL_FILL);
	glCallList(modelo.lista_desenho);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_TEXTURE_2D);

	// Normais em amarelo, sem iluminacao
	if (mostrar_normais) {
		glDisable(GL_LIGHTING);
		glColor3f(1.0f, 0.9f, 0.2f);
		glCallList(modelo.lista_normais);
		glEnable(GL_LIGHTING);
	}

	glPopMatrix();
}

// Desenha uma pequena esfera na posicao de cada luz
void desenhar_luzes() {
	glDisable(GL_LIGHTING);
	for (const Luz& luz : luzes) {
		cor_da_luz(luz);
		glPushMatrix();
		glTranslatef(luz.posicao[0], luz.posicao[1], luz.posicao[2]);
		glutSolidSphere(0.05, 12, 12);
		glPopMatrix();
	}
	glEnable(GL_LIGHTING);
}

// Painel de texto sobre a cena: modelo exibido, luzes e opcoes ativas
void desenhar_hud() {
	// Projecao ortografica em pixels, sem iluminacao e sem teste de profundidade
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, largura_janela, 0, altura_janela);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	const int LINHA = 18; // distancia entre as linhas, em pixels
	int y = altura_janela - 22;

	// Topo: modelo exibido e o que foi lido do arquivo
	cor_opcao(true);
	if (modelos.empty()) {
		escrever(10, y, "Nenhum modelo carregado: coloque arquivos .obj na pasta '" + PASTA_MODELOS + "'");
	}
	else {
		const Modelo& modelo = modelos[modelo_atual];
		escrever(10, y, "[Tab] " + modelo.nome + "  (" + std::to_string(modelo_atual + 1)
			+ " de " + std::to_string(modelos.size()) + ")");

		std::string dados = std::to_string(modelo.vertices.size()) + " v, "
			+ std::to_string(modelo.texturas.size()) + " vt, "
			+ std::to_string(modelo.normais.size()) + " vn, "
			+ std::to_string(modelo.faces_arquivo) + " faces -> "
			+ std::to_string(modelo.triangulos.size()) + " triangulos";
		if (modelo.normais_calculadas) {
			dados += " (normais calculadas)";
		}
		escrever(10, y - LINHA, dados);
	}

	// Rodape: luzes, componentes e opcoes de desenho
	y = 10 + LINHA * 5;
	for (int i = 0; i < 3; i++) {
		cor_da_luz(luzes[i]);
		escrever(10, y, "[" + std::to_string(i + 1) + "] luz " + luzes[i].nome
			+ (luzes[i].ligada ? "" : " - desligada"));
		y -= LINHA;
	}

	int x = 10;
	cor_opcao(usar_ambiente);
	x = escrever(x, y, "[Z] ambiente");
	cor_opcao(usar_difusa);
	x = escrever(x, y, "[X] difusa");
	cor_opcao(usar_especular);
	escrever(x, y, "[C] especular");
	y -= LINHA;

	bool modelo_tem_vt = !modelos.empty() && !modelos[modelo_atual].texturas.empty();
	x = 10;
	cor_opcao(usar_textura && modelo_tem_vt);
	x = escrever(x, y, "[T] textura");
	x = escrever(x, y, "[Y] " + texturas[textura_atual].nome);
	if (!modelo_tem_vt) {
		cor_opcao(false);
		escrever(x, y, "(modelo sem coordenadas de textura)");
	}
	y -= LINHA;

	x = 10;
	cor_opcao(modo_wireframe);
	x = escrever(x, y, "[M] wireframe");
	cor_opcao(mostrar_normais);
	escrever(x, y, "[N] normais");

	// Restaura o estado usado no desenho da cena
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}

// Cor que representa a luz no marcador e no painel: a cor difusa clareada
// (para ficar legivel sobre o fundo escuro) quando a luz esta ligada e cinza
// quando esta desligada
void cor_da_luz(const Luz& luz) {
	if (!luz.ligada) {
		glColor3f(0.45f, 0.45f, 0.45f);
		return;
	}
	GLfloat maior = std::max(luz.difusa[0], std::max(luz.difusa[1], luz.difusa[2]));
	glColor3f(0.4f + 0.6f * luz.difusa[0] / maior,
		0.4f + 0.6f * luz.difusa[1] / maior,
		0.4f + 0.6f * luz.difusa[2] / maior);
}

// Itens do painel: branco quando a opcao esta ativa e cinza quando nao esta
void cor_opcao(bool ativa) {
	if (ativa) {
		glColor3f(1.0f, 1.0f, 1.0f);
	}
	else {
		glColor3f(0.45f, 0.45f, 0.45f);
	}
}

// Escreve um texto na posicao (x, y), em pixels a partir do canto inferior
// esquerdo da janela, e devolve onde o proximo texto da mesma linha pode comecar
int escrever(int x, int y, const std::string& texto) {
	glRasterPos2i(x, y);
	glutBitmapString(GLUT_BITMAP_8_BY_13, (const unsigned char*)texto.c_str());
	return x + glutBitmapLength(GLUT_BITMAP_8_BY_13, (const unsigned char*)texto.c_str()) + 16;
}

vertice subtrair(vertice a, vertice b) {
	return { a.x - b.x, a.y - b.y, a.z - b.z };
}

// Vetor perpendicular a "a" e "b", seguindo a regra da mao direita
vertice produto_vetorial(vertice a, vertice b) {
	return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}

// Mesmo vetor com comprimento 1
vertice normalizar(vertice v) {
	double comprimento = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	if (comprimento == 0) {
		return { 0, 1, 0 };
	}
	return { v.x / comprimento, v.y / comprimento, v.z / comprimento };
}
