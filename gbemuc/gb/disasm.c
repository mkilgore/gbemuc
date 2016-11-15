
#include "common.h"

#include <stdint.h>
#include <time.h>

#include "debug.h"
#include "gb.h"
#include "gb/disasm.h"

struct opcode_format {
    const char *format;
    enum {
        OPCODE_NONE,
        OPCODE_16BIT,
        OPCODE_8BIT,
    } type;
};

#define OP_FORM_NONE(form) \
    { .format = form, .type = OPCODE_NONE }

#define OP_FORM_ONE(form) \
    { .format = form, .type = OPCODE_8BIT }

#define OP_FORM_16(form) \
    { .format = form, .type = OPCODE_16BIT }

static struct opcode_format opcode_decode_format_str[256] = {
    [0x00] =
    OP_FORM_NONE("NOP"),
    OP_FORM_16  ("LD BC, 0x%04x"),
    OP_FORM_NONE("LD (BC), A"),
    OP_FORM_NONE("INC BC"),
    OP_FORM_NONE("INC B"),
    OP_FORM_NONE("DEC B"),
    OP_FORM_ONE ("LD B, 0x%02x"),
    OP_FORM_NONE("RLCA"),
    OP_FORM_16  ("LD (0x%04x), SP"),
    OP_FORM_NONE("ADD HL, BC"),
    OP_FORM_NONE("LD A, (BC)"),
    OP_FORM_NONE("DEC BC"),
    OP_FORM_NONE("INC C"),
    OP_FORM_NONE("DEC C"),
    OP_FORM_ONE ("LD C, 0x%02x"),
    OP_FORM_NONE("RRCA"),

    [0x10] =
    OP_FORM_NONE("STOP 0"),
    OP_FORM_16  ("LD DE, 0x%04x"),
    OP_FORM_NONE("LD (DE), A"),
    OP_FORM_NONE("INC DE"),
    OP_FORM_NONE("INC D"),
    OP_FORM_NONE("DEC D"),
    OP_FORM_ONE ("LD D, 0x%02x"),
    OP_FORM_NONE("RLA"),
    OP_FORM_ONE ("JR 0x%02x"),
    OP_FORM_NONE("ADD HL, DE"),
    OP_FORM_NONE("LD A, (DE)"),
    OP_FORM_NONE("DEC DE"),
    OP_FORM_NONE("INC E"),
    OP_FORM_NONE("DEC E"),
    OP_FORM_ONE ("LD E, 0x%02x"),
    OP_FORM_NONE("RRA"),

    [0x20] =
    OP_FORM_ONE ("JR NZ, 0x%02x"),
    OP_FORM_16  ("LD HL, 0x%04x"),
    OP_FORM_NONE("LD (HL+), A"),
    OP_FORM_NONE("INC HL"),
    OP_FORM_NONE("INC H"),
    OP_FORM_NONE("DEC H"),
    OP_FORM_ONE ("LD H, 0x%02x"),
    OP_FORM_NONE("DAA"),
    OP_FORM_ONE ("JR Z, 0x%02x"),
    OP_FORM_NONE("ADD HL, HL"),
    OP_FORM_NONE("LD A, (HL+)"),
    OP_FORM_NONE("DEC HL"),
    OP_FORM_NONE("INC L"),
    OP_FORM_NONE("DEC L"),
    OP_FORM_ONE ("LD L, 0x%02x"),
    OP_FORM_NONE("CPL"),

    [0x30] =
    OP_FORM_ONE ("JR NC, 0x%02x"),
    OP_FORM_16  ("LD SP, 0x%04x"),
    OP_FORM_NONE("LD (HL-), A"),
    OP_FORM_NONE("INC SP"),
    OP_FORM_NONE("INC (HL)"),
    OP_FORM_NONE("DEC (HL)"),
    OP_FORM_ONE ("LD (HL), 0x%02x"),
    OP_FORM_NONE("SCF"),
    OP_FORM_ONE ("JR C, 0x%02x"),
    OP_FORM_NONE("ADD HL, SP"),
    OP_FORM_NONE("LD A, (HL-)"),
    OP_FORM_NONE("DEC SP"),
    OP_FORM_NONE("INC A"),
    OP_FORM_NONE("DEC A"),
    OP_FORM_ONE ("LD A, 0x%02x"),
    OP_FORM_NONE("CCF"),

