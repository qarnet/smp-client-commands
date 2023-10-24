#include "app_sw_img_mgmt.h"
#include "smp_commands_internal.h"

#define CBOR_ENCODER_STATE_NUM 2
#define CBOR_DECODER_STATE_NUM 3
#define CBOR_MAP_MAX_ELEMENT_CNT 2
#define SMP_ECHO_MAP_KEY_MAX_LEN 2
#define SMP_ECHO_MAP_VALUE_MAX_LEN 30
static struct smp_buffer smp_rsp_buff;

static K_SEM_DEFINE(smp_sem, 1, 1);

// zcbor bool decode will trigger error, so zsd->constant_state->stop_on_error needs to be set to false

void exit_function()
{
    img_data_obj.image = -1;
    img_data_obj.len = 0;
    img_data_obj.off = 0;
    img_data_obj.upgrade = -1;
    img_data_obj.sha[0] = 0xff;
    img_data_obj.sha[1] = 0xff;
    img_data_obj.sha[2] = 0xff;
}

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
		return exit_function();
	}


		
	// if (smp_rsp_buff.header.op != SMP_OID_WRITE_RESPONSE) {
	// 	printk("Unexpected operation code (%u)!\n",
	// 			smp_rsp_buff.header.op);
	// 	return exit_function();
	// }
	uint16_t group = ((uint16_t)smp_rsp_buff.header.group_h8) << 8 |
					smp_rsp_buff.header.group_l8;
	// if (group != SMP_GID_OS /* OS */) {
	// 	printk("Unexpected command group (%u)!\n", group);
	// 	return exit_function();
	// }
	switch(group)
	{
		case SMP_GID_OS: break;
		default: printk("Unexpected command group (%u)!\n", group); return exit_function();
	}

	if(smp_rsp_buff.header.id == SMP_CID_APP_IMG_UPLOAD)
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
			return exit_function();
		}
		char map_key[value.len];
		memcpy(map_key, value.value, value.len);

		if(!zcbor_tstr_decode(zsd, &value))
		{
			printk("Decoding error (err: %d)\n", zcbor_pop_error(zsd));
			return exit_function();
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
	else if(smp_rsp_buff.header.id == SMP_CID_APP_IMG_STATE)
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
			return exit_function();
		}
		char map_key[value.len];
		memcpy(map_key, value.value, value.len);

		if(!zcbor_tstr_decode(zsd, &value))
		{
			printk("Decoding error (err: %d)\n", zcbor_pop_error(zsd));
			return exit_function();
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
	else if(smp_rsp_buff.header.id == SMP_CID_APP_IMG_ERASE)
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
	else
	{
		printk("ELSE\n");
	}

	k_sem_give(&smp_sem);
}

int smp_get_state_of_images(struct bt_dfu_smp *dfu_smp, struct smp_state_of_images_rsp *response)
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
	smp_cmd.header.group_l8 = SMP_GID_APP_SOFTWARE_IMG;
	smp_cmd.header.seq = 0;
	smp_cmd.header.id  = SMP_CID_APP_IMG_STATE;

	return bt_dfu_smp_command(dfu_smp, smp_rsp_proc,
				  sizeof(smp_cmd.header) + payload_len,
				  &smp_cmd);
}

int smp_set_state_of_images(struct bt_dfu_smp *dfu_smp, struct smp_state_of_images_rsp *response, uint8_t *hash, bool confirm)
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
	if(NULL != hash)
	{
		char hash_cpy[32];
		memcpy(hash_cpy, hash, sizeof(hash_cpy));
		zcbor_tstr_put_term(zse, "hash");
		zcbor_bstr_put_arr(zse, hash_cpy);
	}
	zcbor_tstr_put_term(zse, "confirm");
	zcbor_bool_put(zse, confirm);
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
	smp_cmd.header.group_l8 = SMP_GID_APP_SOFTWARE_IMG;
	smp_cmd.header.seq = 0;
	smp_cmd.header.id  = SMP_CID_APP_IMG_STATE;

	return bt_dfu_smp_command(dfu_smp, smp_rsp_proc,
				  sizeof(smp_cmd.header) + payload_len,
				  &smp_cmd);
}

int smp_image_upload(struct bt_dfu_smp *dfu_smp, struct smp_image_upload_rsp *response, struct img_data *data)
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
    if((data->image > -1) && (data->image < 10) && (data->off == 0))
    {
        zcbor_tstr_put_term(zse, "image");
        zcbor_uint32_put(zse, data->image);
    }
    if(data->off == 0)
    {
        zcbor_tstr_put_term(zse, "len");
        zcbor_uint32_put(zse, data->len);
    }
    zcbor_tstr_put_term(zse, "off");
    zcbor_uint32_put(zse, data->off);
    if((data->sha[0] != 0xff) && (data->sha[1] != 0xff) && (data->sha[2] != 0xff) && (data->off == 0))
    {
        zcbor_tstr_put_term(zse, "sha");
	    zcbor_bstr_put_arr(zse, data->sha);
    }
    zcbor_tstr_put_term(zse, "data");
    zcbor_bstr_put_arr(zse, data->data);
    if((data->upgrade > -1) && (data->upgrade <= 1) && (data->off == 0))
    {
        zcbor_tstr_put_term(zse, "upgrade");
        zcbor_bool_put(zse, (bool)data->upgrade);
    }

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
	smp_cmd.header.group_l8 = SMP_GID_APP_SOFTWARE_IMG;
	smp_cmd.header.seq = 0;
	smp_cmd.header.id  = SMP_CID_APP_IMG_UPLOAD;

	return bt_dfu_smp_command(dfu_smp, smp_rsp_proc,
				  sizeof(smp_cmd.header) + payload_len,
				  &smp_cmd);
}

int smp_image_erase(struct bt_dfu_smp *dfu_smp, struct smp_image_erase_rsp *response, uint32_t slot)
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
	if(slot >= 0 && slot <= 1)
	{
		zcbor_tstr_put_lit(zse, "slot");
		zcbor_uint32_put(zse, slot);
	}
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
	smp_cmd.header.group_l8 = SMP_GID_APP_SOFTWARE_IMG;
	smp_cmd.header.seq = 0;
	smp_cmd.header.id  = SMP_CID_APP_IMG_ERASE;

	return bt_dfu_smp_command(dfu_smp, smp_rsp_proc,
				  sizeof(smp_cmd.header) + payload_len,
				  &smp_cmd);
}
