/* main.c - Application main entry point */

/*
 * SPDX-FileCopyrightText: 2017 Intel Corporation
 * SPDX-FileContributor: 2018-2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */


//app specific includes
#include "hardware_control.h"
#include "main.h"
#include "board.h"
//end of app specific includes

#include <inttypes.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_ble_mesh_defs.h"
#include "esp_ble_mesh_common_api.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_ble_mesh_provisioning_api.h"
#include "esp_ble_mesh_config_model_api.h"
#include "esp_ble_mesh_generic_model_api.h"
#include "esp_ble_mesh_local_data_operation_api.h"

#include "ble_mesh_example_init.h"

#define TAG "BOARD"

#define CID_ESP 0x02E5

#define CMD_QUEUE_LEN 2 //number of stacked commands should not be much

QueueHandle_t command_queue; // the variable the stores the commands from BLE

static uint8_t dev_uuid[16] = { 0xdd, 0xdd };

static esp_ble_mesh_cfg_srv_t config_server = {
    .relay = ESP_BLE_MESH_RELAY_DISABLED,
    .beacon = ESP_BLE_MESH_BEACON_ENABLED,
#if defined(CONFIG_BLE_MESH_FRIEND)
    .friend_state = ESP_BLE_MESH_FRIEND_ENABLED,
#else
    .friend_state = ESP_BLE_MESH_FRIEND_NOT_SUPPORTED,
#endif
#if defined(CONFIG_BLE_MESH_GATT_PROXY_SERVER)
    .gatt_proxy = ESP_BLE_MESH_GATT_PROXY_ENABLED,
#else
    .gatt_proxy = ESP_BLE_MESH_GATT_PROXY_NOT_SUPPORTED,
#endif
    .default_ttl = 7,
    /* 3 transmissions with 20ms interval */
    .net_transmit = ESP_BLE_MESH_TRANSMIT(2, 20),
    .relay_retransmit = ESP_BLE_MESH_TRANSMIT(2, 20),
};

ESP_BLE_MESH_MODEL_PUB_DEFINE(onoff_pub_0, 2 + 3, ROLE_NODE);
static esp_ble_mesh_gen_onoff_srv_t onoff_server_0 = {
    .rsp_ctrl.get_auto_rsp = ESP_BLE_MESH_SERVER_AUTO_RSP,
    .rsp_ctrl.set_auto_rsp = ESP_BLE_MESH_SERVER_AUTO_RSP,
};



static esp_ble_mesh_model_t root_models[] = {
    ESP_BLE_MESH_MODEL_CFG_SRV(&config_server),
    ESP_BLE_MESH_MODEL_GEN_ONOFF_SRV(&onoff_pub_0, &onoff_server_0),
};


/*
this section of code is for elements command in BLE APP
*/
static esp_ble_mesh_elem_t elements[] = {
    ESP_BLE_MESH_ELEMENT(0, root_models, ESP_BLE_MESH_MODEL_NONE)
};
/*
end of section
*/
static esp_ble_mesh_comp_t composition = {
    .cid = CID_ESP,
    .elements = elements,
    .element_count = ARRAY_SIZE(elements),
};

/* Disable OOB security for SILabs Android app */
static esp_ble_mesh_prov_t provision = {
    .uuid = dev_uuid,
#if 0
    .output_size = 4,
    .output_actions = ESP_BLE_MESH_DISPLAY_NUMBER,
    .input_actions = ESP_BLE_MESH_PUSH,
    .input_size = 4,
#else
    .output_size = 0,
    .output_actions = 0,
#endif
};

static void prov_complete(uint16_t net_idx, uint16_t addr, uint8_t flags, uint32_t iv_index)
{
    ESP_LOGI(TAG, "net_idx: 0x%04x, addr: 0x%04x", net_idx, addr);
    ESP_LOGI(TAG, "flags: 0x%02x, iv_index: 0x%08" PRIx32, flags, iv_index);
    ESP_LOGI(TAG, "Provision complete");
   
}

static void example_handle_gen_onoff_msg(esp_ble_mesh_model_t *model,
                                         esp_ble_mesh_msg_ctx_t *ctx,
                                         esp_ble_mesh_server_recv_gen_onoff_set_t *set)
{
    esp_ble_mesh_gen_onoff_srv_t *srv = model->user_data;

    switch (ctx->recv_op) {
    case ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_GET:
        esp_ble_mesh_server_model_send_msg(model, ctx,
            ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_STATUS, sizeof(srv->state.onoff), &srv->state.onoff);
            printf("/n");
            printf("state is %s",&srv->state.onoff);
            printf("/n");
        break;
    case ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET:
    case ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK:
        if (set->op_en == false) {
            srv->state.onoff = set->onoff;
        } else {
            /* TODO: Delay and state transition */
            srv->state.onoff = set->onoff;
            printf("/n");
            printf("state is %s",&srv->state.onoff);
            printf("/n");
        }
        if (ctx->recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET) {
            esp_ble_mesh_server_model_send_msg(model, ctx,
                ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_STATUS, sizeof(srv->state.onoff), &srv->state.onoff);
                printf("/n");
            printf("state is %s",&srv->state.onoff);
            printf("/n");
        }
        esp_ble_mesh_model_publish(model, ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_STATUS,
            sizeof(srv->state.onoff), &srv->state.onoff, ROLE_NODE);
            printf("/n");
            printf("state is %s",&srv->state.onoff);
            printf("/n");
        break;
    default:
        break;
    }
}