    [0x40] =
    OP_FORM_NONE("LD B, B"),
    OP_FORM_NONE("LD B, C"),
    OP_FORM_NONE("LD B, D"),
    OP_FORM_NONE("LD B, E"),
    OP_FORM_NONE("LD B, H"),
    OP_FORM_NONE("LD B, L"),
    OP_FORM_NONE("LD B, (HL)"),
    OP_FORM_NONE("LD B, A"),
    OP_FORM_NONE("LD C, B"),
    OP_FORM_NONE("LD C, C"),
    OP_FORM_NONE("LD C, D"),
    OP_FORM_NONE("LD C, E"),
    OP_FORM_NONE("LD C, H"),
    OP_FORM_NONE("LD C, L"),
    OP_FORM_NONE("LD C, (HL)"),
    OP_FORM_NONE("LD C, A"),

    [0x50] =
    OP_FORM_NONE("LD D, B"),
    OP_FORM_NONE("LD D, C"),
    OP_FORM_NONE("LD D, D"),
    OP_FORM_NONE("LD D, E"),
    OP_FORM_NONE("LD D, H"),
    OP_FORM_NONE("LD D, L"),
    OP_FORM_NONE("LD D, (HL)"),
    OP_FORM_NONE("LD D, A"),
    OP_FORM_NONE("LD E, B"),
    OP_FORM_NONE("LD E, C"),
    OP_FORM_NONE("LD E, D"),
    OP_FORM_NONE("LD E, E"),
    OP_FORM_NONE("LD E, H"),
    OP_FORM_NONE("LD E, L"),
    OP_FORM_NONE("LD E, (HL)"),
    OP_FORM_NONE("LD E, A"),

    [0x60] =
    OP_FORM_NONE("LD H, B"),
    OP_FORM_NONE("LD H, C"),
    OP_FORM_NONE("LD H, D"),
    OP_FORM_NONE("LD H, E"),
    OP_FORM_NONE("LD H, H"),
    OP_FORM_NONE("LD H, L"),
    OP_FORM_NONE("LD H, (HL)"),
    OP_FORM_NONE("LD H, A"),
    OP_FORM_NONE("LD L, B"),
    OP_FORM_NONE("LD L, C"),
    OP_FORM_NONE("LD L, D"),
    OP_FORM_NONE("LD L, E"),
    OP_FORM_NONE("LD L, H"),
    OP_FORM_NONE("LD L, L"),
    OP_FORM_NONE("LD L, (HL)"),
    OP_FORM_NONE("LD L, A"),

    [0x70] =
    OP_FORM_NONE("LD (HL), B"),
    OP_FORM_NONE("LD (HL), C"),
    OP_FORM_NONE("LD (HL), D"),
    OP_FORM_NONE("LD (HL), E"),
    OP_FORM_NONE("LD (HL), H"),
    OP_FORM_NONE("LD (HL), L"),
    OP_FORM_NONE("HALT"),
    OP_FORM_NONE("LD (HL), A"),
    OP_FORM_NONE("LD A, B"),
    OP_FORM_NONE("LD A, C"),
    OP_FORM_NONE("LD A, D"),
    OP_FORM_NONE("LD A, E"),
    OP_FORM_NONE("LD A, H"),
    OP_FORM_NONE("LD A, L"),
    OP_FORM_NONE("LD A, (HL)"),
    OP_FORM_NONE("LD A, A"),

    [0x80] =
    OP_FORM_NONE("ADD A, B"),
    OP_FORM_NONE("ADD A, C"),
    OP_FORM_NONE("ADD A, D"),
    OP_FORM_NONE("ADD A, E"),
    OP_FORM_NONE("ADD A, H"),
    OP_FORM_NONE("ADD A, L"),
    OP_FORM_NONE("ADD A, (HL)"),
    OP_FORM_NONE("ADD A, A"),
    OP_FORM_NONE("ADC A, B"),
    OP_FORM_NONE("ADC A, C"),
    OP_FORM_NONE("ADC A, D"),
    OP_FORM_NONE("ADC A, E"),
    OP_FORM_NONE("ADC A, H"),
    OP_FORM_NONE("ADC A, L"),
    OP_FORM_NONE("ADC A, (HL)"),
    OP_FORM_NONE("ADC A, A"),

