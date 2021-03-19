#ifndef RISCV_SIM_INSTRACTIONMAKER_H
#define RISCV_SIM_INSTRACTIONMAKER_H
#include "DecoderInstr.h"
#include <utility>
#include <memory>

struct InstructionContainer
{
    using Imm = int32_t;
    Imm immB;
    Imm immI;
    Imm immJ;
    Imm immS;
    Word immU;
    DecodedInstr decoded;
};

class InstructionMaker
{
    public:
        InstructionMaker(Opcode _type)
        {
            type = _type;
        }

        InstructionPtr Get()
        {
            return move(instr);
        }
        void Set(InstructionPtr _instr)
        {
            instr = move(_instr);
        }

        Opcode GetSwitchType()
        {
            return type;
        }

        InstructionPtr virtual operator()() = 0;
        std::unique_ptr<InstructionMaker> virtual CreateInstance(
            InstructionContainer container) = 0;

    protected:
        Opcode type;
        InstructionPtr instr;
};

 template<typename T, typename... Args>
std::unique_ptr<InstructionMaker> CreateInstance(Args&&... args)
{
    return std::unique_ptr<InstructionMaker>(new T(std::forward<Args>(args)...));
}


class OpImmMaker : public InstructionMaker
{
    using Imm = int32_t;
    public:
        
        OpImmMaker() : InstructionMaker(Opcode::OpImm) {}

        std::unique_ptr<InstructionMaker> CreateInstance(
            InstructionContainer container) override
        {
            return std::unique_ptr<InstructionMaker>(
                new OpImmMaker(container.immI, container.decoded));
        }

        InstructionPtr  operator()() override
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
            return move(instr);
        }

    private:

        OpImmMaker(Imm _immI, DecodedInstr _decoded) : InstructionMaker(Opcode::OpImm)
        {
            immI = _immI;
            decoded = _decoded;
        }

        Imm immI;
        DecodedInstr decoded;
};


class OpMaker : public InstructionMaker
{
    public:

        OpMaker() : InstructionMaker(Opcode::Op) {}

        std::unique_ptr<InstructionMaker> CreateInstance(
            InstructionContainer container) override
        {
            return std::unique_ptr<InstructionMaker>(
                new OpMaker(container.decoded));
        }

        InstructionPtr  operator()() override
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
            return move(instr);
        }

    private:

        OpMaker(DecodedInstr _decoded) : InstructionMaker(Opcode::Op)
        {
            decoded = _decoded;
        }

        DecodedInstr decoded;
};


class LuiMaker : public InstructionMaker
{
    public:
        
        LuiMaker() : InstructionMaker(Opcode::Lui) {}

        std::unique_ptr<InstructionMaker> CreateInstance(
            InstructionContainer container) override
        {
            return std::unique_ptr<InstructionMaker>(
                new LuiMaker(container.immU, container.decoded));
        }

        InstructionPtr  operator()() override
        {
            instr->_type = IType::Alu;
            instr->_aluFunc = AluFunc::Add;
            instr->_dst = RId(decoded.u.rd);
            instr->_src1 = 0;
            instr->_imm = immU;
            return move(instr);
        }

    private:

        LuiMaker(Word _immU, DecodedInstr _decoded) : InstructionMaker(Opcode::Lui)
        {
            immU = _immU;
            decoded = _decoded;
        }

        DecodedInstr decoded;
        Word immU;
};

class AuipcMaker : public InstructionMaker
{
    public:
        
        AuipcMaker() : InstructionMaker(Opcode::Auipc) {}

        std::unique_ptr<InstructionMaker> CreateInstance(
            InstructionContainer container) override
        {
            return std::unique_ptr<InstructionMaker>(
                new AuipcMaker(container.immU, container.decoded));
        }

        InstructionPtr  operator()() override
        {
            instr->_type = IType::Auipc;
                instr->_dst = RId(decoded.u.rd);
                instr->_imm = immU;
                return move(instr);
        }

    private:

        AuipcMaker(Word _immU, DecodedInstr _decoded) : InstructionMaker(Opcode::Auipc)
        {
            immU = _immU;
            decoded = _decoded;
        }

        DecodedInstr decoded;
        Word immU;
};

class JalMaker : public InstructionMaker
{
    using Imm = int32_t;
    public:
        
        JalMaker() : InstructionMaker(Opcode::Jal){}

        std::unique_ptr<InstructionMaker> CreateInstance(
            InstructionContainer container) override
        {
            return std::unique_ptr<InstructionMaker>(
                new JalMaker(container.immJ, container.decoded));
        }

        InstructionPtr  operator()() override
        {
            instr->_type = IType::J;
                instr->_brFunc = BrFunc::AT;
                instr->_dst = RId(decoded.j.rd);
                instr->_imm = immJ;
                return move(instr);
        }

    private:

        JalMaker(Imm _immJ, DecodedInstr _decoded) : InstructionMaker(Opcode::Jal)
        {
            immJ = _immJ;
            decoded = _decoded;
        }

        DecodedInstr decoded;
        Imm immJ;
};


class JalrMaker : public InstructionMaker
{
    using Imm = int32_t;
    public:
        
        JalrMaker() : InstructionMaker(Opcode::Jalr) {}

        std::unique_ptr<InstructionMaker> CreateInstance(
            InstructionContainer container) override
        {
            return std::unique_ptr<InstructionMaker>(
                new JalrMaker(container.immI, container.decoded));
        }

