#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "fsl_debug_console.h"

#include "task.h"

#include "aiot_state_api.h"
#include "aiot_sysdep_api.h"
#include "aiot_mqtt_api.h"
#include "aiot_ota_api.h"

#include "flexspi_flash.h"
#include "fsl_common.h"
#include "flash_info.h"
#include "sbl_ota_flag.h"
#if defined(CONFIG_BOOT_ENCRYPTED_XIP)
#include "update_key_context.h"
#endif

/* TODO: 替换为自己设备的三元组 */
char *product_key       = PRODUCT_KEY;
char *device_name       = DEVICE_NAME;
char *device_secret     = DEVICE_SECRET;

/* external/ali_ca_cert.c */
extern const char *ali_ca_cert;
void *g_ota_handle = NULL;
void *g_dl_handle = NULL;
uint32_t g_firmware_size = 0;
uint32_t dstAddr = FLASH_AREA_IMAGE_2_OFFSET;

/* portfiles/aiot_port */
extern aiot_sysdep_portfile_t g_aiot_sysdep_portfile;

int32_t demo_state_logcb(int32_t code, char *message)
{
    if (STATE_HTTP_LOG_RECV_CONTENT != code) {
        PRINTF("%s", message);
    }
    return 0;
}

/* MQTT事件回调函数, 当网络连接/重连/断开时被触发, 事件定义见core/aiot_mqtt_api.h */
void demo_mqtt_event_handler(void *handle, const aiot_mqtt_event_t *const event, void *userdata)
{
    switch (event->type) {
        case AIOT_MQTTEVT_CONNECT: {
            PRINTF("AIOT_MQTTEVT_CONNECT\r\n");
        }
        break;

        case AIOT_MQTTEVT_RECONNECT: {
            PRINTF("AIOT_MQTTEVT_RECONNECT\r\n");
        }
        break;

        case AIOT_MQTTEVT_DISCONNECT: {
            char *cause = (event->data.disconnect == AIOT_MQTTDISCONNEVT_NETWORK_DISCONNECT) ? ("network disconnect") :
                          ("heartbeat disconnect");
            PRINTF("AIOT_MQTTEVT_DISCONNECT: %s\r\n", cause);
        }
        break;
        default: {

        }
    }
}

/* MQTT默认消息处理回调, 当SDK从服务器收到MQTT消息时, 且无对应用户回调处理时被调用 */
void demo_mqtt_default_recv_handler(void *handle, const aiot_mqtt_recv_t *const packet, void *userdata)
{
    switch (packet->type) {
        case AIOT_MQTTRECV_HEARTBEAT_RESPONSE: {
            PRINTF("heartbeat response\r\n");
        }
        break;
        case AIOT_MQTTRECV_SUB_ACK: {
            PRINTF("suback, res: -0x%04X, packet id: %d, max qos: %d\r\n",
                   -packet->data.sub_ack.res, packet->data.sub_ack.packet_id, packet->data.sub_ack.max_qos);
        }
        break;
        case AIOT_MQTTRECV_PUB: {
            PRINTF("pub, qos: %d, topic: %.*s\r\n", packet->data.pub.qos, packet->data.pub.topic_len, packet->data.pub.topic);
            PRINTF("pub, payload: %.*s\r\n", packet->data.pub.payload_len, packet->data.pub.payload);
        }
        break;
        case AIOT_MQTTRECV_PUB_ACK: {
            PRINTF("puback, packet id: %d\r\n", packet->data.pub_ack.packet_id);
        }
        break;
        default: {

        }
    }
}

