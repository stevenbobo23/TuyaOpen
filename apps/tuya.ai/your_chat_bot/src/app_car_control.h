/**
 * @file app_car_control.h
 * @brief 小车控制功能接口定义
 *
 * 该文件定义了小车控制的相关接口，包括初始化、移动控制等功能
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __APP_CAR_CONTROL_H__
#define __APP_CAR_CONTROL_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 小车移动方向枚举
 */
typedef enum {
    CAR_DIRECTION_FORWARD = 0,   // 前进
    CAR_DIRECTION_BACKWARD,      // 后退
    CAR_DIRECTION_LEFT,          // 左转
    CAR_DIRECTION_RIGHT,         // 右转
    CAR_DIRECTION_STOP,          // 停止
    CAR_DIRECTION_MOVE_LEFT,     // 左移
    CAR_DIRECTION_MOVE_RIGHT     // 右移
} car_direction_e;

/**
 * @brief 初始化小车控制模块
 *
 * @return OPERATE_RET 操作结果
 */
OPERATE_RET app_car_control_init(void);

/**
 * @brief 控制小车移动
 *
 * @param direction 移动方向
 * @return OPERATE_RET 操作结果
 */
OPERATE_RET app_car_control_move(car_direction_e direction);

#ifdef __cplusplus
}
#endif

#endif /* __APP_CAR_CONTROL_H__ */