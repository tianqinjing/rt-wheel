/* cmder.h - cmder shell library header file */

/*
 * Copyright (c) 2018 Fangming Chai
 */

/*
modification history
--------------------
01a,06oct18,cfm  writen
*/

#ifndef __CMDER_H__
#define __CMDER_H__

#include <stdint.h>

#include <wheel/ring.h>
#include <wheel/common.h>

/* macros */

#define CMDER_BUFFER_SIZE       128
#define CMDER_NR_ARGUMENTS      8               /* including the command name */

#undef  RTW_CONFIG_CMDER_TIRE

#define CMDER_SECTION_NAME      cmder_cmds

#define __RTW_CMDER_CMD_DEF(name, desc, cmd, tag)                              \
const cmder_cmd_t __CONCAT (__cmder_, tag) _RTW_SECTION (CMDER_SECTION_NAME) = \
    {                                                                          \
    name,                                                                      \
    desc,                                                                      \
    cmd                                                                        \
    }

#define RTW_CMDER_CMD_DEF(name, desc, cmd)                                     \
    __RTW_CMDER_CMD_DEF (name, desc, cmd, __LINE__)

#define CMDER_PRINT_LALIGN      0
#define CMDER_PRINT_RALIGN      1

/* typedefs */

typedef struct cmder
    {
    char         input_buff [CMDER_BUFFER_SIZE];
    uint8_t      input_idx;
    uint8_t      compl_idx;
    uint8_t      shift_idx;
    uint8_t      position;
    int          line_end;
    ring_t       his_cmd;
    ring_t       his_idx;
    int       (* getchar) (uintptr_t);
    void      (* putchar) (uintptr_t, char);
    int       (* putstr)  (uintptr_t, const char *);
    uintptr_t    arg;       /* argument for those routines */
    } cmder_t;

typedef struct cmder_cmd
    {
    const char * name;
    const char * desc;
    int       (* cmd) (cmder_t * cmder, int argc, char * argv []);
    } cmder_cmd_t;

enum cmder_vk
    {
    cmder_vk_nul = 0x00,
    cmder_vk_ctrla,
    cmder_vk_ctrlb,
    cmder_vk_ctrlc,
    cmder_vk_ctrld,
    cmder_vk_ctrle,
    cmder_vk_ctrlf,
    cmder_vk_ctrlg,
    cmder_vk_ctrlh,
    cmder_vk_ctrli,
    cmder_vk_ctrlj,
    cmder_vk_ctrlk,
    cmder_vk_ctrll,
    cmder_vk_ctrlm,
    cmder_vk_ctrln,
    cmder_vk_ctrlo,
    cmder_vk_ctrlp,
    cmder_vk_ctrlq,
    cmder_vk_ctrlr,
    cmder_vk_ctrls,
    cmder_vk_ctrlt,
    cmder_vk_ctrlu,
    cmder_vk_ctrlv,
    cmder_vk_ctrlw,
    cmder_vk_ctrlx,
    cmder_vk_ctrly,
    cmder_vk_ctrlz,

    cmder_vk_esc,
    cmder_vk_fs,
    cmder_vk_gs,
    cmder_vk_rs,
    cmder_vk_us,

    /* printable, 0x20~0x7e */

    cmder_vk_del = 0x7f,

    cmder_vk_home,
    cmder_vk_end,
    cmder_vk_up,
    cmder_vk_down,
    cmder_vk_left,
    cmder_vk_right,
    };

/* externs */

extern void cmder_loop  (cmder_t * cmder);
extern void cmder_print (cmder_t * cmder, const char * str, size_t len, int align);

#endif  /* __CMDER_H__ */

