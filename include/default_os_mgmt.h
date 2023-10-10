#ifndef _DEFAULT_OS_MGMT_H_
#define _DEFAULT_OS_MGMT_H_


#include "default_os_mgmt.h"
#include "smp_internal.h"

int smp_echo(struct bt_dfu_smp *dfu_smp);
int smp_task_stats(struct bt_dfu_smp *dfu_smp);
int smp_mem_pool_stats(struct bt_dfu_smp *dfu_smp);
int smp_sys_reset(struct bt_dfu_smp *dfu_smp);
int smp_mcumgr_params(struct bt_dfu_smp *dfu_smp);
int smp_bootloader_info(struct bt_dfu_smp *dfu_smp);

#endif