        InstructionPtr operator()() override
        {
            instr->_type = IType::Jr;
                instr->_brFunc = BrFunc::AT;
                instr->_dst = RId(decoded.i.rd);
                instr->_src1 = RId(decoded.i.rs1);
                instr->_imm = immI;
                return move(instr);
        }

    private:

        JalrMaker(Imm _immI, DecodedInstr _decoded) : InstructionMaker(Opcode::Jalr)
        {
            immI = _immI;
            decoded = _decoded;
        }

        DecodedInstr decoded;
        Imm immI;
};


class BranchMaker : public InstructionMaker
{
    using Imm = int32_t;
    public:
        
        BranchMaker() : InstructionMaker(Opcode::Branch) {}

        std::unique_ptr<InstructionMaker> CreateInstance(
            InstructionContainer container) override
        {
            return std::unique_ptr<InstructionMaker>(
                new BranchMaker(container.immB, container.decoded));
        }

        InstructionPtr operator()() override
        {
            instr->_type = IType::Br;
                instr->_brFunc = static_cast<BrFunc>(decoded.b.funct3);
                instr->_src1 = RId(decoded.b.rs1);
                instr->_src2 = RId(decoded.b.rs2);
                instr->_imm = immB;
                return move(instr);
        }

    private:

        BranchMaker(Imm _immB, DecodedInstr _decoded) : InstructionMaker(Opcode::Branch)
        {
            immB = _immB;
            decoded = _decoded;
        }

        DecodedInstr decoded;
        Imm immB;
};

class LoadMaker : public InstructionMaker
{
    using Imm = int32_t;
    public:
        
        LoadMaker() : InstructionMaker(Opcode::Load) {}

        std::unique_ptr<InstructionMaker> CreateInstance(
            InstructionContainer container) override
        {
            return std::unique_ptr<InstructionMaker>(
                new LoadMaker(container.immI, container.decoded));
        }

        InstructionPtr operator()() override
        {
            instr->_type = decoded.i.funct3 == fnLW ? IType::Ld : IType::Unsupported;
                instr->_aluFunc = AluFunc::Add;
                instr->_dst = RId(decoded.i.rd);
                instr->_src1 = RId(decoded.i.rs1);
                instr->_imm = immI;
                return move(instr);
        }

    private:

        LoadMaker(Imm _immI, DecodedInstr _decoded) : InstructionMaker(Opcode::Load)
        {
            immI = _immI;
            decoded = _decoded;
        }

        DecodedInstr decoded;
        Imm immI;
};


class StoreMaker : public InstructionMaker
{
    using Imm = int32_t;
    public:
        
        StoreMaker() : InstructionMaker(Opcode::Store) {}

        std::unique_ptr<InstructionMaker> CreateInstance(
            InstructionContainer container) override
        {
            return std::unique_ptr<InstructionMaker>(
                new StoreMaker(container.immS, container.decoded));
        }


        InstructionPtr operator()() override
        {
            instr->_type = decoded.i.funct3 == fnSW ? IType::St : IType::Unsupported;
                instr->_aluFunc = AluFunc::Add;
                instr->_src1 = RId(decoded.s.rs1);
                instr->_src2 = RId(decoded.s.rs2);
                instr->_imm = immS;
                return move(instr);
        }

    private:

        StoreMaker(Imm _immS, DecodedInstr _decoded) : InstructionMaker(Opcode::Store)
        {
            immS = _immS;
            decoded = _decoded;
        }

        DecodedInstr decoded;
        Imm immS;
};


class SystemMaker : public InstructionMaker
{
    using Imm = int32_t;
    public:
        
        SystemMaker() : InstructionMaker(Opcode::System) {}

        std::unique_ptr<InstructionMaker> CreateInstance(
            InstructionContainer container) override
        {
            return std::unique_ptr<InstructionMaker>(
                new SystemMaker(container.immS, container.decoded));
        }

        InstructionPtr operator()() override
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
                return move(instr);
        }

    private:

        SystemMaker(Imm _immI, DecodedInstr _decoded) : InstructionMaker(Opcode::System)
        {
            immI = _immI;
            decoded = _decoded;
        }

        DecodedInstr decoded;
        Imm immI;
};

class MiscMemMaker : public InstructionMaker
{
    public:
        
        MiscMemMaker() : InstructionMaker(Opcode::MiscMem)
        {
        }

        std::unique_ptr<InstructionMaker> CreateInstance(
            InstructionContainer container) override
        {
            return std::unique_ptr<InstructionMaker>(
                new MiscMemMaker());
        }

        InstructionPtr operator()() override
        {
            return move(instr);
        }

    private:
};


class AmoMaker : public InstructionMaker
{
    public:
        
        AmoMaker() : InstructionMaker(Opcode::Amo)
        {
        }

        std::unique_ptr<InstructionMaker> CreateInstance(
            InstructionContainer container) override
        {
            return std::unique_ptr<InstructionMaker>(
                new AmoMaker());
        }

        InstructionPtr operator()() override
        {
            return move(instr);
        }

    private:
};

class DefaultMaker : public InstructionMaker
{
    public:
        
        DefaultMaker() : InstructionMaker(Opcode::System)
        {
        }

        std::unique_ptr<InstructionMaker> CreateInstance(
            InstructionContainer container) override
        {
            return std::unique_ptr<InstructionMaker>(
                new DefaultMaker());
        }

        InstructionPtr operator()() override
        {
            instr->_type = IType::Unsupported;
            instr->_aluFunc = AluFunc::None;
            instr->_brFunc = BrFunc::NT;
            return move(instr);
        }

    private:
};

#endif //RISCV_SIM_INSTRACTIONMAKER_H