/* 下载收包回调, 用户调用 aiot_download_recv() 后, SDK收到数据会进入这个函数, 把下载到的数据交给用户 */
/* TODO: 一般来说, 设备升级时, 会在这个回调中, 把下载到的数据写到Flash上 */
void user_download_recv_handler(void *handle, const aiot_download_recv_t *packet, void *userdata)
{
    int32_t percent = 0;
    int32_t last_percent = 0;
    uint32_t data_buffer_len = 0;

    /* 目前只支持 packet->type 为 AIOT_DLRECV_HTTPBODY 的情况*/
    if (!packet || AIOT_DLRECV_HTTPBODY != packet->type) {
        return;
    }
    percent = packet->data.percent;

    if (percent < 0) {
        /* digest校验错误 */
        PRINTF("exception happend, percent is %d\r\n", percent);
        if (userdata) {
            free(userdata);
        }
        return;
    }

    /* userdata可以存放 demo_download_recv_handler() 的不同次进入之间, 需要共享的数据 */
    /* 这里用来存放上一次进入本回调函数时, 下载的固件进度百分比 */
    if (userdata) {
        last_percent = *((uint32_t *)(userdata));
    }
    data_buffer_len = packet->data.len;
    
    /*
     *       如果烧写flash失败, 还应该调用 aiot_download_report_progress(handle, -4) 将失败上报给云平台
     *       备注:协议中, 与云平台商定的错误码在 aiot_ota_protocol_errcode_t 类型中, 例如
     *            -1: 表示升级失败
     *            -2: 表示下载失败
     *            -3: 表示校验失败
     *            -4: 表示烧写失败
     */
    // 将下载的固件保存到flash上,分段下载，每一段一保存
    status_t status;
    volatile uint32_t primask;   
            
    primask = DisableGlobalIRQ();
    status = sfw_flash_write(dstAddr, packet->data.buffer, packet->data.len);
	if (status)
	{
		aiot_download_report_progress(handle, -4);
	}
    dstAddr += packet->data.len;
    EnableGlobalIRQ(primask);
	
    /* 简化输出, 只有距离上次的下载进度增加5%以上时, 才会打印进度, 并向服务器上报进度 */
    if (percent - last_percent >= 5 || percent == 100) {
        PRINTF("download %03d%% done, +%d bytes\r\n", percent, data_buffer_len);
        aiot_download_report_progress(handle, percent);

        if (userdata) {
            *((uint32_t *)(userdata)) = percent;
        }
        if (percent == 100 && userdata) {
            free(userdata);
        }
    }  
	
    if (percent == 100) {
        
        write_update_type(UPDATE_TYPE_ALI_CLOUD);
        enable_image();
#if defined(CONFIG_BOOT_ENCRYPTED_XIP)
        update_key_context();
#endif
        PRINTF("Down finished all.SystemReset Now...");
        NVIC_SystemReset();
    }    
}

