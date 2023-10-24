#include "default_os_mgmt.h"
#include "smp_commands_internal.h"

#define CBOR_ENCODER_STATE_NUM 2
#define CBOR_DECODER_STATE_NUM 3
#define CBOR_MAP_MAX_ELEMENT_CNT 2
#define SMP_ECHO_MAP_KEY_MAX_LEN 2
#define SMP_ECHO_MAP_VALUE_MAX_LEN 30
static struct smp_buffer smp_rsp_buff;

struct k_sem smp_sem;
K_SEM_DEFINE(smp_sem, 1, 1);

enum smp_mcumgr_params_key
{
    SMP_MCUMGR_PARAMS_BUF_SIZE,
    SMP_MCUMGR_PARAMS_BUF_COUNT,
};
char smp_mcumgr_params_keys[][25] = 
{
    {"buf_size"},
    {"buf_count"}
};

union full_response
{
	struct smp_echo_rsp echo;
	struct smp_task_stats_rsp task_stats;
	struct smp_mem_pool_stats_rsp mem_pool_stats;
	struct smp_sys_reset_rsp sys_reset;
	struct smp_mcumgr_params_rsp mcumgr_params;
	struct smp_info_rsp info;
	struct smp_bootloader_info_rsp bootloader_info;
} full_response_instance;

