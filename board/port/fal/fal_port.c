/*
 * Copyright (c) 2006-2023, Evlers Developers
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date         Author      Notes
 * 2024-01-11   Evlers      first implementation
 */

#include "rtthread.h"

#define DBG_TAG             "fal.port"
#define DBG_LVL             DBG_INFO
#include "rtdbg.h"

#ifdef RT_USING_FAL

#include "fal.h"
#include "easyflash.h"


static int rt_fal_init (void)
{
    /* Initializes the flash abstraction layer */
    fal_init();

#ifdef PKG_USING_EASYFLASH
    /* Initializes the easyflash */
    easyflash_init();
#endif /* PKG_USING_EASYFLASH */

    return RT_EOK;
}
INIT_COMPONENT_EXPORT(rt_fal_init);

#endif /* RT_USING_FAL */