/* 用户通过 aiot_ota_setopt() 注册的OTA消息处理回调, 如果SDK收到了OTA相关的MQTT消息, 会自动识别, 调用这个回调函数 */
void user_ota_recv_handler(void *ota_handle, aiot_ota_recv_t *ota_msg, void *userdata)
{
    switch (ota_msg->type) {
        case AIOT_OTARECV_FOTA: {
            uint16_t port = 80;  
            uint32_t max_buffer_len = 2048;
            aiot_sysdep_network_cred_t cred;
            void *dl_handle = NULL;
            void *last_percent = NULL;

            if (NULL == ota_msg->task_desc) {
                break;
            }

            dl_handle = aiot_download_init();
            if (NULL == dl_handle) {
                break;
            }

            last_percent = malloc(sizeof(uint32_t));
            if (NULL == last_percent) {
                aiot_download_deinit(&dl_handle);
                break;
            }
            memset(last_percent, 0, sizeof(uint32_t));

            PRINTF("OTA target firmware version: %s, size: %u Bytes\r\n", ota_msg->task_desc->version,
                   ota_msg->task_desc->size_total);
#ifdef SOC_REMAP_ENABLE
            uint8_t image_position;
            sfw_flash_read(REMAP_FLAG_ADDRESS, &image_position, 1);
            if(image_position == 0x01)
            {
              dstAddr = FLASH_AREA_IMAGE_2_OFFSET;
            }
            else if(image_position == 0x02)
            {
              dstAddr = FLASH_AREA_IMAGE_1_OFFSET;
            }
#endif
            /* 将要写入的地址，之后size_total大小的区域，利用sfw_flash_erase擦一下 */
            status_t status;
            volatile uint32_t primask;
//            uint32_t dstAddr = FLASH_AREA_IMAGE_2_OFFSET;
            
            primask = DisableGlobalIRQ();
            status = sfw_flash_erase(dstAddr, FLASH_AREA_IMAGE_1_SIZE);
            EnableGlobalIRQ(primask);
            
            if (status) 
            {
                PRINTF("erase failed.\r\n");
//                goto exit;
            } 
              
            if (NULL != ota_msg->task_desc->extra_data) {
                PRINTF("extra data: %s\r\n", ota_msg->task_desc->extra_data);
            }

            g_firmware_size = ota_msg->task_desc->size_total;

            memset(&cred, 0, sizeof(aiot_sysdep_network_cred_t));
            cred.option = AIOT_SYSDEP_NETWORK_CRED_SVRCERT_CA;
            cred.max_tls_fragment = 16384;
            cred.x509_server_cert = ali_ca_cert;
            cred.x509_server_cert_len = strlen(ali_ca_cert);
            uint32_t end = 0;  //此处设为0，代表一次性下载完整个固件，若要分两段下载，可设为g_firmware_size / 2;
            /* 设置下载时为TLS下载 */
            if ((STATE_SUCCESS != aiot_download_setopt(dl_handle, AIOT_DLOPT_NETWORK_CRED, (void *)(&cred)))
                /* 设置下载时访问的服务器端口号 */
                || (STATE_SUCCESS != aiot_download_setopt(dl_handle, AIOT_DLOPT_NETWORK_PORT, (void *)(&port)))
                /* 设置下载的任务信息, 通过输入参数 ota_msg 中的 task_desc 成员得到, 内含下载地址, 固件大小, 固件签名等 */
                || (STATE_SUCCESS != aiot_download_setopt(dl_handle, AIOT_DLOPT_TASK_DESC, (void *)(ota_msg->task_desc)))
                /* 设置下载内容到达时, SDK将调用的回调函数 */
                || (STATE_SUCCESS != aiot_download_setopt(dl_handle, AIOT_DLOPT_RECV_HANDLER, (void *)(user_download_recv_handler)))
                /* 设置单次下载最大的buffer长度, 每当这个长度的内存读满了后会通知用户 */
                || (STATE_SUCCESS != aiot_download_setopt(dl_handle, AIOT_DLOPT_BODY_BUFFER_MAX_LEN, (void *)(&max_buffer_len)))
                /* 设置 AIOT_DLOPT_RECV_HANDLER 的不同次调用之间共享的数据, 比如例程把进度存在这里 */
                || (STATE_SUCCESS != aiot_download_setopt(dl_handle, AIOT_DLOPT_USERDATA, (void *)last_percent))
                /* 指明下载方式是按照range下载, 并且当前只下载一半 */
                || (STATE_SUCCESS != aiot_download_setopt(dl_handle, AIOT_DLOPT_RANGE_END, (void *)&end))
                /* 发送http的GET请求给http服务器 */
                || (STATE_SUCCESS != aiot_download_send_request(dl_handle))) {
                aiot_download_deinit(&dl_handle);
                free(last_percent);
                break;
            }
            g_dl_handle = dl_handle;
            break;
        }
        default:
            break;
    }
}


