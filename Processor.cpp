// Scalar Pipeline Processor
// Vignesh Gaikoti CS22B025
// Dulla Sarath Vamsi CS22B072

#include <bits/stdc++.h>
using namespace std;

/*
 Decalring Global variables for Instruction Cache,
 Data Cache, register file and program counter
*/

int8_t ICache[256], PC = 0;
int8_t DCache[256], RF[16];
bool Valid[16];

bool halt = 0, fetch = 1;
int nof_arith = 0, nof_log = 0, nof_shift = 0, nof_data = 0, nof_ld = 0, nof_con = 0, nof_halt = 0;
int nof_stalls = 0, nof_dstalls = 0, nof_cstalls = 0, cycles = 1, nof_inst = 0;

/* This class represents an instruction in a C++ program. */
class Instruction
{
private:
    uint8_t Next_PC;
    int8_t IR[4], OP;
    int8_t rs1, rs2, Imm, ALU_Output, LMD;

public:
    int stage = 0;
    bool stall_it = 0;

    bool Valid_reg(int8_t reg_ad)
    {
        if (Valid[reg_ad])
            stall_it = 0;
        else
            stall_it = 1;
        return stall_it;
    }

    /**
     * The IF function in C++ reads instructions from the instruction cache and updates the instruction
     * register and program counter.
     */
    void IF()
    {
        // cout << "IF" << endl;
        IR[0] = (ICache[PC] >> 4) & 15;
        IR[1] = (ICache[PC]) & 15;
        IR[2] = (ICache[PC + 1] >> 4) & 15;
        IR[3] = (ICache[PC + 1]) & 15;
        // cout << +IR[0] << " " << +IR[1] << " " << +IR[2] << " " << +IR[3] << endl;
        PC = PC + 2;
        Next_PC = PC;
        stage = 1;
    }

    /**
     * The function `ID` in C++ processes different types of instructions based on the opcode and
     * updates registers and flags accordingly.
     */
    void ID()
    {
        // cout << "ID" << endl;
        OP = +IR[0];
        // cout << +OP << endl;

        switch (OP)
        {
        case 0: // ADD Instruction
        case 1: // SUB Instruction
        case 2: // MUL Instruction
        case 4: // AND Instruction
        case 5: // OR Instruction
        case 6: // XOR Instruction
            // All the Above Instructions are common
            if (Valid_reg(IR[2]))
                return;
            if (Valid_reg(IR[3]))
                return;
            rs1 = RF[IR[2]];
            rs2 = RF[IR[3]];
            Valid[IR[1]] = false;
            break;
        case 3: // INC Instruction
            if (Valid_reg(IR[1]))
                return;
            rs1 = RF[IR[1]];
            Valid[IR[1]] = false;
            break;
        case 7: // NOT Instruction
            if (Valid_reg(IR[2]))
                return;
            rs1 = RF[IR[2]];
            Valid[IR[1]] = false;
            break;
        case 8: // SLLi Instruction
        case 9: // SRLI Instruction
            if (Valid_reg(IR[2]))
                return;
            rs1 = RF[IR[2]];
            Imm = IR[3];
            Valid[IR[1]] = false;
            break;
        case 10: // LI Instruction
            Imm = (IR[2] << 4) + IR[3];
            Valid[IR[1]] = false;
            break;
        case 11: // LD Instruction
            if (Valid_reg(IR[2]))
                return;
            rs1 = RF[IR[2]];
            Valid[IR[1]] = false;
            Imm = IR[3];
            if (Imm > 7)
                Imm = Imm - 16;
            break;
        case 12: // ST Instruction
            // cout << Valid[IR[1]] << " " << Valid[IR[2]] << endl;
            if (Valid_reg(IR[1]))
                return;
            if (Valid_reg(IR[2]))
                return;
            rs1 = RF[IR[2]];
            // cout << "yes";
            Imm = IR[3];
            if (Imm > 7)
                Imm = Imm - 16;
            break;
        case 13: // JMP instruction
            Imm = (IR[1] << 4) + (IR[2]);
            fetch = 0;
            break;
        case 14: // BEQZ instruction
            if (Valid_reg(IR[1]))
                return;
            Imm = (IR[2] << 4) + IR[3];
            // cout << +IR[1];
            rs1 = RF[IR[1]];
            // cout << +rs1 << endl;
            fetch = 0;
            break;
        case 15: // HLT instruction
            halt = 1;
            break;
        }
        stage = 2;
    }

    void EX()
    {
        // cout << "EX" << endl;
        switch (OP)
        {
        case 0:
            ALU_Output = rs1 + rs2;
            break;
        case 1:
            ALU_Output = rs1 - rs2;
            break;
        case 2:
            ALU_Output = rs1 * rs2;
            break;
        case 3:
            ALU_Output = rs1 + 1;
            break;
        case 4:
            ALU_Output = rs1 & rs2;
            break;
        case 5:
            ALU_Output = rs1 | rs2;
            break;
        case 6:
            ALU_Output = rs1 ^ rs2;
            break;
        case 7:
            ALU_Output = ~rs1;
            break;
        case 8:
            ALU_Output = rs1 << Imm;
            break;
        case 9:
            ALU_Output = rs1 >> Imm;
            break;
        case 10:
            ALU_Output = Imm;
            break;
        case 11:
            ALU_Output = rs1 + Imm;
            break;
        case 12:
            ALU_Output = rs1 + Imm;
            break;
        case 13:
            ALU_Output = Next_PC + Imm * 2;
            PC = ALU_Output;
            break;
        case 14:
            ALU_Output = Next_PC + Imm * 2;
            // cout << +rs1 << endl;
            if (rs1 == 0)
                PC = ALU_Output;
            break;
        case 15:
            break;
        }
        stage = 3;
    }

