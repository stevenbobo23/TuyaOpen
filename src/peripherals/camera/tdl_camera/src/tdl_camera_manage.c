/**
 * @file tdl_camera_manage.c
 * @version 0.1
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 */

#include "tuya_cloud_types.h"
#include "tuya_list.h"
#include "tal_api.h"
#include "tkl_dvp.h"
#include "tkl_memory.h"

#include "tdl_camera_manage.h"
#include "tdl_camera_driver.h"

/***********************************************************
************************macro define************************
***********************************************************/
#define CAMERA_RAW_FRAME_BUFF_CNT           (2)
#define CAMERA_ENCODE_FRAME_BUFF_CNT        (CAMERA_RAW_FRAME_BUFF_CNT << 2)

#define CAMERA_RAW_PER_PIXEL_MAX_BYTE       (3)
#define CAMERA_ENCODE_MIN_COMP_PCT          (20) // uint:ENCODE

#if defined(ENABLE_EXT_RAM) && (ENABLE_EXT_RAM==1)
#define TDL_CAMERA_FRAME_MALLOC    tkl_system_psram_malloc
#define TDL_CAMERA_FRAME_FREE      tkl_system_psram_free
#else
#define TDL_CAMERA_FRAME_MALLOC    tal_malloc
#define TDL_CAMERA_FRAME_FREE      tal_free
#endif

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct {
    struct tuya_list_head       node;
    bool                        is_open;
    char                        name[CAMERA_DEV_NAME_MAX_LEN + 1];
    MUTEX_HANDLE                mutex;
   
    TDL_CAMERA_DEV_INFO_T       info;
    TDL_CAMERA_GET_FRAME_CB     get_raw_frame_cb;
    TDL_CAMERA_GET_FRAME_CB     get_encoded_frame_cb;

    struct tuya_list_head       raw_frame_node_list;
    struct tuya_list_head       encoded_frame_node_list;

    TDD_CAMERA_DEV_HANDLE_T     tdd_hdl;
    TDD_CAMERA_INTFS_T          intfs;
} CAMERA_DEVICE_T;

typedef struct {
    struct tuya_list_head       node;
    TDD_CAMERA_FRAME_T          tdd_frame;
} CAMERA_FRAME_NODE_T;

typedef struct {
    QUEUE_HANDLE                raw_frame_queue;
    QUEUE_HANDLE                encoded_frame_queue;
    THREAD_HANDLE               raw_thrd;
    THREAD_HANDLE               encoded_thrd;
}CAMERA_MANAGE_INFO_T;

typedef struct {
    TDD_CAMERA_FRAME_T         *tdd_frame;
    CAMERA_DEVICE_T            *dev;
} CAMERA_MSG_T;

/***********************************************************
***********************variable define**********************
***********************************************************/
static struct tuya_list_head sg_camera_list = LIST_HEAD_INIT(sg_camera_list);
static CAMERA_MANAGE_INFO_T sg_camera_manage;

/***********************************************************
***********************function define**********************
***********************************************************/
static CAMERA_DEVICE_T *__find_camera_device(char *name)
{
    CAMERA_DEVICE_T *camera_dev = NULL;
    struct tuya_list_head *pos = NULL;

    if (NULL == name) {
        return NULL;
    }

    tuya_list_for_each(pos, &sg_camera_list){
        camera_dev = tuya_list_entry(pos, CAMERA_DEVICE_T, node);
        if (0 == strncmp(camera_dev->name, name, CAMERA_DEV_NAME_MAX_LEN)) {
            return camera_dev;
        }
    }

    return NULL;
}

static CAMERA_DEVICE_T *__find_camera_device_from_tdd(TDD_CAMERA_DEV_HANDLE_T tdd_hdl)
{
    CAMERA_DEVICE_T *camera_dev = NULL;
    struct tuya_list_head *pos = NULL;

    if (NULL == tdd_hdl) {
        return NULL;
    }

    tuya_list_for_each(pos, &sg_camera_list){
        camera_dev = tuya_list_entry(pos, CAMERA_DEVICE_T, node);
        if (camera_dev->tdd_hdl == tdd_hdl) {
            return camera_dev;
        }
    }

    return NULL;
}

