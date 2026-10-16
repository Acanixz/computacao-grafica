# Modelos 3D (.obj) com Iluminação

Trabalho M2 da disciplina de Computação Gráfica. O projeto parte do cubo em wireframe do Trabalho M1 e passa a carregar modelos 3D de arquivos `.obj`. As faces são desenhadas como triângulos, iluminadas por três luzes pontuais e com texturas lidas de arquivos BMP.

Neste trabalho a translação, a rotação e a escala usam as funções do OpenGL `glTranslated`, `glRotated` e `glScaled`, permitidas pelo enunciado. A projeção em perspectiva usa `gluPerspective`.

## Funcionalidades

- Leitura de arquivos `.obj` com posições (`v`), coordenadas de textura (`vt`), normais (`vn`) e faces (`f`);
- Faces nos formatos `v`, `v/vt`, `v//vn` e `v/vt/vn`, incluindo índices negativos;
- Divisão de faces com mais de três cantos em triângulos;
- Cálculo das normais quando o arquivo não as fornece;
- Desenho das faces com `GL_TRIANGLES`: cada vértice leva sua normal, sua coordenada de textura e sua posição;
- Três luzes pontuais com componentes ambiente, difusa e especular, ligadas e desligadas individualmente;
- Ativação e desativação de cada componente (ambiente, difusa e especular) em todas as luzes;
- Texturas BMP aplicadas com as coordenadas `vt` do modelo, além de uma textura xadrez gerada no código;
- Translação, rotação e escala pelo teclado e pelo mouse;
- Modo wireframe e exibição das normais;
- Painel na janela com o modelo exibido, o estado de cada luz e as opções ativas.

## Tecnologias utilizadas

- C++17;
- OpenGL e GLU;
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
2. Abra o arquivo `ComputacaoGrafica/ComputacaoGrafica.sln` no Visual Studio 2022.
3. Na barra superior do Visual Studio, selecione a configuração `Debug`.
4. Selecione a plataforma `x64`.
5. Compile a solução com `Ctrl + Shift + B`.
6. Execute o programa com `Ctrl + F5`.

O programa deve ser executado pelo Visual Studio, que o roda dentro da pasta do projeto (`ComputacaoGrafica/ComputacaoGrafica`). É nessa pasta que ficam as pastas `modelos` e `texturas`.

## Modelos e texturas

Ao iniciar, o programa carrega, em ordem alfabética:

- todos os arquivos `.obj` da pasta `ComputacaoGrafica/ComputacaoGrafica/modelos`;
- todos os arquivos `.bmp` (24 bits, sem compressão) da pasta `ComputacaoGrafica/ComputacaoGrafica/texturas`.

Apenas um modelo é exibido por vez. A tecla `Tab` troca o modelo e a tecla `Y` troca a textura. Para usar outros modelos, como os disponíveis no AVA, basta copiar os arquivos `.obj` para a pasta `modelos`.

As texturas são as imagens da pasta `templates/Textures`, do exemplo de texturas da disciplina. Os arquivos de material (`.mtl`) não são lidos: a textura aplicada é sempre a escolhida com a tecla `Y`.

A pasta `modelos` traz quatro modelos de exemplo, cada um testando uma parte da leitura:

| Arquivo | Modelo | O que testa |
| --- | --- | --- |
| `cubo.obj` | Cubo escrito à mão | Faces de 4 cantos no formato `v/vt/vn` |
| `esfera.obj` | Esfera | Triângulos nos polos e quadrados no resto; textura cobrindo a esfera inteira |
| `lata.obj` | Cilindro | Tampas com 48 cantos e posições compartilhadas com normais e coordenadas de textura diferentes |
| `no_trevo.obj` | Nó de trevo | Arquivo sem `vt` e sem `vn`: as normais são calculadas pelo programa |

Também é possível informar arquivos `.obj` como argumentos do programa (em **Propriedades do projeto > Depuração > Argumentos de Comando**). Nesse caso, a pasta `modelos` não é lida.

## Controles

