#ifndef _DEFAULT_OS_MGMT_H_
#define _DEFAULT_OS_MGMT_H_


#include "default_os_mgmt.h"
#include "smp_internal.h"

void smp_echo(struct bt_dfu_smp *dfu_smp);
void smp_task_stats(struct bt_dfu_smp *dfu_smp);
void smp_mem_pool_stats(struct bt_dfu_smp *dfu_smp);
void smp_sys_reset(struct bt_dfu_smp *dfu_smp);
void smp_mcumgr_params(struct bt_dfu_smp *dfu_smp);
void smp_bootloader_info(struct bt_dfu_smp *dfu_smp);

#endif