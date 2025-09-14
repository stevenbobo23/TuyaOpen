/**
 * @file tdl_camera_driver.h
 * @version 0.1
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 */

#ifndef __TDL_CAMERA_DRIVER_H__
#define __TDL_CAMERA_DRIVER_H__

#include "tuya_cloud_types.h"
#include "tdl_camera_manage.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/
#define CAMERA_DEV_NAME_MAX_LEN 32

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef enum {
	TUYA_CAMERA_PPI_DEFAULT     = 0,
	TUYA_CAMERA_PPI_128X128     = (128 << 16) | 128,
	TUYA_CAMERA_PPI_170X320     = (170 << 16) | 320,
	TUYA_CAMERA_PPI_320X240     = (320 << 16) | 240,
	TUYA_CAMERA_PPI_320X480     = (320 << 16) | 480,
	TUYA_CAMERA_PPI_360X480     = (360 << 16) | 480,
	TUYA_CAMERA_PPI_400X400     = (400 << 16) | 400,
	TUYA_CAMERA_PPI_412X412     = (412 << 16) | 412,
	TUYA_CAMERA_PPI_454X454     = (454 << 16) | 454,
	TUYA_CAMERA_PPI_480X272     = (480 << 16) | 272,
	TUYA_CAMERA_PPI_480X320     = (480 << 16) | 320,
	TUYA_CAMERA_PPI_480X480     = (480 << 16) | 480,
	TUYA_CAMERA_PPI_640X480     = (640 << 16) | 480,
	TUYA_CAMERA_PPI_480X800     = (480 << 16) | 800,
	TUYA_CAMERA_PPI_480X854     = (480 << 16) | 854,
	TUYA_CAMERA_PPI_480X864     = (480 << 16) | 864,
	TUYA_CAMERA_PPI_720X288     = (720 << 16) | 288,
	TUYA_CAMERA_PPI_720X576     = (720 << 16) | 576,
	TUYA_CAMERA_PPI_720X1280    = (720 << 16) | 1280,
	TUYA_CAMERA_PPI_854X480     = (854 << 16) | 480,
	TUYA_CAMERA_PPI_800X480     = (800 << 16) | 480,
	TUYA_CAMERA_PPI_864X480     = (864 << 16) | 480,
	TUYA_CAMERA_PPI_960X480     = (960 << 16) | 480,
	TUYA_CAMERA_PPI_800X600     = (800 << 16) | 600,
	TUYA_CAMERA_PPI_1024X600    = (1024 << 16) | 600,
	TUYA_CAMERA_PPI_1280X720    = (1280 << 16) | 720,
	TUYA_CAMERA_PPI_1600X1200   = (1600 << 16) | 1200,
	TUYA_CAMERA_PPI_1920X1080   = (1920 << 16) | 1080,
	TUYA_CAMERA_PPI_2304X1296   = (2304 << 16) | 1296,
	TUYA_CAMERA_PPI_7680X4320   = (7680 << 16) | 4320,
}TUYA_CAMERA_PPI_E;

typedef struct {
    TUYA_GPIO_NUM_E   pin;
    TUYA_GPIO_LEVEL_E active_level;
} TUYA_CAMERA_IO_CTRL_T;

typedef void* TDD_CAMERA_DEV_HANDLE_T;

typedef struct {
    TDL_CAMERA_TYPE_E         type;
    uint16_t                  max_fps;
    uint32_t                  max_width;
    uint32_t                  max_height;
    TUYA_FRAME_FMT_E          fmt;
} TDD_CAMERA_DEV_INFO_T;

typedef struct {
    uint16_t                  fps;
    uint16_t                  width;
    uint16_t                  height;
    TDL_CAMERA_FMT_E          out_fmt;
} TDD_CAMERA_OPEN_CFG_T;

typedef struct {
    OPERATE_RET (*open )(TDD_CAMERA_DEV_HANDLE_T device, TDD_CAMERA_OPEN_CFG_T *cfg);
    OPERATE_RET (*close)(TDD_CAMERA_DEV_HANDLE_T device);
} TDD_CAMERA_INTFS_T;

typedef struct {
    void              *sys_param;            // system use, user do not care
    TDL_CAMERA_FRAME_T frame;
    uint8_t            rsv[128];
} TDD_CAMERA_FRAME_T;

/***********************************************************
********************function declaration********************
***********************************************************/
OPERATE_RET tdl_camera_device_register(char *name, TDD_CAMERA_DEV_HANDLE_T tdd_hdl, \
                                       TDD_CAMERA_INTFS_T *intfs, TDD_CAMERA_DEV_INFO_T *dev_info);
                                    
TDD_CAMERA_FRAME_T *tdl_camera_create_tdd_frame(TDD_CAMERA_DEV_HANDLE_T tdd_hdl, TUYA_FRAME_FMT_E fmt);

void tdl_camera_release_tdd_frame(TDD_CAMERA_DEV_HANDLE_T tdd_hdl, TDD_CAMERA_FRAME_T *frame);

OPERATE_RET tdl_camera_post_tdd_frame(TDD_CAMERA_DEV_HANDLE_T tdd_hdl, TDD_CAMERA_FRAME_T *frame);   






#ifdef __cplusplus
}
#endif

#endif /* __TDL_CAMERA_DRIVER_H__ */