static bool __is_camera_frame_encoded(TUYA_FRAME_FMT_E fmt)
{
    bool is_encoded = false;

	switch (fmt)
	{
	case TUYA_FRAME_FMT_YUV422:
	case TUYA_FRAME_FMT_YUV420:
    case TUYA_FRAME_FMT_RGB565:
    case TUYA_FRAME_FMT_RGB888:
        is_encoded = false;
		break;
    case TUYA_FRAME_FMT_JPEG:
    case TUYA_FRAME_FMT_H264:
		is_encoded = true;
		break;
	default:
		break;
	}

	return is_encoded;
}

static OPERATE_RET __camera_frame_node_init(struct tuya_list_head *phead, uint32_t node_num,\
                                            uint32_t buf_len)
{
    CAMERA_FRAME_NODE_T *frame_node = NULL;
    uint32_t i;

    if(NULL == phead || 0 == buf_len || 0 == node_num) {
        return OPRT_INVALID_PARM;
    }

    for (i = 0; i < node_num; i++) {
        NEW_LIST_NODE(CAMERA_FRAME_NODE_T, frame_node);
        if (NULL == frame_node) {
            return OPRT_MALLOC_FAILED;
        }
        memset(frame_node, 0, sizeof(CAMERA_FRAME_NODE_T));

        frame_node->tdd_frame.frame.data = TDL_CAMERA_FRAME_MALLOC(buf_len);
        if (NULL == frame_node->tdd_frame.frame.data) {
            FreeNode(frame_node);
            return OPRT_MALLOC_FAILED;
        }
        frame_node->tdd_frame.frame.data_len = buf_len;
        frame_node->tdd_frame.sys_param = (void *)frame_node;

        PR_DEBUG("phead:%p next:%p pre:%p", phead, \
        phead->next,phead->prev);

        tuya_list_add(&frame_node->node, phead);
    }

    return OPRT_OK;
}

static void __raw_flow_task(void *args)
{
    CAMERA_MSG_T msg;

	while (1)
	{
		tal_queue_fetch(sg_camera_manage.raw_frame_queue, &msg, SEM_WAIT_FOREVER);
		if(NULL == msg.dev || NULL == msg.tdd_frame) {
            continue;
        }

		if (msg.dev->get_raw_frame_cb) {
            msg.dev->get_raw_frame_cb((TDL_CAMERA_HANDLE_T)msg.dev, &msg.tdd_frame->frame);
        }

		tdl_camera_release_tdd_frame(msg.dev->tdd_hdl, msg.tdd_frame);
	}
}

static void __encoded_flow_task(void *args)
{
    CAMERA_MSG_T msg;

	while (1)
	{
		tal_queue_fetch(sg_camera_manage.encoded_frame_queue, &msg, SEM_WAIT_FOREVER);
		if(NULL == msg.dev || NULL == msg.tdd_frame) {
            continue;
        }

		if (msg.dev->get_encoded_frame_cb) {
            msg.dev->get_encoded_frame_cb((TDL_CAMERA_HANDLE_T)msg.dev, &msg.tdd_frame->frame);
        }

		tdl_camera_release_tdd_frame(msg.dev->tdd_hdl, msg.tdd_frame);
	}
}

static OPERATE_RET __camera_manage_init(TDL_CAMERA_FMT_E out_fmt)
{
    OPERATE_RET rt;

    if(out_fmt & TDL_IMG_FMT_RAW_MASK) {
        if(NULL == sg_camera_manage.raw_frame_queue) {
            TUYA_CALL_ERR_RETURN(tal_queue_create_init(&(sg_camera_manage.raw_frame_queue),\
                                                     sizeof(CAMERA_MSG_T), CAMERA_RAW_FRAME_BUFF_CNT));
        }
    
        if(NULL == sg_camera_manage.raw_thrd) {
            THREAD_CFG_T thread_cfg = {4096, THREAD_PRIO_1, "raw_flow_task"};
            TUYA_CALL_ERR_RETURN(tal_thread_create_and_start(&(sg_camera_manage.raw_thrd), NULL, NULL,\
                                                             __raw_flow_task, NULL, &thread_cfg));
        }
    }

    if(out_fmt & TDL_IMG_FMT_ENCODED_MASK) {
        if(NULL == sg_camera_manage.encoded_frame_queue) {
            TUYA_CALL_ERR_RETURN(tal_queue_create_init(&(sg_camera_manage.encoded_frame_queue),\
                                                     sizeof(CAMERA_MSG_T), CAMERA_RAW_FRAME_BUFF_CNT));
        }
    
        if(NULL == sg_camera_manage.encoded_thrd) {
            THREAD_CFG_T thread_cfg = {4096, THREAD_PRIO_1, "encoded_flow_task"};
            TUYA_CALL_ERR_RETURN(tal_thread_create_and_start(&(sg_camera_manage.encoded_thrd), NULL, NULL,\
                                                             __encoded_flow_task, NULL, &thread_cfg));
        }
    }

    return OPRT_OK;
}