static void smp_rsp_proc(struct bt_dfu_smp *dfu_smp)
{
	uint8_t *p_outdata = (uint8_t *)(&smp_rsp_buff);
	const struct bt_dfu_smp_rsp_state *rsp_state;

	rsp_state = bt_dfu_smp_rsp_state(dfu_smp);
	printk("response part received, size: %zu.\n",
	       rsp_state->chunk_size);

	if (rsp_state->offset + rsp_state->chunk_size > sizeof(smp_rsp_buff)) {
		printk("Response size buffer overflow\n");
	} else {
		p_outdata += rsp_state->offset;
		memcpy(p_outdata,
		       rsp_state->data,
		       rsp_state->chunk_size);
	}

	if (!bt_dfu_smp_rsp_total_check(dfu_smp))
	{
		return;
	}


		
	// if (smp_rsp_buff.header.op != SMP_OID_WRITE_RESPONSE) {
	// 	printk("Unexpected operation code (%u)!\n",
	// 			smp_rsp_buff.header.op);
	// 	return;
	// }
	uint16_t group = ((uint16_t)smp_rsp_buff.header.group_h8) << 8 |
					smp_rsp_buff.header.group_l8;
	// if (group != SMP_GID_OS /* OS */) {
	// 	printk("Unexpected command group (%u)!\n", group);
	// 	return;
	// }
	switch(group)
	{
		case SMP_GID_OS: break;
		default: printk("Unexpected command group (%u)!\n", group); return;
	}

	if(smp_rsp_buff.header.id == SMP_CID_OS_ECHO)
	{
		size_t payload_len = ((uint16_t)smp_rsp_buff.header.len_h8) << 8 |
				      smp_rsp_buff.header.len_l8;

		zcbor_state_t zsd[CBOR_DECODER_STATE_NUM];
		struct zcbor_string value = {0};
		bool ok;

		printk("ECHO\n");

		zcbor_new_decode_state(zsd, ARRAY_SIZE(zsd), smp_rsp_buff.payload, payload_len, 1);

		/* Stop decoding on the error. */
		zsd->constant_state->stop_on_error = true;

		zcbor_map_start_decode(zsd);
		
		if(!zcbor_tstr_decode(zsd, &value))
		{
			printk("Decoding error (err: %d)\n", zcbor_pop_error(zsd));
			return;
		}
		char map_key[value.len];
		memcpy(map_key, value.value, value.len);

		if(!zcbor_tstr_decode(zsd, &value))
		{
			printk("Decoding error (err: %d)\n", zcbor_pop_error(zsd));
			return;
		}
		char map_value[value.len];
		memcpy(map_value, value.value, value.len);

		if(!zcbor_map_end_decode(zsd))
		{
			printk("Map end decode failed\n");
		}

		if (zcbor_check_error(zsd)) {
			/* Print textual representation of the received CBOR map. */
			printk("{_\"%s\": \"%s\"}\n", map_key, map_value);
		} else {
			printk("Cannot print received CBOR stream (err: %d)\n",
			       zcbor_pop_error(zsd));
		}
	}
	else if(smp_rsp_buff.header.id == SMP_CID_OS_CONSOLE)
	{
		size_t payload_len = ((uint16_t)smp_rsp_buff.header.len_h8) << 8 |
				      smp_rsp_buff.header.len_l8;

		printk("CONSOLE\n");

		zcbor_state_t zsd[CBOR_DECODER_STATE_NUM];
		struct zcbor_string value = {0};
		bool ok;

		zcbor_new_decode_state(zsd, ARRAY_SIZE(zsd), smp_rsp_buff.payload, payload_len, 1);

		/* Stop decoding on the error. */
		zsd->constant_state->stop_on_error = true;

		zcbor_map_start_decode(zsd);
		
		if(!zcbor_tstr_decode(zsd, &value))
		{
			printk("Decoding error (err: %d)\n", zcbor_pop_error(zsd));
			return;
		}
		char map_key[value.len];
		memcpy(map_key, value.value, value.len);

		if(!zcbor_tstr_decode(zsd, &value))
		{
			printk("Decoding error (err: %d)\n", zcbor_pop_error(zsd));
			return;
		}
		char map_value[value.len];
		memcpy(map_value, value.value, value.len);

		zcbor_map_end_decode(zsd);

		if (zcbor_check_error(zsd)) {
			/* Print textual representation of the received CBOR map. */
			printk("{_\"%s\": \"%s\"}\n", map_key, map_value);
		} else {
			printk("Cannot print received CBOR stream (err: %d)\n",
			       zcbor_pop_error(zsd));
		}
	}
	else if(smp_rsp_buff.header.id == SMP_CID_OS_TASK_STAT)
	{
		size_t payload_len = ((uint16_t)smp_rsp_buff.header.len_h8) << 8 |
				      smp_rsp_buff.header.len_l8;

		zcbor_state_t zsd[CBOR_DECODER_STATE_NUM];
		struct zcbor_string value = {0};
		bool ok;

		printk("TASK_STAT\n");

		zcbor_new_decode_state(zsd, ARRAY_SIZE(zsd), smp_rsp_buff.payload, payload_len, 1);

		/* Stop decoding on the error. */
		zsd->constant_state->stop_on_error = true;

		zcbor_tstr_decode(zsd, &value);

		char test[value.len];
		test[value.len] = '\0';

		printk("%s\n", test);
		printk("%s\n", value.value);

		zcbor_map_start_decode(zsd);



		zcbor_map_end_decode(zsd);

		if (zcbor_check_error(zsd)) {
			/* Print textual representation of the received CBOR map. */
		} else {
			printk("Cannot print received CBOR stream (err: %d)\n",
			       zcbor_pop_error(zsd));
		}
	}
	else if(smp_rsp_buff.header.id == SMP_CID_OS_MEM_POOL_STAT)
	{
		size_t payload_len = ((uint16_t)smp_rsp_buff.header.len_h8) << 8 |
				      smp_rsp_buff.header.len_l8;

		zcbor_state_t zsd[CBOR_DECODER_STATE_NUM];
		struct zcbor_string value = {0};
		bool ok;

		printk("MEM_POOP_STAT\n");

		zcbor_new_decode_state(zsd, ARRAY_SIZE(zsd), smp_rsp_buff.payload, payload_len, 1);

		/* Stop decoding on the error. */
		zsd->constant_state->stop_on_error = true;

		zcbor_list_start_decode(zsd);

		zcbor_tstr_decode(zsd, &value);

		printk("Size: %d\n", value.len);

		zcbor_list_end_decode(zsd);

		if (zcbor_check_error(zsd)) {
			/* Print textual representation of the received CBOR map. */
		} else {
			printk("Cannot print received CBOR stream (err: %d)\n",
			       zcbor_pop_error(zsd));
		}
	}
	else if(smp_rsp_buff.header.id == SMP_CID_OS_RESET)
	{
		 size_t payload_len = ((uint16_t)smp_rsp_buff.header.len_h8) << 8 |
				      smp_rsp_buff.header.len_l8;

		zcbor_state_t zsd[CBOR_DECODER_STATE_NUM];
		struct zcbor_string value = {0};
		bool ok;

		printk("RESET\n");

		zcbor_new_decode_state(zsd, ARRAY_SIZE(zsd), smp_rsp_buff.payload, payload_len, 1);

		/* Stop decoding on the error. */
		zsd->constant_state->stop_on_error = true;

		zcbor_map_start_decode(zsd);
		// Should be empty
		zcbor_map_end_decode(zsd);

		if (zcbor_check_error(zsd)) {
			/* Print textual representation of the received CBOR map. */
		} else {
			printk("Cannot print received CBOR stream (err: %d)\n",
			       zcbor_pop_error(zsd));
		}
	}
	else if(smp_rsp_buff.header.id == SMP_CID_OS_MCUMGR_PARAMS)
	{
		size_t payload_len = ((uint16_t)smp_rsp_buff.header.len_h8) << 8 |
				      smp_rsp_buff.header.len_l8;

		zcbor_state_t zsd[CBOR_DECODER_STATE_NUM];
		struct zcbor_string value = {0};
		bool ok;

		printk("MCUMGR_PARAMS\n");

		zcbor_new_decode_state(zsd, ARRAY_SIZE(zsd), smp_rsp_buff.payload, payload_len, 2);

		/* Stop decoding on the error. */
		zsd->constant_state->stop_on_error = true;

		zcbor_map_start_decode(zsd);
	
		while(!zcbor_tstr_decode(zsd, &value))
		{
			for(int i = 0; i < ARRAY_SIZE(smp_mcumgr_params_keys); i++)
			{
				if(0 == strncmp(value.value, smp_mcumgr_params_keys[SMP_MCUMGR_PARAMS_BUF_COUNT], value.len))
				{
					zcbor_uint32_decode(zsd, &full_response_instance.mcumgr_params.buf_count);
				}
				if(0 == strncmp(value.value, smp_mcumgr_params_keys[SMP_MCUMGR_PARAMS_BUF_SIZE], value.len))
				{
					zcbor_uint32_decode(zsd, &full_response_instance.mcumgr_params.buf_size);
				}
			}
		}

		zcbor_map_end_decode(zsd);

		if (zcbor_check_error(zsd)) {
			/* Print textual representation of the received CBOR map. */
		} else {
			printk("Cannot print received CBOR stream (err: %d)\n",
			       zcbor_pop_error(zsd));
		}
	}
	else if(smp_rsp_buff.header.id == SMP_CID_OS_INFO)
	{
		size_t payload_len = ((uint16_t)smp_rsp_buff.header.len_h8) << 8 |
				      smp_rsp_buff.header.len_l8;

		zcbor_state_t zsd[CBOR_DECODER_STATE_NUM];
		struct zcbor_string value = {0};
		bool ok;

		printk("INFO\n");

		zcbor_new_decode_state(zsd, ARRAY_SIZE(zsd), smp_rsp_buff.payload, payload_len, 1);

		/* Stop decoding on the error. */
		zsd->constant_state->stop_on_error = true;

		zcbor_map_start_decode(zsd);

		zcbor_map_end_decode(zsd);

		if (zcbor_check_error(zsd)) {
			/* Print textual representation of the received CBOR map. */
		} else {
			printk("Cannot print received CBOR stream (err: %d)\n",
			       zcbor_pop_error(zsd));
		}
	}
	else if(smp_rsp_buff.header.id == SMP_CID_OS_INFO)
	{
		size_t payload_len = ((uint16_t)smp_rsp_buff.header.len_h8) << 8 |
				      smp_rsp_buff.header.len_l8;

		zcbor_state_t zsd[CBOR_DECODER_STATE_NUM];
		struct zcbor_string value = {0};
		bool ok;

		printk("INFO\n");

		zcbor_new_decode_state(zsd, ARRAY_SIZE(zsd), smp_rsp_buff.payload, payload_len, 1);

		/* Stop decoding on the error. */
		zsd->constant_state->stop_on_error = true;

		zcbor_map_start_decode(zsd);

		zcbor_map_end_decode(zsd);

		if (zcbor_check_error(zsd)) {
			/* Print textual representation of the received CBOR map. */
		} else {
			printk("Cannot print received CBOR stream (err: %d)\n",
			       zcbor_pop_error(zsd));
		}
	}
	
	
	
	
	
	else if(smp_rsp_buff.header.id == SMP_CID_OS_BOOTLOADER_INFO)
	{
		size_t payload_len = ((uint16_t)smp_rsp_buff.header.len_h8) << 8 |
				      smp_rsp_buff.header.len_l8;

		zcbor_state_t zsd[CBOR_DECODER_STATE_NUM];
		struct zcbor_string value = {0};
		bool ok;

		printk("TASK_STAT\n");

		zcbor_new_decode_state(zsd, ARRAY_SIZE(zsd), smp_rsp_buff.payload, payload_len, 1);

		/* Stop decoding on the error. */
		zsd->constant_state->stop_on_error = true;

		zcbor_tstr_decode(zsd, &value);

		char test[value.len];
		test[value.len] = '\0';

		printk("%s\n", test);
		printk("%s\n", value.value);

		zcbor_map_start_decode(zsd);



		zcbor_map_end_decode(zsd);

		if (zcbor_check_error(zsd)) {
			/* Print textual representation of the received CBOR map. */
		} else {
			printk("Cannot print received CBOR stream (err: %d)\n",
			       zcbor_pop_error(zsd));
		}
	}
	else
	{
		printk("ELSE\n");
	}

	k_sem_give(&smp_sem);
}

