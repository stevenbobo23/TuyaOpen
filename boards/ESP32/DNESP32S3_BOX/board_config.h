/**
 * @file board_config.h
 * @brief board_config module is used to
 * @version 0.1
 * @date 2025-04-23
 */

#ifndef __BOARD_CONFIG_H__
#define __BOARD_CONFIG_H__

#include "sdkconfig.h"
#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/
#define I2S_INPUT_SAMPLE_RATE  (16000)
#define I2S_OUTPUT_SAMPLE_RATE (16000)

/* I2C port and GPIOs */
#define I2C_NUM    (0)
#define I2C_SCL_IO (45)
#define I2C_SDA_IO (48)

/* I2S port and GPIOs */
#define I2S_NUM    (0)
#define I2S_MCK_IO (-1)
#define I2S_BCK_IO (21)
#define I2S_WS_IO  (13)

#define I2S_DO_IO (14)
#define I2S_DI_IO (47)

#define GPIO_OUTPUT_PA (-1)

#define AUDIO_CODEC_DMA_DESC_NUM  (6)
#define AUDIO_CODEC_DMA_FRAME_NUM (240)
#define AUDIO_CODEC_ES8311_ADDR   (0x30)

/* lcd */
#define LCD_I80_CS  (1)
#define LCD_I80_DC  (2)
#define LCD_I80_RD  (41)
#define LCD_I80_WR  (42)
#define LCD_I80_RST (-1)

#define LCD_I80_D0 (40)
#define LCD_I80_D1 (39)
#define LCD_I80_D2 (38)
#define LCD_I80_D3 (12)
#define LCD_I80_D4 (11)
#define LCD_I80_D5 (10)
#define LCD_I80_D6 (9)
#define LCD_I80_D7 (46)

#define DISPLAY_WIDTH  320
#define DISPLAY_HEIGHT 240

/* io expander start */
#define IO_EXPANDER_TYPE_UNKNOWN 0
#define IO_EXPANDER_TYPE_TCA9554 1
#define IO_EXPANDER_TYPE_XL9555  2

#define BOARD_IO_EXPANDER_TYPE IO_EXPANDER_TYPE_XL9555

#define IO_EXPANDER_XL9555_ADDR_000 (0x20)
#define IO_EXPANDER_XL9555_ADDR     IO_EXPANDER_XL9555_ADDR_000

#define EX_IO_AP_INT   (0x0001 << 0)
#define EX_IO_QMA_INT  (0x0001 << 1)
#define EX_IO_BEEP     (0x0001 << 2)
#define EX_IO_KEY_1    (0x0001 << 3)
#define EX_IO_KEY_0    (0x0001 << 4)
#define EX_IO_SPK_CTRL (0x0001 << 5)
#define EX_IO_CTP_RST  (0x0001 << 6)
#define EX_IO_LCD_BL   (0x0001 << 7)
#define EX_IO_LED_R    (0x0001 << 8)
#define EX_IO_CTP_INT  (0x0001 << 9)
#define EX_IO_1_2      (0x0001 << 10)
#define EX_IO_1_3      (0x0001 << 11)
#define EX_IO_1_4      (0x0001 << 12)
#define EX_IO_1_5      (0x0001 << 13)
#define EX_IO_1_6      (0x0001 << 14)
#define EX_IO_1_7      (0x0001 << 15)

/* io expander end */

/* display */
#define DISPLAY_TYPE_UNKNOWN        0
#define DISPLAY_TYPE_OLED_SSD1306   1
#define DISPLAY_TYPE_LCD_SH8601     2
#define DISPLAY_TYPE_LCD_ST7789_80  3
#define DISPLAY_TYPE_LCD_ST7789_SPI 4

#define BOARD_DISPLAY_TYPE DISPLAY_TYPE_LCD_ST7789_80

/* lvgl config */
#define DISPLAY_BUFFER_SIZE (DISPLAY_WIDTH * 10)

#define DISPLAY_MONOCHROME false

/* rotation */
#define DISPLAY_SWAP_XY  true
#define DISPLAY_MIRROR_X true
#define DISPLAY_MIRROR_Y false

#define DISPLAY_COLOR_FORMAT LV_COLOR_FORMAT_RGB565

// Only one of DISPLAY_BUFF_SPIRAM and DISPLAY_BUFF_DMA can be selected
#define DISPLAY_BUFF_SPIRAM 0
#define DISPLAY_BUFF_DMA    1

#define DISPLAY_SWAP_BYTES 1

/***********************************************************
***********************typedef define***********************
***********************************************************/

/***********************************************************
********************function declaration********************
***********************************************************/

int board_display_init(void);

void *board_display_get_panel_io_handle(void);

void *board_display_get_panel_handle(void);

#ifdef __cplusplus
}
#endif

#endif /* __BOARD_CONFIG_H__ */
