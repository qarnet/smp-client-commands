#ifndef _DEFAULT_OS_MGMT_H_
#define _DEFAULT_OS_MGMT_H_


#include "default_os_mgmt.h"
#include "smp_internal.h"

struct smp_echo_rsp 
{
    char *echo_string;
};
int smp_echo(struct bt_dfu_smp *dfu_smp, struct smp_echo_rsp *response);

struct smp_task_stats_rsp 
{

};
int smp_task_stats(struct bt_dfu_smp *dfu_smp, struct smp_task_stats_rsp *response);

struct smp_mem_pool_stats_rsp 
{

};
int smp_mem_pool_stats(struct bt_dfu_smp *dfu_smp, struct smp_mem_pool_stats_rsp *response);

struct smp_sys_reset_rsp 
{

};
int smp_sys_reset(struct bt_dfu_smp *dfu_smp, struct smp_sys_reset_rsp *response);

struct smp_mcumgr_params_rsp 
{
    uint32_t buf_size;
    uint32_t buf_count;
};
int smp_mcumgr_params(struct bt_dfu_smp *dfu_smp, struct smp_mcumgr_params_rsp *response);

struct smp_info_rsp 
{

};
int smp_info(struct bt_dfu_smp *dfu_smp, struct smp_info_rsp *response);

struct smp_bootloader_info_rsp 
{
    char str_rsp[50];
};
int smp_bootloader_info(struct bt_dfu_smp *dfu_smp, struct smp_bootloader_info_rsp *response);


#endif