    [0x90] =
    OP_FORM_NONE("SUB B"),
    OP_FORM_NONE("SUB C"),
    OP_FORM_NONE("SUB D"),
    OP_FORM_NONE("SUB E"),
    OP_FORM_NONE("SUB H"),
    OP_FORM_NONE("SUB L"),
    OP_FORM_NONE("SUB (HL)"),
    OP_FORM_NONE("SUB A"),
    OP_FORM_NONE("SBC A, B"),
    OP_FORM_NONE("SBC A, C"),
    OP_FORM_NONE("SBC A, D"),
    OP_FORM_NONE("SBC A, E"),
    OP_FORM_NONE("SBC A, H"),
    OP_FORM_NONE("SBC A, L"),
    OP_FORM_NONE("SBC A, (HL)"),
    OP_FORM_NONE("SBC A, A"),

    [0xA0] =
    OP_FORM_NONE("AND B"),
    OP_FORM_NONE("AND C"),
    OP_FORM_NONE("AND D"),
    OP_FORM_NONE("AND E"),
    OP_FORM_NONE("AND H"),
    OP_FORM_NONE("AND L"),
    OP_FORM_NONE("AND (HL)"),
    OP_FORM_NONE("AND A"),
    OP_FORM_NONE("XOR B"),
    OP_FORM_NONE("XOR C"),
    OP_FORM_NONE("XOR D"),
    OP_FORM_NONE("XOR E"),
    OP_FORM_NONE("XOR H"),
    OP_FORM_NONE("XOR L"),
    OP_FORM_NONE("XOR (HL)"),
    OP_FORM_NONE("XOR A"),

    [0xB0] =
    OP_FORM_NONE("OR B"),
    OP_FORM_NONE("OR C"),
    OP_FORM_NONE("OR D"),
    OP_FORM_NONE("OR E"),
    OP_FORM_NONE("OR H"),
    OP_FORM_NONE("OR L"),
    OP_FORM_NONE("OR (HL)"),
    OP_FORM_NONE("OR A"),
    OP_FORM_NONE("CP B"),
    OP_FORM_NONE("CP C"),
    OP_FORM_NONE("CP D"),
    OP_FORM_NONE("CP E"),
    OP_FORM_NONE("CP H"),
    OP_FORM_NONE("CP L"),
    OP_FORM_NONE("CP (HL)"),
    OP_FORM_NONE("CP A"),

    [0xC0] =
    OP_FORM_NONE("RET NZ"),
    OP_FORM_NONE("POP BC"),
    OP_FORM_16  ("JP NZ, 0x%04x"),
    OP_FORM_16  ("JP 0x%04x"),
    OP_FORM_16  ("CALL NZ, 0x%04x"),
    OP_FORM_NONE("PUSH BC"),
    OP_FORM_ONE ("ADD A, 0x%02x"),
    OP_FORM_NONE("RST 0x00"),
    OP_FORM_NONE("RET Z"),
    OP_FORM_NONE("RET"),
    OP_FORM_16  ("JP Z, 0x%04x"),
    OP_FORM_NONE("CB"),
    OP_FORM_16  ("CALL Z, 0x%04x"),
    OP_FORM_16  ("CALL 0x%04x"),
    OP_FORM_ONE ("ADC A, 0x%02x"),
    OP_FORM_NONE("RST 0x08"),

    [0xD0] =
    OP_FORM_NONE("RET NC"),
    OP_FORM_NONE("POP DE"),
    OP_FORM_16  ("JP NC, 0x%04x"),
    OP_FORM_NONE(""),
    OP_FORM_16  ("CALL NC, 0x%04x"),
    OP_FORM_NONE("PUSH DE"),
    OP_FORM_ONE ("SUB 0x%02x"),
    OP_FORM_NONE("RST 0x10"),
    OP_FORM_NONE("RET C"),
    OP_FORM_NONE("RETI"),
    OP_FORM_16  ("JP C, 0x%04x"),
    OP_FORM_NONE(""),
    OP_FORM_16  ("CALL C, 0x%04x"),
    OP_FORM_NONE(""),
    OP_FORM_ONE ("SBC A, 0x%02x"),
    OP_FORM_NONE("RST 0x18"),

