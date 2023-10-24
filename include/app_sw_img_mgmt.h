#ifndef _APP_SW_IMG_MGMT_H_
#define _APP_SW_IMG_MGMT_H_

#include "smp_internal.h"

#define DATA_CHUNK_SIZE 128

struct img_data
{
    int8_t image; // Should only be present, when "off" 0. Assumed 0
    int32_t len; // Optional length of an image. Must appear when "off" is 0
    int32_t off;
    uint8_t sha[16]; // Should only be present, when "off" 0. Optional        . Of full image
    uint8_t data[128];
    uint8_t upgrade; // Optional, sets that only upgrades allowed
};
volatile struct img_data img_data_obj = 
{
    .image = -1,
    .len = 0,
    .off = 0,
    .upgrade = -1,
    .sha = {0xff, 0xff, 0xff}
};

struct smp_state_of_images_rsp 
{
    uint32_t image;
    uint32_t slot;
    char version[25];
    uint8_t hash[32];
    bool bootable;
    bool pending;
    bool confirmed;
    bool active;
    bool permanent;
};
int smp_get_state_of_images(struct bt_dfu_smp *dfu_smp, struct smp_state_of_images_rsp *response);
int smp_set_state_of_images(struct bt_dfu_smp *dfu_smp, struct smp_state_of_images_rsp *response, uint8_t *hash, bool confirm);

struct smp_image_upload_rsp 
{
    char *echo_string;
};
int smp_image_upload(struct bt_dfu_smp *dfu_smp, struct smp_image_upload_rsp *response, struct img_data *data);

struct smp_image_erase_rsp 
{
    char *echo_string;
};
int smp_image_erase(struct bt_dfu_smp *dfu_smp, struct smp_image_erase_rsp *response, uint32_t slot);

union app_sw_img_rsp
{   
    struct smp_get_state_of_images_rsp get_state_of_images;
    struct smp_set_state_of_images_rsp set_state_of_images;
    struct smp_image_upload_rsp image_upload;
    struct smp_image_erase_rsp image_erase;
} full_app_sw_img_rsp;

#endif