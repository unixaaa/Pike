#ifndef PEEP_H
#define PEEP_H

#include "dynamic_buffer.h"
extern dynamic_buffer instrbuf;

/* Prototypes begin here */
struct p_instr_s;
void init_bytecode();
void exit_bytecode();
int insert_opcode(unsigned int f,
		  INT32 b,
		  INT32 current_line,
		  struct pike_string *current_file);
int insert_opcode2(int f,int current_line, struct pike_string *current_file);
void update_arg(int instr,INT32 arg);
void ins_f_byte(unsigned int b);
void assemble();
void asm_opt();
/* Prototypes end here */

#endif