    [0xE0] =
    OP_FORM_ONE ("LD (0x00FF + 0x%02x), A"),
    OP_FORM_NONE("POP HL"),
    OP_FORM_NONE("LD (C), A"),
    OP_FORM_NONE(""),
    OP_FORM_NONE(""),
    OP_FORM_NONE("PUSH HL"),
    OP_FORM_ONE ("AND 0x%02x"),
    OP_FORM_NONE("RST 0x20"),
    OP_FORM_ONE ("AND SP, 0x%02x"),
    OP_FORM_NONE("JP (HL)"),
    OP_FORM_16  ("LD (0x%04x), A"),
    OP_FORM_NONE(""),
    OP_FORM_NONE(""),
    OP_FORM_NONE(""),
    OP_FORM_ONE ("XOR 0x%02x"),
    OP_FORM_NONE("RST 0x28"),

    [0xF0] =
    OP_FORM_ONE ("LD A, (0x00FF + 0x%02x)"),
    OP_FORM_NONE("POP HL"),
    OP_FORM_NONE("LD (C), A"),
    OP_FORM_NONE("DI"),
    OP_FORM_NONE(""),
    OP_FORM_NONE("PUSH AF"),
    OP_FORM_ONE ("OR 0x%02x"),
    OP_FORM_NONE("RST 0x30"),
    OP_FORM_ONE ("LD HL, SP + 0x%02x"),
    OP_FORM_NONE("LD SP, HL"),
    OP_FORM_16  ("LD A, (0x%04x"),
    OP_FORM_NONE("EI"),
    OP_FORM_NONE(""),
    OP_FORM_NONE(""),
    OP_FORM_ONE ("CP 0x%02x"),
    OP_FORM_NONE("RST 0x38"),
};

static struct opcode_format opcode_cb_decode_format_str[256] = {
    [0x00] =
    OP_FORM_NONE("RLC B"),
    OP_FORM_NONE("RLC C"),
    OP_FORM_NONE("RLC D"),
    OP_FORM_NONE("RLC E"),
    OP_FORM_NONE("RLC H"),
    OP_FORM_NONE("RLC L"),
    OP_FORM_NONE("RLC (HL)"),
    OP_FORM_NONE("RLC A"),
    OP_FORM_NONE("RRC B"),
    OP_FORM_NONE("RRC C"),
    OP_FORM_NONE("RRC D"),
    OP_FORM_NONE("RRC E"),
    OP_FORM_NONE("RRC H"),
    OP_FORM_NONE("RRC L"),
    OP_FORM_NONE("RRC (HL)"),
    OP_FORM_NONE("RRC A"),

    [0x10] =
    OP_FORM_NONE("RL B"),
    OP_FORM_NONE("RL C"),
    OP_FORM_NONE("RL D"),
    OP_FORM_NONE("RL E"),
    OP_FORM_NONE("RL H"),
    OP_FORM_NONE("RL L"),
    OP_FORM_NONE("RL (HL)"),
    OP_FORM_NONE("RL A"),
    OP_FORM_NONE("RR B"),
    OP_FORM_NONE("RR C"),
    OP_FORM_NONE("RR D"),
    OP_FORM_NONE("RR E"),
    OP_FORM_NONE("RR H"),
    OP_FORM_NONE("RR L"),
    OP_FORM_NONE("RR (HL)"),
    OP_FORM_NONE("RR A"),

    [0x20] =
    OP_FORM_NONE("SLA B"),
    OP_FORM_NONE("SLA C"),
    OP_FORM_NONE("SLA D"),
    OP_FORM_NONE("SLA E"),
    OP_FORM_NONE("SLA H"),
    OP_FORM_NONE("SLA L"),
    OP_FORM_NONE("SLA (HL)"),
    OP_FORM_NONE("SLA A"),
    OP_FORM_NONE("SRA B"),
    OP_FORM_NONE("SRA C"),
    OP_FORM_NONE("SRA D"),
    OP_FORM_NONE("SRA E"),
    OP_FORM_NONE("SRA H"),
    OP_FORM_NONE("SRA L"),
    OP_FORM_NONE("SRA (HL)"),
    OP_FORM_NONE("SRA A"),

