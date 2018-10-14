/* driver_init.c - driver initialization library */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,16sep18,cfm  writen
*/

#include <wheel/compiler.h>
#include <wheel/driver.h>

_RTW_IMPORT_SECTION_START (DRIVER_SECTION_NAME);
_RTW_IMPORT_SECTION_END   (DRIVER_SECTION_NAME);

/**
 * driver_init - driver initialization routine
 *
 * return: 0 on success, negtive value on error
 */

int driver_init (void)
    {
    driver_init_t * init;

    for (init  = (driver_init_t *) _RTW_SECTION_START (DRIVER_SECTION_NAME);
         init != (driver_init_t *) _RTW_SECTION_END   (DRIVER_SECTION_NAME);
         init++)
        {
        if ((*init) ())
            {
            return -1;
            }
        }

    return 0;
    }

