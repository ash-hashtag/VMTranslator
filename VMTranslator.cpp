#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <chrono>


using namespace std;

struct Timer{
    long long m_start, m_end;
    Timer()
    {
        m_start = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
    }
    ~Timer()
    {
        m_end = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
        double duration = (m_end - m_start) * 0.001;
        cout << "Time taken: " << duration << " ms\n";
    }
};

namespace fs = std::filesystem;
using std::ofstream;

string filename = "";
unsigned short int currentline = 0;
unsigned short int labelNumber = 0;
const string endCode = "(END)\n@END\n0;JMP\n", initCode = "//Init\n@261\nD=A\n@SP\nM=D\n";
ofstream outfile;

//PARSER//
string cmd = "", type = "";
unsigned int indx;
string AsmCode = "";

string InsManager();
void segments(string ins);
string processIns(string line);
string outputpath(string filepath);

void GetCurrentLine(string inss)
{
    unsigned short int i;
    for (i = 0; inss[i]; i++)
    {
        if (inss[i] == '\n')
            currentline++;
        if (inss[i] == '(')
            currentline--;
    }
}

void WriteOut(string inpath)
{
    fstream file;
    cout << "Opening: "<< inpath << endl;
    file.open(inpath, ios::in);
    if (file.is_open() && outfile.is_open())
    {
        unsigned short int i = 100;
        string tp;
        while (getline(file, tp) || i--)
        {
            if (tp.length())
            {
                segments(processIns(tp));
                AsmCode = InsManager();
                GetCurrentLine(AsmCode);
                if (cmd.length())
                    outfile<<"// "<< tp << endl;
                outfile<<AsmCode;
            }
        }
    }
    else
        cout<< "ERROR Opening files\n";
    file.close();
}


string processIns(string line)
{
    unsigned int i = 0, j = 0;
    unsigned short int cpos = 0;
    for (i = (line.length()-1); i > 0; i--)
    {
        if (line[i] == '/')
            cpos = i;
        if (((line[i] >= '0' && line[i] <= '9') || (line[i] >= 'a' && line[i] <= 'z') || (line[i] >= 'A' && line[i] <= 'Z')) && i < cpos)
        {
            j = ++i;
            break;
        }
    }
    i = 0;
    if ( cpos == 0 )
        j = line.length();
    if (line[0] == ' ')
    {
        for (i = 0; line[i] == ' '; i++);
    }
    if (line[0] == '/')
        j = i;
    return line.substr(i, j-i);
}

void segments(string ins)
{
    cmd = "";
    type = "";
    indx = 0;
    unsigned short int i;
    if (ins.length())
    {
        for (i = 0; ins[i] != ' ' && ins[i]; i++)
            cmd += ins[i];
        if (i < ins.length())
            for(i = ++i; ins[i] != ' ' && ins[i]; i++)
                type += ins[i];
        if (i < ins.length())
            for (i = ++i; ins[i] != ' ' && ins[i] != '\t' && ins[i] != '/' && ins[i]; i++)
                indx = indx*10 + (unsigned int)(ins[i] - 48); 
    }
}

void writeDir(string filepath)
{
    for (const auto & entry : fs::directory_iterator(filepath))
    {
        string fpath = entry.path().string();
        //cout << fpath << endl;
        if (fpath.find(".vm") != string::npos)
        {
            //get FileName
            string reset = outputpath(fpath);
            if (filename == "Sys")
                WriteOut(fpath);
            //call a function writes out asm
        }
    }
    for (const auto & entry : fs::directory_iterator(filepath))
    {
        string fpath = entry.path().string();
        if (fpath.find(".vm") != string::npos)
        {
            //get FileName
            string reset = outputpath(fpath);
            if (filename != "Sys")
                WriteOut(fpath);
            //call a function writes out asm
        }
    }
}
bool isDir = false;
string outputpath(string filepath)
{
    string opath = "";
    char slash = (filepath.find('\\') != string::npos) ? '\\' : '/';
    unsigned short int j = filepath.find_last_of(slash) + 1, i = filepath.length()-1;
    if (filepath.find(".vm") != string::npos)
    {
        i = filepath.find(".vm");
        opath = filepath.substr(0, i);
        filename = filepath.substr(j, i-j);
    }
    else
    {
        opath = filepath;
        if (filepath[i] == slash)
        {
            j = filepath.find_last_of(slash, i-1) +1;
        }
        else
        {
            i++;
            opath += slash;
        }
        filename = filepath.substr(j, i-j);
        opath += filename;
        isDir = true;
    }
    return opath + ".asm";
}