    [0x30] =
    OP_FORM_NONE("SWAP B"),
    OP_FORM_NONE("SWAP C"),
    OP_FORM_NONE("SWAP D"),
    OP_FORM_NONE("SWAP E"),
    OP_FORM_NONE("SWAP H"),
    OP_FORM_NONE("SWAP L"),
    OP_FORM_NONE("SWAP (HL)"),
    OP_FORM_NONE("SWAP A"),
    OP_FORM_NONE("SRL B"),
    OP_FORM_NONE("SRL C"),
    OP_FORM_NONE("SRL D"),
    OP_FORM_NONE("SRL E"),
    OP_FORM_NONE("SRL H"),
    OP_FORM_NONE("SRL L"),
    OP_FORM_NONE("SRL (HL)"),
    OP_FORM_NONE("SRL A"),

    [0x40] =
    OP_FORM_NONE("BIT 0, B"),
    OP_FORM_NONE("BIT 0, C"),
    OP_FORM_NONE("BIT 0, D"),
    OP_FORM_NONE("BIT 0, E"),
    OP_FORM_NONE("BIT 0, H"),
    OP_FORM_NONE("BIT 0, L"),
    OP_FORM_NONE("BIT 0, (HL)"),
    OP_FORM_NONE("BIT 1, A"),
    OP_FORM_NONE("BIT 1, B"),
    OP_FORM_NONE("BIT 1, C"),
    OP_FORM_NONE("BIT 1, D"),
    OP_FORM_NONE("BIT 1, E"),
    OP_FORM_NONE("BIT 1, H"),
    OP_FORM_NONE("BIT 1, L"),
    OP_FORM_NONE("BIT 1, (HL)"),
    OP_FORM_NONE("BIT 1, A"),

    [0x50] =
    OP_FORM_NONE("BIT 2, B"),
    OP_FORM_NONE("BIT 2, C"),
    OP_FORM_NONE("BIT 2, D"),
    OP_FORM_NONE("BIT 2, E"),
    OP_FORM_NONE("BIT 2, H"),
    OP_FORM_NONE("BIT 2, L"),
    OP_FORM_NONE("BIT 2, (HL)"),
    OP_FORM_NONE("BIT 3, A"),
    OP_FORM_NONE("BIT 3, B"),
    OP_FORM_NONE("BIT 3, C"),
    OP_FORM_NONE("BIT 3, D"),
    OP_FORM_NONE("BIT 3, E"),
    OP_FORM_NONE("BIT 3, H"),
    OP_FORM_NONE("BIT 3, L"),
    OP_FORM_NONE("BIT 3, (HL)"),
    OP_FORM_NONE("BIT 3, A"),

    [0x60] =
    OP_FORM_NONE("BIT 4, B"),
    OP_FORM_NONE("BIT 4, C"),
    OP_FORM_NONE("BIT 4, D"),
    OP_FORM_NONE("BIT 4, E"),
    OP_FORM_NONE("BIT 4, H"),
    OP_FORM_NONE("BIT 4, L"),
    OP_FORM_NONE("BIT 4, (HL)"),
    OP_FORM_NONE("BIT 5, A"),
    OP_FORM_NONE("BIT 5, B"),
    OP_FORM_NONE("BIT 5, C"),
    OP_FORM_NONE("BIT 5, D"),
    OP_FORM_NONE("BIT 5, E"),
    OP_FORM_NONE("BIT 5, H"),
    OP_FORM_NONE("BIT 5, L"),
    OP_FORM_NONE("BIT 5, (HL)"),
    OP_FORM_NONE("BIT 5, A"),

