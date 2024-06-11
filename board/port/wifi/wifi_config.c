/*
 * Copyright (c) 2006-2023, Evlers Developers
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date         Author      Notes
 * 2024-01-08   Evlers      first implementation
 * 2024-06-11   Evlers      fix an exception caused by reading a nonexistent environment variable
 */
#include <stdio.h>
#include <stdlib.h>
#include <rtthread.h>

#ifdef RT_USING_WIFI

#include <wlan_mgnt.h>
#include <wlan_cfg.h>
#include <wlan_prot.h>

#include <easyflash.h>
#include <fal.h>


static int read_cfg(void *buff, int len)
{
    size_t saved_len = 0;

    ef_get_env_blob("wlan_cfg_info", buff, len, &saved_len);
    if (saved_len == 0)
    {
        return 0;
    }
    
    return len;
}

static int get_len(void)
{
    int len = 0;
    size_t saved_len = 0;

    ef_get_env_blob("wlan_cfg_len", &len, sizeof(len), &saved_len);
    if (saved_len == 0)
    {
        return 0;
    }

    return len;
}

static int write_cfg(void *buff, int len)
{
    /* set and store the wlan config lengths to Env */
    ef_set_env_blob("wlan_cfg_len", &len, sizeof(len));

    /* set and store the wlan config information to Env */
    ef_set_env_blob("wlan_cfg_info", buff, len);

    return len;
}

static const struct rt_wlan_cfg_ops ops =
{
    read_cfg,
    get_len,
    write_cfg
};

int wlan_autoconnect_init(void)
{
    rt_wlan_cfg_set_ops(&ops);
    rt_wlan_cfg_cache_refresh();

    return RT_EOK;
}
INIT_APP_EXPORT(wlan_autoconnect_init);

#endif /* RT_USING_WIFI */