int smp_echo(struct bt_dfu_smp *dfu_smp, struct smp_echo_rsp *response)
{
	static struct smp_buffer smp_cmd;
	zcbor_state_t zse[CBOR_ENCODER_STATE_NUM];
	size_t payload_len;

	k_sem_take(&smp_sem, K_SECONDS(1));

	zcbor_new_encode_state(zse, ARRAY_SIZE(zse), smp_cmd.payload,
			       sizeof(smp_cmd.payload), 0);

	/* Stop encoding on the error. */
	zse->constant_state->stop_on_error = true;

	zcbor_map_start_encode(zse, CBOR_MAP_MAX_ELEMENT_CNT);
	zcbor_tstr_put_lit(zse, "d");
	zcbor_tstr_put_term(zse, "string");
	zcbor_map_end_encode(zse, CBOR_MAP_MAX_ELEMENT_CNT);

	if (!zcbor_check_error(zse)) {
		printk("Failed to encode SMP echo packet, err: %d\n", zcbor_pop_error(zse));
		return -EFAULT;
	}

	payload_len = (size_t)(zse->payload - smp_cmd.payload);

	smp_cmd.header.op = SMP_OID_WRITE;
	smp_cmd.header.flags = 0;
	smp_cmd.header.len_h8 = (uint8_t)((payload_len >> 8) & 0xFF);
	smp_cmd.header.len_l8 = (uint8_t)((payload_len >> 0) & 0xFF);
	smp_cmd.header.group_h8 = 0;
	smp_cmd.header.group_l8 = SMP_GID_OS;
	smp_cmd.header.seq = 0;
	smp_cmd.header.id  = SMP_CID_OS_ECHO;

	return bt_dfu_smp_command(dfu_smp, smp_rsp_proc,
				  sizeof(smp_cmd.header) + payload_len,
				  &smp_cmd);
}