static void example_ble_mesh_provisioning_cb(esp_ble_mesh_prov_cb_event_t event,
                                             esp_ble_mesh_prov_cb_param_t *param)
{
    switch (event) {
    case ESP_BLE_MESH_PROV_REGISTER_COMP_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_PROV_REGISTER_COMP_EVT, err_code %d", param->prov_register_comp.err_code);
        break;
    case ESP_BLE_MESH_NODE_PROV_ENABLE_COMP_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_NODE_PROV_ENABLE_COMP_EVT, err_code %d", param->node_prov_enable_comp.err_code);
        break;
    case ESP_BLE_MESH_NODE_PROV_LINK_OPEN_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_NODE_PROV_LINK_OPEN_EVT, bearer %s",
            param->node_prov_link_open.bearer == ESP_BLE_MESH_PROV_ADV ? "PB-ADV" : "PB-GATT");
        break;
    case ESP_BLE_MESH_NODE_PROV_LINK_CLOSE_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_NODE_PROV_LINK_CLOSE_EVT, bearer %s",
            param->node_prov_link_close.bearer == ESP_BLE_MESH_PROV_ADV ? "PB-ADV" : "PB-GATT");
        break;
    case ESP_BLE_MESH_NODE_PROV_COMPLETE_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_NODE_PROV_COMPLETE_EVT");
        prov_complete(param->node_prov_complete.net_idx, param->node_prov_complete.addr,
            param->node_prov_complete.flags, param->node_prov_complete.iv_index);
        break;
    case ESP_BLE_MESH_NODE_PROV_RESET_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_NODE_PROV_RESET_EVT");
        break;
    case ESP_BLE_MESH_NODE_SET_UNPROV_DEV_NAME_COMP_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_NODE_SET_UNPROV_DEV_NAME_COMP_EVT, err_code %d", param->node_set_unprov_dev_name_comp.err_code);
        break;
    default:
        break;
    }
}

static void ble_mesh_generic_server_cb(esp_ble_mesh_generic_server_cb_event_t event,
                                               esp_ble_mesh_generic_server_cb_param_t *param)
{
    esp_ble_mesh_gen_onoff_srv_t *srv;
    ESP_LOGI(TAG, "event 0x%02x, opcode 0x%04" PRIx32 ", src 0x%04x, dst 0x%04x",
        event, param->ctx.recv_op, param->ctx.addr, param->ctx.recv_dst);

    switch (event) {
    case ESP_BLE_MESH_GENERIC_SERVER_STATE_CHANGE_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_GENERIC_SERVER_STATE_CHANGE_EVT");
        if (param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET ||
            param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK) {
            ESP_LOGI(TAG, "onoff 0x%02x", param->value.state_change.onoff_set.onoff);
            /*param->value.state_change.onoff_set.onoff is the varibale for on/off switch witch is changed by the command from BLE*/
            char local_command='\0';//stores data received via BLE
            
            //start of switch
            /*in this case the variable "param->value.state_change.onoff_set.onoff" only gets '0' or '1' from the BLE */
            switch(param->value.state_change.onoff_set.onoff)
            {
                case 1:
                    ESP_LOGI("BLE_CMD","up\n");
                    local_command='u';
                    break;
                case 0:
                    ESP_LOGI("BLE_CMD","down\n");
                    local_command='d';
                    break;
                default:
                    ESP_LOGW("BLE_CMD","undefined\n");
                    local_command='\0';
                    break;
            }
            //end of switch
            printf("\n");
            printf("%d\n",param->value.state_change.onoff_set.onoff);
            /*
            section to put local command in queue
            */
            if(local_command=='d'||local_command=='u')
            {
        
                if(xQueueSend(command_queue,(void*)&local_command,50)!=pdTRUE)
                    {
                        //we encounted a queue full error
                        ESP_LOGW(CMD_TAG, "command queue full, try later");
                    }
                else
                    {
                         ESP_LOGI(CMD_TAG, "command will be executed");
                    }
            }
            else
            {
                //it is an undefine command so put '/0' in queue
                char undef_cmd='\0';
                if(xQueueSend(command_queue,(void*)&undef_cmd,50)!=pdTRUE)
                    {
                        //we encounted a queue full error
                        ESP_LOGW(CMD_TAG, "undef cmd could not be registered, try later");
                    }
                else
                    {
                        ESP_LOGW(CMD_TAG, "undef cmd was registered");
                    }
            }
            /*
            end of section to put local command in queue
            */
   
        }   
        break;
    case ESP_BLE_MESH_GENERIC_SERVER_RECV_GET_MSG_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_GENERIC_SERVER_RECV_GET_MSG_EVT");
        if (param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_GET) {
            srv = param->model->user_data;
            ESP_LOGI(TAG, "onoff 0x%02x", srv->state.onoff);
            example_handle_gen_onoff_msg(param->model, &param->ctx, NULL);
        }
        break;
    case ESP_BLE_MESH_GENERIC_SERVER_RECV_SET_MSG_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_GENERIC_SERVER_RECV_SET_MSG_EVT");
        if (param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET ||
            param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK) {
            ESP_LOGI(TAG, "onoff 0x%02x, tid 0x%02x", param->value.set.onoff.onoff, param->value.set.onoff.tid);
            if (param->value.set.onoff.op_en) {
                ESP_LOGI(TAG, "trans_time 0x%02x, delay 0x%02x",
                    param->value.set.onoff.trans_time, param->value.set.onoff.delay);
            }
            example_handle_gen_onoff_msg(param->model, &param->ctx, &param->value.set.onoff);
        }
        break;
    default:
        ESP_LOGE(TAG, "Unknown Generic Server event 0x%02x", event);
        break;
    }
}

