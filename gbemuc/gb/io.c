
#include "common.h"

#include <stdint.h>
#include <string.h>

#include "slist.h"
#include "gb.h"
#include "gb/gpu.h"
#include "gb/io.h"
#include "debug.h"

uint8_t gb_emu_io_read8(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    uint8_t ret = 0;

    switch (addr + low) {
    case GB_IO_GPU_CTL:
        ret = emu->gpu.ctl;
        break;

    case GB_IO_GPU_STATUS:
        ret = emu->gpu.status & 0xF8;
        ret |= emu->gpu.mode;
        /* FIXME: Insert LY == LYC check here */
        break;

    case GB_IO_GPU_SCRY:
        ret = emu->gpu.scroll_y;
        break;

    case GB_IO_GPU_SCRX:
        ret = emu->gpu.scroll_x;
        break;

    case GB_IO_GPU_LY:
        ret = emu->gpu.cur_line;
        break;
    }

    return ret;
}

void gb_emu_io_write8(struct gb_emu *emu, uint16_t addr, uint16_t low, uint8_t byte)
{
    DEBUG_PRINTF("WRITE TO: 0x%02x - 0x%02x\n", addr + low, byte);
    switch (addr + low) {
    case GB_IO_BIOS_FLAG:
        if (byte == 1)
            emu->mmu.bios_flag = 1;
        break;

    case GB_IO_GPU_CTL:
        gb_gpu_ctl_change(&emu->gpu, byte);
        break;

    case GB_IO_GPU_STATUS:
        /* BIT 2 is preserved */
        emu->gpu.ctl &= 0x04;
        emu->gpu.ctl |= byte;
        break;

    case GB_IO_GPU_SCRY:
        emu->gpu.scroll_y = byte;
        break;

    case GB_IO_GPU_SCRX:
        emu->gpu.scroll_x = byte;
        break;

    case GB_IO_GPU_LY:
        emu->gpu.cur_line = 0;
        break;
    }
}

