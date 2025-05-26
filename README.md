This is a Karel interpreter, a modified version of Omegaup's Karel.js. It provides minor changes to work with [CMS ReKarel](https://github.com/kishtarn555/cms_rekarel)

# Building the project

Clone the project, then run
```
mkdir bin
```
Next run
```
make karel
```

# Installing
After building the project run:
```
sudo install -m 755 karel /usr/local/bin
```

# Usage

`karel bytecode.kp [--world | --result]`

The executable takes the world.in through stdin.

## Arguments 
* `bytecode.kp`, path to a bytecode file, the compiled code.
* `--result`, if passed, it passes the result (world.out) to stdout (**default behaviour**)
* `--world`, if passed, it outputs to stdout the input world

## Run behaviour
Depending on the run of the code, it may give a certain exit signal and output to stderr

| Status                  | Exit signal | Stderr                                | Description                                                        |
|-------------------------|-------------|---------------------------------------|--------------------------------------------------------------------|
| USAGE                   | 1           | USAGE karel [--dump={world,result}] program.kx < world.in > world.out | The arguments of the command were wrong. |
| OK                      | 0           | ----                                  | No error, execution succeeded.                                     |
| WALL                    | 16          | MOVIMIENTO INVALIDO                   | Karel tried to move into a wall.                                   |
| WORLDUNDERFLOW          | 17          | ZUMBADOR INVALIDO (MUNDO)             | Karel tried to take a beeper on an empty cell.                     |
| BAGUNDERFLOW            | 18          | ZUMBADOR INVALIDO (MOCHILA)           | Karel tried to leave a beeper with an empty bag.                   |
| STACK                   | 19          | STACK OVERFLOW                        | Karel suffered a stack overflow.                                   |
| STACKMEMORY             | 20          | LIMITE DE MEMORIA DEL STACK           | Karel exceeded the stack memory limits.                            |
| CALLSIZE                | 21          | LIMITE DE LONGITUD DE LLAMADA         | Karel exceeded the number of parameters permitted in a call.       |
| INTEGEROVERFLOW         | 22          | INTEGER OVERFLOW                      | Karel exceeded the upper limit of a parameter.                     |
| INTEGERUNDERFLOW        | 23          | INTEGER UNDERFLOW                     | Karel exceeded the lower limit of a parameter.                     |
| WORLDOVERFLOW           | 24          | DEMASIADOS ZUMBADORES (MUNDO)         | Karel exceeded the upper limit of beepers in a cell.               |
| INSTRUCTION             | 48          | LIMITE DE INSTRUCCIONES GENERAL       | Karel exceeded the general number of allowed instructions.         |
| INSTRUCTION_LEFT        | 49          | LIMITE DE INSTRUCCIONES IZQUIERDA     | Karel exceeded the number of allowed turnleft.                     |
| INSTRUCTION_FORWARD     | 50          | LIMITE DE INSTRUCCIONES AVANZA        | Karel exceeded the number of allowed move.                         |
| INSTRUCTION_PICKBUZZER  | 51          | LIMITE DE INSTRUCCIONES COGE_ZUMBADOR | Karel exceeded the number of allowed pickbeeper.                   |
| INSTRUCTION_LEAVEBUZZER | 52          | LIMITE DE INSTRUCCIONES DEJA_ZUMBADOR | Karel exceeded the number of allowed putbeeper.                    |

> Notice that what is usually considered RTE has only 16 bit on, while errors that are considered TLE or Instruction limit exceeded (ILE) have both the 16 and 32 bit on.

## ReKarel project map

Here's a map for exploring the ReKarel project:


| Repo  | Description |
| --- | --- |
| [ReKarel](https://github.com/kishtarn555/ReKarel/) | Web IDE for ReKarel | 
| [Core](https://github.com/kishtarn555/rekarel-core) | JS compiler, interpreter and transpiler |
| [CLI](https://github.com/kishtarn555/rekarel-cli) | Node command line interface for the core |
| **CPP Interpreter** | Faster C++ interpreter, runs bytecode compiled by the CLI compiler |
| [CMS](https://github.com/kishtarn555/cms_rekarel) | Adds ReKarel support to [CMS](https://github.com/cms-dev/cms) |
| [KarelCaseGenerator](https://github.com/kishtarn555/KarelCaseGenerator/) | Python Case Generator |

![image](https://github.com/user-attachments/assets/a0f155d3-780a-41dd-a2a2-89ebbd04a2b3)
