#pragma once

enum BusInstruction
{
    BUS_SET_INDEX = 0xf0,
    BUS_GET_UID = 0xf1,
    BUS_MODE_IO0 = 0xf2,
    BUS_MODE_IO1 = 0xf3,
    BUS_WRITE_FLASH = 0xf4,
    BUS_JUMP_TO_FLASH = 0xf5,
    BUS_EXECUTE = 0xf6,
    BUS_BLINK = 0xfe
};