    [0x70] =
    OP_FORM_NONE("BIT 6, B"),
    OP_FORM_NONE("BIT 6, C"),
    OP_FORM_NONE("BIT 6, D"),
    OP_FORM_NONE("BIT 6, E"),
    OP_FORM_NONE("BIT 6, H"),
    OP_FORM_NONE("BIT 6, L"),
    OP_FORM_NONE("BIT 6, (HL)"),
    OP_FORM_NONE("BIT 7, A"),
    OP_FORM_NONE("BIT 7, B"),
    OP_FORM_NONE("BIT 7, C"),
    OP_FORM_NONE("BIT 7, D"),
    OP_FORM_NONE("BIT 7, E"),
    OP_FORM_NONE("BIT 7, H"),
    OP_FORM_NONE("BIT 7, L"),
    OP_FORM_NONE("BIT 7, (HL)"),
    OP_FORM_NONE("BIT 7, A"),

    [0x80] =
    OP_FORM_NONE("RES 0, B"),
    OP_FORM_NONE("RES 0, C"),
    OP_FORM_NONE("RES 0, D"),
    OP_FORM_NONE("RES 0, E"),
    OP_FORM_NONE("RES 0, H"),
    OP_FORM_NONE("RES 0, L"),
    OP_FORM_NONE("RES 0, (HL)"),
    OP_FORM_NONE("RES 0, A"),
    OP_FORM_NONE("RES 1, B"),
    OP_FORM_NONE("RES 1, C"),
    OP_FORM_NONE("RES 1, D"),
    OP_FORM_NONE("RES 1, E"),
    OP_FORM_NONE("RES 1, H"),
    OP_FORM_NONE("RES 1, L"),
    OP_FORM_NONE("RES 1, (HL)"),
    OP_FORM_NONE("RES 1, A"),

    [0x90] =
    OP_FORM_NONE("RES 2, B"),
    OP_FORM_NONE("RES 2, C"),
    OP_FORM_NONE("RES 2, D"),
    OP_FORM_NONE("RES 2, E"),
    OP_FORM_NONE("RES 2, H"),
    OP_FORM_NONE("RES 2, L"),
    OP_FORM_NONE("RES 2, (HL)"),
    OP_FORM_NONE("RES 2, A"),
    OP_FORM_NONE("RES 3, B"),
    OP_FORM_NONE("RES 3, C"),
    OP_FORM_NONE("RES 3, D"),
    OP_FORM_NONE("RES 3, E"),
    OP_FORM_NONE("RES 3, H"),
    OP_FORM_NONE("RES 3, L"),
    OP_FORM_NONE("RES 3, (HL)"),
    OP_FORM_NONE("RES 3, A"),

    [0xA0] =
    OP_FORM_NONE("RES 4, B"),
    OP_FORM_NONE("RES 4, C"),
    OP_FORM_NONE("RES 4, D"),
    OP_FORM_NONE("RES 4, E"),
    OP_FORM_NONE("RES 4, H"),
    OP_FORM_NONE("RES 4, L"),
    OP_FORM_NONE("RES 4, (HL)"),
    OP_FORM_NONE("RES 4, A"),
    OP_FORM_NONE("RES 5, B"),
    OP_FORM_NONE("RES 5, C"),
    OP_FORM_NONE("RES 5, D"),
    OP_FORM_NONE("RES 5, E"),
    OP_FORM_NONE("RES 5, H"),
    OP_FORM_NONE("RES 5, L"),
    OP_FORM_NONE("RES 5, (HL)"),
    OP_FORM_NONE("RES 5, A"),

    [0xB0] =
    OP_FORM_NONE("RES 6, B"),
    OP_FORM_NONE("RES 6, C"),
    OP_FORM_NONE("RES 6, D"),
    OP_FORM_NONE("RES 6, E"),
    OP_FORM_NONE("RES 6, H"),
    OP_FORM_NONE("RES 6, L"),
    OP_FORM_NONE("RES 6, (HL)"),
    OP_FORM_NONE("RES 6, A"),
    OP_FORM_NONE("RES 7, B"),
    OP_FORM_NONE("RES 7, C"),
    OP_FORM_NONE("RES 7, D"),
    OP_FORM_NONE("RES 7, E"),
    OP_FORM_NONE("RES 7, H"),
    OP_FORM_NONE("RES 7, L"),
    OP_FORM_NONE("RES 7, (HL)"),
    OP_FORM_NONE("RES 7, A"),

