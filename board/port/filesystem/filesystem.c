/*
 * Copyright (c) 2006-2023, Evlers Developers
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date         Author      Notes
 * 2024-03-13   Evlers      first implementation
 */

#include <rtthread.h>

#ifdef BSP_USING_FS
#if DFS_FILESYSTEMS_MAX < 4
#error "Please define DFS_FILESYSTEMS_MAX more than 4"
#endif
#if DFS_FILESYSTEM_TYPES_MAX < 4
#error "Please define DFS_FILESYSTEM_TYPES_MAX more than 4"
#endif

#ifdef BSP_USING_SPI_FLASH_FS
#include "fal.h"
#endif

#include <dfs_fs.h>
#include "dfs_romfs.h"
#include "drv_sdmmc.h"
#include "drv_gpio.h"

#define DBG_TAG     "filesystem"
#define DBG_LVL     DBG_INFO
#include <rtdbg.h>

/* File system partition name on FAL partition table */
#define FAL_FS_PART_NAME                "filesystem"

static const struct romfs_dirent _romfs_root[] = {
    {ROMFS_DIRENT_DIR, "flash", RT_NULL, 0},
    {ROMFS_DIRENT_DIR, "sdcard", RT_NULL, 0}};

const struct romfs_dirent romfs_root = {
    ROMFS_DIRENT_DIR, "/", (rt_uint8_t *)_romfs_root, sizeof(_romfs_root) / sizeof(_romfs_root[0])};

#ifdef BSP_USING_SDCARD_FS

#if defined(RT_USING_WIFI_HOST_DRIVER) && defined(WHD_RESOURCES_IN_EXTERNAL_STORAGE_FS)
struct rt_completion fs_mount_comp;
#endif /* RT_USING_WIFI_HOST_DRIVER */

/* SD Card hot plug detection pin */
#define SD_CHECK_PIN GET_PIN(D, 5)

static void _sdcard_mount(void)
{
    rt_device_t device;

    device = rt_device_find("sd0");
    if (device == NULL)
    {
        mmcsd_wait_cd_changed(0);
        stm32_mmcsd_change();
        mmcsd_wait_cd_changed(RT_WAITING_FOREVER);
        device = rt_device_find("sd0");
    }
    if (device != RT_NULL)
    {
        if (dfs_mount("sd0", "/sdcard", "elm", 0, 0) == RT_EOK)
        {
#if defined(RT_USING_WIFI_HOST_DRIVER) && defined(WHD_RESOURCES_IN_EXTERNAL_STORAGE_FS)
            rt_kprintf("Filesystem mounted, notifying waiting tasks...\n");
            rt_completion_done(&fs_mount_comp);
#endif /* RT_USING_WIFI_HOST_DRIVER */
            LOG_I("sd card mount to '/sdcard'");
        }
        else
        {
            LOG_W("sd card mount to '/sdcard' failed!");
        }
    }
}

static void _sdcard_unmount(void)
{
    rt_thread_mdelay(200);
    dfs_unmount("/sdcard");
    LOG_I("Unmount \"/sdcard\"");

    mmcsd_wait_cd_changed(0);
    stm32_mmcsd_change();
    mmcsd_wait_cd_changed(RT_WAITING_FOREVER);
}

static void sd_mount(void *parameter)
{
    rt_uint8_t re_sd_check_pin = 0;
    rt_thread_mdelay(200);
    while (1)
    {
        rt_thread_mdelay(200);
        if (!re_sd_check_pin && (re_sd_check_pin = rt_pin_read(SD_CHECK_PIN)) != 0)
        {
            _sdcard_mount();
        }

        if (re_sd_check_pin && (re_sd_check_pin = rt_pin_read(SD_CHECK_PIN)) == 0)
        {
            _sdcard_unmount();
        }
    }
}

#endif /* BSP_USING_SDCARD_FS */

int mount_init(void)
{
    if (dfs_mount(RT_NULL, "/", "rom", 0, &(romfs_root)) != 0)
    {
        LOG_E("rom mount to '/' failed!");
    }
#ifdef BSP_USING_SPI_FLASH_FS

    /* Create a block device using the flash abstraction layer */
    if (fal_partition_find(FAL_FS_PART_NAME) != NULL && fal_blk_device_create(FAL_FS_PART_NAME) != NULL)
    {
        /* Mount the FAT file system to the root directory */
        if (dfs_mount(FAL_FS_PART_NAME, "/flash", "elm", 0, 0) != 0)
        {
            if (!dfs_mkfs("elm", FAL_FS_PART_NAME))
            {
                if (dfs_mount(FAL_FS_PART_NAME, "/flash", "elm", 0, 0) != 0)
                {
                    LOG_E("The fat file system failed to be mounted!");
                }
            }
            else
            {
                LOG_E("FAT file system formatting failed!");
            }
        }
    }

#endif

#ifdef BSP_USING_SDCARD_FS
    rt_thread_t tid;

    rt_pin_mode(SD_CHECK_PIN, PIN_MODE_INPUT_PULLUP);

    tid = rt_thread_create("sd_mount", sd_mount, RT_NULL,
                           2048, RT_THREAD_PRIORITY_MAX - 2, 20);
    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }
    else
    {
        LOG_E("create sd_mount thread err!");
    }
#endif
    return RT_EOK;
}
INIT_ENV_EXPORT(mount_init);

#if defined(RT_USING_WIFI_HOST_DRIVER) && defined(WHD_RESOURCES_IN_EXTERNAL_STORAGE_FS)
#ifdef BSP_USING_SDCARD_FS
static int _whd_fs_mount_init(void)
{
    rt_completion_init(&fs_mount_comp);
    return 0;
}
INIT_BOARD_EXPORT(_whd_fs_mount_init);

void whd_wait_fs_mount (void)
{
    rt_kprintf("Waiting for filesystem to mount...\n");
    rt_completion_wait(&fs_mount_comp, RT_WAITING_FOREVER);
}
#else
#error "Please define BSP_USING_SDCARD in your board config to enable sdcard mount feature."
#endif /* BSP_USING_SDCARD */
#endif /* RT_USING_WIFI_HOST_DRIVER */

#endif /* BSP_USING_FS */
