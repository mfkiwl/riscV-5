//=============================================================
// 
// Copyright (c) 2021 Simon Southwell. All rights reserved.
//
// Date: 24th July 2021
//
// Contains the instruction execution methods for the
// rv32csr_cpu derived class
//
// This file is part of the base RISC-V instruction set simulator
// (rv32m_cpu).
//
// This code is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This code is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this code. If not, see <http://www.gnu.org/licenses/>.
//
//=============================================================

#include "rv32m_cpu.h"

// -----------------------------------------------------------
// Constructor
// -----------------------------------------------------------

rv32m_cpu::rv32m_cpu(FILE* dbgfp) : RV32_M_INHERITANCE_CLASS(dbgfp)
{
    // Update arith table
    arith_tbl[0x01]    = {false, mul_str,      RV32I_INSTR_FMT_R,   (pFunc_t)&rv32m_cpu::mul };     /*MUL (note: arith_tbl entries at 0 (ADD) and 0x20 (SUB) already initialised in rv32i_cpu*/
    sll_tbl[0x00]      = {false, sll_str,      RV32I_INSTR_FMT_R,   (pFunc_t)&rv32i_cpu::sllr };    /*SLL*/
    sll_tbl[0x01]      = {false, mulh_str,     RV32I_INSTR_FMT_R,   (pFunc_t)&rv32m_cpu::mulh };    /*MULH*/
    slt_tbl[0x00]      = {false, slt_str,      RV32I_INSTR_FMT_R,   (pFunc_t)&rv32i_cpu::sltr };    /*SLT*/
    slt_tbl[0x01]      = {false, mulhsu_str,   RV32I_INSTR_FMT_R,   (pFunc_t)&rv32m_cpu::mulhsu };  /*MULHSU*/
    sltu_tbl[0x00]     = {false, sltu_str,     RV32I_INSTR_FMT_R,   (pFunc_t)&rv32i_cpu::sltur };   /*SLTU*/
    sltu_tbl[0x01]     = {false, mulhu_str,    RV32I_INSTR_FMT_R,   (pFunc_t)&rv32m_cpu::mulhu };   /*MULHU*/
    xor_tbl[0x00]      = {false, xor_str,      RV32I_INSTR_FMT_R,   (pFunc_t)&rv32i_cpu::xorr };    /*XOR*/
    xor_tbl[0x01]      = {false, div_str,      RV32I_INSTR_FMT_R,   (pFunc_t)&rv32m_cpu::div };     /*DIV*/
    srr_tbl[0x01]      = {false, divu_str,     RV32I_INSTR_FMT_R,   (pFunc_t)&rv32m_cpu::divu };    /*DIVU (note: srr_tbl entries at 0 (SRL) and 0x20 (SRA) already initialised in rv32i_cpu*/
    or_tbl[0x00]       = {false, or_str,       RV32I_INSTR_FMT_R,   (pFunc_t)&rv32i_cpu::orr };     /*OR*/
    or_tbl[0x01]       = {false, rem_str,      RV32I_INSTR_FMT_R,   (pFunc_t)&rv32m_cpu::rem };     /*REM*/
    and_tbl[0x00]      = {false, and_str,      RV32I_INSTR_FMT_R,   (pFunc_t)&rv32i_cpu::andr };    /*OR*/
    and_tbl[0x01]      = {false, remu_str,     RV32I_INSTR_FMT_R,   (pFunc_t)&rv32m_cpu::remu };    /*REMU*/

    // Initialise unused entries from local teriary tables to reserved instructions method
    for (int i = 2; i < RV32I_NUM_TERTIARY_OPCODES; i++)
    {
        sll_tbl[i]     = {false, reserved_str, RV32I_INSTR_ILLEGAL, (pFunc_t)&rv32i_cpu::reserved };
        slt_tbl[i]     = {false, reserved_str, RV32I_INSTR_ILLEGAL, (pFunc_t)&rv32i_cpu::reserved };
        sltu_tbl[i]    = {false, reserved_str, RV32I_INSTR_ILLEGAL, (pFunc_t)&rv32i_cpu::reserved };
        xor_tbl[i]     = {false, reserved_str, RV32I_INSTR_ILLEGAL, (pFunc_t)&rv32i_cpu::reserved };
        or_tbl[i]      = {false, reserved_str, RV32I_INSTR_ILLEGAL, (pFunc_t)&rv32i_cpu::reserved };
        and_tbl[i]     = {false, reserved_str, RV32I_INSTR_ILLEGAL, (pFunc_t)&rv32i_cpu::reserved };
    }

    int idx = 0;

    INIT_TBL_WITH_SUBTBL(op_tbl[idx], arith_tbl); idx++;
    INIT_TBL_WITH_SUBTBL(op_tbl[idx], sll_tbl); idx++;
    INIT_TBL_WITH_SUBTBL(op_tbl[idx], slt_tbl); idx++;
    INIT_TBL_WITH_SUBTBL(op_tbl[idx], sltu_tbl); idx++; 
    INIT_TBL_WITH_SUBTBL(op_tbl[idx], xor_tbl); idx++;
    INIT_TBL_WITH_SUBTBL(op_tbl[idx], srr_tbl); idx++; 
    INIT_TBL_WITH_SUBTBL(op_tbl[idx], or_tbl); idx++;
    INIT_TBL_WITH_SUBTBL(op_tbl[idx], and_tbl); idx++; 
}