string getLabels(string label)
{
    if (label == "local")
        return "@LCL\n";
    else if (label == "argument")
        return "@ARG\n";
    else if (label == "this")
        return "@THIS\n";
    else if (label == "that")
        return "@THAT\n";
    else if(label == "temp")
        return "@R5\n";
    else
        return "";
}

//CODEWRITER

string writeArithmetic(string ins)
{
    string asmCode = "";
    //or
    if (ins == "or")
    {
        asmCode +="@SP\n"
                "AM=M-1\n"
                "D=M\n"
                "A=A-1\n"
                "M=D|M\n";
    }

    //eq gt lt 
    else if (ins.length() == 2)
    {
        asmCode += "@SP\n"
                    "AM=M-1\n"
                    "D=M\n"
                    "A=A-1\n"
                    "D=D-M\n"
                    "@";
        asmCode += to_string(currentline + 7 + 5);
        asmCode += "\n";

        if (ins == "eq")
            asmCode += "D;JEQ\n";
        else if (ins == "lt")
            asmCode += "D;JGT\n";
        else if (ins == "gt")
            asmCode += "D;JLT\n";
        
        asmCode += "@SP\n"
                    "A=M-1\n"
                    "M=0\n"
                    "@";
        asmCode += to_string(currentline + 7 + 5 + 3);
        asmCode +="\n0;JMP\n"
                    "@SP\n"
                    "A=M-1\n"
                    "M=-1\n";
    }
    
    //add sub and not neg
    else if(ins.length() == 3)
    {
        if (ins == "not")
        {
            asmCode += "@SP\n"
                        "A=M-1\n"
                        "M=!M\n";
        }
        else if (ins == "neg")
        {
            asmCode += "D=0\n"
                        "@SP\n"
                        "A=M-1\n"
                        "M=D-M\n";
        }
        else
        {
            asmCode +="@SP\n"
                    "AM=M-1\n"
                    "D=M\n"
                    "A=A-1\n";
            if (ins == "add")
                asmCode += "M=D+M\n";
            else if (ins == "sub")
                asmCode += "M=M-D\n";
            else if (ins == "and")
                asmCode += "M=D&M\n";
        }
    }
    return asmCode;
}

string writeGoto(string label)
{
    return "@" + label +"\n0;JMP\n";
}

string writeLabels(string label)
{
    return "(" + label + ")\n";
}

string writeIf(string label)
{
    return "@SP\nAM=M-1\nD=M\n@"+ label + "\nD;JNE\n";
}

string writeCallFunction(string funcName, unsigned short int numArgs)
{
    const string finishpush = "@SP\nA=M\nM=D\n@SP\nM=M+1\n";
    string asmCode = "@return-address";
    asmCode += to_string(labelNumber);
    asmCode += "\nD=A\n" + finishpush;

    asmCode += "@LCL\nD=M\n" + finishpush;

    asmCode += "@ARG\nD=M\n" + finishpush;

    asmCode += "@THIS\nD=M\n" + finishpush;

    asmCode += "@THAT\nD=M\n" + finishpush;

    //reposition ARG
    asmCode += "@SP\nD=M\n@" + to_string(numArgs);
    asmCode += "\nD=D-A\n@5\nD=D-A\n@ARG\nM=D\n";

    //reposition LCL
    asmCode += "@SP\nD=M\n@LCL\nM=D\n";

    //transfer Control
    asmCode += writeGoto(funcName);

    //declare return address
    asmCode += writeLabels("return-address" + to_string(labelNumber));
    
    labelNumber++;
    
    return asmCode;
}

string writeReturn()
{
    //set frame = lcl
    return "@LCL\nD=M\n@R11\nM=D\n"

    //set RET = FRAME - 5
    "@5\nA=D-A\nD=M\n@R12\nM=D\n"

    //set ARG = pop()
    //"@ARG\nD=M\n@0\nD=D+A\n@R13\nM=D\n@SP\nAM=M-1\n"
    //"D=M\n@R13\nA=M\nM=D\n"
    "@SP\nAM=M-1\nD=M\n@ARG\nA=M\nM=D\n"

    //restore SP of the caller
    "@ARG\nD=M\n@SP\nM=D+1\n"

    //restor THIS of the caller
    "@R11\nD=M-1\nAM=D\nD=M\n@THAT\nM=D\n"

    //restor THIS of the caller
    "@R11\nD=M-1\nAM=D\nD=M\n@THIS\nM=D\n"

    //restore ARG of the caller
    "@R11\nD=M-1\nAM=D\nD=M\n@ARG\nM=D\n"

    //restore LCL of the caller
    "@R11\nD=M-1\nAM=D\nD=M\n@LCL\nM=D\n"

    //goto RET
    "@R12\nA=M\n0;JMP\n";

}

