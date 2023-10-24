#ifndef _SMP_COMMANDS_INTERNAL_H_
#define _SMP_COMMANDS_INTERNAL_H_

typedef enum
{
    SMP_OID_READ = 0,
    SMP_OID_READ_RESPONSE = 1,
    SMP_OID_WRITE = 2,
    SMP_OID_WRITE_RESPONSE = 3
} smp_op_id_t;

typedef enum 
{
    SMP_GID_OS = 0,
    SMP_GID_APP_SOFTWARE_IMG = 1,
    SMP_GID_STATS = 2,
    SMP_GID_SETTINGS = 3,
    SMP_GID_APP_SYS_LOG = 4,
    SMP_GID_RUN_TIME_TESTS = 5,
    SMP_GID_SPLIT_IMG = 6,
    SMP_GID_TEST_CRASH = 7,
    SMP_GID_FILE = 8,
    SMP_GID_SHELL = 9,
    SMP_GID_ZEPHYR = 63,
    SMP_GID_BASE = 64
} smp_management_group_id_t;

typedef enum
{
    SMP_CID_OS_ECHO = 0,
    SMP_CID_OS_CONSOLE = 1,
    SMP_CID_OS_TASK_STAT = 2,
    SMP_CID_OS_MEM_POOL_STAT = 3,
    SMP_CID_OS_DATE_TIME = 4,
    SMP_CID_OS_RESET = 5,
    SMP_CID_OS_MCUMGR_PARAMS = 6,
    SMP_CID_OS_INFO = 7,
    SMP_CID_OS_BOOTLOADER_INFO = 8
} smp_os_command_id_t;

typedef enum
{
    SMP_CID_APP_IMG_STATE = 0,
    SMP_CID_APP_IMG_UPLOAD = 1,
    SMP_CID_APP_IMG_FILE = 2,
    SMP_CID_APP_IMG_CORELIST = 3,
    SMP_CID_APP_IMG_CORELOAD = 4,
    SMP_CID_APP_IMG_ERASE = 5
} smp_app_image_command_id_t;

typedef enum
{
    SMP_CID_STATS_GROUP_DATA = 0,
    SMP_CID_STATS_LIST_GROUPS = 1
} smp_stat_command_id_t;

typedef enum
{
    SMP_CID_SETTINGS_READ_WRITE = 0,
    SMP_CID_SETTINGS_DELETE = 1,
    SMP_CID_SETTINGS_COMMIT = 2,
    SMP_CID_SETTINGS_LOAD_SAVE = 3    
} smp_settings_command_id_t;

typedef enum
{
    SMP_CID_FILE_DOWNLOAD = 0,
    SMP_CID_FILE_STATUS = 1,
    SMP_CID_FILE_HASH_CHCKSUM = 2,
    SMP_CID_FILE_SUPPORTED_FILE_HASH_CHKSUM = 3,
    SMP_CID_FILE_CLOSE = 4
} smp_file_command_id_t;

typedef enum
{
    SMP_CID_SHELL_EXECUTE = 0
} smp_shell_command_id_t;

#endif