int smp_task_stats(struct bt_dfu_smp *dfu_smp, struct smp_task_stats_rsp *response)
{
	static struct smp_buffer smp_cmd;
	zcbor_state_t zse[CBOR_ENCODER_STATE_NUM];
	size_t payload_len;

	k_sem_take(&smp_sem, K_SECONDS(1));

	zcbor_new_encode_state(zse, ARRAY_SIZE(zse), smp_cmd.payload,
			       sizeof(smp_cmd.payload), 0);

	/* Stop encoding on the error. */
	zse->constant_state->stop_on_error = true;

	zcbor_map_start_encode(zse, 0);
	zcbor_map_end_encode(zse, 0);

	if (!zcbor_check_error(zse)) {
		printk("Failed to encode SMP echo packet, err: %d\n", zcbor_pop_error(zse));
		return -EFAULT;
	}

	payload_len = (size_t)(zse->payload - smp_cmd.payload);

	smp_cmd.header.op = SMP_OID_READ;
	smp_cmd.header.flags = 0;
	smp_cmd.header.len_h8 = (uint8_t)((payload_len >> 8) & 0xFF);
	smp_cmd.header.len_l8 = (uint8_t)((payload_len >> 0) & 0xFF);
	smp_cmd.header.group_h8 = 0;
	smp_cmd.header.group_l8 = SMP_GID_OS;
	smp_cmd.header.seq = 0;
	smp_cmd.header.id  = SMP_CID_OS_TASK_STAT;

	return bt_dfu_smp_command(dfu_smp, smp_rsp_proc,
				  sizeof(smp_cmd.header) + payload_len,
				  &smp_cmd);
}

