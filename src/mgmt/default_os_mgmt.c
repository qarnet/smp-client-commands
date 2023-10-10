#include "default_os_mgmt.h"
#include "smp_commands_internal.h"

#define CBOR_ENCODER_STATE_NUM 2
#define CBOR_DECODER_STATE_NUM 3
#define CBOR_MAP_MAX_ELEMENT_CNT 2
#define SMP_ECHO_MAP_KEY_MAX_LEN 2
#define SMP_ECHO_MAP_VALUE_MAX_LEN 30
static struct smp_buffer smp_rsp_buff;

static void smp_rsp_proc(struct bt_dfu_smp *dfu_smp)
{
	uint8_t *p_outdata = (uint8_t *)(&smp_rsp_buff);
	const struct bt_dfu_smp_rsp_state *rsp_state;

	rsp_state = bt_dfu_smp_rsp_state(dfu_smp);
	printk("Echo response part received, size: %zu.\n",
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
		printk("Total response not received\n");
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


	if(smp_rsp_buff.header.id == SMP_CID_OS_CONSOLE)
	{

	}
	else if(smp_rsp_buff.header.id == SMP_CID_OS_ECHO)
	{
		printk("Total response received - decoding\n");
		if (smp_rsp_buff.header.op != SMP_OID_WRITE_RESPONSE) {
			printk("Unexpected operation code (%u)!\n",
			       smp_rsp_buff.header.op);
			return;
		}
		uint16_t group = ((uint16_t)smp_rsp_buff.header.group_h8) << 8 |
				      smp_rsp_buff.header.group_l8;
		if (group != SMP_GID_OS /* OS */) {
			printk("Unexpected command group (%u)!\n", group);
			return;
		}
		if (smp_rsp_buff.header.id != SMP_CID_OS_ECHO /* ECHO */) {
			printk("Unexpected command (%u)",
			       smp_rsp_buff.header.id);
			return;
		}
		size_t payload_len = ((uint16_t)smp_rsp_buff.header.len_h8) << 8 |
				      smp_rsp_buff.header.len_l8;

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
	else if(smp_rsp_buff.header.id == SMP_CID_OS_INFO)
	{
		
	}
	else if(smp_rsp_buff.header.id == SMP_CID_OS_MCUMGR_PARAMS)
	{
		
	}
	else if(smp_rsp_buff.header.id == SMP_CID_OS_MEM_POOL_STAT)
	{
		
	}
	else if(smp_rsp_buff.header.id == SMP_CID_OS_RESET)
	{
		
	}
	else if(smp_rsp_buff.header.id == SMP_CID_OS_TASK_STAT)
	{
		
	}

}

static void smp_echo_rsp_proc(struct bt_dfu_smp *dfu_smp)
{
	uint8_t *p_outdata = (uint8_t *)(&smp_rsp_buff);
	const struct bt_dfu_smp_rsp_state *rsp_state;

	rsp_state = bt_dfu_smp_rsp_state(dfu_smp);
	printk("Echo response part received, size: %zu.\n",
	       rsp_state->chunk_size);

	if (rsp_state->offset + rsp_state->chunk_size > sizeof(smp_rsp_buff)) {
		printk("Response size buffer overflow\n");
	} else {
		p_outdata += rsp_state->offset;
		memcpy(p_outdata,
		       rsp_state->data,
		       rsp_state->chunk_size);
	}

	if (bt_dfu_smp_rsp_total_check(dfu_smp)) {
		printk("Total response received - decoding\n");
		if (smp_rsp_buff.header.op != SMP_OID_WRITE_RESPONSE) {
			printk("Unexpected operation code (%u)!\n",
			       smp_rsp_buff.header.op);
			return;
		}
		uint16_t group = ((uint16_t)smp_rsp_buff.header.group_h8) << 8 |
				      smp_rsp_buff.header.group_l8;
		if (group != SMP_GID_OS /* OS */) {
			printk("Unexpected command group (%u)!\n", group);
			return;
		}
		if (smp_rsp_buff.header.id != SMP_CID_OS_ECHO /* ECHO */) {
			printk("Unexpected command (%u)",
			       smp_rsp_buff.header.id);
			return;
		}
		size_t payload_len = ((uint16_t)smp_rsp_buff.header.len_h8) << 8 |
				      smp_rsp_buff.header.len_l8;

		zcbor_state_t zsd[CBOR_DECODER_STATE_NUM];
		struct zcbor_string value = {0};
		char map_key[SMP_ECHO_MAP_KEY_MAX_LEN];
		char map_value[SMP_ECHO_MAP_VALUE_MAX_LEN];
		bool ok;

		zcbor_new_decode_state(zsd, ARRAY_SIZE(zsd), smp_rsp_buff.payload, payload_len, 1);

		/* Stop decoding on the error. */
		zsd->constant_state->stop_on_error = true;

		zcbor_map_start_decode(zsd);
		
		ok = zcbor_tstr_decode(zsd, &value);

		if (!ok) {
			printk("Decoding error (err: %d)\n", zcbor_pop_error(zsd));
			return;
		} else if ((value.len != 1) || (*value.value != 'r')) {
			printk("Invalid data received.\n");
			return;
		} else {
			/* Do nothing */
		}

		map_key[0] = value.value[0];

		/* Add string NULL terminator */
		map_key[1] = '\0';

		ok = zcbor_tstr_decode(zsd, &value);

		if (!ok) {
			printk("Decoding error (err: %d)\n", zcbor_pop_error(zsd));
			return;
		} else if (value.len > (sizeof(map_value) - 1)) {
			printk("To small buffer for received data.\n");
			return;
		} else {
			/* Do nothing */
		}

		memcpy(map_value, value.value, value.len);

		/* Add string NULL terminator */
		map_value[value.len] = '\0';

		zcbor_map_end_decode(zsd);

		if (zcbor_check_error(zsd)) {
			/* Print textual representation of the received CBOR map. */
			printk("{_\"%s\": \"%s\"}\n", map_key, map_value);
		} else {
			printk("Cannot print received CBOR stream (err: %d)\n",
			       zcbor_pop_error(zsd));
		}
	}
}

int smp_echo(struct bt_dfu_smp *dfu_smp)
{
	static struct smp_buffer smp_cmd;
	zcbor_state_t zse[CBOR_ENCODER_STATE_NUM];
	size_t payload_len;

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

	return bt_dfu_smp_command(dfu_smp, smp_echo_rsp_proc,
				  sizeof(smp_cmd.header) + payload_len,
				  &smp_cmd);
}

int smp_task_stats(struct bt_dfu_smp *dfu_smp)
{
	static struct smp_buffer smp_cmd;
	zcbor_state_t zse[CBOR_ENCODER_STATE_NUM];
	size_t payload_len;

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

	return bt_dfu_smp_command(dfu_smp, smp_echo_rsp_proc,
				  sizeof(smp_cmd.header) + payload_len,
				  &smp_cmd);
}

int smp_mem_pool_stats(struct bt_dfu_smp *dfu_smp)
{
	static struct smp_buffer smp_cmd;
	zcbor_state_t zse[CBOR_ENCODER_STATE_NUM];
	size_t payload_len;

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

	return bt_dfu_smp_command(dfu_smp, smp_echo_rsp_proc,
				  sizeof(smp_cmd.header) + payload_len,
				  &smp_cmd);
}

int smp_sys_reset(struct bt_dfu_smp *dfu_smp)
{
	static struct smp_buffer smp_cmd;
	zcbor_state_t zse[CBOR_ENCODER_STATE_NUM];
	size_t payload_len;

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

	return bt_dfu_smp_command(dfu_smp, smp_echo_rsp_proc,
				  sizeof(smp_cmd.header) + payload_len,
				  &smp_cmd);
}

int smp_mcumgr_params(struct bt_dfu_smp *dfu_smp)
{
	static struct smp_buffer smp_cmd;
	zcbor_state_t zse[CBOR_ENCODER_STATE_NUM];
	size_t payload_len;

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

	return bt_dfu_smp_command(dfu_smp, smp_echo_rsp_proc,
				  sizeof(smp_cmd.header) + payload_len,
				  &smp_cmd);
}

int smp_bootloader_info(struct bt_dfu_smp *dfu_smp)
{
	static struct smp_buffer smp_cmd;
	zcbor_state_t zse[CBOR_ENCODER_STATE_NUM];
	size_t payload_len;

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

	return bt_dfu_smp_command(dfu_smp, smp_echo_rsp_proc,
				  sizeof(smp_cmd.header) + payload_len,
				  &smp_cmd);
}