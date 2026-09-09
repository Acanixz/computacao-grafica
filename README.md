# Cubo 3D em Wireframe

Trabalho M1 da disciplina de Computação Gráfica. O projeto adapta o código-base de um polígono bidimensional para representar e manipular um cubo em três dimensões.

O cubo é definido por oito vértices e doze arestas. Sua exibição utiliza projeção em perspectiva calculada manualmente, enquanto as operações de translação, rotação e escala são aplicadas diretamente sobre os vértices. As funções `glTranslate`, `glRotate` e `glScale` não são utilizadas.

## Funcionalidades

- Representação de um cubo com oito vértices tridimensionais;
- Descrição das doze arestas do cubo;
- Renderização em wireframe com `GL_LINES`;
- Projeção em perspectiva de pontos 3D para a tela 2D;
- Translação nos eixos X, Y e Z;
- Rotação nos eixos X, Y e Z;
- Escala uniforme nos três eixos;
- Rotação pelo teclado e pelo movimento do mouse;
- Rotação automática com controle de velocidade;
- Restauração do cubo ao estado inicial.

## Tecnologias utilizadas

- C++;
- OpenGL;
- FreeGLUT;
- Microsoft Visual Studio 2022.

## Requisitos

Para compilar e executar o projeto, é necessário utilizar:

- Windows;
- Visual Studio 2022 com o componente **Desenvolvimento para desktop com C++** instalado;
- Plataforma `x64`;
- Configuração `Debug`.

Os arquivos necessários do FreeGLUT já estão incluídos no projeto.

## Como compilar e executar

1. Extraia todos os arquivos do projeto, mantendo a estrutura original das pastas.
2. Abra o arquivo `ComputacaoGrafica.sln` no Visual Studio 2022.
3. Na barra superior do Visual Studio, selecione a configuração `Debug`.
4. Selecione a plataforma `x64`.
5. Compile a solução com `Ctrl + Shift + B`.
6. Execute o programa com `Ctrl + F5`.

Após a compilação, o projeto copia automaticamente o arquivo `freeglut.dll` necessário para a pasta do executável.

## Controles

| Tecla ou ação | Operação |
| --- | --- |
| Seta para cima | Move o cubo no sentido positivo do eixo Y |
| Seta para baixo | Move o cubo no sentido negativo do eixo Y |
| Seta para a direita | Move o cubo no sentido positivo do eixo X |
| Seta para a esquerda | Move o cubo no sentido negativo do eixo X |
| Page Up | Move o cubo no sentido positivo do eixo Z, aproximando-o da câmera |
| Page Down | Move o cubo no sentido negativo do eixo Z, afastando-o da câmera |
| W / S | Rotaciona o cubo em torno do eixo X |
| A / D | Rotaciona o cubo em torno do eixo Y |
| Q / E | Rotaciona o cubo em torno do eixo Z |
| O | Diminui a escala do cubo |
| P | Aumenta a escala do cubo |
| Arrastar com o botão esquerdo do mouse | Rotaciona o cubo nos eixos X e Y |
| R | Liga ou desliga a rotação automática |
| K / L | Diminui ou aumenta a velocidade da rotação automática |
| Espaço | Restaura o cubo ao estado inicial |
| Esc | Encerra o programa |

As teclas de letras devem ser utilizadas em formato minúsculo.

## Estrutura principal do código

O arquivo `ComputacaoGrafica.cpp` contém toda a implementação do trabalho.

### Estruturas de dados

- `vertice`: armazena as coordenadas X, Y e Z de um ponto no espaço;
- `ponto2d`: armazena as coordenadas resultantes da projeção para a tela;
- `aresta`: guarda os índices dos dois vértices conectados;
- `Poligono`: reúne o centro, os vértices, as arestas e os valores acumulados de escala e rotação.

### Funções principais

- `criar_cubo()`: cria os oito vértices e as doze arestas do cubo;
- `projetar()`: converte um vértice tridimensional em um ponto bidimensional usando perspectiva;
- `movimentar()`: aplica translação ao centro e a todos os vértices;
- `escalar()`: altera o tamanho do cubo em relação ao seu centro;
- `rotacionar()`: aplica rotações em torno dos eixos X, Y e Z, usando o centro como pivô;
- `desenhar()`: projeta os vértices e desenha cada aresta com `GL_LINES`;
- `keyboard()` e `keyboard_special()`: processam os comandos do teclado;
- `mouse()` e `mouse_arrasto()`: processam a rotação realizada pelo mouse;
- `redraw()`: atualiza a animação e solicita uma nova renderização.

## Projeção em perspectiva

A câmera virtual permanece na posição `(0, 0, 400)`. Para cada vértice, a distância até a câmera é calculada por:

```text
profundidade = CAMERA_Z - z
```

O fator de perspectiva é calculado por:

```text
fator = DISTANCIA_FOCAL / profundidade
```

As coordenadas projetadas são obtidas por:

```text
x_tela = CENTRO_TELA_X + x * fator
y_tela = CENTRO_TELA_Y + y * fator
```

Com isso, pontos mais próximos da câmera aparecem maiores e pontos mais distantes aparecem menores.

## Transformações e centro do cubo

As transformações são calculadas diretamente no código, sem utilizar `glTranslate`, `glRotate` ou `glScale`.

Na translação, o deslocamento é somado ao centro e a todos os vértices. Na rotação e na escala, cada vértice é temporariamente deslocado para um sistema de coordenadas em que o centro do cubo corresponde à origem. A transformação é aplicada e, em seguida, o centro é somado novamente. Esse processo mantém o centro como pivô da operação.

## Observações

- O cubo é desenhado somente pelas arestas, sem faces preenchidas.
- Os ângulos de rotação são informados em radianos.
- A ordem de aplicação das rotações é X, Y e Z.
- A configuração atualmente preparada e validada no Visual Studio é `Debug | x64`.
