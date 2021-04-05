
#ifndef RISCV_SIM_DECODER_H
#define RISCV_SIM_DECODER_H

#include "SwitchMaker.h"
#include "Instruction.h"

// This decoder implementation is stateless, so it could be a function as well
class Decoder
{

public:

    InstructionPtr Decode(Word data)
    {

        DecodedInstr decoded{data};

        auto instr = (*sMaker).DoOperation(static_cast<Opcode>(decoded.i.opcode), decoded);
        

        if (instr->_dst.value_or(0) == 0)
            instr->_dst.reset();

        return instr;
    }

private:
    class InstructionMaker;

    class _SwitchMaker
	{
        private:
            using UniqueSwitchPtr = std::unique_ptr<InstructionMaker>;

            SwitchMaker<Opcode, UniqueSwitchPtr> sMaker;
	    public:

            _SwitchMaker()
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

            SwitchMaker<Opcode, UniqueSwitchPtr>& operator*()
            {
                return sMaker;
            }
	};

    using Imm = int32_t;

    static _SwitchMaker inline sMaker = _SwitchMaker();

    static Imm SignExtend(Imm i, unsigned sbit)
    {
        return i + ((0xffffffff << (sbit + 1)) * ((i & (1u << sbit)) >> sbit));
    }




    union DecodedInstr
    {
        Word instr;
        struct rType
        {
            uint32_t opcode : 7;
            uint32_t rd : 5;
            uint32_t funct3 : 3;
            uint32_t rs1 : 5;
            uint32_t rs2 : 5;
            uint32_t reserved1 : 5;
            uint32_t aluSel : 1;
            uint32_t reserved2 : 1;
        } r;
        struct iType
        {
            uint32_t opcode : 7;
            uint32_t rd : 5;
            uint32_t funct3 : 3;
            uint32_t rs1 : 5;
            uint32_t imm11_0 : 12;
        } i;
        struct sType
        {
            uint32_t opcode : 7;
            uint32_t imm4_0 : 5;
            uint32_t funct3 : 3;
            uint32_t rs1 : 5;
            uint32_t rs2 : 5;
            uint32_t imm11_5 : 7;
        } s;
        struct bType
        {
            uint32_t opcode : 7;
            uint32_t imm11 : 1;
            uint32_t imm4_1 : 4;
            uint32_t funct3 : 3;
            uint32_t rs1 : 5;
            uint32_t rs2 : 5;
            uint32_t imm10_5 : 6;
            uint32_t imm12 : 1;
        } b;
        struct uType
        {
            uint32_t opcode : 7;
            uint32_t rd : 5;
            uint32_t imm31_12 : 20;
        } u;
        struct jType
        {
            uint32_t opcode : 7;
            uint32_t rd : 5;
            uint32_t imm19_12 : 8;
            uint32_t imm11 : 1;
            uint32_t imm10_1 : 10;
            uint32_t imm20 : 1;
        } j;

    };

    class InstructionMaker
    {
        public:
            InstructionMaker(Opcode _type)
            {
                type = _type;
            }

            Opcode GetSwitchType()
            {
                return type;
            }

            InstructionPtr virtual operator()(DecodedInstr decoded) = 0;

        protected:
            Opcode type;

            InstructionPtr GetNewInstraction()
            {
                return std::make_unique<Instruction>();
            }

            Imm GetimmI(DecodedInstr decoded)
            {
                return SignExtend(decoded.i.imm11_0, 11);
            }

            Imm GetimmS(DecodedInstr decoded)
            {
                return SignExtend(decoded.s.imm11_5 << 5u | decoded.s.imm4_0, 11);
            }

            Word GetimmU(DecodedInstr decoded)
            {
                return decoded.u.imm31_12 << 12u;
            }

            Imm GetimmB(DecodedInstr decoded)
            {
                return SignExtend((decoded.b.imm12 << 12u) | (decoded.b.imm11 << 11u) |
                              (decoded.b.imm10_5 << 5u) | (decoded.b.imm4_1 << 1u),
                              12);
            }

            Imm GetimmJ(DecodedInstr decoded)
            {
                return SignExtend((decoded.j.imm20 << 20u) | (decoded.j.imm19_12 << 12u) |
                              (decoded.j.imm11 << 11u) | (decoded.j.imm10_1 << 1u),
                              20);
            }
    };

