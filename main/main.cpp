/*
MIT License

Copyright (c) 2022 Sukesh Ashok Kumar

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

static const char *TAG = "ESP32-TUX";
#include "main.hpp"
#include "esp_log.h"
#include "mqtt_client.h"

extern lv_obj_t *label_ana_val_list[];
extern lv_obj_t *arc_array_analog[];
extern lv_obj_t *label_di_obj_list[];
extern lv_obj_t *label_do_obj_list[];

extern std::vector<std::string> label_di_values;
extern std::vector<std::string> label_do_values;

extern "C" void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
extern "C" void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        MQTT_CONNEECTED=1;
        // msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
        // ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        // msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
        // ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        // msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
        // ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        MQTT_CONNEECTED=0;
        break;

    case MQTT_EVENT_SUBSCRIBED:
        //ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        //msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        //ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
    {
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        // If the received json data asks for digital input status
        std::string mqtt_message(event->data, event->data_len);
        parse_mqttmessage(mqtt_message.c_str());
        break;
    }
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

// Connect to Mqtthq public broker server 
esp_mqtt_client_handle_t client = NULL;
static void mqtt_app_start(void)
{
    ESP_LOGI(TAG, "STARTING MQTT");
    // esp_mqtt_client_config_t mqttConfig = {
    //   .broker.address.uri = "mqtt://public.mqtthq.com:1883"};
    
    esp_mqtt_client_config_t mqttConfig = {
        .broker = {
            .address = {
                .uri = "mqtt://broker.hivemq.com:1883",
            },
        },
    };

    client = esp_mqtt_client_init(&mqttConfig);
    esp_mqtt_client_register_event(client, (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}

static void set_timezone()
{
    // Update local timezone
    //setenv("TZ", cfg->TimeZone, 1);
    setenv("TZ", CONFIG_TIMEZONE_STRING, 1);
    tzset();    
}

// GEt time from internal RTC and update date/time of the clock
static void update_datetime_ui()
{
    // If we are on another screen where lbl_time is not valid
    //if (!lv_obj_is_valid(lbl_time)) return;

    // date/time format reference => https://cplusplus.com/reference/ctime/strftime/
    time_t now;
    struct tm datetimeinfo;
    time(&now);
    localtime_r(&now, &datetimeinfo);

    // tm_year will be (1970 - 1900) = 70, if not set
    if (datetimeinfo.tm_year < 100) // Time travel not supported :P
    {
        // ESP_LOGD(TAG, "Date/time not set yet [%d]",datetimeinfo.tm_year);    
        return;
    }

    // Send update time to UI with payload using lv_msg
    lv_msg_send(MSG_TIME_CHANGED, &datetimeinfo);
}

static const char* get_id_string(esp_event_base_t base, int32_t id) {
    //if (base == TUX_EVENTS) {
        switch(id) {
            case TUX_EVENT_DATETIME_SET:
                return "TUX_EVENT_DATETIME_SET";
            case TUX_EVENT_OTA_STARTED:
                return "TUX_EVENT_OTA_STARTED";
            case TUX_EVENT_OTA_IN_PROGRESS:
                return "TUX_EVENT_OTA_IN_PROGRESS";
            case TUX_EVENT_OTA_ROLLBACK:
                return "TUX_EVENT_OTA_ROLLBACK";
            case TUX_EVENT_OTA_COMPLETED:
                return "TUX_EVENT_OTA_COMPLETED";
            case TUX_EVENT_OTA_FAILED:
                return "TUX_EVENT_OTA_FAILED";
            case TUX_EVENT_OTA_ABORTED:
                return "TUX_EVENT_OTA_ABORTED";
            case TUX_EVENT_WEATHER_UPDATED:
                return "TUX_EVENT_WEATHER_UPDATED";
            case TUX_EVENT_THEME_CHANGED:
                return "TUX_EVENT_THEME_CHANGED";
            default:
                return "TUX_EVENT_UNKNOWN";        
        }
    //} 
}

// tux event handler
static void tux_event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
    ESP_LOGD(TAG, "tux_event_handler => %s:%s", event_base, get_id_string(event_base, event_id));
    if (event_base != TUX_EVENTS) return;   // bye bye - me not invited :(

    // Handle TUX_EVENTS
    if (event_id == TUX_EVENT_DATETIME_SET) {

        set_timezone();

        // update clock
        update_datetime_ui();

        // Enable timer after the date/time is set.
        lv_timer_ready(timer_weather); 

    } else if (event_id == TUX_EVENT_OTA_STARTED) {
        // OTA Started
        char buffer[150] = {0};
        snprintf(buffer,sizeof(buffer),"OTA: %s",(char*)event_data);
        lv_msg_send(MSG_OTA_STATUS,buffer);

    } else if (event_id == TUX_EVENT_OTA_IN_PROGRESS) {
        // OTA In Progress - progressbar?
        char buffer[150] = {0};
        int bytes_read = (*(int *)event_data)/1024;
        snprintf(buffer,sizeof(buffer),"OTA: Data read : %dkb", bytes_read);
        lv_msg_send(MSG_OTA_STATUS,buffer);

    } else if (event_id == TUX_EVENT_OTA_ROLLBACK) {
        // OTA Rollback - god knows why!
        char buffer[150] = {0};
        snprintf(buffer,sizeof(buffer),"OTA: %s", (char*)event_data);
        lv_msg_send(MSG_OTA_STATUS,buffer);

    } else if (event_id == TUX_EVENT_OTA_COMPLETED) {
        // OTA Completed - YAY! Success
        char buffer[150] = {0};
        snprintf(buffer,sizeof(buffer),"OTA: %s", (char*)event_data);
        lv_msg_send(MSG_OTA_STATUS,buffer);

        // wait before reboot
        vTaskDelay(3000 / portTICK_PERIOD_MS);

    } else if (event_id == TUX_EVENT_OTA_ABORTED) {
        // OTA Aborted - Not a good day for updates
        char buffer[150] = {0};
        snprintf(buffer,sizeof(buffer),"OTA: %s", (char*)event_data);
        lv_msg_send(MSG_OTA_STATUS,buffer);

    } else if (event_id == TUX_EVENT_OTA_FAILED) {
        // OTA Failed - huh! - maybe in red color?
        char buffer[150] = {0};
        snprintf(buffer,sizeof(buffer),"OTA: %s", (char*)event_data);
        lv_msg_send(MSG_OTA_STATUS,buffer);

    } else if (event_id == TUX_EVENT_WEATHER_UPDATED) {
        // Weather updates - summer?

    } else if (event_id == TUX_EVENT_THEME_CHANGED) {
        // Theme changes - time to play with dark & light
    }
}                          

// Wifi & IP related event handler
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
    //ESP_LOGD(TAG, "%s:%s: wifi_event_handler", event_base, get_id_string(event_base, event_id));
    if (event_base == WIFI_EVENT  && event_id == WIFI_EVENT_STA_CONNECTED)
    {
        is_wifi_connected = true;
        lv_timer_ready(timer_datetime);   // start timer

        // After OTA device restart, RTC will have time but not timezone
        set_timezone();

        // Not a warning but just for highlight
        ESP_LOGW(TAG,"WIFI_EVENT_STA_CONNECTED");
        lv_msg_send(MSG_WIFI_CONNECTED,NULL);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        is_wifi_connected = false;        
        lv_timer_pause(timer_datetime);   // stop/pause timer

        ESP_LOGW(TAG,"WIFI_EVENT_STA_DISCONNECTED");
        lv_msg_send(MSG_WIFI_DISCONNECTED,NULL);
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        is_wifi_connected = true;
        lv_timer_ready(timer_datetime);   // start timer

        ESP_LOGW(TAG,"IP_EVENT_STA_GOT_IP");
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;

        snprintf(ip_payload,sizeof(ip_payload),"%d.%d.%d.%d", IP2STR(&event->ip_info.ip));
        
        // We got IP, lets update time from SNTP. RTC keeps time unless powered off
        xTaskCreate(configure_time, "config_time", 1024*4, NULL, 3, NULL);

        mqtt_app_start();

    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_LOST_IP)
    {
        is_wifi_connected = false;
        ESP_LOGW(TAG,"IP_EVENT_STA_LOST_IP");
    }
    else if (event_base == WIFI_PROV_EVENT && event_id == WIFI_PROV_START) {
        ESP_LOGW(TAG,"WIFI_PROV_START");
    }
    else if (event_base == WIFI_PROV_EVENT && event_id == WIFI_PROV_CRED_RECV) {
        ESP_LOGW(TAG,"WIFI_PROV_CRED_RECV");
    }
    else if (event_base == WIFI_PROV_EVENT && event_id == WIFI_PROV_CRED_FAIL) {
        ESP_LOGW(TAG,"WIFI_PROV_CRED_FAIL");
    }
    else if (event_base == WIFI_PROV_EVENT && event_id == WIFI_PROV_CRED_SUCCESS) {
        ESP_LOGW(TAG,"WIFI_PROV_CRED_SUCCESS");
        // FIXME Refresh IP details once provision is successfull
    }
    else if (event_base == WIFI_PROV_EVENT && event_id == WIFI_PROV_END) {
        ESP_LOGW(TAG,"WIFI_PROV_END");
    }
    else if (event_base == WIFI_PROV_EVENT && event_id == WIFI_PROV_SHOWQR) {
        ESP_LOGW(TAG,"WIFI_PROV_SHOWQR");
        strcpy(qr_payload,(char*)event_data);   // Add qr payload to the variable
    }
}

extern "C" void app_main(void)
{
    esp_log_level_set(TAG, ESP_LOG_DEBUG);      // enable DEBUG logs for this App
    //esp_log_level_set("SettingsConfig", ESP_LOG_DEBUG);    
    esp_log_level_set("wifi", ESP_LOG_WARN);    // enable WARN logs from WiFi stack

    // Print device info
    ESP_LOGE(TAG,"\n%s",device_info().c_str());

    //Initialize NVS
    esp_err_t err = nvs_flash_init();

    // NVS partition contains new data format or unable to access
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err); 

    // Init SPIFF - needed for lvgl images
    init_spiff();

#ifdef SD_SUPPORTED
    // Initializing SDSPI 
    if (init_sdspi() == ESP_OK) // SD SPI
    {
        is_sdcard_enabled = true;
    }
#endif   
//********************** CONFIG HELPER TESTING STARTS

     //cfg = new SettingsConfig("/sdcard/settings.json");    // yet to test
    cfg = new SettingsConfig("/spiffs/settings.json");
    // Save settings
    cfg->save_config();   // save default loaded settings
    // Load values
    cfg->load_config();
    // Change device name
    cfg->DeviceName = "FessiT IO Gateway";
    cfg->WeatherAPIkey = CONFIG_WEATHER_API_KEY;
    cfg->WeatherLocation = CONFIG_WEATHER_LOCATION;
    cfg->WeatherProvider = CONFIG_WEATHER_OWM_URL;
    // Change brightness
    cfg->Brightness=250;
    // Save settings again
    cfg->save_config();
    cfg->load_config();

//******************************************** 
    owm = new OpenWeatherMap();
//********************** CONFIG HELPER TESTING ENDS

    lcd.init();         // Initialize LovyanGFX
    lcd.initDMA();      // Init DMA
    lv_init();          // Initialize lvgl

    if (lv_display_init() != ESP_OK) // Configure LVGL
    {
        ESP_LOGE(TAG, "LVGL setup failed!!!");
    }

    // /* Initialize the event loop */
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* Register for event handler for Wi-Fi, IP related events */
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));
    
    /* Events related to provisioning */
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));

    // TUX EVENTS
    ESP_ERROR_CHECK(esp_event_handler_instance_register(TUX_EVENTS, ESP_EVENT_ANY_ID, tux_event_handler, NULL, NULL));

    // LV_FS integration & print readme.txt from the root for testing
    lv_print_readme_txt("F:/readme.txt");   // SPIFF / FAT
    lv_print_readme_txt("S:/readme.txt");   // SDCARD