string writePushPop(bool c, string typ, unsigned short int index)
{
    string asmCode = "", ind = to_string(index);
    if (c)
    {
        bool f = false;
        if (typ == "pointer")
        {
            if (index == 0)
                asmCode += "@THIS\n";
            else if (index == 1)
                asmCode += "@THAT\n";
            asmCode += "D=M\n";
            f = true;
        }

        else if (typ == "static")
        {
            
            asmCode += "@";
            asmCode += filename;
            asmCode += ".";
            asmCode += ind;
            asmCode += "\nD=M\n";
            f = true;
        }
        else if (typ == "constant" || index > 0)
        {
            asmCode += "@";
            asmCode += ind;
            asmCode += "\nD=A\n";
            if (typ == "constant")
                f = true;
        }
        
        else if (index == 0 && typ != "constant"){
            asmCode += getLabels(typ);
            asmCode += "A=M\nD=M\n";
            f = true;
        }

        if (!(f) && index > 0)
        {
            asmCode += getLabels(typ);
            if (typ == "temp")
                asmCode += "A=D+A\n";
            else
                asmCode += "A=D+M\n";
            asmCode += "D=M\n";
        }
        asmCode +=  "@SP\n"
                    "A=M\n"
                    "M=D\n"
                    "@SP\n"
                    "M=M+1\n";
    }

    //pop
    else
    {
        if (index == 0 || typ == "pointer" || typ == "static")
        {
            asmCode += "@SP\n"
                        "AM=M-1\n"
                        "D=M\n";
            if (typ == "pointer")
            {
                if (index == 0)
                    asmCode += "@THIS\n";
                else if (index == 1)
                    asmCode += "@THAT\n";
            }
            else if (typ == "static")
            {
                asmCode += "@";
                asmCode += filename;
                asmCode += ".";
                asmCode += ind;
                asmCode += "\n";

            }
            else
                asmCode += getLabels(typ);
        }

        else if (index > 0)
        {
            asmCode += "@";
            asmCode += ind;
            asmCode += "\nD=A\n";
            asmCode += getLabels(typ);
            if (typ == "temp")
                asmCode += "D=D+A\n";
            else
                asmCode += "D=D+M\n";

            asmCode += "@R13\n"
                        "M=D\n"
                        "@SP\n"
                        "AM=M-1\n"
                        "D=M\n"
                        "@R13\n";
            if (type == "temp")
                asmCode += "A=M\n";
        }
        if (type != "pointer" && type != "static" && type != "temp")
            asmCode += "A=M\n";
        asmCode += "M=D\n";
    }
    return asmCode;
}

string writeFunction(string funcName, unsigned short int numLocals)
{
    string asmCode = writeLabels(funcName);
    for (unsigned short int i = 0; i < numLocals; i++)
        asmCode += writePushPop(true, "constant", 0);
    return asmCode;
}


string InsManager()
{
    if (cmd.length())
    {
        //push
        if (cmd == "push")
            return writePushPop(true, type, indx);
        //pop
        else if (cmd == "pop")
            return writePushPop(false, type, indx);
        //Label
        else if (cmd == "label"){
            return writeLabels(type);
        }

        //goto
        else if (cmd == "goto")
            return writeGoto(type);

        else if (cmd == "if-goto")
            return writeIf(type);

        else if (cmd == "call")
            return writeCallFunction(type, indx);

        else if (cmd == "function")
            return writeFunction(type, indx);
        
        else if (cmd == "return")
            return writeReturn();

        else
            return writeArithmetic(cmd);
    }
    else
        return "";
}


int main(int argc, char *argv[])
{
    Timer timer;
    string inpath;
    inpath = argv[1];
    cout << "Input path: "<<inpath<<endl;
    cout << "Output Path: " << outputpath(inpath) << endl;
    currentline = 4;
    outfile.open(outputpath(inpath), ios::out);
    if (outfile.is_open())
    {
        if (filename == "FibonacciElement" || filename == "StaticsTest")
            outfile << initCode;
        if (isDir)
            writeDir(inpath);
        else
            WriteOut(inpath);
        outfile << endCode;
    }
    else
        cout << "ERROR: Outputfile can't be created"<< endl;
    outfile.close();
    return 0;
}