TDL_CAMERA_HANDLE_T tdl_camera_find_dev(char *name)
{
    return (TDL_CAMERA_HANDLE_T)__find_camera_device(name);
}

OPERATE_RET tdl_camera_dev_get_info(TDL_CAMERA_HANDLE_T camera_hdl, TDL_CAMERA_DEV_INFO_T *dev_info)
{
    CAMERA_DEVICE_T *camera_dev = (CAMERA_DEVICE_T *)camera_hdl;

    if (NULL == camera_dev || NULL == dev_info) {
        return OPRT_INVALID_PARM;
    }

    memcpy(dev_info, &camera_dev->info, sizeof(TDL_CAMERA_DEV_INFO_T));

    return OPRT_OK;
}

OPERATE_RET tdl_camera_dev_open(TDL_CAMERA_HANDLE_T camera_hdl,  TDL_CAMERA_CFG_T *cfg)
{
    OPERATE_RET rt = OPRT_OK;
    CAMERA_DEVICE_T *camera_dev = NULL;
    uint32_t raw_buf_len = 0;

    if(NULL == camera_hdl || NULL == cfg) {
        return OPRT_INVALID_PARM;
    }

    camera_dev = (CAMERA_DEVICE_T *)camera_hdl;

    TUYA_CALL_ERR_RETURN(__camera_manage_init(cfg->out_fmt));

    raw_buf_len = cfg->width * cfg->height * CAMERA_RAW_PER_PIXEL_MAX_BYTE;

    if(cfg->out_fmt & TDL_IMG_FMT_RAW_MASK) {
        PR_DEBUG("raw_frame_node_list:%p next:%p pre:%p", camera_dev->raw_frame_node_list, \
            camera_dev->raw_frame_node_list.next,camera_dev->raw_frame_node_list.prev);
        TUYA_CALL_ERR_RETURN(__camera_frame_node_init(&camera_dev->raw_frame_node_list, \
                                                      CAMERA_RAW_FRAME_BUFF_CNT, raw_buf_len));
        camera_dev->get_raw_frame_cb = cfg->get_frame_cb;
    }

    if(cfg->out_fmt & TDL_IMG_FMT_ENCODED_MASK) {
        uint32_t encoded_buf_len = (raw_buf_len * CAMERA_ENCODE_MIN_COMP_PCT + 99) / 100;
        TUYA_CALL_ERR_RETURN(__camera_frame_node_init(&camera_dev->encoded_frame_node_list, \
                                                      CAMERA_ENCODE_FRAME_BUFF_CNT, encoded_buf_len));
        camera_dev->get_encoded_frame_cb = cfg->get_encoded_frame_cb;
    }  
    
    camera_dev->info.fps     = cfg->fps;
    camera_dev->info.width   = cfg->width;
    camera_dev->info.height  = cfg->height;
    camera_dev->info.out_fmt = cfg->out_fmt;

    if(camera_dev->intfs.open) {
        TDD_CAMERA_OPEN_CFG_T open_cfg;

        open_cfg.fps     = camera_dev->info.fps;
        open_cfg.width   = camera_dev->info.width;
        open_cfg.height  = camera_dev->info.height;
        open_cfg.out_fmt = camera_dev->info.out_fmt;

        TUYA_CALL_ERR_RETURN(camera_dev->intfs.open(camera_dev->tdd_hdl, &open_cfg));
    }

    return OPRT_OK;
}