/* Push LVGL/UI to its own UI task later*/
    // Splash screen
    lvgl_acquire();
    create_splash_screen();
    lvgl_release();


    // Initialize the network interface which is required to run
    // any transport protocols such as MQTT. This is prerequisites
    esp_err_t ret = esp_netif_init();
    assert(ret == ESP_OK);

    // Main UI
    lvgl_acquire();
    lv_setup_styles();    
    show_ui();
    lvgl_release();
/* Push these to its own UI task later*/

#ifdef SD_SUPPORTED
    // Icon status color update
    lv_msg_send(MSG_SDCARD_STATUS,&is_sdcard_enabled);
#endif

    // Wifi Provision and connection.
    // Use idf.py menuconfig to configure 
    // Use SoftAP only / BLE has some issues
    // Tuning PSRAM options visible only in IDF5, so will wait till then for BLE.
    xTaskCreate(provision_wifi, "wifi_prov", 1024*8, NULL, 3, NULL);

    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());

    // Date/Time update timer - once per sec
    timer_datetime = lv_timer_create(timer_datetime_callback, 1000,  NULL);
    //lv_timer_pause(timer_datetime); // enable only when wifi is connected

    // Weather update timer - Once per min (60*1000) or maybe once in 10 mins (10*60*1000)
    timer_weather = lv_timer_create(timer_weather_callback, WEATHER_UPDATE_INTERVAL,  NULL);

    //lv_timer_set_repeat_count(timer_weather,1);
    //lv_timer_pause(timer_weather);  // enable after wifi is connected

    // Subscribe to page change events in the UI
    /* SPELLING MISTAKE IN API BUG => https://github.com/lvgl/lvgl/issues/3822 */
    lv_msg_subsribe(MSG_PAGE_HOME, tux_ui_change_cb, NULL);
    lv_msg_subsribe(MSG_PAGE_REMOTE, tux_ui_change_cb, NULL);
    lv_msg_subsribe(MSG_PAGE_SETTINGS, tux_ui_change_cb, NULL);
    lv_msg_subsribe(MSG_PAGE_OTA, tux_ui_change_cb, NULL);
    lv_msg_subsribe(MSG_OTA_INITIATE, tux_ui_change_cb, NULL);    // Initiate OTA

    lv_msg_subsribe(MSG_PAGE_ANALOG_VAL, tux_analog_change_cb, NULL);    // Initiate OTA
    lv_msg_subsribe(MSG_PAGE_DI_VAL, tux_di_change_cb, NULL);
    lv_msg_subsribe(MSG_PAGE_DO_VAL, tux_do_change_cb, NULL);

    // Date/Time update timer - once per sec
    //timer_digitalio_getstatus = lv_timer_create(digitalio_getstatus_callback, 1000,  NULL);

    // Create freertos job to publish mqtt message 
    xTaskCreate(publish_digital_input_get_status, "Publisher_Task", 1024 * 5, NULL, 5, NULL);

    timer_update = lv_timer_create(update_realtime_values_hmi, 5000,  NULL);
}