static void ble_mesh_config_server_cb(esp_ble_mesh_cfg_server_cb_event_t event,
                                              esp_ble_mesh_cfg_server_cb_param_t *param)
{
    if (event == ESP_BLE_MESH_CFG_SERVER_STATE_CHANGE_EVT) {
        switch (param->ctx.recv_op) {
        case ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD:
            ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD");
            ESP_LOGI(TAG, "net_idx 0x%04x, app_idx 0x%04x",
                param->value.state_change.appkey_add.net_idx,
                param->value.state_change.appkey_add.app_idx);
            ESP_LOG_BUFFER_HEX("AppKey", param->value.state_change.appkey_add.app_key, 16);
            break;
        case ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND:
            ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND");
            ESP_LOGI(TAG, "elem_addr 0x%04x, app_idx 0x%04x, cid 0x%04x, mod_id 0x%04x",
                param->value.state_change.mod_app_bind.element_addr,
                param->value.state_change.mod_app_bind.app_idx,
                param->value.state_change.mod_app_bind.company_id,
                param->value.state_change.mod_app_bind.model_id);
            break;
        case ESP_BLE_MESH_MODEL_OP_MODEL_SUB_ADD:
            ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_MODEL_SUB_ADD");
            ESP_LOGI(TAG, "elem_addr 0x%04x, sub_addr 0x%04x, cid 0x%04x, mod_id 0x%04x",
                param->value.state_change.mod_sub_add.element_addr,
                param->value.state_change.mod_sub_add.sub_addr,
                param->value.state_change.mod_sub_add.company_id,
                param->value.state_change.mod_sub_add.model_id);
            break;
        default:
            break;
        }
    }
}

static esp_err_t ble_mesh_init(void)
{
    esp_err_t err = ESP_OK;

    esp_ble_mesh_register_prov_callback(example_ble_mesh_provisioning_cb);
    esp_ble_mesh_register_config_server_callback(ble_mesh_config_server_cb);
    esp_ble_mesh_register_generic_server_callback(ble_mesh_generic_server_cb);

    err = esp_ble_mesh_init(&provision, &composition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize mesh stack (err %d)", err);
        return err;
    }

    err = esp_ble_mesh_node_prov_enable(ESP_BLE_MESH_PROV_ADV | ESP_BLE_MESH_PROV_GATT);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable mesh node (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "BLE Mesh Node initialized");


    return err;
}

void app_main(void)
{
    //create queue as well
    
    command_queue=xQueueCreate(CMD_QUEUE_LEN,sizeof(char));
    esp_err_t err;

    ESP_LOGI(TAG, "Initializing...");

    board_init();
    

    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    err = bluetooth_init();
    if (err) {
        ESP_LOGE(TAG, "esp32_bluetooth_init failed (err %d)", err);
        return;
    }

    ble_mesh_get_dev_uuid(dev_uuid);

    /* Initialize the Bluetooth Mesh Subsystem */
    err = ble_mesh_init();
    if (err) {
        ESP_LOGE(TAG, "Bluetooth mesh init failed (err %d)", err);
    }

    xTaskCreatePinnedToCore(
    hardware_control_fun, 
    "control task",
    30000,
    NULL,
    1,
    &hardware_control_task,
    my_app_cpu
   );

    while(1){
        vTaskDelay(5000/portTICK_PERIOD_MS);
    }
 
}
