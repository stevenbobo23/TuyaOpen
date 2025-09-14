/**
 * @file app_car_control.c
 * @brief 小车控制功能实现
 *
 * 该文件实现了小车控制的相关功能，包括初始化、移动控制等
 * 通过HTTP请求控制小车移动
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 *
 */

#include "app_car_control.h"
#include "http_client_interface.h"
#include "tal_api.h"
#include "tkl_output.h"

/* 小车控制服务器配置 */
#define CAR_CONTROL_SERVER "192.168.101.68"
#define CAR_CONTROL_PORT   5555
#define CAR_CONTROL_PATH   "/send_action"
#define HTTP_REQUEST_TIMEOUT 5000  // 5秒超时

/**
 * @brief 初始化小车控制模块
 *
 * @return OPERATE_RET 操作结果
 */
OPERATE_RET app_car_control_init(void)
{
    PR_DEBUG("小车控制模块初始化");
    return OPRT_OK;
}

/**
 * @brief 发送HTTP请求控制小车
 *
 * @param key 控制按键
 * @return OPERATE_RET 操作结果
 */
__attribute__((used)) static OPERATE_RET _send_car_control_request(const char *key)
{
    OPERATE_RET rt = OPRT_OK;
    char body[64] = {0};
    
    /* 构建请求体 */
    if (key && key[0] != '\0') {
        snprintf(body, sizeof(body), "{\"keys\": [\"%s\"]}", key);
    } else {
        /* 发送空数组表示停止 */
        snprintf(body, sizeof(body), "{\"keys\": []}");
    }
    
    /* HTTP 响应 */
    http_client_response_t http_response = {0};
    
    /* HTTP 头部 */
    http_client_header_t headers[] = {
        {.key = "Content-Type", .value = "application/json"}
    };
    
    PR_DEBUG("发送小车控制请求: %s", body);
    
    /* 发送 HTTP 请求 */
    http_client_status_t http_status = http_client_request(
        &(const http_client_request_t){
            .host = CAR_CONTROL_SERVER,
            .port = CAR_CONTROL_PORT,
            .method = "POST",
            .path = CAR_CONTROL_PATH,
            .headers = headers,
            .headers_count = sizeof(headers) / sizeof(http_client_header_t),
            .body = (const uint8_t *)body,
            .body_length = strlen(body),
            .timeout_ms = HTTP_REQUEST_TIMEOUT
        },
        &http_response);
    
    if (HTTP_CLIENT_SUCCESS != http_status) {
        PR_ERR("小车控制请求发送失败: %d", http_status);
        rt = OPRT_LINK_CORE_HTTP_CLIENT_SEND_ERROR;
        goto err_exit;
    }
    
    PR_DEBUG("小车控制请求成功，响应: %s", http_response.body ? (const char *)http_response.body : "无响应");
    
err_exit:
    http_client_free(&http_response);
    return rt;
}

/**
 * @brief 控制小车移动
 *
 * @param direction 移动方向
 * @return OPERATE_RET 操作结果
 */
__attribute__((used)) OPERATE_RET app_car_control_move(car_direction_e direction)
{
    const char *key = NULL;
    
    /* 根据方向选择控制按键 */
    switch (direction) {
        case CAR_DIRECTION_FORWARD:
            key = "w";
            break;
        case CAR_DIRECTION_BACKWARD:
            key = "s";
            break;
        case CAR_DIRECTION_LEFT:
            key = "z";
            break;
        case CAR_DIRECTION_RIGHT:
            key = "x";
            break;
        case CAR_DIRECTION_STOP:
            /* 发送空数组表示停止 */
            return _send_car_control_request("");
        case CAR_DIRECTION_MOVE_LEFT:
            key = "a";
            break;
        case CAR_DIRECTION_MOVE_RIGHT:
            key = "d";
            break;
        default:
            PR_ERR("无效的小车移动方向: %d", direction);
            return OPRT_INVALID_PARM;
    }
    
    PR_DEBUG("控制小车移动，方向: %d, 按键: %s", direction, key);
    return _send_car_control_request(key);
}