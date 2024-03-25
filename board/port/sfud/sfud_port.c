/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date             Author      Notes
 * 2024-03-24       Evlers      first implementation
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <spi_flash.h>
#include <drv_spi.h>
#include "drv_gpio.h"

static int rt_sfud_flash_init (void)
{
    extern rt_spi_flash_device_t rt_sfud_flash_probe(const char *spi_flash_dev_name, const char *spi_dev_name);
    extern int fal_init(void);

    rt_hw_spi_device_attach("spi1", "spi10", GET_PIN(A, 4));

    /* initialize SPI Flash device */
    rt_sfud_flash_probe("norflash0", "spi10");

    fal_init();

    return 0;
}
#ifndef FIRMWARE_EXEC_USING_QEMU
INIT_DEVICE_EXPORT(rt_sfud_flash_init);
#endif