// Publish message on MQTT topic
static void publish_digital_input_get_status(void *params)
{
  while (true)
  {
    if(MQTT_CONNEECTED)
    {
        // Create a JSON object
        cJSON *jRequest = cJSON_CreateObject();
        if (!jRequest)
        {
            ESP_LOGE("MQTT", "Failed to create JSON object");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            continue;
        }
        // Add the request type to the JSON object
        cJSON_AddStringToObject(jRequest, "requestType", "DIGetStatus");
        // Convert the JSON object to a string
        char *json_string = cJSON_PrintUnformatted(jRequest);
        if (!json_string)
        {
            ESP_LOGE("MQTT", "Failed to print JSON object as string");
            cJSON_Delete(jRequest);
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            continue;
        }

        // Log the JSON string before publishing
        ESP_LOGI("MQTT", "Publishing JSON: %s", json_string);

        // Publish the JSON string to the MQTT topic
        int msg_id = esp_mqtt_client_publish(client, "/topic/qos0", json_string, 0, 0, 0);
        if (msg_id == -1)
        {
            ESP_LOGE("MQTT", "Failed to publish message");
        } 
        else 
        {
            ESP_LOGI("MQTT", "Published message with ID: %d", msg_id);
        }

        // Free the allocated JSON string and JSON object
        cJSON_free(json_string);
        cJSON_Delete(jRequest);
    }
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}

static void digitalio_getstatus_callback(lv_timer_t * timer)
{
    // Publish a message to get the status of the digital io

}

static void timer_datetime_callback(lv_timer_t * timer)
{
    // Battery icon animation
    if (battery_value>100) battery_value=0;
    battery_value+=10;
    
    lv_msg_send(MSG_BATTERY_STATUS,&battery_value);
    update_datetime_ui();
}

static void timer_weather_callback(lv_timer_t * timer)
{
    if (cfg->WeatherAPIkey.empty()) {   // If API key not defined skip weather update
        ESP_LOGW(TAG,"Weather API Key not set");
        return;
    }

    // Update weather and trigger UI update
    owm->request_weather_update();
    lv_msg_send(MSG_WEATHER_CHANGED, owm);
}

// Callback to notify App UI change
static void tux_ui_change_cb(void * s, lv_msg_t *m)
{
    LV_UNUSED(s);
    unsigned int page_id = lv_msg_get_id(m);
    const char * msg_payload = (const char *)lv_msg_get_payload(m);
    const char * msg_data = (const char *)lv_msg_get_user_data(m);

    //ESP_LOGW(TAG,"[%" PRIu32 "] page event triggered",page_id);

    switch (page_id)
    {
        case MSG_PAGE_HOME:
            UPDATE_TASK_STATE = DATA_UPDATE_NONE;
            // Update date/time and current weather
            // Date/time is updated every second anyway
            lv_msg_send(MSG_WEATHER_CHANGED, owm);
            break;
        case MSG_PAGE_REMOTE:
            // Trigger loading buttons data
            UPDATE_TASK_STATE = DATA_UPDATE_NONE;
            break;
        case MSG_PAGE_SETTINGS:
            UPDATE_TASK_STATE = DATA_UPDATE_NONE;
            if (!is_wifi_connected)  {// Provisioning mode
                lv_msg_send(MSG_WIFI_PROV_MODE, qr_payload);
                //lv_msg_send(MSG_QRCODE_CHANGED, qr_payload);
            } else {
                lv_msg_send(MSG_WIFI_CONNECTED, ip_payload);
            }
            break;
        case MSG_PAGE_OTA:
            UPDATE_TASK_STATE = DATA_UPDATE_NONE;
            // Update firmware current version info
            lv_msg_send(MSG_DEVICE_INFO,device_info().c_str());
            break;
        case MSG_OTA_INITIATE:
            UPDATE_TASK_STATE = DATA_UPDATE_NONE;
            // OTA update from button trigger
            xTaskCreate(run_ota_task, "run_ota_task", 1024 * 8, NULL, 5, NULL);
            break;
    }
}

static string device_info()
{
    std::string s_chip_info = "";

    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);

    // CPU Speed - 80Mhz / 160 Mhz / 240Mhz
    rtc_cpu_freq_config_t conf;
    rtc_clk_cpu_freq_get_config(&conf);

    multi_heap_info_t info;    
	heap_caps_get_info(&info, MALLOC_CAP_SPIRAM);
    float psramsize = (info.total_free_bytes + info.total_allocated_bytes) / (1024.0 * 1024.0);

    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_app_desc_t running_app_info;
    
    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
        s_chip_info += fmt::format("Firmware Ver : {}\n",running_app_info.version);
        s_chip_info += fmt::format("Project Name : {}\n",running_app_info.project_name);
        // running_app_info.time
        // running_app_info.date
    }
    s_chip_info += fmt::format("IDF Version  : {}\n\n",esp_get_idf_version());

    s_chip_info += fmt::format("Controller   : {} Rev.{}\n",CONFIG_IDF_TARGET,chip_info.revision);  
    //s_chip_info += fmt::format("\nModel         : {}",chip_info.model); // esp_chip_model_t type
    s_chip_info += fmt::format("CPU Cores    : {}\n", (chip_info.cores==2)? "Dual Core" : "Single Core");
    s_chip_info += fmt::format("CPU Speed    : {}Mhz\n",conf.freq_mhz);
    if(esp_flash_get_size(NULL, &flash_size) == ESP_OK) {
    s_chip_info += fmt::format("Flash Size   : {}MB {}\n",flash_size / (1024 * 1024),
                                            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "[embedded]" : "[external]");
    }
    s_chip_info += fmt::format("PSRAM Size   : {}MB {}\n",static_cast<int>(round(psramsize)),
                                            (chip_info.features & CHIP_FEATURE_EMB_PSRAM) ? "[embedded]" : "[external]");

    s_chip_info += fmt::format("Connectivity : {}{}{}\n",(chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "2.4GHz WIFI" : "NA",
                                                    (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
                                                    (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");
    //s_chip_info += fmt::format("\nIEEE 802.15.4 : {}",string((chip_info.features & CHIP_FEATURE_IEEE802154) ? "YES" : "NA"));

    //ESP_LOGE(TAG,"\n%s",device_info().c_str());
    return s_chip_info;
}

static void parse_mqttmessage(const char* jsondata)
{
    printf("DATA FOR PARSING=%s\r\n", jsondata);
    // read json file to string    
    cJSON *jsonMQTTReqData = cJSON_Parse(jsondata);
     // Get root element item
    std::string RequestType = cJSON_GetObjectItem(jsonMQTTReqData,"requestType")->valuestring;
    ESP_LOGI(TAG,"Data:\n%s and request type: %s ", jsondata, RequestType.c_str());
    // Cleanup
    cJSON_Delete(jsonMQTTReqData);

    if(RequestType == "DIGetStatus")
    {
        publish_DIGetStatus();
    }
}

static void publish_DIGetStatus()
{
    if(MQTT_CONNEECTED)
    {
        // Create a JSON object
        cJSON *jRequest = cJSON_CreateObject();
        if (!jRequest)
        {
            ESP_LOGE("MQTT", "Failed to create JSON object");
            return;
        }
        // Add the request type to the JSON object
        cJSON_AddStringToObject(jRequest, "responseType", "DIRespStatus");

        // Add 4 digital input statuses
        cJSON *digitalInputs = cJSON_CreateObject();
        if (digitalInputs == NULL) {
            ESP_LOGE("MQTT", "Failed to create digital input JSON object");
            cJSON_Delete(jRequest);
            return;
        }

        cJSON_AddBoolToObject(digitalInputs, "DI1", true);  // Example: Digital Input 1 is ON
        cJSON_AddBoolToObject(digitalInputs, "DI2", false); // Example: Digital Input 2 is OFF
        cJSON_AddBoolToObject(digitalInputs, "DI3", true);  // Example: Digital Input 3 is ON
        cJSON_AddBoolToObject(digitalInputs, "DI4", false); // Example: Digital Input 4 is OFF

        // Add the digital input statuses object to the main JSON
        cJSON_AddItemToObject(jRequest, "digitalInputs", digitalInputs);

        // Convert the JSON object to a string
        char *json_string = cJSON_PrintUnformatted(jRequest);
        if (!json_string)
        {
            ESP_LOGE("MQTT", "Failed to print JSON object as string");
            cJSON_Delete(jRequest);
            return;
        }

        // Log the JSON string before publishing
        ESP_LOGI("MQTT", "Publishing response JSON: %s", json_string);

        // Publish the JSON string to the MQTT topic
        int msg_id = esp_mqtt_client_publish(client, "/topic/qos1", json_string, 0, 0, 0);
        if (msg_id == -1)
        {
            ESP_LOGE("MQTT", "Failed to publish message");
        } 
        else 
        {
            ESP_LOGI("MQTT", "Published message with ID: %d", msg_id);
        }

        // Free the allocated JSON string and JSON object
        cJSON_free(json_string);
        cJSON_Delete(jRequest);
    }
}

// Callback to notify App UI change
static void tux_analog_change_cb(void * s, lv_msg_t *m)
{
    LV_UNUSED(s);
    unsigned int msg_id = lv_msg_get_id(m);
    ESP_LOGI("ANALOG", "message with ID: %d", msg_id);

    char new_value[16];
    int new_analog_value;
    // Example: Use msg_id to determine which label to update
    for (int i=0; i < ANALOG_LABEL_COUNT; i++) {
        // Simulate getting a new value for the label
        new_analog_value = rand() % 100;  // Replace with real analog value
        snprintf(new_value, sizeof(new_value), "V.%d", new_analog_value);

        // Update the respective label
        lv_label_set_text(label_ana_val_list[i], new_value);
    }
    UPDATE_TASK_STATE = ANALOG_DATA_UPDATE;
}

static void update_realtime_values_hmi(lv_timer_t * timer)
{
    switch(UPDATE_TASK_STATE)
    {
        case ANALOG_DATA_UPDATE:
        {
            int new_analog_value;
            char new_value[16];
            // Example: Use msg_id to determine which label to update
            for (int i=0; i < ANALOG_LABEL_COUNT; i++) {
                // Simulate getting a new value for the label
                new_analog_value = rand() % 100;  // Replace with real analog value
                snprintf(new_value, sizeof(new_value), "V.%d", new_analog_value);

                // Update the respective label
                lv_label_set_text(label_ana_val_list[i], new_value);

                lv_arc_set_mode(arc_array_analog[i], LV_ARC_MODE_NORMAL);  // Ensure single indicator
                lv_arc_set_value(arc_array_analog[i], new_analog_value);
            }
            break;
        }
        case DI_DATA_UPDATE:
        {
            int new_di_value;
            char new_value[16];
            // Example: Use msg_id to determine which label to update
            for (int i=0; i < ANALOG_LABEL_COUNT; i++) {
                // Simulate getting a new value for the label
                new_di_value = rand() % 100;  // Replace with real analog value
                if(new_di_value < 50)
                {
                    snprintf(new_value, sizeof(new_value), "OFF");
                    label_di_values.at(i) = std::string(new_value);
                    // Update the respective label
                    lv_label_set_text(label_di_obj_list[i], new_value);
                    lv_obj_set_style_text_color(label_di_obj_list[i], lv_color_hex(0xb53e02), 0);  // Black text color
                    lv_obj_set_style_border_color(label_di_circle_list[i], lv_color_hex(0xb53e02), 0);
                }
                else
                {
                    snprintf(new_value, sizeof(new_value), "ON");
                    label_di_values.at(i) = std::string(new_value);
                    // Update the respective label
                    lv_label_set_text(label_di_obj_list[i], new_value);
                    lv_obj_set_style_text_color(label_di_obj_list[i], lv_color_hex(0x307A34), 0);  // Black text color
                    lv_obj_set_style_border_color(label_di_circle_list[i], lv_color_hex(0x578133), 0);
                }
            }
            break;
        }
        case DO_DATA_UPDATE:
        {
            int new_di_value;
            char new_value[16];
            // Example: Use msg_id to determine which label to update
            for (int i=0; i < ANALOG_LABEL_COUNT; i++) {
                // Simulate getting a new value for the label
                new_di_value = rand() % 100;  // Replace with real analog value
                if(new_di_value < 50)
                {
                    snprintf(new_value, sizeof(new_value), "OFF");
                    label_do_values.at(i) = std::string(new_value);
                    // Update the respective label
                    lv_label_set_text(label_do_obj_list[i], new_value);
                    lv_obj_set_style_text_color(label_do_obj_list[i], lv_color_hex(0xb53e02), 0);  // Black text color
                    lv_obj_set_style_border_color(label_do_circle_list[i], lv_color_hex(0xb53e02), 0);
                }
                else
                {
                    snprintf(new_value, sizeof(new_value), "ON");
                    label_do_values.at(i) = std::string(new_value);
                    // Update the respective label
                    lv_label_set_text(label_do_obj_list[i], new_value);
                    lv_obj_set_style_text_color(label_do_obj_list[i], lv_color_hex(0x307A34), 0);  // Black text color
                    lv_obj_set_style_border_color(label_do_circle_list[i], lv_color_hex(0x578133), 0);
                }
            }
            break;
        }
        default:
            break;
    }
}

static void tux_di_change_cb(void * s, lv_msg_t *m)
{
    LV_UNUSED(s);
    unsigned int msg_id = lv_msg_get_id(m);
    ESP_LOGI("DI", "message with ID: %d", msg_id);

    UPDATE_TASK_STATE = DI_DATA_UPDATE;
}

static void tux_do_change_cb(void * s, lv_msg_t *m)
{
    LV_UNUSED(s);
    unsigned int msg_id = lv_msg_get_id(m);
    ESP_LOGI("DO", "message with ID: %d", msg_id);

    UPDATE_TASK_STATE = DO_DATA_UPDATE;
}