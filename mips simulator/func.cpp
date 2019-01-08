#include "func.h"

using namespace std;
vector<Instuiction> ins;
int reg[32];
int words[32];
int cycle=1;
vector<string> split(string phrase, string delim) {
    vector<string> data;
	char* str = &phrase[0];
	char * pch;
	pch = strtok (str, delim.c_str());
	while (pch != NULL ) {
		data.push_back(string(pch));
		pch = strtok (NULL , delim.c_str());
	}
	return data;
}

bool input()
{
    fstream f;
    f.open(file);
    if(!f.is_open())   return 0; //not open return false
    string line;
    int num=1;
    while(getline(f, line))     //save instruction in vect
    {
        if (line==""||line=="\n") break;
        Instuiction temp;
        vector<string> temp2=split(line," ,"); 
        temp.ins_name=temp2[0];
        if(temp2[0]=="add"||temp2[0]=="sub")
        {
            temp.rd=temp2[1];
            temp.rs=temp2[2];
            temp.rt=temp2[3];
        }
        else if (temp2[0]=="beq")
        {
            temp.rd=temp2[3];
            temp.rs=temp2[1];
            temp.rt=temp2[2];
        }
        else if(temp2[0]=="sw")   
        {
            temp.rd=temp2[2];
            temp.rs="";
            temp.rt=temp2[1];
        }
        else    //lw
        {
            temp.rd=temp2[1];
            temp.rs=temp2[2];
            temp.rt="";
        }
        temp.number=num;
        ins.push_back(temp);
        num++;
    }    
    f.close();
    return 1;
}

void main_proc()
{
    for(int i=0;i<32;i++) //init reg mem (reg0=0 else=1)
    {
        if(i==0) reg[i]=0;
        else reg[i]=1;
        words[i]=1;
    }
    int pc=1;
    bool stall=0;
    deque<Exe> state(5);
    fstream out;
    out.open("result.txt", fstream::out|fstream::trunc);
    if(!out.is_open())
    {
        cout<<"error"<<endl;
        return;
    }
    while(1)    //infinit loooooop
    {
        Exe executing;    
        executing.number=0;             // number 0 =no ins
        if(pc<=ins.size()&& !stall )
        {
            executing.number=pc;
            state.push_front(executing);   
            pc++;     
        }
        else if (stall) //stall insert empty struct in ex
        {     
            deque<Exe>::iterator it=state.begin();
            it+=2;
            state.insert(it,executing); 
        }   
        else //no instruct
        {
            state.push_front(executing);
        }   
        if(state.size()==6) state.pop_back();//5 state    

        if(state[4].number!=0) ins_wb(state[4]);//wb
        if(state[3].number!=0) ins_Mem(state[3]);//mem
        if(state[2].number!=0) ins_ex(state[2]);//exe    
        
        if(state[1].number!=0 && !stall) ins_decode(state[1]);//id   
        stall=check(state,stall);
        
                 
        if(state[0].number!=0) ins_fetch(state[0]);//if
        
        out<<"cycle :"<<cycle<<endl;
        for(int i=4;i>=0;i--) 
            if(state[i].number!=0) 
            { 

                Exe t=state[i];
                if(i==0)      out<<"    "<<t.ins_name<<" : IF  "<<endl;
                else if(i==1) out<<"    "<<t.ins_name<<" : ID  "<<endl;
                else if(i==2) out<<"    "<<t.ins_name<<" : EXE "
                                        <<t.signal.rd<<t.signal.al
                                        <<" "<<t.signal.br<<t.signal.mr<<t.signal.mw
                                        <<" "<<t.signal.rw<<t.signal.mtr<<endl;
                else if(i==3) out<<"    "<<t.ins_name<<" : MEM "
                                        <<" "<<t.signal.br<<t.signal.mr<<t.signal.mw
                                        <<" "<<t.signal.rw<<t.signal.mtr<<endl;
                else          out<<"    "<<t.ins_name<<" : WB  "
                                        <<" "<<t.signal.rw<<t.signal.mtr<<endl;
            }

        if(state[1].ins_name=="beq" && !stall && reg[state[1].rs]==reg[state[1].rt])
        {
            pc+=state[1].rd-1;
            state[0].number=0;
        }
        
        if(ins[ins.size()-1].WB) //last ins finish
            break;
        cycle++;
    }
    out<<"cycle number : "<<cycle<<endl;
    out<<"r   w"<<endl;
    for(int i=0;i<32;i++) out<<reg[i]<<"    "<<words[i]<<endl;
    out.close();
}