    void MA()
    {
        // cout << "MA" << endl;
        switch (OP)
        {
        case 10:
            LMD = Imm;
            break;
        case 11:
            LMD = DCache[ALU_Output];
            break;
        case 12:
            DCache[ALU_Output] = RF[IR[1]];
            break;
        case 13:
        case 14:
            fetch = 1;
            break;
        }
        stage = 4;
    }

    void WB()
    {
        // cout << "WB" << endl;
        if (OP < 10)
        {
            RF[IR[1]] = ALU_Output;
            Valid[IR[1]] = true;
        }
        else if (OP < 12)
        {
            RF[IR[1]] = LMD;
            Valid[IR[1]] = true;
        }
        stage = 5;
        Instruction_Count();
    }

    bool Next_stage()
    {
        switch (stage)
        {
        case 1:
            ID();
            break;
        case 2:
            EX();
            break;
        case 3:
            MA();
            break;
        case 4:
            WB();
            break;
        }
        return stall_it;
    }

    void Instruction_Count()
    {
        if (OP < 4)
            nof_arith++;
        else if (OP < 8)
            nof_log++;
        else if (OP < 10)
            nof_shift++;
        else if (OP < 13)
        {
            if (OP == 10)
                nof_ld++;
            else
                nof_data++;
        }
        else if (OP < 15)
            nof_con++;
        else if (OP == 15)
            nof_halt++;
    }
};

void ReadFile(int8_t arr[], int size, string s)
{
    ifstream ifile;
    ifile.open(s);
    for (int i = 0; i < size; i++)
    {
        std::string hex1;
        ifile >> hex1;
        arr[i] = std::stoi(hex1, nullptr, 16);
    }
    ifile.close();
}

int main()
{
    for (int i = 0; i < 16; i++)
        Valid[i] = 1;
    // cout << "Start" << endl;
    /* The `ReadFile` function is reading data from text files and storing it in arrays. */
    ReadFile(ICache, 256, "input/ICache.txt");
    ReadFile(DCache, 256, "input/DCache.txt");
    ReadFile(RF, 16, "input/RF.txt");
    // for (int i = 0; i < 16; i++)
    // {
    //     cout << +RF[i] << " ";
    // }
    queue<Instruction> Processor;
    Instruction I1;
    I1.IF();
    cout << endl;
    Processor.push(I1);
    // cout << "loopstart" << endl;
    while (!Processor.empty())
    {
        // cout << cycles << endl;
        cycles++;
        bool is_stalled = 0;
        unsigned int n = Processor.size();
        // cout << n << endl;
        for (int i = 0; i < n; i++)
        {
            Instruction I = Processor.front();
            if (I.Next_stage() == 1)
                is_stalled = 1;
            Processor.pop();
            if (I.stage < 5)
                Processor.push(I);
        }
        // cout << fetch << " " << is_stalled << " " << halt << " " << endl;
        if (!fetch)
        {
            nof_stalls++;
            nof_cstalls++;
        }
        else if (is_stalled)
        {
            nof_stalls++;
            nof_dstalls++;
        }
        else if (!halt)
        {
            // cout << "push ins" << endl;
            nof_inst++;
            Instruction nxt;
            nxt.IF();
            Processor.push(nxt);
        }
        // cout << +RF[1] << endl;
        // cout << endl;
    }
    if (halt == 1)
        nof_inst++;
    double CPI = (double)cycles / (double)nof_inst;
    ofstream ofile;
    ofile.open("output/Output.txt");
    ofile << "Total number of instructions executed:" << nof_inst << endl;
    ofile << "Number of instructions in each class " << endl;
    ofile << "Arithmetic instructions              :" << nof_arith << endl;
    ofile << "Logical instructions                 :" << nof_log << endl;
    ofile << "Shift instructions                   :" << nof_shift << endl;
    ofile << "Memory instructions                  :" << nof_data << endl;
    ofile << "Load immediate instructions          :" << nof_ld << endl;
    ofile << "Control instructions                 :" << nof_con << endl;
    ofile << "Halt instructions                    :" << nof_halt << endl;
    ofile << "Cycles Per Instruction               :" << CPI << endl;
    ofile << "Total number of stalls               :" << nof_stalls << endl;
    ofile << "Data stalls (RAW)                    :" << nof_dstalls << endl;
    ofile << "Control stalls                       :" << nof_cstalls;
    ofile.close();
    ofile.open("output/DCache.txt");
    for (int i = 0; i < 256; i++)
    {
        int n = (DCache[i] >> 4) & 15;
        int m = (DCache[i]) & 15;
        char c[1], d[1], a = '0', b = '0';
        sprintf(c, "%X", n);
        if (c[0] >= 'A' && c[0] <= 'Z')
            a = c[0] + 'a' - 'A';
        else if (c[0] >= '0' && c[0] <= '9')
            a = c[0];
        ofile << a;
        sprintf(d, "%X", m);
        if (d[0] >= 'A' && d[0] <= 'Z')
            b = d[0] + 'a' - 'A';
        else if (d[0] >= '0' && d[0] <= '9')
            b = d[0];
        ofile << b;
        if (i != 255)
            ofile << endl;
    }
    ofile.close();
    // cout << "end";
    return 0;
}