    template<typename T, typename... Args>
    static std::unique_ptr<InstructionMaker> CreateInstance(Args&&... args)
    {
        return std::unique_ptr<InstructionMaker>(new T(std::forward<Args>(args)...));
    }


    class OpImmMaker : public InstructionMaker
    {
        using Imm = int32_t;
        public:
            
            OpImmMaker() : InstructionMaker(Opcode::OpImm) {}

            InstructionPtr operator()(DecodedInstr decoded) override
            {
                auto instr = GetNewInstraction();
                instr->_imm = GetimmI(decoded);
                instr->_type = IType::Alu;
                instr->_aluFunc = static_cast<AluFunc>(decoded.i.funct3);
                if (instr->_aluFunc == AluFunc::Sr)
                {
                    instr->_aluFunc = decoded.r.aluSel ? AluFunc::Sra : AluFunc::Srl;
                    instr->_imm.value() &= 31u;
                }
                instr->_dst = RId(decoded.i.rd);
                instr->_src1 = RId(decoded.i.rs1);
                return instr;
            }
    };


    class OpMaker : public InstructionMaker
    {
        public:

            OpMaker() : InstructionMaker(Opcode::Op) {}

            InstructionPtr operator()(DecodedInstr decoded) override
            {
                auto instr = GetNewInstraction();
                instr->_type = IType::Alu;
                auto funct3 = AluFunc(decoded.r.funct3);
                if (funct3 == AluFunc::Add)
                {
                    instr->_aluFunc = decoded.r.aluSel == 0 ? AluFunc::Add : AluFunc::Sub;
                }
                else if (funct3 == AluFunc::Sr)
                {
                    instr->_aluFunc = decoded.r.aluSel ? AluFunc::Sra : AluFunc::Srl;
                }
                else
                {
                    instr->_aluFunc = funct3;
                }
                instr->_dst = RId(decoded.r.rd);
                instr->_src1 = RId(decoded.r.rs1);
                instr->_src2 = RId(decoded.r.rs2);
                return instr;
	        }

    };


    class LuiMaker : public InstructionMaker
    {
        public:

            LuiMaker() : InstructionMaker(Opcode::Lui) {}

            InstructionPtr operator()(DecodedInstr decoded) override
            {
                auto instr = GetNewInstraction();
                 instr->_type = IType::Alu;
                instr->_aluFunc = AluFunc::Add;
                instr->_dst = RId(decoded.u.rd);
                instr->_src1 = 0;
                instr->_imm = GetimmU(decoded);
                return instr;
            }
    };

    class AuipcMaker : public InstructionMaker
    {
        public:
            
            AuipcMaker() : InstructionMaker(Opcode::Auipc) {}

            InstructionPtr operator()(DecodedInstr decoded) override
            {
                auto instr = GetNewInstraction();
                instr->_type = IType::Auipc;
                instr->_dst = RId(decoded.u.rd);
                instr->_imm = GetimmU(decoded);
                return instr;
            }

        private:
    };

    class JalMaker : public InstructionMaker
    {
        using Imm = int32_t;
        public:
            
            JalMaker() : InstructionMaker(Opcode::Jal){}
            
            InstructionPtr operator()(DecodedInstr decoded) override
            {
                auto instr = GetNewInstraction();
                instr->_type = IType::J;
                instr->_brFunc = BrFunc::AT;
                instr->_dst = RId(decoded.j.rd);
                instr->_imm = GetimmJ(decoded);
                return instr;
            }

        private:
    };


    class JalrMaker : public InstructionMaker
    {
        using Imm = int32_t;
        public:
            
            JalrMaker() : InstructionMaker(Opcode::Jalr) {}

            InstructionPtr operator()(DecodedInstr decoded) override
            {
                auto instr = GetNewInstraction();
                instr->_type = IType::Jr;
                instr->_brFunc = BrFunc::AT;
                instr->_dst = RId(decoded.i.rd);
                instr->_src1 = RId(decoded.i.rs1);
                instr->_imm = GetimmI(decoded);
                return instr;
            }
    };


