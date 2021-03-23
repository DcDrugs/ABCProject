
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
            = { immB, immI, immJ, immS, immU, decoded, instr};

        Opcode type = static_cast<Opcode>(decoded.i.opcode);
        (*sMaker).DoOperation(type, container);
        

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

    Imm SignExtend(Imm i, unsigned sbit)
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



    struct InstructionContainer
    {   
        using Imm = int32_t;
        Imm immB;
        Imm immI;
        Imm immJ;
        Imm immS;
        Word immU;
        DecodedInstr decoded;
        InstructionPtr& instr;
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

            void virtual operator()(InstructionContainer container) = 0;

        protected:
            Opcode type;
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

            void operator()(InstructionContainer container) override
            {
                (*this)(container.instr, container.immI, container.decoded);
            }

            void operator()(InstructionPtr& instr, Imm immI, DecodedInstr decoded)
            {
                instr->_imm = immI;
                instr->_type = IType::Alu;
                instr->_aluFunc = static_cast<AluFunc>(decoded.i.funct3);
                if (instr->_aluFunc == AluFunc::Sr)
                {
                    instr->_aluFunc = decoded.r.aluSel ? AluFunc::Sra : AluFunc::Srl;
                    instr->_imm.value() &= 31u;
                }
                instr->_dst = RId(decoded.i.rd);
                instr->_src1 = RId(decoded.i.rs1);
            }
    };


    class OpMaker : public InstructionMaker
    {
        public:

            OpMaker() : InstructionMaker(Opcode::Op) {}

            void operator()(InstructionContainer container) override
            {
                (*this)(container.instr, container.decoded);
            }

            void operator()(InstructionPtr& instr, DecodedInstr decoded)
            {
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
            }

        private:

            DecodedInstr decoded;
    };


    class LuiMaker : public InstructionMaker
    {
        public:

            LuiMaker() : InstructionMaker(Opcode::Lui) {}

            void operator()(InstructionContainer container) override
            {
                (*this)(container.instr, container.immU, container.decoded);
            }

            void operator()(InstructionPtr& instr, Word immU,
                DecodedInstr decoded)
            {
                instr->_type = IType::Alu;
                instr->_aluFunc = AluFunc::Add;
                instr->_dst = RId(decoded.u.rd);
                instr->_src1 = 0;
                instr->_imm = immU;
            }

        private:
    };

    class AuipcMaker : public InstructionMaker
    {
        public:
            
            AuipcMaker() : InstructionMaker(Opcode::Auipc) {}

            void operator()(InstructionContainer container) override
            {
                (*this)(container.instr, container.immU, container.decoded);
            }

            void  operator()(InstructionPtr& instr, Word immU,
                DecodedInstr decoded)
            {
                instr->_type = IType::Auipc;
                instr->_dst = RId(decoded.u.rd);
                instr->_imm = immU;
            }

        private:
    };

    class JalMaker : public InstructionMaker
    {
        using Imm = int32_t;
        public:
            
            JalMaker() : InstructionMaker(Opcode::Jal){}
            
            void operator()(InstructionContainer container) override
            {
                (*this)(container.instr, container.immJ, container.decoded);
            }

            void operator()(InstructionPtr& instr, Imm immJ,
                DecodedInstr decoded)
            {
                instr->_type = IType::J;
                instr->_brFunc = BrFunc::AT;
                instr->_dst = RId(decoded.j.rd);
                instr->_imm = immJ;
            }

        private:
    };


    class JalrMaker : public InstructionMaker
    {
        using Imm = int32_t;
        public:
            
            JalrMaker() : InstructionMaker(Opcode::Jalr) {}

            void operator()(InstructionContainer container) override
            {
                (*this)(container.instr, container.immI, container.decoded);
            }

            void operator()(InstructionPtr& instr, Imm immI,
                DecodedInstr decoded)
            {
                instr->_type = IType::Jr;
                instr->_brFunc = BrFunc::AT;
                instr->_dst = RId(decoded.i.rd);
                instr->_src1 = RId(decoded.i.rs1);
                instr->_imm = immI;
            }
    };


    class BranchMaker : public InstructionMaker
    {
        using Imm = int32_t;
        public:
            
            BranchMaker() : InstructionMaker(Opcode::Branch) {}

            void operator()(InstructionContainer container) override
            {
                (*this)(container.instr, container.immB, container.decoded);
            }

            void operator()(InstructionPtr& instr, Imm immB,
                DecodedInstr decoded)
            {
                instr->_type = IType::Br;
                instr->_brFunc = static_cast<BrFunc>(decoded.b.funct3);
                instr->_src1 = RId(decoded.b.rs1);
                instr->_src2 = RId(decoded.b.rs2);
                instr->_imm = immB;
            }
    };

    class LoadMaker : public InstructionMaker
    {
        using Imm = int32_t;
        public:
            
            LoadMaker() : InstructionMaker(Opcode::Load) {}

            void operator()(InstructionContainer container) override
            {
                (*this)(container.instr, container.immI, container.decoded);
            }

            void operator()(InstructionPtr& instr,  Imm immI,
                DecodedInstr decoded)
            {
                instr->_type = decoded.i.funct3 == fnLW ? IType::Ld : IType::Unsupported;
                instr->_aluFunc = AluFunc::Add;
                instr->_dst = RId(decoded.i.rd);
                instr->_src1 = RId(decoded.i.rs1);
                instr->_imm = immI;
            }

    };


    class StoreMaker : public InstructionMaker
    {
        using Imm = int32_t;
        public:
            
            StoreMaker() : InstructionMaker(Opcode::Store) {}

            void operator()(InstructionContainer container) override
            {
                (*this)(container.instr, container.immS, container.decoded);
            }


            void operator()(InstructionPtr& instr, Imm immS, 
                DecodedInstr decoded)
            {
                instr->_type = decoded.i.funct3 == fnSW ? IType::St : IType::Unsupported;
                instr->_aluFunc = AluFunc::Add;
                instr->_src1 = RId(decoded.s.rs1);
                instr->_src2 = RId(decoded.s.rs2);
                instr->_imm = immS;
            }
    };


    class SystemMaker : public InstructionMaker
    {
        using Imm = int32_t;
        public:
            
            SystemMaker() : InstructionMaker(Opcode::System) {}

            void operator()(InstructionContainer container) override
            {
                (*this)(container.instr, container.immI, container.decoded);
            }

            void operator()(InstructionPtr& instr, Imm immI,
                DecodedInstr decoded)
            {
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
                instr->_csr = static_cast<CsrIdx>(immI & 0xfff);
            }

    };

    class MiscMemMaker : public InstructionMaker
    {
        public:
            
            MiscMemMaker() : InstructionMaker(Opcode::MiscMem)
            {
            }

            void operator()(InstructionContainer container) override
            {
                (*this)(container.instr);
            }

            void operator()(InstructionPtr& instr)
            {
                instr->_type = IType::Unsupported;
                instr->_aluFunc = AluFunc::None;
                instr->_brFunc = BrFunc::NT;
            }

    };


    class AmoMaker : public InstructionMaker
    {
        public:
            
            AmoMaker() : InstructionMaker(Opcode::Amo)
            {
            }

            void operator()(InstructionContainer container) override
            {
                (*this)(container.instr);
            }

            void operator()(InstructionPtr& instr)
            {
                instr->_type = IType::Unsupported;
                instr->_aluFunc = AluFunc::None;
                instr->_brFunc = BrFunc::NT;
            }

    };

    class DefaultMaker : public InstructionMaker
    {
        public:
            
            DefaultMaker() : InstructionMaker(Opcode::System)
            {
            }

            void operator()(InstructionContainer container) override
            {
                (*this)(container.instr);
            }

            void operator()(InstructionPtr& instr)
            {
                instr->_type = IType::Unsupported;
                instr->_aluFunc = AluFunc::None;
                instr->_brFunc = BrFunc::NT;
            }

    };
  
};

#endif //RISCV_SIM_DECODER_H