OPERATE_RET tdl_camera_dev_close(TDL_CAMERA_HANDLE_T camera_hdl)
{
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tdl_camera_device_register(char *name, TDD_CAMERA_DEV_HANDLE_T tdd_hdl, \
                                       TDD_CAMERA_INTFS_T *intfs, TDD_CAMERA_DEV_INFO_T *dev_info)
{
    CAMERA_DEVICE_T *camera_dev = NULL;

    if (NULL == name || NULL == tdd_hdl || NULL == intfs || NULL == dev_info) {
        return OPRT_INVALID_PARM;
    }

    NEW_LIST_NODE(CAMERA_DEVICE_T, camera_dev);
    if (NULL == camera_dev) {
        return OPRT_MALLOC_FAILED;
    }
    memset(camera_dev, 0, sizeof(CAMERA_DEVICE_T));

    strncpy(camera_dev->name, name, CAMERA_DEV_NAME_MAX_LEN);

    camera_dev->info.type        = dev_info->type;
    camera_dev->info.max_fps     = dev_info->max_fps;
    camera_dev->info.max_width   = dev_info->max_width;
    camera_dev->info.max_height  = dev_info->max_height;
    camera_dev->info.sr_fmt      = dev_info->fmt;

    INIT_LIST_HEAD(&(camera_dev->raw_frame_node_list));
    INIT_LIST_HEAD(&(camera_dev->encoded_frame_node_list));

    PR_DEBUG("raw_frame_node_list:%p next:%p pre:%p", &camera_dev->raw_frame_node_list, \
            camera_dev->raw_frame_node_list.next,camera_dev->raw_frame_node_list.prev);

    camera_dev->tdd_hdl = tdd_hdl;

    memcpy(&camera_dev->intfs, intfs, sizeof(TDD_CAMERA_INTFS_T));

    tuya_list_add(&camera_dev->node, &sg_camera_list);

    return OPRT_OK;
}

TDD_CAMERA_FRAME_T *tdl_camera_create_tdd_frame(TDD_CAMERA_DEV_HANDLE_T tdd_hdl, TUYA_FRAME_FMT_E fmt)
{
    CAMERA_DEVICE_T *camera_dev = NULL;
    struct tuya_list_head *pframe_list = NULL;
    CAMERA_FRAME_NODE_T *pnode = NULL;

    camera_dev = __find_camera_device_from_tdd(tdd_hdl);
    if (NULL == camera_dev) {
        PR_ERR("can not find camera device");
        return NULL;
    }

    pframe_list = (false == __is_camera_frame_encoded(fmt)) ? \
                  &camera_dev->raw_frame_node_list : &camera_dev->encoded_frame_node_list;

    if(tuya_list_empty(pframe_list)) {
        PR_ERR("no free frame node");
        return NULL;
    }

    pnode = tuya_list_entry(pframe_list->next, CAMERA_FRAME_NODE_T, node);

    tuya_list_del(&pnode->node);

    pnode->tdd_frame.frame.fmt = fmt;

    return &pnode->tdd_frame;
}

void tdl_camera_release_tdd_frame(TDD_CAMERA_DEV_HANDLE_T tdd_hdl, TDD_CAMERA_FRAME_T *frame)
{    
    CAMERA_DEVICE_T *camera_dev = NULL;
    struct tuya_list_head *pframe_list = NULL;
    CAMERA_FRAME_NODE_T *pnode = NULL;

    if(NULL == frame || NULL == tdd_hdl) {
        return;
    }

    if(NULL == frame->sys_param) {
        PR_ERR("frame sys_param is NULL");
        return;
    }

    camera_dev = __find_camera_device_from_tdd(tdd_hdl);
    if (NULL == camera_dev) {
        return;
    }

    pframe_list = (false == __is_camera_frame_encoded(frame->frame.fmt)) ? \
                  &camera_dev->raw_frame_node_list : &camera_dev->encoded_frame_node_list;

    pnode = (CAMERA_FRAME_NODE_T *)frame->sys_param;

    tuya_list_add_tail(&pnode->node, pframe_list);

    return;
}

OPERATE_RET tdl_camera_post_tdd_frame(TDD_CAMERA_DEV_HANDLE_T tdd_hdl, TDD_CAMERA_FRAME_T *frame)
{
    CAMERA_MSG_T msg;
    QUEUE_HANDLE queue;
    CAMERA_DEVICE_T *camera_dev = NULL;

    if(NULL == frame || NULL == tdd_hdl) {
        return OPRT_INVALID_PARM;
    }

    if(NULL == frame->sys_param) {
        PR_ERR("frame sys_param is NULL");
        return OPRT_INVALID_PARM;
    }

    queue = (false == __is_camera_frame_encoded(frame->frame.fmt)) ? \
                sg_camera_manage.raw_frame_queue : sg_camera_manage.encoded_frame_queue;
    if(NULL == queue) {
        PR_ERR("queue is null");
        return OPRT_COM_ERROR;
    }

    camera_dev = __find_camera_device_from_tdd(tdd_hdl);
    if (NULL == camera_dev) {
        return OPRT_COM_ERROR;
    }

    msg.tdd_frame = frame;
    msg.dev       = camera_dev;

    return tal_queue_post(queue, &msg, 0);
}