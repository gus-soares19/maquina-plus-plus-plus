# Maquina+++ (M3+)

## O que é?

Maquina+++ (Machine+++/M3+) é um projeto da Universidade Regional de Blumenau (FURB) desenvolvido em 2014 por [Jean Jung](https://github.com/jejung) como uma evolução da sua primeira versão intitulada Maquina++ criada por [Jonathan M Borges](https://github.com/jonathan-m-borges/) em 2003. Todos os detalhes podem ser encontrados [aqui](https://github.com/jejung/maquina-plus-plus).


___
## Objetivo desse projeto

Esse projeto tem como objetivo criar uma versão da M3+ portável para o sistema operacional NuttX rodando em um ESP32 e, para isso, foi implementada em C no modelo de aplicação cliente-servidor. A interface web serve como meio de comunicação entre o cliente (usuário) com o restante da aplicação, que é composta pelos analisadores léxico, sintático e semântico e um interpretador para que seja possível executar o assembly.

___
## Características da Aplicação

### Fluxo

A aplicação foi separada em três partes: serviço web, analisador e interpretador. O serviço web é composto pela
interface web e pelo servidor, que recebe as requisições vindas da interface e encaminha para o analisador. O analisador,
responsável por analisar lexica, sintatica e semanticamente o código, poderá retornar para o servidor uma mensagem de erro caso encontre algum problema no código ou então irá enviar para o interpretador o assembly validado a fim de interpretá-lo, podendo se comunicar com o hardware ou retornar para o servidor uma mensagem de sucesso ou erro que será devolvida ao cliente, encerrando o fluxo.

<img src="imagens\fluxograma.png">

### Interface web

A interface possui 4 parâmetros importantes para a aplicação:
1. campo de texto para inserção de código assembly;
2. campos de texto enumerados de IN0 até IN3 representando as portas INPUT da M3+;
3. campo numérico para informar o tempo máximo de execução da aplicação em segundos;
4. campo numérico para informar o intervalo entre cada instrução também em segundos.

O parâmetro 3 serve para evitar que códigos cíclicos executem indefinidamente, impossibilitando novas execuções. Já o o parâmetro 4 serve para aumentar o tempo entre uma instrução e outra, permitindo visualizar o valor dos LEDs do hardware com facilidade.

<img src="imagens\interface.png">

### Hardware

O sistema embarcado construído para o projeto foi baseado na estrutura da M+++ e possui as seguintes
características:
* 1 ESP32-WROOM-32;
* 4 PCF8574P;
* 1 protoboard;
* 32 resistores CR12 – 1/8W – 330 Ohms;
* 2 resistores CR12 – 1/8W – 10K Ohms;
* 32 LEDs vermelhas;
* jumpers macho-macho.

<img src="imagens\hardware.png">

O ESP32, contornado pelo quadrado vermelho, é o microcontrolador em que o NuttX está instalado e a aplicação
é executada. Para se comunicar com os dispositivos I2C PCF8574P, as portas definidas como GPIO21 e GPIO22, SDA e
SCL respectivamente, passam pelos resistores R1 e R2 de 10K Ohms para reduzir a corrente e então se conectam às portas
15 e 14 de cada PCF8574P. O quadrado verde agrupa os dispositivos I2C, os pinos P0 até P7 estão conectados no lado
positivo de cada LED, destacados no quadrado roxo, e o lado negativo se conecta ao GND do ESP32 por meio de resistores
de 330 Ohms.

___
## Configuração do NuttX

Algumas configurações são necessárias no NuttX para que seja possível rodar a aplicação. [Nesse arquivo](config/NuttX.pdf) são listados todos os passos.

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