    [0xC0] =
    OP_FORM_NONE("SET 0, B"),
    OP_FORM_NONE("SET 0, C"),
    OP_FORM_NONE("SET 0, D"),
    OP_FORM_NONE("SET 0, E"),
    OP_FORM_NONE("SET 0, H"),
    OP_FORM_NONE("SET 0, L"),
    OP_FORM_NONE("SET 0, (HL)"),
    OP_FORM_NONE("SET 0, A"),
    OP_FORM_NONE("SET 1, B"),
    OP_FORM_NONE("SET 1, C"),
    OP_FORM_NONE("SET 1, D"),
    OP_FORM_NONE("SET 1, E"),
    OP_FORM_NONE("SET 1, H"),
    OP_FORM_NONE("SET 1, L"),
    OP_FORM_NONE("SET 1, (HL)"),
    OP_FORM_NONE("SET 1, A"),

    [0xD0] =
    OP_FORM_NONE("SET 2, B"),
    OP_FORM_NONE("SET 2, C"),
    OP_FORM_NONE("SET 2, D"),
    OP_FORM_NONE("SET 2, E"),
    OP_FORM_NONE("SET 2, H"),
    OP_FORM_NONE("SET 2, L"),
    OP_FORM_NONE("SET 2, (HL)"),
    OP_FORM_NONE("SET 2, A"),
    OP_FORM_NONE("SET 3, B"),
    OP_FORM_NONE("SET 3, C"),
    OP_FORM_NONE("SET 3, D"),
    OP_FORM_NONE("SET 3, E"),
    OP_FORM_NONE("SET 3, H"),
    OP_FORM_NONE("SET 3, L"),
    OP_FORM_NONE("SET 3, (HL)"),
    OP_FORM_NONE("SET 3, A"),

    [0xE0] =
    OP_FORM_NONE("SET 4, B"),
    OP_FORM_NONE("SET 4, C"),
    OP_FORM_NONE("SET 4, D"),
    OP_FORM_NONE("SET 4, E"),
    OP_FORM_NONE("SET 4, H"),
    OP_FORM_NONE("SET 4, L"),
    OP_FORM_NONE("SET 4, (HL)"),
    OP_FORM_NONE("SET 4, A"),
    OP_FORM_NONE("SET 5, B"),
    OP_FORM_NONE("SET 5, C"),
    OP_FORM_NONE("SET 5, D"),
    OP_FORM_NONE("SET 5, E"),
    OP_FORM_NONE("SET 5, H"),
    OP_FORM_NONE("SET 5, L"),
    OP_FORM_NONE("SET 5, (HL)"),
    OP_FORM_NONE("SET 5, A"),

    [0xF0] =
    OP_FORM_NONE("SET 6, B"),
    OP_FORM_NONE("SET 6, C"),
    OP_FORM_NONE("SET 6, D"),
    OP_FORM_NONE("SET 6, E"),
    OP_FORM_NONE("SET 6, H"),
    OP_FORM_NONE("SET 6, L"),
    OP_FORM_NONE("SET 6, (HL)"),
    OP_FORM_NONE("SET 6, A"),
    OP_FORM_NONE("SET 7, B"),
    OP_FORM_NONE("SET 7, C"),
    OP_FORM_NONE("SET 7, D"),
    OP_FORM_NONE("SET 7, E"),
    OP_FORM_NONE("SET 7, H"),
    OP_FORM_NONE("SET 7, L"),
    OP_FORM_NONE("SET 7, (HL)"),
    OP_FORM_NONE("SET 7, A"),
};

void gb_disasm_inst(char *buf, uint8_t *bytes)
{
    struct opcode_format *format = NULL;
    uint8_t opcode;

    opcode = bytes[0];

    if (opcode != 0xCB)
        format = opcode_decode_format_str + opcode;
    else
        format = opcode_cb_decode_format_str + opcode;

    switch (format->type) {
    case OPCODE_NONE:
        sprintf(buf, format->format);
        break;

    case OPCODE_8BIT:
        sprintf(buf, format->format, bytes[1]);
        break;

    case OPCODE_16BIT:
        sprintf(buf, format->format, bytes[1] + (((uint16_t)bytes[2]) << 8));
        break;
    }
}