    class BranchMaker : public InstructionMaker
    {
        using Imm = int32_t;
        public:
            
            BranchMaker() : InstructionMaker(Opcode::Branch) {}

            InstructionPtr operator()(DecodedInstr decoded) override
            {
                auto instr = GetNewInstraction();
                instr->_type = IType::Br;
                instr->_brFunc = static_cast<BrFunc>(decoded.b.funct3);
                instr->_src1 = RId(decoded.b.rs1);
                instr->_src2 = RId(decoded.b.rs2);
                instr->_imm = GetimmB(decoded);
                return instr;
            }
    };

    class LoadMaker : public InstructionMaker
    {
        using Imm = int32_t;
        public:
            
            LoadMaker() : InstructionMaker(Opcode::Load) {}

            InstructionPtr operator()(DecodedInstr decoded) override
            {
                auto instr = GetNewInstraction();
                instr->_type = decoded.i.funct3 == fnLW ? IType::Ld : IType::Unsupported;
                instr->_aluFunc = AluFunc::Add;
                instr->_dst = RId(decoded.i.rd);
                instr->_src1 = RId(decoded.i.rs1);
                instr->_imm = GetimmI(decoded);
                return instr;
            }

    };


    class StoreMaker : public InstructionMaker
    {
        using Imm = int32_t;
        public:
            
            StoreMaker() : InstructionMaker(Opcode::Store) {}

            InstructionPtr operator()(DecodedInstr decoded) override
            {
                auto instr = GetNewInstraction();
                instr->_type = decoded.i.funct3 == fnSW ? IType::St : IType::Unsupported;
                instr->_aluFunc = AluFunc::Add;
                instr->_src1 = RId(decoded.s.rs1);
                instr->_src2 = RId(decoded.s.rs2);
                instr->_imm = GetimmS(decoded);
                return instr;
            }
    };


    class SystemMaker : public InstructionMaker
    {
        using Imm = int32_t;
        public:
            
            SystemMaker() : InstructionMaker(Opcode::System) {}

            InstructionPtr operator()(DecodedInstr decoded) override
            {
                auto instr = GetNewInstraction();
                if (decoded.i.funct3 == fnCSRRW && decoded.i.rd == 0)
                {
                    instr->_type = IType::Csrw;
                }
                else if (decoded.i.funct3 == fnCSRRS && decoded.i.rs1 == 0)
                {
                    instr->_type = IType::Csrr;
                }
                instr->_dst = RId(decoded.i.rd);
                instr->_src1 = RId(decoded.i.rs1);
                instr->_csr = static_cast<CsrIdx>(GetimmI(decoded) & 0xfff);
                return instr;
            }

    };

    class MiscMemMaker : public InstructionMaker
    {
        public:
            
            MiscMemMaker() : InstructionMaker(Opcode::MiscMem)
            {
            }

            InstructionPtr operator()(DecodedInstr decoded) override
            {
                return (*this)();
            }

            InstructionPtr operator()()
            {
                auto instr = GetNewInstraction();
                instr->_type = IType::Unsupported;
                instr->_aluFunc = AluFunc::None;
                instr->_brFunc = BrFunc::NT;
                return instr;
            }

    };


    class AmoMaker : public InstructionMaker
    {
        public:
            
            AmoMaker() : InstructionMaker(Opcode::Amo)
            {
            }

            InstructionPtr operator()(DecodedInstr decoded) override
            {
                return (*this)();
            }

            InstructionPtr operator()()
            {
                auto instr = GetNewInstraction();
                instr->_type = IType::Unsupported;
                instr->_aluFunc = AluFunc::None;
                instr->_brFunc = BrFunc::NT;
                return instr;
            }

    };

    class DefaultMaker : public InstructionMaker
    {
        public:
            
            DefaultMaker() : InstructionMaker(Opcode::System)
            {
            }

            InstructionPtr operator()(DecodedInstr decoded) override
            {
                return (*this)();
            }

            InstructionPtr operator()()
            {
                auto instr = GetNewInstraction();
                instr->_type = IType::Unsupported;
                instr->_aluFunc = AluFunc::None;
                instr->_brFunc = BrFunc::NT;
                return instr;
            }
    };
};

#endif //RISCV_SIM_DECODER_H