| Tecla ou ação | Operação |
| --- | --- |
| Setas | Move o modelo nos eixos X e Y |
| Page Up | Move o modelo no sentido positivo do eixo Z, aproximando-o da câmera |
| Page Down | Move o modelo no sentido negativo do eixo Z, afastando-o da câmera |
| Arrastar com o botão direito do mouse | Move o modelo nos eixos X e Y, acompanhando o cursor |
| W / S | Rotaciona o modelo em torno do eixo X |
| A / D | Rotaciona o modelo em torno do eixo Y |
| Q / E | Rotaciona o modelo em torno do eixo Z |
| Arrastar com o botão esquerdo do mouse | Rotaciona o modelo nos eixos X e Y |
| O / P | Diminui / aumenta a escala do modelo |
| Roda do mouse | Diminui / aumenta a escala do modelo |
| R | Liga ou desliga a rotação automática |
| K / L | Diminui ou aumenta a velocidade da rotação automática |
| Espaço | Volta o modelo ao estado inicial |
| Tab | Troca o modelo exibido |
| 1 / 2 / 3 | Liga ou desliga a luz principal, a lateral e a inferior |
| Z / X / C | Liga ou desliga a componente ambiente, difusa e especular de todas as luzes |
| T | Liga ou desliga a textura |
| Y | Troca a textura |
| M | Alterna entre faces preenchidas e wireframe |
| N | Mostra ou esconde as normais dos vértices |
| Esc | Encerra o programa |

As teclas de letras funcionam em minúsculo e em maiúsculo. A lista de controles também é exibida no console ao iniciar o programa.

## Estrutura principal do código

O arquivo `ComputacaoGrafica.cpp` contém toda a implementação do trabalho.

### Estruturas de dados

- `vertice`: coordenadas X, Y e Z de uma posição ou de uma normal;
- `coord_textura`: coordenadas U e V de um ponto da textura;
- `canto`: índices da posição, da coordenada de textura e da normal de um canto de face (`-1` quando o dado não existe);
- `triangulo`: os três cantos de uma face;
- `Modelo`: listas lidas do arquivo, triângulos, translação, rotação, escala e as display lists do modelo;
- `imagem_bmp` e `Textura`: imagem lida de um BMP e textura já enviada ao OpenGL;
- `Luz`: identificador, posição e componentes ambiente, difusa e especular de uma luz, além do estado ligada ou desligada.

### Funções principais

- `carregar_obj()`: lê o arquivo `.obj` linha a linha e monta as listas do modelo;
- `ler_canto()` e `converter_indice()`: interpretam cada canto de face e convertem seus índices;
- `calcular_normais()`: calcula as normais que faltam no arquivo;
- `centralizar()`: leva o modelo para a origem e ajusta seu tamanho;
- `carregar_bmp()` e `carregar_texturas()`: leem as imagens BMP e criam as texturas;
- `compilar()`: grava os triângulos e as normais do modelo em display lists;
- `desenhar()`: aplica as transformações e desenha o modelo;
- `aplicar_luzes()`: envia ao OpenGL as componentes e o estado de cada luz;
- `movimentar()`, `escalar()` e `rotacionar()`: alteram a translação, a escala e a rotação do modelo;
- `resetar()`: volta o modelo ao estado inicial;
- `desenhar_hud()`: escreve o painel de informações sobre a cena;
- `keyboard()`, `keyboard_special()`, `mouse()`, `mouse_arrasto()` e `mouse_roda()`: processam os comandos do teclado e do mouse.

## Leitura do arquivo .obj

Cada linha do arquivo começa com o tipo do dado:

```text
v  x y z        posição de um vértice
vt u v          coordenada de textura
vn x y z        normal
f  a b c ...    face
```

Cada canto de face pode indicar só a posição (`7`), a posição e a textura (`7/3`), a posição e a normal (`7//2`) ou os três dados (`7/3/2`). No `.obj` os índices começam em 1; índices negativos contam a partir do fim da lista lida até aquele ponto (`-1` é o último). As demais linhas, como comentários (`#`), grupos (`o`, `g`, `s`) e materiais (`mtllib`, `usemtl`), são ignoradas.

Faces com mais de três cantos são divididas em triângulos em leque, todos partindo do primeiro canto:

```text
f 1 2 3 4 5   ->   (1, 2, 3), (1, 3, 4), (1, 4, 5)
```

Triângulos que apontam para posições inexistentes são descartados. Índices de textura ou de normal fora das listas são tratados como ausentes.

