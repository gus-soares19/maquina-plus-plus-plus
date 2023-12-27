#ifndef CONTEXTUALIZER_CONSTANTS_H
#define CONTEXTUALIZER_CONSTANTS_H

#define AC_AC__ROM_AC__RET 11
#define AC_REG__ROM_REG__DRRAM_AC 12
#define AC_RAM__ROM_RAM__AC_DRAM 13
#define AC_OUT__JMP__PUSHREG 14
#define REG_AC__JMPC__POPREG 15
#define RAM_AC__JMPZ__PUSHA 16
#define IN_AC__CALL__POPA 17
#define DONT_USE 18

#define FIRST_DUMMY_BYTE 0xABCD
#define SECOND_DUMMY_BYTE 0x16384
#define ADD 1
#define SUB 2
#define AND 3
#define OR 4
#define XOR 5
#define NOT 6
#define MOV 7
#define INC 8

#define PROGRAM_START 50
#define INSTRUCTION_START 51
#define PROGRAM_END 52
#define HD 10
#define IN_OUT_FLAG 35
#define RAM_RECORD 36
#define ROM_PREPARE 37
#define AUX_RECORD 38
#define BYTE_RECORD 40
#define REND 41
#define LABEL_RECORD 43
#define LABEL_TRANSLATE 42

#define REG_B 31
#define REG_C 32
#define REG_D 33
#define REG_E 34

#define _FF 0xFF
#define _1F 0x1F
#define _3F 0x3F
#define _20 0x20
#define _5F 0x5F
#define _40 0x40
#define _7F 0x7F
#define _60 0x60
#define _9F 0x9F
#define _80 0x80
#define _BF 0xBF
#define _A0 0xA0
#define _DF 0xDF
#define _C0 0xC0
#define _E0 0xE0
#define _07 0x07
#define _F8 0xF8
#define _F9 0xF9
#define _01 0x01
#define _FA 0xFA
#define _02 0x02
#define _FB 0xFB
#define _03 0x03
#define _FC 0xFC
#define _04 0x04
#define _FD 0xFD
#define _05 0x05
#define _FE 0xFE
#define _06 0x06
#define _E7 0xE7
#define _08 0x08
#define _EF 0xEF
#define _10 0x10
#define _F7 0xF7
#define _18 0x18

#define IN0 "IN0"
#define IN1 "IN1"
#define IN2 "IN2"
#define IN3 "IN3"
#define OUT0 "OUT0"
#define OUT1 "OUT1"
#define OUT2 "OUT2"
#define OUT3 "OUT3"

#define HEXA_PREFIX "0x"

#endif