void ins_fetch(Exe &a)
{
    a.ins_name=ins[a.number-1].ins_name;
    ins[a.number-1].IF=1;
}

void ins_decode(Exe &a)  //get rs rt rd and convert to int
{
    int index =a.number - 1;  
    vector<string> str2=split(ins[index].rt,"$");     
    if (a.ins_name=="add"||a.ins_name=="sub")   
    {       
        vector<string> str1=split(ins[index].rs,"$");
        vector<string> str3=split(ins[index].rd,"$");
        a.rs=stoi(str1[0]);    
        a.rt=stoi(str2[0]);    
        a.rd=stoi(str3[0]);
        a.signal.rd='1';
        a.signal.al='0';
        a.signal.br='0';
        a.signal.mr='0';
        a.signal.mw='0';
        a.signal.rw='1';
        a.signal.mtr='0';
    }
    else if(a.ins_name=="lw")
    {
         vector<string> str1=split(ins[index].rs,"$()");
         vector<string> str3=split(ins[index].rd,"$");
         a.rs=stoi(str1[0]);  
         a.rt=stoi(str1[1]);
         a.rd=stoi(str3[0]);
         a.signal.rd='0';
         a.signal.al='1';
         a.signal.br='0';
         a.signal.mr='0';
         a.signal.mw='0';
         a.signal.rw='1';
         a.signal.mtr='1';
    }
    else if(a.ins_name=="sw")
    {
         vector<string> str3=split(ins[index].rd,"$()");
         a.rs=stoi(str3[0]);  
         a.rt=stoi(str2[0]);
         a.rd=stoi(str3[1]);
         a.signal.rd='X';
         a.signal.al='1';
         a.signal.br='0';
         a.signal.mr='0';
         a.signal.mw='1';
         a.signal.rw='0';
         a.signal.mtr='X';
    }
    else //beq
    {
        vector<string> str1=split(ins[index].rs,"$");
        vector<string> str3=split(ins[index].rd,"$");
        a.rs=stoi(str1[0]);
        a.rt=stoi(str2[0]);
        a.rd=stoi(str3[0]);
        a.signal.rd='X';
        a.signal.al='0';
        a.signal.br='1';
        a.signal.mr='0';
        a.signal.mw='0';
        a.signal.rw='0';
        a.signal.mtr='X';
    }
    ins[index].ID=1;
}

bool check(deque<Exe> a,bool stall)
{
    if(stall)
        if(a[2].number && a[3].number)  return 0;
    if(a[2].number!=0){
        if(a[1].rs==a[2].rd||a[1].rt==a[2].rd) return 1;
    }
    if(a[3].number!=0){
        if(a[1].rs==a[3].rd||a[1].rt==a[3].rd) return 1;
    }
}

void ins_ex(Exe &a)
{
    if (a.ins_name=="add")
    {
        a.temp=reg[a.rs]+reg[a.rt];
    }
    else if(a.ins_name=="sub")
    {
        a.temp=reg[a.rs]-reg[a.rt];
    }
    else if(a.ins_name=="lw"||a.ins_name=="sw")
    {
        a.temp=a.rs/4;
    }
    
    ins[a.number-1].EXE=1;
}

void ins_Mem(Exe &a)
{
    if(a.ins_name=="sw") words[a.temp+a.rd]=reg[a.rt];
    ins[a.number-1].MEM=1;
}

void ins_wb(Exe a)
{
    if (a.ins_name=="add"||a.ins_name=="sub")
    {
        reg[a.rd]=a.temp;
    }
    else if (a.ins_name=="lw")
    {
        reg[a.rd]=words[a.temp+a.rt];
    }
    ins[a.number-1].WB=1;
}