Se algum canto não tiver normal, ela é calculada a partir das faces. A normal de cada triângulo é o produto vetorial de duas de suas arestas. Cada vértice soma as normais dos triângulos que o utilizam e o resultado é normalizado, o que deixa a iluminação suave entre faces vizinhas.

Como os arquivos `.obj` usam escalas muito diferentes, depois da leitura o modelo é centralizado. O centro da caixa que envolve o modelo vai para a origem e o vértice mais distante fica a 1 unidade do centro. Assim qualquer modelo aparece inteiro na tela.

## Iluminação

A cena tem três luzes pontuais (`w = 1` na posição), cada uma com uma cor para que o efeito de ligar e desligar seja fácil de perceber:

| Luz | Tecla | Posição | Cor |
| --- | --- | --- | --- |
| `GL_LIGHT0`, principal | 1 | `(1.0, 1.0, 1.5)`, acima e à direita | Branca |
| `GL_LIGHT1`, lateral | 2 | `(-1.6, 0.2, 0.8)`, à esquerda | Laranja |
| `GL_LIGHT2`, inferior | 3 | `(0.3, -1.2, 1.0)`, abaixo | Azul |

Cada luz define as componentes ambiente, difusa e especular com `glLightfv`. Ao desligar uma componente com `Z`, `X` ou `C`, ela é enviada zerada para todas as luzes, sem perder os valores originais. A luz ambiente global é zero: toda a luz da cena vem das três luzes. Por isso, com todas desligadas, o modelo fica preto.

A posição das luzes é enviada a cada quadro, depois da câmera e antes das transformações do modelo. Desse modo, as luzes ficam paradas na cena enquanto o modelo se move. Cada luz aparece como uma pequena esfera na sua cor quando ligada e cinza quando desligada.

O material dos modelos é cinza claro com reflexo especular (`GL_SHININESS` 40). Como `glScaled` também altera o tamanho das normais, `GL_NORMALIZE` está ativado para que a iluminação continue correta depois da escala.

## Texturas

A leitura dos arquivos BMP segue a função `getBMPData` do exemplo de texturas, com dois cuidados a mais:

- no arquivo, cada linha da imagem é completada com bytes extras até um múltiplo de 4;
- as cores estão na ordem BGR e são convertidas para RGB.

As imagens são enviadas ao OpenGL com `gluBuild2DMipmaps`, que também gera versões reduzidas da imagem para quando o modelo aparece pequeno na tela. O modo `GL_MODULATE` multiplica a cor da textura pela cor calculada na iluminação, de modo que o modelo texturizado continua iluminado. Modelos sem coordenadas de textura (`vt`) são desenhados sem textura.

Além das imagens, o programa cria uma textura xadrez no código, como no exemplo de texturas. Ela deixa fácil ver como as coordenadas de textura estão distribuídas no modelo.

## Transformações

O modelo guarda sua posição, os ângulos de rotação (em graus) e a escala de cada eixo. Ao desenhar, essas transformações são aplicadas com:

```cpp
glTranslated(posicao.x, posicao.y, posicao.z);
glRotated(rotacao.x, 1, 0, 0);
glRotated(rotacao.y, 0, 1, 0);
glRotated(rotacao.z, 0, 0, 1);
glScaled(escala.x, escala.y, escala.z);
```

O OpenGL aplica ao vértice primeiro a última transformação chamada. Assim, cada vértice é escalado, depois rotacionado e por último transladado. Como o modelo foi centralizado na origem ao ser carregado, a escala e a rotação acontecem em torno do seu centro.

Os triângulos de cada modelo são gravados uma única vez em uma display list (`glNewList`) e repetidos a cada quadro com `glCallList`, o que deixa leve o desenho de modelos grandes.

## Observações

- A configuração preparada e validada no Visual Studio é `Debug | x64`.
- Arquivos grandes demoram alguns segundos para carregar na configuração `Debug` (um modelo com 250 mil triângulos leva cerca de 5 segundos).
- Em computadores sem driver de vídeo, como máquinas virtuais que usam o OpenGL 1.1 do próprio Windows, o FreeGLUT escreve no console o aviso `fgInitGL2: fghGenBuffers is NULL`. O aviso não afeta o programa.
