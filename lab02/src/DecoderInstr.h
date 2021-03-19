#ifndef RISCV_SIM_DECODERINSTR_H
#define RISCV_SIM_DECODERINSTR_H

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

    #endif //RISCV_SIM_DECODERINSTR_H