int ota_main(int argc, char *argv[])
{
    int32_t res = STATE_SUCCESS;
    void *mqtt_handle = NULL;
    int8_t      public_instance = 1;  /* 用公共实例, 该参数要设置为1. 若用独享实例, 要将该参数设置为0 */
    char *url = "iot-as-mqtt.cn-shanghai.aliyuncs.com"; /* 阿里云平台上海站点的域名后缀 */
    char host[100] = {0}; /* 用这个数组拼接设备连接的云平台站点全地址, 规则是 ${productKey}.iot-as-mqtt.cn-shanghai.aliyuncs.com */
    uint16_t port = 443;  /* 无论设备是否使用TLS连接阿里云平台, 目的端口都是443 */
    aiot_sysdep_network_cred_t cred; /* 安全凭据结构体, 如果要用TLS, 这个结构体中配置CA证书等参数 */
    char *cur_version = NULL;
    void *ota_handle = NULL;
    uint32_t timeout_ms = 0;

    /* 配置SDK的底层依赖 */
    aiot_sysdep_set_portfile(&g_aiot_sysdep_portfile);

    /* 配置SDK的日志输出 */
    aiot_state_set_logcb(demo_state_logcb);

    /* 创建SDK的安全凭据, 用于建立TLS连接 */
    memset(&cred, 0, sizeof(aiot_sysdep_network_cred_t));
    cred.option = AIOT_SYSDEP_NETWORK_CRED_NONE;  //SVRCERT_CA;  /* 使用RSA证书校验MQTT服务端 */
    cred.max_tls_fragment = 16384; /* 最大的分片长度为16K, 其它可选值还有4K, 2K, 1K, 0.5K */
    cred.sni_enabled = 1;                               /* TLS建连时, 支持Server Name Indicator */
    cred.x509_server_cert = ali_ca_cert;                 /* 用来验证MQTT服务端的RSA根证书 */
    cred.x509_server_cert_len = strlen(ali_ca_cert);     /* 用来验证MQTT服务端的RSA根证书长度 */

    /* 创建1个MQTT客户端实例并内部初始化默认参数 */
    mqtt_handle = aiot_mqtt_init();
    if (mqtt_handle == NULL) {
        return -1;
    }

    if (1 == public_instance) {
        snprintf(host, 100, "%s.%s", product_key, url);
    } else {
        snprintf(host, 100, "%s", url);
    }

    /* 配置MQTT服务器地址 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_HOST, (void *)host);
    /* 配置MQTT服务器端口 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_PORT, (void *)&port);
    /* 配置设备productKey */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_PRODUCT_KEY, (void *)product_key);
    /* 配置设备deviceName */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_DEVICE_NAME, (void *)device_name);
    /* 配置设备deviceSecret */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_DEVICE_SECRET, (void *)device_secret);
    /* 配置网络连接的安全凭据, 上面已经创建好了 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_NETWORK_CRED, (void *)&cred);
    /* 配置MQTT默认消息接收回调函数 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_RECV_HANDLER, (void *)demo_mqtt_default_recv_handler);
    /* 配置MQTT事件回调函数 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_EVENT_HANDLER, (void *)demo_mqtt_event_handler);

    /* 与MQTT例程不同的是, 这里需要增加创建OTA会话实例的语句 */
    ota_handle = aiot_ota_init();
    if (NULL == ota_handle) {
        goto exit;
    }

    /* 用以下语句, 把OTA会话和MQTT会话关联起来 */
    aiot_ota_setopt(ota_handle, AIOT_OTAOPT_MQTT_HANDLE, mqtt_handle);
    /* 用以下语句, 设置OTA会话的数据接收回调, SDK收到OTA相关推送时, 会进入这个回调函数 */
    aiot_ota_setopt(ota_handle, AIOT_OTAOPT_RECV_HANDLER, (void *)user_ota_recv_handler);
    g_ota_handle = ota_handle;

    /* 与服务器建立MQTT连接 */
    res = aiot_mqtt_connect(mqtt_handle);
    if (res < STATE_SUCCESS) {
        PRINTF("aiot_mqtt_connect failed: -0x%04X\r\n", -res);
        /* 尝试建立连接失败, 销毁MQTT实例, 回收资源 */
        goto exit;
    }

    /*   TODO: 非常重要!!!
     *
     *   cur_version 要根据用户实际情况, 改成从设备的配置区获取, 要反映真实的版本号, 而不能像示例这样写为固定值
     *
     *   1. 如果设备从未上报过版本号, 在控制台网页将无法部署升级任务
     *   2. 如果设备升级完成后, 上报的不是新的版本号, 在控制台网页将会显示升级失败
     *
     */

    cur_version = "1.8.0";  //更改为所需要更新的版本，如1.1.0
        
    uint8_t last_update_type;
    
    sfw_flash_read(UPDATE_TYPE_FLAG_ADDRESS, &last_update_type, 1);
    
    if(last_update_type == UPDATE_TYPE_ALI_CLOUD)
    {
        PRINTF("Update done, the last update type: %s\r\n",
               last_update_type == UPDATE_TYPE_SDCARD ? "SD Card" :
               last_update_type == UPDATE_TYPE_UDISK ? "U-Disk" :
               last_update_type == UPDATE_TYPE_AWS_CLOUD ? "AWS platform" :
               last_update_type == UPDATE_TYPE_ALI_CLOUD ? "ALI platform" : 
                         "BUG; can't happend");
        write_image_ok();        
    }
    
    /* 演示MQTT连接建立起来之后, 就可以上报当前设备的版本号了 */
    res = aiot_ota_report_version(ota_handle, cur_version);
    if (res < STATE_SUCCESS) {
        PRINTF("report version failed, code is -0x%04X\r\n", -res);
    }

    while (1) {
        aiot_mqtt_process(mqtt_handle);
        res = aiot_mqtt_recv(mqtt_handle);

        if (res == STATE_SYS_DEPEND_NWK_CLOSED) {
            vTaskDelay(1000);  
        }
        if (NULL != g_dl_handle) {
            /* 完成固件的接收前, 将mqtt的收包超时调整到100ms, 以减少两次固件下载动作之间的时间间隔 */
            int32_t ret = aiot_download_recv(g_dl_handle);
            timeout_ms = 100;
            aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_RECV_TIMEOUT_MS, (void *)&timeout_ms);

            /* 本例子中, 一次性下载完整个固件。若要将整个固件分为2个等份来下载, 每次下载一半, 第一半下载完成时会进入这个分支 */
            if (STATE_DOWNLOAD_RANGE_FINISHED == ret) {
                PRINTF("the first half download finished\n");

                /* 当第一次下载完成后, 要重新设置range的start/end值, 下载另外一半 */

                /*
                 * 非常重要:
                 *
                 *       上一次下载的范围是[0, g_firmware_size/2],
                 *       因此这一次下载需要从 [g_firmware_size/2 + 1, 固件结束地址],
                 *       其中固件结束地址填0的话就表示直到下载结束
                 *
                 */

                uint32_t start = g_firmware_size / 2 + 1;
                uint32_t end = 0;
                aiot_download_setopt(g_dl_handle, AIOT_DLOPT_RANGE_START, (void *)&start);
                /* range_end如果指定为0, 服务器就理解为下载到文件末尾后结束 */
                aiot_download_setopt(g_dl_handle, AIOT_DLOPT_RANGE_END, (void *)&end);
                /* 向服务器发起下一次下载的请求 */
                aiot_download_send_request(g_dl_handle);
                continue;
            }

            /* 整个固件下载完成 */
            if (ret == STATE_DOWNLOAD_FINISHED) {
                PRINTF("down finished all\n");
                aiot_download_deinit(&g_dl_handle);
                /* 完成固件的接收后, 将mqtt的收包超时调整回到默认值5000ms */
                timeout_ms = 5000;
                aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_RECV_TIMEOUT_MS, (void *)&timeout_ms);
                continue;
            }

            /* 下载的内容超出了固件的总大小, 可能是用户把range划分错了, range之间有重叠 */
            if (ret == STATE_DOWNLOAD_FETCH_TOO_MANY) {
                PRINTF("downloaded more than expeced bytes, please check range start/end settings\n");
                aiot_download_deinit(&g_dl_handle);
                /* 完成固件的接收后, 将mqtt的收包超时调整回到默认值5000ms */
                timeout_ms = 5000;
                aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_RECV_TIMEOUT_MS, (void *)&timeout_ms);
                continue;
            }

            if (STATE_DOWNLOAD_RENEWAL_REQUEST_SENT == ret) {
                PRINTF("download renewal request has been sent successfully\r\n");
                continue;
            }

            if (ret <= STATE_SUCCESS) {
                PRINTF("download failed, error code is %d, try to send renewal request\r\n", ret);
                continue;
            }

        }
    }

    /* 断开MQTT连接, 一般不会运行到这里 */
/*
    res = aiot_mqtt_disconnect(mqtt_handle);
    if (res < STATE_SUCCESS) {
        PRINTF("aiot_mqtt_disconnect failed: -0x%04X\r\n", -res);
        goto exit;
    }
*/

exit:
    while (1) {
        /* 销毁MQTT实例, 一般不会运行到这里 */
        res = aiot_mqtt_deinit(&mqtt_handle);

        if (res < STATE_SUCCESS) {
            PRINTF("aiot_mqtt_deinit failed: -0x%04X\r\n", -res);
            return -1;
        } else {
            break;
        }
    }

    /* 销毁OTA实例, 一般不会运行到这里 */
    aiot_ota_deinit(&ota_handle);

    return 0;
}


