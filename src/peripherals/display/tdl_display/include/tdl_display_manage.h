/**
 * @file tdl_display_manage.h
 * @brief TDL display management system header file
 *
 * This file provides the core display management functionality for the TDL (Tuya Display Layer)
 * system. It includes display device registration, initialization, control functions, and
 * hardware abstraction for various display interfaces. The management system handles display
 * lifecycle, power control, backlight control, and provides unified API for display operations.
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TDL_DISPLAY_MANAGE_H__
#define __TDL_DISPLAY_MANAGE_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef enum  {
    TUYA_DISPLAY_RGB = 0,
    TUYA_DISPLAY_8080,
    TUYA_DISPLAY_QSPI,
    TUYA_DISPLAY_SPI,
    TUYA_DISPLAY_I2C,
}TUYA_DISPLAY_TYPE_E;

typedef void*  TDL_DISP_HANDLE_T;

typedef enum {
    DISP_FB_TP_SRAM = 0,
    DISP_FB_TP_PSRAM,
} DISP_FB_RAM_TP_E;

typedef struct TDL_DISP_FRAME_BUFF_T TDL_DISP_FRAME_BUFF_T;

typedef void (*FRAME_BUFF_FREE_CB)(TDL_DISP_FRAME_BUFF_T *frame_buff);

struct TDL_DISP_FRAME_BUFF_T {
    DISP_FB_RAM_TP_E type;
    TUYA_DISPLAY_PIXEL_FMT_E fmt;
    uint16_t width;
    uint16_t height;
    FRAME_BUFF_FREE_CB free_cb;
    uint32_t len;
    uint8_t *frame;
};

typedef struct {
    TUYA_DISPLAY_TYPE_E type;
    TUYA_DISPLAY_ROTATION_E rotation;
    uint16_t width;
    uint16_t height;
    TUYA_DISPLAY_PIXEL_FMT_E fmt;
    bool                     is_swap;
} TDL_DISP_DEV_INFO_T;

/***********************************************************
********************function declaration********************
***********************************************************/
/**
 * @brief Finds a registered display device by its name.
 *
 * @param name The name of the display device to find.
 *
 * @return Returns a handle to the found display device, or NULL if no matching device is found.
 */
TDL_DISP_HANDLE_T tdl_disp_find_dev(char *name);

/**
 * @brief Retrieves information about a registered display device.
 *
 * This function copies the display device's information, such as type, width, height, 
 * pixel format, and rotation, into the provided output structure.
 *
 * @param disp_hdl Handle to the display device.
 * @param dev_info Pointer to the structure where display information will be stored.
 *
 * @return Returns OPRT_OK on success, or an appropriate error code if the operation fails.
 */
OPERATE_RET tdl_disp_dev_get_info(TDL_DISP_HANDLE_T disp_hdl, TDL_DISP_DEV_INFO_T *dev_info);

/**
 * @brief Opens and initializes a display device.
 *
 * This function prepares the specified display device for operation by initializing 
 * its power control, mutex, and invoking the device-specific open function if available.
 *
 * @param disp_hdl Handle to the display device to be opened.
 *
 * @return Returns OPRT_OK on success, or an appropriate error code if opening the device fails.
 */
OPERATE_RET tdl_disp_dev_open(TDL_DISP_HANDLE_T disp_hdl);

/**
 * @brief Sets the brightness level of the display's backlight.
 *
 * This function controls the backlight of the specified display device using either 
 * GPIO or PWM, depending on the configured backlight type.
 *
 * @param disp_hdl Handle to the display device.
 * @param brightness The desired brightness level (0 for off, non-zero for on).
 *
 * @return Returns OPRT_OK on success, or an appropriate error code if setting the brightness fails.
 */
OPERATE_RET tdl_disp_set_brightness(TDL_DISP_HANDLE_T disp_hdl, uint8_t brightness);

/**
 * @brief Creates and initializes a frame buffer for display operations.
 *
 * This function allocates memory for a frame buffer based on the specified type and length. 
 * It also ensures proper memory alignment for efficient data processing.
 *
 * @param type Type of memory to allocate (e.g., SRAM or PSRAM).
 * @param len Length of the frame buffer data in bytes.
 *
 * @return Returns a pointer to the allocated `TDL_DISP_FRAME_BUFF_T` structure on success, 
 *         or NULL if memory allocation fails.
 */
TDL_DISP_FRAME_BUFF_T *tdl_disp_create_frame_buff(DISP_FB_RAM_TP_E type, uint32_t len);

/**
 * @brief Frees a previously allocated frame buffer.
 *
 * This function releases the memory associated with the specified frame buffer, 
 * taking into account the type of memory (SRAM or PSRAM) used for allocation.
 *
 * @param frame_buff Pointer to the frame buffer to be freed.
 *
 * @return None.
 */
void tdl_disp_free_frame_buff(TDL_DISP_FRAME_BUFF_T *frame_buff);

/**
 * @brief Flushes the frame buffer to the display device.
 *
 * This function sends the contents of the provided frame buffer to the display device 
 * for rendering. It checks if the device is open and if the flush interface is available.
 *
 * @param disp_hdl Handle to the display device.
 * @param frame_buff Pointer to the frame buffer containing pixel data to be displayed.
 *
 * @return Returns OPRT_OK on success, or an appropriate error code if flushing fails.
 */
OPERATE_RET tdl_disp_dev_flush(TDL_DISP_HANDLE_T disp_hdl, TDL_DISP_FRAME_BUFF_T *frame_buff);

/**
 * @brief Closes and deinitializes a display device.
 *
 * This function shuts down the specified display device by invoking the device-specific 
 * close function (if available), deinitializing backlight control, and power control GPIOs.
 *
 * @param disp_hdl Handle to the display device to be closed.
 *
 * @return Returns OPRT_OK on success, or an appropriate error code if closing the device fails.
 */
OPERATE_RET tdl_disp_dev_close(TDL_DISP_HANDLE_T disp_hdl);


#ifdef __cplusplus
}
#endif

#endif /* __TDL_DISPLAY_MANAGE_H__ */
