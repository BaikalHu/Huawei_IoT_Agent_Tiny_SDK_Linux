/*----------------------------------------------------------------------------
 * Copyright (c) <2016-2018>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/

#include "agent_tiny_demo.h"
#include <pthread.h>
#include <unistd.h>

volatile uint8_t bh_temp=0;
volatile uint8_t bh_str=0;

#define DEFAULT_SERVER_IPV4 "139.159.140.34"/*Huawei */
//#define DEFAULT_SERVER_IPV4 "139.159.139.219"

#define LWM2M_LIFE_TIME     50000

#define IOT_PSK_VALUE_LENGTH    12
#define BS_PSK_VALUE_LENGTH     12

char * g_endpoint_name = "11110001"; // HuaWei OceanConnect

#ifdef WITH_DTLS
char* g_endpoint_name_s = "11110001";
char* g_endpoint_name_iots = "66667777";
char* g_endpoint_name_bs = "22224444";
unsigned char g_psk_iot_value[IOT_PSK_VALUE_LENGTH] = {0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33};  //0x33 -> 0x32
unsigned char g_psk_bs_value[BS_PSK_VALUE_LENGTH] = {0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33};
#endif

 void* g_phandle = NULL;
static atiny_device_info_t g_device_info;
static atiny_param_t g_atiny_params;

void ack_callback(atiny_report_type_e type, int cookie, data_send_status_e status)
{
//    printf("type:%d cookie:%d status:%d\n", type,cookie, status);
}

void app_data_report(void)
{
    uint8_t buf[5] = {0,67,6,5,9};
		//buf[0] = bh_temp;
		//buf[1] = bh_str;
    data_report_t report_data;
    int cnt = 0;

    report_data.buf = buf;
    report_data.callback = ack_callback;
    report_data.cookie = 0;
    report_data.len = sizeof(buf);
    report_data.type = APP_DATA;
    do
    {
        report_data.cookie = cnt;
        cnt++;
        atiny_data_report(g_phandle, &report_data);
        atiny_data_change(g_phandle, DEVICE_MEMORY_FREE);
        sleep(60);
    } while(1);
}

#ifdef CONFIG_FEATURE_FOTA
void agent_tiny_fota_init(void)
{
    atiny_fota_storage_device_s *storage_device = NULL ;
    fota_hardware_s *hardware = NULL;
    fota_pack_device_info_s device_info;

    (void)hal_init_fota();

    (void)hal_get_fota_device(&storage_device, &hardware);

    device_info.hardware = hardware;
    device_info.storage_device = storage_device;
    device_info.head_info_notify  = NULL;
    (void)fota_set_pack_device(fota_get_pack_device(), &device_info);
}
#endif

void agent_tiny_entry(void)
{
    pthread_t tidp;
    atiny_param_t* atiny_params;
    atiny_security_param_t  *iot_security_param = NULL;
    atiny_security_param_t  *bs_security_param = NULL;

    atiny_device_info_t *device_info = &g_device_info;

#ifdef CONFIG_FEATURE_FOTA
    agent_tiny_fota_init();
#endif

#ifdef WITH_DTLS
    device_info->endpoint_name = g_endpoint_name_s;
#else
    device_info->endpoint_name = g_endpoint_name;
#endif

    device_info->manufacturer = "NBB";    // HuaWei OceanConnect

    atiny_params = &g_atiny_params;
    atiny_params->server_params.binding = "UQ";
    //atiny_params->server_params.life_time = LWM2M_LIFE_TIME;
    atiny_params->server_params.life_time = 20;
    atiny_params->server_params.storing_cnt = 0;

    atiny_params->bootstrap_mode = BOOTSTRAP_FACTORY;

    //pay attention: index 0 for iot server, index 1 for bootstrap server.
    iot_security_param = &(atiny_params->security_params[0]);
    bs_security_param = &(atiny_params->security_params[1]);


    iot_security_param->server_ip = DEFAULT_SERVER_IPV4;
    bs_security_param->server_ip = DEFAULT_SERVER_IPV4;

#ifdef WITH_DTLS
    iot_security_param->server_port = "5684";
    bs_security_param->server_port = "5684";

    iot_security_param->psk_Id = g_endpoint_name_iots;
    iot_security_param->psk = (char*)g_psk_iot_value;
    iot_security_param->psk_len = IOT_PSK_VALUE_LENGTH;

    bs_security_param->psk_Id = g_endpoint_name_bs;
    bs_security_param->psk = (char*)g_psk_bs_value;
    bs_security_param->psk_len = BS_PSK_VALUE_LENGTH;
#else
    iot_security_param->server_port = "5683";
    bs_security_param->server_port = "5683";
#endif

    if(ATINY_OK != atiny_init(atiny_params, &g_phandle))
    {
        return;
    }


    if ((pthread_create(&tidp, NULL, app_data_report, NULL)) == -1)
    {
        printf("create error!\n");
        return 1;
    }


    (void)atiny_bind(device_info, g_phandle);
}
