# Maquina+++ (M3+)

## O que é?

Maquina+++ (Machine+++/M3+) é um projeto da Universidade Regional de Blumenau (FURB) desenvolvido em 2014 por [Jean Jung](https://github.com/jejung) como uma evolução da sua primeira versão intitulada Maquina++ criada por [Jonathan M Borges](https://github.com/jonathan-m-borges/) em 2003. Todos os detalhes podem ser encontrados [aqui](https://github.com/jejung/maquina-plus-plus).


___
## Objetivo desse projeto

Esse projeto tem como objetivo criar uma versão da M3+ portável para o sistema operacional NuttX rodando em um ESP32 e, para isso, foi implementada em C no modelo de aplicação cliente-servidor. A interface web serve como meio de comunicação entre o cliente (usuário) e o restante da aplicação, que é composta pelos analisadores léxico, sintático e semântico e um interpretador para que seja possível executar o assembly.

___
## Características da Aplicação

### Fluxo

A aplicação foi separada em três partes: serviço web, analisador e interpretador. O serviço web é composto pela
interface web e pelo servidor, que recebe as requisições vindas da interface e encaminha para o analisador. O analisador,
responsável por analisar lexica, sintatica e semanticamente o código, poderá retornar para o servidor uma mensagem de erro caso encontre algum problema no código ou então irá enviar para o interpretador o assembly validado a fim de interpretá-lo, podendo se comunicar com o hardware ou retornar para o servidor uma mensagem de sucesso ou erro que será devolvida ao cliente, encerrando o fluxo.

<img src="imagens\fluxograma.png" style="display: block; margin: auto; width:50%;">

### Interface web

A interface possui 4 parâmetros importantes para a aplicação:
1. campo de texto para inserção de código assembly;
2. campo numérico para informar o tempo máximo de execução da aplicação em segundos;
3. campo numérico para informar o intervalo entre cada instrução também em segundos;
4. grupo de checkbox para alternar entre compilação e interpretação do código assembly.

O parâmetro 2 serve para evitar que códigos cíclicos executem indefinidamente, impossibilitando novas execuções. Já o parâmetro 3 serve para aumentar o tempo entre uma instrução e outra, permitindo visualizar o valor dos LEDs do hardware com facilidade. Além disso, é possível compilar código enquanto outro é interpretado, mas não é possível interpretar dois ao mesmo tempo.

<img src="imagens\interface.png" style="display: block; margin: auto; width:90%;">

### Hardware

O sistema embarcado construído para o projeto foi baseado na estrutura da M+++ e possui os seguintes
componentes:
* 1 ESP32-WROOM-32;
* 1 capacitor eletrolítico 1UF/63V (105°C);
* 1 MM74C922WMX;
* 1 protoboard disposto conforme imagem abaixo;
* 2 barras de pinos fêmea PCI 1X20 (Passo 2.54mm, 180°);
* 2 resistores 4K7 Ohms (SMD 0805 1/8W, 5% Precisão);
* 4 CD4511 (Decodificador para Display de 7 Segmentos);
* 4 displays 7 Segmentos HS-3191AS Catodo (cor vermelha, 7.5X13mm);
* 8 barras de pinos fêmea PCI 1x5 (Passo 2.54mm, 180°);
* 8 PCFs 8574P;
* 12 capacitores cerâmicos 100NF/50V (SMD 0805);
* 12 soquetes estampados 16 Pinos;
* 16 LEDs SMD 0805 (cor vermelha);
* 16 resistores 220R Ohms (SMD 0805 1/8W, 5% Precisão);
* 32 chaves Tácteis KFC-A06-3X6X2.5 - 2T 180G SMD;
* 32 resistores 10K Ohms (SMD 0805 1/8W, 1% Precisão);
* 32 resistores 470R Ohms (SMD 0805 1/8W, 5% Precisão);

<img src="imagens\hardware.png">

___
## Configuração do NuttX

Algumas configurações são necessárias no NuttX para que seja possível rodar a aplicação. [Nesse arquivo](config/NuttX.pdf) são listados todos os passos e [aqui](config/defconfig) o arquivo de configuração do NuttX.

___
## Características do assembly

### Conjunto de instruções
|Tipo|Instruções|
|-|-|
|Matemáticas|**ADD**, **INC**, **SUB**|
|Lógicas|**AND**, **OR**, **XOR**, **NOT**|
|Alteração de fluxo|**JMP**, **JMPC**, **JMPZ**, **CALL**, **RET**|
|Sobreposição|**MOV**|
|Deslocamento|**POP**, **PUSH**|

### Registradores
|#|Registrador|
|:-:|:-------------:|
|1|**A**|
|2|**B**|
|3|**C**|
|4|**D**|
|5|**E**|

### Portas I/O
|Tipo|Portas|
|-|-|
|Entrada (input)|**IN0**, **IN1**, **IN2**, **IN3**|
|Saída (output)|**OUT0**, **OUT1**, **OUT2**, **OUT3**|

### BNF
```
<program> ::= <instruction>*
<instruction> ::= <label>? <operation> <parameter> <coma> <parameter> <semicolon>
<label> ::= <alpha> (<alpha>)* <colon>
<operation> ::= "ADD" | "AND" | "CALL" | "INC" | "JMP" | "JMPC" | "JMPZ" | "MOV" | "NOT" | "OR" | "POP" | "PUSH" | "RET" | "SUB" | "XOR"
<parameter> ::= <register> | <input> | <output> | <hexa>
<register> ::= "A" | "B" | "C" | "D" | "E"
<input> ::= "IN0" | "IN1" | "IN2" | "IN3"
<output> ::= "OUT0" | "OUT1" | "OUT2" | "OUT3"
<hexa> ::= "00" | ... | "FF"
<alpha> ::= "a" | ... | "z" | "A" | ... | "Z"
<coma> ::= ","
<semicolon> ::= ";"
<colon> ::= ":"
```

### Exemplos
```
LEITURA:
MOV IN0,A;
MOV A,B;
MOV IN1,A;
AND B,A;
MOV A,B;
MOV IN2,A;
NOT A,A;
MOV A,C;
MOV IN3,A;
OR C,A;
AND B,A;
MOV A,OUT3;
JMP LEITURA;
```
```
LOOP:
MOV FF, A;
MOV A, OUT0;
MOV 00, A;
MOV A, OUT0;
JMP LOOP;
```
```
MOV 00,B;
MOV 00,C;
MOV 00,D;
MOV 00,E;
DISPLAY:
MOV B,A;
MOV A,OUT1;
MOV C,A;
MOV A,OUT0;
MOV D,A;
MOV A,OUT2;
MOV E,A;
MOV A,OUT3;
MOV B,A;
INC A,B;
MOV B,A;
SUB 0A,A;
JMPZ INCDEZSS;
JMP DISPLAY;
INCDEZSS:
MOV 00,B;
MOV C,A;
INC A,C;
MOV C,A;
SUB 06,A;
JMPZ INCUNIDMM;
JMP DISPLAY;
INCUNIDMM:
MOV 00,C;
MOV D,A;
INC A,D;
MOV D,A;
SUB 0A,A;
JMPZ INCDEZMM;
JMP DISPLAY;
INCDEZMM:
MOV 00,D;
MOV E,A;
INC A,E;
MOV E,A;
SUB 06,A;
JMPZ ZERADEZMM;
JMP DISPLAY;
ZERADEZMM:
MOV 00,E;
JMP DISPLAY;
```
```
LOOP:
MOV 44,A;
MOV A,OUT1;
JMP LOOP;
```
```
LOOP:
MOV 55,A;
MOV 66,B;
ADD B,A;
MOV A,OUT1;
JMP LOOP;
```
```
LOOP:
MOV 55,A;
MOV 66,B;
CALL SUM;
JMP LOOP;
SUM:
ADD A,B;
RET;
JMP LOOP;
```
```
MOV 11,B;
MOV 11,C;
MOV 11,D;
MOV 11,E;
MOV B,A;
MOV C,A;
MOV D,A;
MOV E,A;
MOV A,OUT1;
MOV E,A;
MOV A,OUT2;
LOOP:
JMP LOOP;
```