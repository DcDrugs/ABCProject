
#ifndef RISCV_SIM_CPU_H
#define RISCV_SIM_CPU_H

#include "Memory.h"
#include "Decoder.h"
#include "RegisterFile.h"
#include "CsrFile.h"
#include "Executor.h"

class Cpu
{
public:
    Cpu(Memory& mem)
        : _mem(mem)
    {

    }

    void ProcessInstruction()
    {
<<<<<<< HEAD
        auto instr = _decoder.Decode(_mem.Request(_ip));
        _rf.Read(instr);
        _csrf.Read(instr);

        _exe.Execute(instr, _ip);
        _rf.Write(instr);
        _csrf.Write(instr);
        _csrf.InstructionExecuted();
        _ip = instr->_nextIp;
=======
        /* YOUR CODE HERE */
>>>>>>> 288a0d4e7794452fa1c5e2203d20a995068bda9c
    }

    void Reset(Word ip)
    {
        _csrf.Reset();
        _ip = ip;
    }

    std::optional<CpuToHostData> GetMessage()
    {
        return _csrf.GetMessage();
    }

private:
    Reg32 _ip;
    Decoder _decoder;
    RegisterFile _rf;
    CsrFile _csrf;
    Executor _exe;
    Memory& _mem;
};


#endif //RISCV_SIM_CPU_H
