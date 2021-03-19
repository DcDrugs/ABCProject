
#ifndef RISCV_SIM_DECODER_H
#define RISCV_SIM_DECODER_H

#include "DecoderInstr.h"
#include "SwitchMaker.h"
#include "InstractionMaker.h"

// This decoder implementation is stateless, so it could be a function as well
class Decoder
{

public:

    InstructionPtr Decode(Word data)
    {
        if(!isMakerUpload)
        {
            AddToSwitchMaker();
            isMakerUpload = true;
        }

        DecodedInstr decoded{data};

        InstructionPtr instr = std::make_unique<Instruction>();
        Imm immI = SignExtend(decoded.i.imm11_0, 11);
        Imm immS = SignExtend(decoded.s.imm11_5 << 5u | decoded.s.imm4_0, 11);
        Word immU = decoded.u.imm31_12 << 12u;
        Imm immB = SignExtend((decoded.b.imm12 << 12u) | (decoded.b.imm11 << 11u) |
                              (decoded.b.imm10_5 << 5u) | (decoded.b.imm4_1 << 1u),
                              12);
        Imm immJ = SignExtend((decoded.j.imm20 << 20u) | (decoded.j.imm19_12 << 12u) |
                              (decoded.j.imm11 << 11u) | (decoded.j.imm10_1 << 1u),
                              20);

        InstructionContainer container 
            = { immB, immI, immJ, immS, immU, decoded };

        Opcode type = static_cast<Opcode>(decoded.i.opcode);
        auto instrFunc = sMaker.CrateInstance(type, container);
        instrFunc->Set(move(instr));
        instr = (*instrFunc)();
        

        if (instr->_dst.value_or(0) == 0)
            instr->_dst.reset();

        return instr;
    }

private:
    using Imm = int32_t;
    using UniqueSwitchPtr = std::unique_ptr<InstructionMaker>;

    static SwitchMaker<Opcode, UniqueSwitchPtr> sMaker;
    bool isMakerUpload = true;

    Imm SignExtend(Imm i, unsigned sbit)
    {
        return i + ((0xffffffff << (sbit + 1)) * ((i & (1u << sbit)) >> sbit));
    }

    static void AddToSwitchMaker()
    {
        sMaker.AddCompare(CreateInstance<OpImmMaker>());
        sMaker.AddCompare(CreateInstance<AmoMaker>());
        sMaker.AddCompare(CreateInstance<AuipcMaker>());
        sMaker.AddCompare(CreateInstance<BranchMaker>());
        sMaker.AddCompare(CreateInstance<JalMaker>());
        sMaker.AddCompare(CreateInstance<JalrMaker>());
        sMaker.AddCompare(CreateInstance<LoadMaker>());
        sMaker.AddCompare(CreateInstance<LuiMaker>());
        sMaker.AddCompare(CreateInstance<MiscMemMaker>());
        sMaker.AddCompare(CreateInstance<OpMaker>());
        sMaker.AddCompare(CreateInstance<StoreMaker>());
        sMaker.AddCompare(CreateInstance<SystemMaker>());

        sMaker.AddDefault(CreateInstance<DefaultMaker>());
    }
};

#endif //RISCV_SIM_DECODER_H