int smp_mem_pool_stats(struct bt_dfu_smp *dfu_smp, struct smp_mem_pool_stats_rsp *response)
{
	static struct smp_buffer smp_cmd;
	zcbor_state_t zse[CBOR_ENCODER_STATE_NUM];
	size_t payload_len;

	k_sem_take(&smp_sem, K_SECONDS(1));

	zcbor_new_encode_state(zse, ARRAY_SIZE(zse), smp_cmd.payload,
			       sizeof(smp_cmd.payload), 0);

	/* Stop encoding on the error. */
	zse->constant_state->stop_on_error = true;

	zcbor_map_start_encode(zse, 0);
	zcbor_map_end_encode(zse, 0);

	if (!zcbor_check_error(zse)) {
		printk("Failed to encode SMP echo packet, err: %d\n", zcbor_pop_error(zse));
		return -EFAULT;
	}

	payload_len = (size_t)(zse->payload - smp_cmd.payload);

	smp_cmd.header.op = SMP_OID_READ;
	smp_cmd.header.flags = 0;
	smp_cmd.header.len_h8 = (uint8_t)((payload_len >> 8) & 0xFF);
	smp_cmd.header.len_l8 = (uint8_t)((payload_len >> 0) & 0xFF);
	smp_cmd.header.group_h8 = 0;
	smp_cmd.header.group_l8 = SMP_GID_OS;
	smp_cmd.header.seq = 0;
	smp_cmd.header.id  = SMP_CID_OS_MEM_POOL_STAT;

	return bt_dfu_smp_command(dfu_smp, smp_rsp_proc,
				  sizeof(smp_cmd.header) + payload_len,
				  &smp_cmd);
}

int smp_sys_reset(struct bt_dfu_smp *dfu_smp, struct smp_sys_reset_rsp *response)
{
	static struct smp_buffer smp_cmd;
	zcbor_state_t zse[CBOR_ENCODER_STATE_NUM];
	size_t payload_len;

	k_sem_take(&smp_sem, K_SECONDS(1));

	zcbor_new_encode_state(zse, ARRAY_SIZE(zse), smp_cmd.payload,
			       sizeof(smp_cmd.payload), 0);

	/* Stop encoding on the error. */
	zse->constant_state->stop_on_error = true;

	zcbor_map_start_encode(zse, 0);
	zcbor_map_end_encode(zse, 0);

	if (!zcbor_check_error(zse)) {
		printk("Failed to encode SMP echo packet, err: %d\n", zcbor_pop_error(zse));
		return -EFAULT;
	}

	payload_len = (size_t)(zse->payload - smp_cmd.payload);

	smp_cmd.header.op = SMP_OID_WRITE;
	smp_cmd.header.flags = 0;
	smp_cmd.header.len_h8 = (uint8_t)((payload_len >> 8) & 0xFF);
	smp_cmd.header.len_l8 = (uint8_t)((payload_len >> 0) & 0xFF);
	smp_cmd.header.group_h8 = 0;
	smp_cmd.header.group_l8 = SMP_GID_OS;
	smp_cmd.header.seq = 0;
	smp_cmd.header.id  = SMP_CID_OS_RESET;

	return bt_dfu_smp_command(dfu_smp, smp_rsp_proc,
				  sizeof(smp_cmd.header) + payload_len,
				  &smp_cmd);
}

