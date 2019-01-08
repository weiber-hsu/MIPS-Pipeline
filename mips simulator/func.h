#ifndef FUNC_H
#define FUNC_H

#include<iostream>
#include<fstream>
#include<string>
#include<cstring>
#include<vector>
#include<deque>
#include<list>
typedef struct instuiction
{  
    bool IF=0,ID=0,EXE=0,MEM=0,WB=0;
    std::string ins_name,rs,rt,rd;
    int number; //beq used
}Instuiction;

typedef struct Signal
{
    char rd,al,br,mr,mw,mtr,rw;
}Signal;

typedef struct ex_ins
{
    std::string ins_name;
    int number,rs,rt,rd,temp;      
    Signal signal;      
}Exe;

//lw --->rd=reg ,rs=mem index,rt=mem,temp=rs/4
//sw --->rd=mem ,rs=mem index,rt=reg,temp=rs/4
//beq -->rd=jump,rs=reg      ,rt=reg,
extern std::vector<Instuiction> ins;
const std::string file="memory.txt";
extern int reg[32];
extern int words[32];
extern int cycle;

bool check(std::deque<Exe> a,bool stall);

void ins_fetch(Exe &a);
void ins_decode(Exe &a);
void ins_ex(Exe &a);
void ins_Mem(Exe &a);
void ins_wb(Exe a);

bool input();
void main_proc();

#endif