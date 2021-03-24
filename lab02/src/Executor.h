
#ifndef RISCV_SIM_EXECUTOR_H
#define RISCV_SIM_EXECUTOR_H

#include "Instruction.h"
<<<<<<< HEAD
#include <memory>
#include <unordered_map>
#include <math.h>
#include <limits>
=======
>>>>>>> 288a0d4e7794452fa1c5e2203d20a995068bda9c

class Executor
{
public:
    void Execute(InstructionPtr& instr, Word ip)
    {
        DoAlu(instr, ip);
        ChangeAddress(instr, ip);
    }
private:

    void DoAlu(InstructionPtr& instr, Word ip)
    {
        Word res = 0;
        if(instr->_src1 && (instr->_src2 || instr->_imm))
        {
            res = GetOperation.at(instr->_aluFunc)(instr->_src1Val, 
                (instr->_imm)? instr->_imm.value() : instr->_src2Val );
            if(instr->_type == IType::Ld || instr->_type == IType::St)
            {
                instr->_addr = res;
            }
        }
        instr->_data = checklist.at(instr->_type)(instr, ip, res);
    }

    void ChangeAddress(InstructionPtr& instr, Word ip)
    {
        if((instr->_type == IType::Br || instr->_type == IType::J) 
            && GetTransition.at(instr->_brFunc)(instr))
            instr->_nextIp = GetChangeAdress.at(instr->_type)(instr, ip);
        else if(instr->_type == IType::Jr && GetTransition.at(instr->_brFunc)(instr))
                instr->_nextIp = instr->_src1Val + instr->_imm.value();
        else
            instr->_nextIp = ip + 4;
    }

    const std::unordered_map<IType, Word(*)(InstructionPtr& instr, Word ip, Word res)> checklist = {
                {IType::Csrr, GetCsrr},
                {IType::Csrw, GetCsrw},
                {IType::St, GetSt},
                {IType::J, GetJorJr},
                {IType::Jr, GetJorJr},
                {IType::Auipc, GetAuipc},
                {IType::Alu, GetDefault},
                {IType::Ld, GetDefault},
                {IType::Br, GetDefault}
            };

    const std::unordered_map<BrFunc, bool(*)(InstructionPtr& instr)> GetTransition = {
                {BrFunc::Eq, GetEq},
                {BrFunc::Neq, GetNeq},
                {BrFunc::Lt, GetLt},
                {BrFunc::Ltu, GetLtu},
                {BrFunc::Ge, GetGe},
                {BrFunc::Geu, GetGeu},
                {BrFunc::AT, GetAt},
                {BrFunc::NT, GetNt}
            };

    const std::unordered_map<AluFunc, Word(*)(Word, Word)> GetOperation = {
                {AluFunc::Add, GetAdd},
                {AluFunc::Sub, GetSub},
                {AluFunc::And, GetAnd},
                {AluFunc::Or, GetOr},
                {AluFunc::Xor, GetXor},
                {AluFunc::Slt, GetSlt},
                {AluFunc::Sltu, GetSltu},
                {AluFunc::Sll, GetSll},
                {AluFunc::Srl, GetSrl},
                {AluFunc::Sra, GetSra} 
            };

     const std::unordered_map<IType, Word(*)(InstructionPtr& instr, Word ip)> GetChangeAdress = {
                {IType::J, GetBrAndJ},
                {IType::Br, GetBrAndJ}
            };

    static Word GetDefault(InstructionPtr& instr, Word ip, Word res)
    {
        return res;
    }

    static Word GetCsrr(InstructionPtr& instr, Word ip, Word tmp)
    {
        return instr->_csrVal;
    }

    static Word GetCsrw(InstructionPtr& instr, Word ip, Word tmp)
    {
        return instr->_src1Val;
    }

    static Word GetSt(InstructionPtr& instr, Word ip, Word tmp)
    {
        return instr->_src2Val;
    }

    static Word GetJorJr(InstructionPtr& instr, Word ip, Word tmp)
    {
        return ip + 4;
    }

    static Word GetAuipc(InstructionPtr& instr, Word ip, Word tmp)
    {
        return ip + instr->_imm.value();
    }

    static bool GetEq(InstructionPtr& instr)
    {
        return instr->_src1Val == instr->_src2Val;
    }

    static bool GetNeq(InstructionPtr& instr)
    {
        return !GetEq(instr);
    }

    static bool GetLt(InstructionPtr& instr)
    {
        return GetSlt(instr->_src1Val, instr->_src2Val);
    }

    static bool GetLtu (InstructionPtr& instr)
    {
        return GetSltu(instr->_src1Val, instr->_src2Val);
    }

    static bool GetGe(InstructionPtr& instr)
    {
        return !GetLt(instr);
    }

    static bool GetGeu(InstructionPtr& instr)
    {
        return !GetLtu(instr);
    }

    static bool GetAt(InstructionPtr& instr)
    {
        return true;
    }

    static bool GetNt(InstructionPtr& instr)
    {
        return false;
    }
   
    static Word GetAdd(Word first, Word second)
    {
        return first + second;
    }

    static Word GetSub(Word first, Word second)
    {
        return first - second;
    }


    static Word GetAnd(Word first, Word second)
    {
        return first & second;
    }

    static Word GetOr(Word first, Word second)
    {
        return first | second;
    }

    static Word GetXor(Word first, Word second)
    {
        return first ^ second;
    }

    static Word GetSlt(Word first, Word second)
    {
        int32_t resF =  static_cast<int32_t>(first);
        int32_t resS = static_cast<int32_t>(second);
        return resF < resS;
    }

    static Word GetSltu(Word first, Word second)
    {
        return first < second;
    }

    static Word GetSll(Word first, Word second)
    {
        Word thrid = second % 32;
        return first << thrid;
    }

    static Word GetSrl(Word first, Word second)         // исправить
    {
        Word thrid = second % 32;
        int32_t res = static_cast<int32_t>(first);
        return res >> thrid;
    }

    static Word GetSra(Word first, Word second)
    {
        Word thrid = second % 32;
        return first >> thrid;
    }

    static Word GetBrAndJ(InstructionPtr& instr, Word ip)
    {
        return  ip + instr->_imm.value();
    }
};

#endif // RISCV_SIM_EXECUTOR_H