int smp_mcumgr_params(struct bt_dfu_smp *dfu_smp, struct smp_mcumgr_params_rsp *response)
{
	static struct smp_buffer smp_cmd;
	zcbor_state_t zse[CBOR_ENCODER_STATE_NUM];
	size_t payload_len;

	k_sem_take(&smp_sem, K_SECONDS(1));

	zcbor_new_encode_state(zse, ARRAY_SIZE(zse), smp_cmd.payload,
			       sizeof(smp_cmd.payload), 0);

	/* Stop encoding on the error. */
	zse->constant_state->stop_on_error = true;

	zcbor_map_start_encode(zse, 0);
	zcbor_map_end_encode(zse, 0);

	if (!zcbor_check_error(zse)) {
		printk("Failed to encode SMP echo packet, err: %d\n", zcbor_pop_error(zse));
		return -EFAULT;
	}

	payload_len = (size_t)(zse->payload - smp_cmd.payload);

	smp_cmd.header.op = SMP_OID_READ;
	smp_cmd.header.flags = 0;
	smp_cmd.header.len_h8 = (uint8_t)((payload_len >> 8) & 0xFF);
	smp_cmd.header.len_l8 = (uint8_t)((payload_len >> 0) & 0xFF);
	smp_cmd.header.group_h8 = 0;
	smp_cmd.header.group_l8 = SMP_GID_OS;
	smp_cmd.header.seq = 0;
	smp_cmd.header.id  = SMP_CID_OS_MCUMGR_PARAMS;

	return bt_dfu_smp_command(dfu_smp, smp_rsp_proc,
				  sizeof(smp_cmd.header) + payload_len,
				  &smp_cmd);
}

int smp_bootloader_info(struct bt_dfu_smp *dfu_smp, struct smp_bootloader_info_rsp *response)
{
	static struct smp_buffer smp_cmd;
	zcbor_state_t zse[CBOR_ENCODER_STATE_NUM];
	size_t payload_len;

	k_sem_take(&smp_sem, K_SECONDS(1));

	zcbor_new_encode_state(zse, ARRAY_SIZE(zse), smp_cmd.payload,
			       sizeof(smp_cmd.payload), 0);

	/* Stop encoding on the error. */
	zse->constant_state->stop_on_error = true;

	zcbor_map_start_encode(zse, CBOR_MAP_MAX_ELEMENT_CNT);
	zcbor_tstr_put_lit(zse, "format");
	zcbor_tstr_put_term(zse, "a");
	zcbor_map_end_encode(zse, CBOR_MAP_MAX_ELEMENT_CNT);

	if (!zcbor_check_error(zse)) {
		printk("Failed to encode SMP echo packet, err: %d\n", zcbor_pop_error(zse));
		return -EFAULT;
	}

	payload_len = (size_t)(zse->payload - smp_cmd.payload);

	smp_cmd.header.op = SMP_OID_READ;
	smp_cmd.header.flags = 0;
	smp_cmd.header.len_h8 = (uint8_t)((payload_len >> 8) & 0xFF);
	smp_cmd.header.len_l8 = (uint8_t)((payload_len >> 0) & 0xFF);
	smp_cmd.header.group_h8 = 0;
	smp_cmd.header.group_l8 = SMP_GID_OS;
	smp_cmd.header.seq = 0;
	smp_cmd.header.id  = SMP_CID_OS_INFO;

	return bt_dfu_smp_command(dfu_smp, smp_rsp_proc,
				  sizeof(smp_cmd.header) + payload_len,
				  &smp_cmd);
}