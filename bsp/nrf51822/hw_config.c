#include <wheel/mem.h>

extern char __bss_end__ [];

struct phys_mem system_phys_mem [] =
    {
        { __bss_end__, (char *) 0x20008000, },
        { 0, 0 }
    };