void rv32m_cpu::mul(const p_rv32i_decode_t d)
{
    RV32I_DISASSEM_R_TYPE(d->instr, d->entry.instr_name, d->rd, d->rs1, d->rs2);

    if (d->rd)
    {
        int64_t product = (int64_t)state.hart[curr_hart].x[d->rs1] * (int64_t)state.hart[curr_hart].x[d->rs2];
        state.hart[curr_hart].x[d->rd] = (uint32_t)(product & 0xffffffffULL);
    }

    increment_pc();
}

void rv32m_cpu::mulh                            (const p_rv32i_decode_t d)
{
    RV32I_DISASSEM_R_TYPE(d->instr, d->entry.instr_name, d->rd, d->rs1, d->rs2);

    if (d->rd)
    {
        int64_t a = (int32_t)(state.hart[curr_hart].x[d->rs1]);
        int64_t b = (int32_t)(state.hart[curr_hart].x[d->rs2]);

        int64_t product = a * b;
        state.hart[curr_hart].x[d->rd] = (uint32_t)((product >> 32) & 0xffffffffULL);
    }

    increment_pc();
}

void rv32m_cpu::mulhsu                          (const p_rv32i_decode_t d)
{
    RV32I_DISASSEM_R_TYPE(d->instr, d->entry.instr_name, d->rd, d->rs1, d->rs2);

    if (d->rd)
    {
        int64_t  a = (int32_t)(state.hart[curr_hart].x[d->rs1]);
        uint64_t b = state.hart[curr_hart].x[d->rs2];

        int64_t product = a * b;
        state.hart[curr_hart].x[d->rd] = (uint32_t)((product >> 32) & 0xffffffffULL);
    }

    increment_pc();
}

void rv32m_cpu::mulhu                           (const p_rv32i_decode_t d)
{
    RV32I_DISASSEM_R_TYPE(d->instr, d->entry.instr_name, d->rd, d->rs1, d->rs2);

    if (d->rd)
    {
        uint64_t a = state.hart[curr_hart].x[d->rs1];
        uint64_t b = state.hart[curr_hart].x[d->rs2];

        uint64_t product = a * b;
        state.hart[curr_hart].x[d->rd] = (uint32_t)((product >> 32) & 0xffffffffULL);
    }

    increment_pc();
}

void rv32m_cpu::div                             (const p_rv32i_decode_t d)
{
    RV32I_DISASSEM_R_TYPE(d->instr, d->entry.instr_name, d->rd, d->rs1, d->rs2);

    if (d->rd)
    {
        int32_t div_result;

        int32_t a = state.hart[curr_hart].x[d->rs1];
        int32_t b = state.hart[curr_hart].x[d->rs2];

        // Division by zero (Vol1. 7.2)
        if (b == 0)
        {
            div_result = -1;
        }
        // Overflow (vol1. 7,2)
        else if (a == 0x80000000LL and b == 0xffffffffLL)
        {
            div_result = 0x80000000;
        }
        else
        {
            div_result = a / b;
        }
        state.hart[curr_hart].x[d->rd] = (uint32_t)div_result;
    }

    increment_pc();
}

void rv32m_cpu::divu                            (const p_rv32i_decode_t d)
{
    RV32I_DISASSEM_R_TYPE(d->instr, d->entry.instr_name, d->rd, d->rs1, d->rs2);

    if (d->rd)
    {
        uint32_t div_result;

        uint32_t a = state.hart[curr_hart].x[d->rs1];
        uint32_t b = state.hart[curr_hart].x[d->rs2];

        // Division by zero (Vol1. 7.2)
        if (b == 0)
        {
            div_result = -1;
        }
        else
        {
            div_result = a / b;
        }
        state.hart[curr_hart].x[d->rd] = div_result;
    }

    increment_pc();
}

void rv32m_cpu::rem                             (const p_rv32i_decode_t d)
{
    RV32I_DISASSEM_R_TYPE(d->instr, d->entry.instr_name, d->rd, d->rs1, d->rs2);

    if (d->rd)
    {
        int32_t rem_result;

        int32_t a = state.hart[curr_hart].x[d->rs1];
        int32_t b = state.hart[curr_hart].x[d->rs2];

        // Division by zero (Vol1. 7.2)
        if (b == 0)
        {
            rem_result = a;
        }
        // Overflow (vol1. 7,2)
        else if (a == 0x80000000LL and b == 0xffffffffLL)
        {
            rem_result = 0;
        }
        else
        {
           rem_result = a % b;
        }
        state.hart[curr_hart].x[d->rd] = (uint32_t)rem_result;
    }

    increment_pc();
}

void rv32m_cpu::remu                            (const p_rv32i_decode_t d)
{
    RV32I_DISASSEM_R_TYPE(d->instr, d->entry.instr_name, d->rd, d->rs1, d->rs2);

    if (d->rd)
    {
        uint32_t rem_result;

        uint32_t a = state.hart[curr_hart].x[d->rs1];
        uint32_t b = state.hart[curr_hart].x[d->rs2];

        // Division by zero (Vol1. 7.2)
        if (b == 0)
        {
            rem_result = a;
        }
        else
        {
            rem_result = a % b;
        }
        state.hart[curr_hart].x[d->rd] = rem_result;
    }

    increment_pc();
}
