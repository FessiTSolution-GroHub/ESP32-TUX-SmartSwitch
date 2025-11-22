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

#include "ota.h"
#include "widgets/tux_panel.h"
#include <fmt/core.h>
#include <fmt/format.h>
#include "OpenWeatherMap.hpp"
#include "apps/weather/weathericons.h"
#include "events/gui_events.hpp"
#include <esp_partition.h>
#include <vector>


LV_IMG_DECLARE(dev_bg)
//LV_IMG_DECLARE(tux_logo)

// LV_FONT_DECLARE(font_7seg_64)
// LV_FONT_DECLARE(font_7seg_60)
// LV_FONT_DECLARE(font_7seg_58)
LV_FONT_DECLARE(font_7seg_56)

//LV_FONT_DECLARE(font_robotomono_12)
LV_FONT_DECLARE(font_robotomono_13)

LV_FONT_DECLARE(font_fa_14)
#define FA_SYMBOL_BLE "\xEF\x8A\x94"      // 0xf294
#define FA_SYMBOL_SETTINGS "\xEF\x80\x93" // 0xf0ad

/*********************
 *      DEFINES
 *********************/
#define NUM_CIRCLES 4 // Number of circles

#define HEADER_HEIGHT 30
#define FOOTER_HEIGHT 30
#define ANALOG_LABEL_COUNT 4


extern uint32_t UPDATE_TASK_STATE;
/******************
 *  LV DEFINES
 ******************/
static lv_obj_t *digital_input_circles[NUM_CIRCLES]; // Array to store circle objects
static lv_obj_t *digital_output_circles[NUM_CIRCLES]; // Array to store circle objects
static lv_obj_t *analog_input_icons[NUM_CIRCLES]; // Array to store circle objects

#define MAX_ARCS 4  // Maximum number of arcs you want to store
lv_obj_t* arc_array_analog[MAX_ARCS];  // Array to store arc pointers

std::vector<std::string> label_footer_text_di;
std::vector<std::string> label_footer_text_do;
std::vector<std::string> label_footer_text_ana;
std::vector<std::string> label_ana_values;
std::vector<std::string> label_di_values;

std::vector<std::string> label_do_values;
//std::vector<std::int> label_do_values;

lv_obj_t *label_ana_val_list[ANALOG_LABEL_COUNT];
lv_obj_t *label_di_obj_list[ANALOG_LABEL_COUNT];
lv_obj_t *label_do_obj_list[ANALOG_LABEL_COUNT];
lv_obj_t *label_di_circle_list[ANALOG_LABEL_COUNT];
lv_obj_t *label_do_circle_list[ANALOG_LABEL_COUNT];

// Global variable to reference the keyboard
static lv_obj_t *keyboard;
static lv_obj_t *keyboard_text_area;
static lv_obj_t *text_area_settings;
static lv_obj_t *parent_di_settings;
static lv_obj_t *button_fullview_parent;
static lv_obj_t *tux_settings_title;
static lv_obj_t *tux_settings_panel;

static lv_coord_t which_button_di_settings;
static lv_coord_t which_button_do_settings;
static lv_coord_t which_button_ana_settings;

static const lv_font_t *font_large;
static const lv_font_t *font_normal;
static const lv_font_t *font_symbol;
static const lv_font_t *font_fa;
static const lv_font_t *font_xl;
static const lv_font_t *font_xxl;

static lv_obj_t *panel_header;
static lv_obj_t *panel_title;
static lv_obj_t *panel_status; // Status icons in the header
static lv_obj_t *content_container;
static lv_obj_t *screen_container;
static lv_obj_t *qr_status_container;

// TUX Panels
static lv_obj_t *tux_clock_weather;
static lv_obj_t *tux_digital_input;
static lv_obj_t *tux_digital_output;
static lv_obj_t *tux_analog_input;

// GOUS
static lv_obj_t *tux_di_fullview;
static lv_obj_t *tux_di_settings;
static lv_obj_t *label_footer;

static lv_obj_t *island_wifi;
static lv_obj_t *island_ota;
static lv_obj_t *island_devinfo;
static lv_obj_t *prov_qr;

static lv_obj_t *label_title;
static lv_obj_t *label_message;
static lv_obj_t *lbl_version;
static lv_obj_t *lbl_update_status;
static lv_obj_t *lbl_scan_status;

static lv_obj_t *lbl_device_info;

static lv_obj_t *icon_storage;
static lv_obj_t *icon_wifi;
static lv_obj_t *icon_ble;
static lv_obj_t *icon_battery;

/* Date/Time */
static lv_obj_t *lbl_time;
static lv_obj_t *lbl_ampm;
static lv_obj_t *lbl_date;

static lv_obj_t *lbl_dayofweek;
static lv_obj_t *lbl_dayofmonth;
static lv_obj_t *lbl_curryear;

static lv_obj_t *lbl_digitalinput;
static lv_obj_t *lbl_digitaloutput;
static lv_obj_t *lbl_analoginput;

/* Weather */
static lv_obj_t *lbl_weathericon;
static lv_obj_t *lbl_temp;
static lv_obj_t *lbl_hl;

static lv_obj_t *lbl_wifi_status;

static lv_coord_t screen_h;
static lv_coord_t screen_w;

/******************
 *  LVL STYLES
 ******************/
static lv_style_t style_content_bg;

static lv_style_t style_message;
static lv_style_t style_title;
static lv_style_t style_iconstatus;
static lv_style_t style_storage;
static lv_style_t style_wifi;
static lv_style_t style_ble;
static lv_style_t style_battery;

static lv_style_t style_ui_island;

static lv_style_t style_glow;

/******************
 *  LVL ANIMATION
 ******************/
static lv_anim_t anim_labelscroll;

void anim_move_left_x(lv_obj_t * TargetObject, int start_x, int end_x, int delay);
void tux_anim_callback_set_x(lv_anim_t * a, int32_t v);

void anim_move_left_y(lv_obj_t * TargetObject, int start_y, int end_y, int delay);
void tux_anim_callback_set_y(lv_anim_t * a, int32_t v);

void anim_fade_in(lv_obj_t * TargetObject, int delay);
void anim_fade_out(lv_obj_t * TargetObject, int delay);

void tux_anim_callback_set_opacity(lv_anim_t * a, int32_t v);
/******************
 *  LVL FUNCS & EVENTS
 ******************/
static void create_page_home(lv_obj_t *parent);
static void create_page_settings(lv_obj_t *parent);
static void create_page_updates(lv_obj_t *parent);
static void create_page_remote(lv_obj_t *parent);

static void create_page_digitalinput(lv_obj_t *parent);
static void tux_panel_di_fullview(lv_obj_t *parent);
static void button_click_event_handler(lv_event_t *e);
static void circle_event_handler(lv_event_t *e);
static void ok_button_event_handler(lv_event_t *e);
static void tux_panel_di_settings(lv_obj_t *parent, int button_id);
static void text_area_event_handler(lv_event_t *e);
static void keyboard_event_handler(lv_event_t *e);

static void create_page_digitaloutput(lv_obj_t *parent);
static void tux_panel_do_fullview(lv_obj_t *parent);
static void button_outp_click_event_handler(lv_event_t *e);
static void tux_panel_do_settings(lv_obj_t *parent, int button_id);
static void ok_button_outp_event_handler(lv_event_t *e);

static void create_page_analog(lv_obj_t *parent);
static void tux_panel_ana_fullview(lv_obj_t *parent);
static void button_ana_click_event_handler(lv_event_t *e);
static void tux_panel_ana_settings(lv_obj_t *parent, int button_id);
static void ok_button_ana_event_handler(lv_event_t *e);

// Home page islands
static void tux_panel_clock_weather(lv_obj_t *parent);
static void tux_panel_digital_input(lv_obj_t *parent);
static void tux_panel_digital_output(lv_obj_t *parent);
static void tux_panel_analog_input(lv_obj_t *parent);
static void tux_panel_config(lv_obj_t *parent);

// Setting page islands
static void tux_panel_devinfo(lv_obj_t *parent);
static void tux_panel_ota(lv_obj_t *parent);
static void tux_panel_wifi(lv_obj_t *parent);

static void create_header(lv_obj_t *parent);
static void create_footer(lv_obj_t *parent);

static void footer_message(const char *fmt, ...);
static void create_splash_screen();
static void switch_theme(bool dark);
static void qrcode_ui(lv_obj_t *parent);
static void show_ui();

static const char* get_firmware_version();

static void rotate_event_handler(lv_event_t *e);
static void theme_switch_event_handler(lv_event_t *e);
static void espwifi_event_handler(lv_event_t* e);
//static void espble_event_handler(lv_event_t *e);
static void checkupdates_event_handler(lv_event_t *e);
static void home_clicked_eventhandler(lv_event_t *e);
static void status_clicked_eventhandler(lv_event_t *e);
static void footer_button_event_handler(lv_event_t *e);
static void panel_di_event_handler(lv_event_t * e);
static void goback_di_event_handler(lv_event_t * e);

static void panel_do_event_handler(lv_event_t * e);
static void panel_ana_event_handler(lv_event_t * e);

// static void new_theme_apply_cb(lv_theme_t * th, lv_obj_t * obj);

/* MSG Events */
void datetime_event_cb(lv_event_t * e);
void weather_event_cb(lv_event_t * e);

static void status_change_cb(void * s, lv_msg_t *m);
static void lv_update_battery(uint batval);
static void set_weather_icon(string weatherIcon);

static int current_page = 0;

void lv_setup_styles()
{
    font_symbol = &lv_font_montserrat_14;
    font_normal = &lv_font_montserrat_14;
    font_large = &lv_font_montserrat_16;
    font_xl = &lv_font_montserrat_24;
    font_xxl = &lv_font_montserrat_32;
    font_fa = &font_fa_14;

    screen_h = lv_obj_get_height(lv_scr_act());
    screen_w = lv_obj_get_width(lv_scr_act());

    /* CONTENT CONTAINER BACKGROUND */
    lv_style_init(&style_content_bg);
    lv_style_set_bg_opa(&style_content_bg, LV_OPA_50);
    lv_style_set_radius(&style_content_bg, 0);

// Enabling wallpaper image slows down scrolling perf etc...
#if defined(CONFIG_WALLPAPER_IMAGE)
    // Image Background
    // CF_INDEXED_8_BIT for smaller size - resolution 480x480
    // NOTE: Dynamic loading bg from SPIFF makes screen perf bad
    if (lv_fs_is_ready('F')) { // NO SD CARD load default
        ESP_LOGW(TAG,"Loading - F:/bg/dev_bg9.bin");
        lv_style_set_bg_img_src(&style_content_bg, "F:/bg/dev_bg9.bin");    
    } else {
        ESP_LOGW(TAG,"Loading - from firmware");
        lv_style_set_bg_img_src(&style_content_bg, &dev_bg);
    }
    //lv_style_set_bg_img_src(&style_content_bg, &dev_bg);
    // lv_style_set_bg_img_opa(&style_content_bg,LV_OPA_50);
#else
    ESP_LOGW(TAG,"Using Gradient");
    // Gradient Background
    static lv_grad_dsc_t grad;
    grad.dir = LV_GRAD_DIR_VER;
    grad.stops_count = 2;
    grad.stops[0].color = lv_color_make(31,32,34) ;
    grad.stops[1].color = lv_palette_main(LV_PALETTE_BLUE);
    grad.stops[0].frac = 150;
    grad.stops[1].frac = 190;
    lv_style_set_bg_grad(&style_content_bg, &grad);
#endif

    // DASHBOARD TITLE
    lv_style_init(&style_title);
    lv_style_set_text_font(&style_title, font_large);
    lv_style_set_align(&style_title, LV_ALIGN_LEFT_MID);
    lv_style_set_pad_left(&style_title, 15);
    lv_style_set_border_width(&style_title, 0);
    lv_style_set_size(&style_title, LV_SIZE_CONTENT);

    // HEADER STATUS ICON PANEL
    lv_style_init(&style_iconstatus);
    lv_style_set_size(&style_iconstatus, LV_SIZE_CONTENT);
    lv_style_set_pad_all(&style_iconstatus, 0);
    lv_style_set_border_width(&style_iconstatus, 0);
    lv_style_set_align(&style_iconstatus, LV_ALIGN_RIGHT_MID);
    lv_style_set_pad_right(&style_iconstatus, 15);

    lv_style_set_layout(&style_iconstatus, LV_LAYOUT_FLEX);
    lv_style_set_flex_flow(&style_iconstatus, LV_FLEX_FLOW_ROW);
    lv_style_set_flex_main_place(&style_iconstatus, LV_FLEX_ALIGN_CENTER);
    lv_style_set_flex_track_place(&style_iconstatus, LV_FLEX_ALIGN_SPACE_BETWEEN);
    lv_style_set_pad_row(&style_iconstatus, 3);

    // BATTERY
    lv_style_init(&style_battery);
    lv_style_set_text_font(&style_battery, font_symbol);
    lv_style_set_align(&style_battery, LV_ALIGN_RIGHT_MID);
    lv_style_set_text_color(&style_battery, lv_palette_main(LV_PALETTE_RED));

    // SD CARD
    lv_style_init(&style_storage);
    lv_style_set_text_font(&style_storage, font_symbol);
    lv_style_set_align(&style_storage, LV_ALIGN_RIGHT_MID);

    // WIFI
    lv_style_init(&style_wifi);
    lv_style_set_text_font(&style_wifi, font_symbol);
    lv_style_set_align(&style_wifi, LV_ALIGN_RIGHT_MID);

    // BLE
    lv_style_init(&style_ble);
    lv_style_set_text_font(&style_ble, font_fa);
    lv_style_set_align(&style_ble, LV_ALIGN_RIGHT_MID);

    // FOOTER MESSAGE & ANIMATION
    lv_anim_init(&anim_labelscroll);
    lv_anim_set_delay(&anim_labelscroll, 1000);        // Wait 1 second to start the first scroll
    lv_anim_set_repeat_delay(&anim_labelscroll, 3000); // Repeat the scroll 3 seconds after the label scrolls back to the initial position

    lv_style_init(&style_message);
    lv_style_set_anim(&style_message, &anim_labelscroll); // Set animation for the style
    // lv_style_set_text_color(&style_message, lv_palette_main(LV_PALETTE_RED));
    lv_style_set_opa(&style_message, LV_OPA_COVER);
    lv_style_set_text_font(&style_message, font_normal);
    lv_style_set_align(&style_message, LV_ALIGN_LEFT_MID);
    lv_style_set_pad_left(&style_message, 15);
    lv_style_set_pad_right(&style_message, 15);

    // UI ISLANDS
    lv_style_init(&style_ui_island);
    lv_style_set_bg_color(&style_ui_island, bg_theme_color);
    lv_style_set_bg_opa(&style_ui_island, LV_OPA_80);
    lv_style_set_border_color(&style_ui_island, bg_theme_color);
    //lv_style_set_border_opa(&style_ui_island, LV_OPA_80);
    lv_style_set_border_width(&style_ui_island, 1);
    lv_style_set_radius(&style_ui_island, 10);

    // FOOTER NAV BUTTONS
    lv_style_init(&style_glow);
    lv_style_set_bg_opa(&style_glow, LV_OPA_COVER);
    lv_style_set_border_width(&style_glow,0);
    lv_style_set_bg_color(&style_glow, lv_palette_main(LV_PALETTE_RED));

    /*Add a shadow*/
    // lv_style_set_shadow_width(&style_glow, 10);
    // lv_style_set_shadow_color(&style_glow, lv_palette_main(LV_PALETTE_RED));
    // lv_style_set_shadow_ofs_x(&style_glow, 5);
    // lv_style_set_shadow_ofs_y(&style_glow, 5);    
}

static void create_header(lv_obj_t *parent)
{
    // HEADER PANEL
    panel_header = lv_obj_create(parent);
    lv_obj_set_size(panel_header, LV_PCT(100), HEADER_HEIGHT);
    lv_obj_set_style_pad_all(panel_header, 0, 0);
    lv_obj_set_style_radius(panel_header, 0, 0);
    lv_obj_set_align(panel_header, LV_ALIGN_TOP_MID);
    lv_obj_set_scrollbar_mode(panel_header, LV_SCROLLBAR_MODE_OFF);

    // HEADER TITLE PANEL
    panel_title = lv_obj_create(panel_header);
    lv_obj_add_style(panel_title, &style_title, 0);
    lv_obj_set_scrollbar_mode(panel_title, LV_SCROLLBAR_MODE_OFF);

    // HEADER TITLE
    label_title = lv_label_create(panel_title);
    lv_label_set_text(label_title, LV_SYMBOL_HOME " FessiT IO Gateway");

    // HEADER STATUS ICON PANEL
    panel_status = lv_obj_create(panel_header);
    lv_obj_add_style(panel_status, &style_iconstatus, 0);
    lv_obj_set_scrollbar_mode(panel_status, LV_SCROLLBAR_MODE_OFF);

    // BLE
    icon_ble = lv_label_create(panel_status);
    lv_label_set_text(icon_ble, FA_SYMBOL_BLE);
    lv_obj_add_style(icon_ble, &style_ble, 0);

    // WIFI
    icon_wifi = lv_label_create(panel_status);
    lv_label_set_text(icon_wifi, LV_SYMBOL_WIFI);
    lv_obj_add_style(icon_wifi, &style_wifi, 0);

    // SD CARD
    icon_storage = lv_label_create(panel_status);
    lv_label_set_text(icon_storage, LV_SYMBOL_SD_CARD);
    lv_obj_add_style(icon_storage, &style_storage, 0);

    // BATTERY
    icon_battery = lv_label_create(panel_status);
    lv_label_set_text(icon_battery, LV_SYMBOL_CHARGE);
    lv_obj_add_style(icon_battery, &style_battery, 0);

    // lv_obj_add_event_cb(panel_title, home_clicked_eventhandler, LV_EVENT_CLICKED, NULL);
    // lv_obj_add_event_cb(panel_status, status_clicked_eventhandler, LV_EVENT_CLICKED, NULL);
}

static void create_footer(lv_obj_t *parent)
{
    lv_obj_t *panel_footer = lv_obj_create(parent);
    lv_obj_set_size(panel_footer, LV_PCT(100), FOOTER_HEIGHT);
    // lv_obj_set_style_bg_color(panel_footer, bg_theme_color, 0);
    lv_obj_set_style_pad_all(panel_footer, 0, 0);
    lv_obj_set_style_radius(panel_footer, 0, 0);
    lv_obj_set_align(panel_footer, LV_ALIGN_BOTTOM_MID);
    lv_obj_set_scrollbar_mode(panel_footer, LV_SCROLLBAR_MODE_OFF);

/*
    // Create Footer label and animate if text is longer
    label_message = lv_label_create(panel_footer); // full screen as the parent
    lv_obj_set_width(label_message, LV_PCT(100));
    lv_label_set_long_mode(label_message, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_add_style(label_message, &style_message, LV_STATE_DEFAULT);
    lv_obj_set_style_align(label_message,LV_ALIGN_BOTTOM_LEFT,0);

    // Show LVGL version in the footer
    footer_message("A Touch UX Template using LVGL v%d.%d.%d", lv_version_major(), lv_version_minor(), lv_version_patch());
*/

    /* REPLACE STATUS BAR WITH BUTTON PANEL FOR NAVIGATION */
    //static const char * btnm_map[] = {LV_SYMBOL_HOME " HOME", LV_SYMBOL_KEYBOARD " REMOTE", FA_SYMBOL_SETTINGS " SETTINGS", LV_SYMBOL_DOWNLOAD " UPDATE", NULL};
    static const char * btnm_map[] = {LV_SYMBOL_HOME, LV_SYMBOL_KEYBOARD,FA_SYMBOL_SETTINGS, LV_SYMBOL_DOWNLOAD,  NULL};
    lv_obj_t * footerButtons = lv_btnmatrix_create(panel_footer);
    lv_btnmatrix_set_map(footerButtons, btnm_map);
    lv_obj_set_style_text_font(footerButtons,&lv_font_montserrat_16,LV_PART_ITEMS);
    lv_obj_set_style_bg_opa(footerButtons,LV_OPA_TRANSP,0);
    lv_obj_set_size(footerButtons,LV_PCT(100),LV_PCT(100));
    lv_obj_set_style_border_width(footerButtons,0,LV_PART_MAIN | LV_PART_ITEMS);
    lv_btnmatrix_set_btn_ctrl_all(footerButtons, LV_BTNMATRIX_CTRL_CHECKABLE);
    
    //lv_obj_set_style_align(footerButtons,LV_ALIGN_TOP_MID,0);
    lv_btnmatrix_set_one_checked(footerButtons, true);   // only 1 button can be checked
    lv_btnmatrix_set_btn_ctrl(footerButtons,0,LV_BTNMATRIX_CTRL_CHECKED);

    // Very important but weird behavior
    lv_obj_set_height(footerButtons,FOOTER_HEIGHT+20);    
    lv_obj_set_style_radius(footerButtons,0,LV_PART_ITEMS);
    lv_obj_set_style_bg_opa(footerButtons,LV_OPA_TRANSP,LV_PART_ITEMS);
    lv_obj_add_style(footerButtons, &style_glow,LV_PART_ITEMS | LV_BTNMATRIX_CTRL_CHECKED); // selected

    lv_obj_align(footerButtons, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(footerButtons, footer_button_event_handler, LV_EVENT_ALL, NULL); 
    
}

static void tux_panel_clock_weather(lv_obj_t *parent)
{
    tux_clock_weather = tux_panel_create(parent, "", 80);
    lv_obj_add_style(tux_clock_weather, &style_ui_island, 0);

    lv_obj_t *cont_panel = tux_panel_get_content(tux_clock_weather);
    lv_obj_set_flex_flow(tux_clock_weather, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(tux_clock_weather, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // ************ Date/Time panel
    lv_obj_t *cont_datetime = lv_obj_create(cont_panel);
    //lv_obj_set_size(cont_datetime,180,120);
    lv_obj_set_size(cont_datetime,180,80);
    lv_obj_set_flex_flow(cont_datetime, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_scrollbar_mode(cont_datetime, LV_SCROLLBAR_MODE_OFF);
    lv_obj_align(cont_datetime,LV_ALIGN_LEFT_MID,0,0);
    lv_obj_set_style_bg_opa(cont_datetime,LV_OPA_TRANSP,0);
    lv_obj_set_style_border_opa(cont_datetime,LV_OPA_TRANSP,0);
    lv_obj_set_style_pad_top(cont_datetime,5,0);

    // MSG - MSG_TIME_CHANGED - EVENT
    lv_obj_add_event_cb(cont_datetime, datetime_event_cb, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subsribe_obj(MSG_TIME_CHANGED, cont_datetime, NULL);

    // Time
    lbl_time = lv_label_create(cont_datetime);
    lv_obj_set_style_align(lbl_time, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_set_style_text_font(lbl_time, &font_7seg_56, 0);
    lv_label_set_text(lbl_time, "10:19");

    // AM/PM
    lbl_ampm = lv_label_create(cont_datetime);
    lv_obj_set_style_align(lbl_ampm, LV_ALIGN_TOP_LEFT, 60);
    lv_label_set_text(lbl_ampm, "AM");

    // Date
    // lbl_date = lv_label_create(cont_datetime);
    // lv_obj_set_style_align(lbl_date, LV_ALIGN_BOTTOM_MID, 0);
    // lv_obj_set_style_text_font(lbl_date, font_large, 0);
    // lv_obj_set_height(lbl_date,30);
    // lv_label_set_text(lbl_date, "waiting for update");

    // ************ Weather panel (panel widen with weekly forecast in landscape)
    lv_obj_t *cont_justdate = lv_obj_create(cont_panel);
    lv_obj_set_size(cont_justdate,100,80);
    lv_obj_set_flex_flow(cont_justdate, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(cont_justdate, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(cont_justdate, LV_SCROLLBAR_MODE_OFF);
    lv_obj_align_to(cont_justdate,cont_datetime,LV_ALIGN_OUT_RIGHT_TOP,0,0);
    lv_obj_set_style_bg_opa(cont_justdate,LV_OPA_TRANSP,0);
    lv_obj_set_style_border_opa(cont_justdate,LV_OPA_TRANSP,0);
    // Added by Gousmodin
    //lv_obj_set_style_pad_gap(cont_justdate,10,0);

    // Day of the week
    lbl_dayofweek = lv_label_create(cont_justdate);
    lv_obj_set_style_align(lbl_dayofweek, LV_ALIGN_TOP_MID, 0);
    lv_obj_set_style_text_font(lbl_dayofweek, &lv_font_montserrat_24, 0);
    lv_label_set_text(lbl_dayofweek, "SAT");

    // day of the month
    lbl_dayofmonth = lv_label_create(cont_justdate);
    lv_obj_set_style_align(lbl_dayofmonth, LV_ALIGN_TOP_MID, 0);
    lv_obj_set_style_text_font(lbl_dayofmonth, font_normal, 0);
    lv_label_set_text(lbl_dayofmonth, "22 Nov, 2025");

    // Year
    // lbl_curryear = lv_label_create(cont_justdate);
    // lv_obj_set_style_align(lbl_curryear, LV_ALIGN_BOTTOM_MID, 0);
    // lv_obj_set_style_text_font(lbl_curryear, font_large, 0);
    // lv_obj_set_height(lbl_curryear,30);
    // lv_label_set_text(lbl_curryear, "1999");

    // // MSG - MSG_WEATHER_CHANGED - EVENT
    // lv_obj_add_event_cb(cont_weather, weather_event_cb, LV_EVENT_MSG_RECEIVED, NULL);
    // lv_msg_subsribe_obj(MSG_WEATHER_CHANGED, cont_weather, NULL);

    // // This only for landscape
    // // lv_obj_t *lbl_unit = lv_label_create(cont_weather);
    // // lv_obj_set_style_text_font(lbl_unit, font_normal, 0);
    // // lv_label_set_text(lbl_unit, "Light rain");

    // // Weather icons
    // lbl_weathericon = lv_label_create(cont_weather);
    // lv_obj_set_style_text_font(lbl_weathericon, &font_fa_weather_42, 0);
    // // "F:/weather/cloud-sun-rain.bin");//10d@2x.bin"
    // lv_label_set_text(lbl_weathericon, FA_WEATHER_SUN);
    // lv_obj_set_style_text_color(lbl_weathericon,lv_palette_main(LV_PALETTE_ORANGE),0);

    // // Temperature
    // lbl_temp = lv_label_create(cont_weather);
    // //lv_obj_set_style_text_font(lbl_temp, &lv_font_montserrat_32, 0);
    // lv_obj_set_style_text_font(lbl_temp, font_xl, 0);
    // lv_obj_set_style_align(lbl_temp, LV_ALIGN_BOTTOM_MID, 0);
    // lv_label_set_text(lbl_temp, "0°C");

    // lbl_hl = lv_label_create(cont_weather);
    // lv_obj_set_style_text_font(lbl_hl, font_normal, 0);
    // lv_obj_set_style_align(lbl_hl, LV_ALIGN_BOTTOM_MID, 0);
    // lv_label_set_text(lbl_hl, "H:0° L:0°");
}

static void tux_panel_digital_input(lv_obj_t *parent)
{
    tux_digital_input = tux_panel_create(parent, "", 50);
    lv_obj_add_style(tux_digital_input, &style_ui_island, 0);
    
    lv_obj_t *digi_ip_panel = tux_panel_get_content(tux_digital_input);
    // Enable flex layout on the digi_ip_panel
    lv_obj_set_flex_flow(digi_ip_panel, LV_FLEX_FLOW_ROW); // Arrange items in a row
    lv_obj_set_flex_align(digi_ip_panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Digital input
    lbl_digitalinput = lv_label_create(digi_ip_panel);
    // Set fixed width and height for the label
    lv_obj_set_size(lbl_digitalinput, 220, 50);  // Width: 220px, Height: 50px
    //lv_obj_set_style_align(lbl_digitalinput, LV_ALIGN_LEFT_MID, 0);
    lv_obj_set_style_text_font(lbl_digitalinput, &lv_font_montserrat_16, 0);
    lv_label_set_text(lbl_digitalinput, "Hall");
    // Set the text color to orange
    lv_obj_set_style_text_color(lbl_digitalinput, lv_color_hex(0xFFA500), 0); // Orange color
    lv_obj_set_style_pad_left(lbl_digitalinput, 20, 0);
    lv_obj_set_style_pad_top(lbl_digitalinput, 10, 0);

    // Icon to the right of the label
    lv_obj_t *icon = lv_label_create(digi_ip_panel);
    //lv_obj_set_style_align(icon, LV_ALIGN_LEFT_MID, 0);
    lv_obj_set_size(icon, 60, 50);  // Width: 220px, Height: 50px
    lv_obj_set_style_text_font(icon, &lv_font_montserrat_16, 0); // Match the label font size
    lv_label_set_text(icon, LV_SYMBOL_RIGHT); // Use a built-in LVGL symbol
    lv_obj_set_style_pad_left(icon, 30, 0);
    lv_obj_set_style_pad_top(icon, 10, 0);

    // Enable the panel to be clickable
    lv_obj_add_flag(icon, LV_OBJ_FLAG_CLICKABLE);
    // Set up an event callback for the panel
    lv_obj_add_event_cb(icon, panel_di_event_handler, LV_EVENT_CLICKED, NULL);
}

static void tux_panel_digital_output(lv_obj_t *parent)
{
    lv_obj_t *main_panel_region = tux_panel_create(parent, "", 50);
    lv_obj_add_style(main_panel_region, &style_ui_island, 0);
    
    lv_obj_t *panel_region = tux_panel_get_content(main_panel_region);
    lv_obj_set_flex_flow(panel_region, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(panel_region, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Digital input
    lv_obj_t *lbl_region = lv_label_create(panel_region);
    // Set fixed width and height for the label
    lv_obj_set_size(lbl_region, 220, 50);  // Width: 220px, Height: 50px
    lv_obj_set_style_text_font(lbl_region, &lv_font_montserrat_16, 0);
    lv_label_set_text(lbl_region, "Kitchen");
    // Set the text color to orange
    lv_obj_set_style_text_color(lbl_region, lv_color_hex(0xFFA500), 0); // Orange color
    lv_obj_set_style_pad_left(lbl_region, 20, 0);
    lv_obj_set_style_pad_top(lbl_region, 10, 0);

    // Icon to the right of the label
    lv_obj_t *icon = lv_label_create(panel_region);
    //lv_obj_set_style_align(icon, LV_ALIGN_LEFT_MID, 0);
    lv_obj_set_size(icon, 60, 50);  // Width: 220px, Height: 50px
    lv_obj_set_style_text_font(icon, &lv_font_montserrat_16, 0); // Match the label font size
    lv_label_set_text(icon, LV_SYMBOL_RIGHT); // Use a built-in LVGL symbol
    lv_obj_set_style_pad_left(icon, 30, 0);
    lv_obj_set_style_pad_top(icon, 10, 0);

    // Enable the panel to be clickable
    lv_obj_add_flag(icon, LV_OBJ_FLAG_CLICKABLE);
    // Set up an event callback for the panel
    lv_obj_add_event_cb(icon, panel_do_event_handler, LV_EVENT_CLICKED, NULL);
}

static void tux_panel_analog_input(lv_obj_t *parent)
{
    lv_obj_t *main_panel_region = tux_panel_create(parent, "", 50);
    lv_obj_add_style(main_panel_region, &style_ui_island, 0);
    
    lv_obj_t *panel_region = tux_panel_get_content(main_panel_region);
    lv_obj_set_flex_flow(panel_region, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(panel_region, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Digital input
    lv_obj_t *lbl_region = lv_label_create(panel_region);
    // Set fixed width and height for the label
    lv_obj_set_size(lbl_region, 220, 50);  // Width: 220px, Height: 50px
    lv_obj_set_style_text_font(lbl_region, &lv_font_montserrat_16, 0);
    lv_label_set_text(lbl_region, "Bedroom");
    // Set the text color to orange
    lv_obj_set_style_text_color(lbl_region, lv_color_hex(0xFFA500), 0); // Orange color
    lv_obj_set_style_pad_left(lbl_region, 20, 0);
    lv_obj_set_style_pad_top(lbl_region, 10, 0);

    // Icon to the right of the label
    lv_obj_t *icon = lv_label_create(panel_region);
    //lv_obj_set_style_align(icon, LV_ALIGN_LEFT_MID, 0);
    lv_obj_set_size(icon, 60, 50);  // Width: 220px, Height: 50px
    lv_obj_set_style_text_font(icon, &lv_font_montserrat_16, 0); // Match the label font size
    lv_label_set_text(icon, LV_SYMBOL_RIGHT); // Use a built-in LVGL symbol
    lv_obj_set_style_pad_left(icon, 30, 0);
    lv_obj_set_style_pad_top(icon, 10, 0);

    // Enable the panel to be clickable
    lv_obj_add_flag(icon, LV_OBJ_FLAG_CLICKABLE);
    // Set up an event callback for the panel
    lv_obj_add_event_cb(icon, panel_ana_event_handler, LV_EVENT_CLICKED, NULL);
}

static lv_obj_t * slider_label;
static void slider_event_cb(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target(e);
    lv_label_set_text_fmt(slider_label,"Brightness : %d",(int)lv_slider_get_value(slider));
    lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 15);
    lcd.setBrightness((int)lv_slider_get_value(slider));
}

static void tux_panel_config(lv_obj_t *parent)
{
    /******** CONFIG & TESTING ********/
    lv_obj_t *island_2 = tux_panel_create(parent, LV_SYMBOL_EDIT " CONFIG", 200);
    lv_obj_add_style(island_2, &style_ui_island, 0);

    // Get Content Area to add UI elements
    lv_obj_t *cont_2 = tux_panel_get_content(island_2);

    lv_obj_set_flex_flow(cont_2, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_style_pad_row(cont_2, 10, 0);
    //lv_obj_set_style_pad_column(cont_2, 5, 0);
    lv_obj_set_flex_align(cont_2, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_END);

    // Screen Brightness
    /*Create a label below the slider*/
    slider_label = lv_label_create(cont_2);
    lv_label_set_text_fmt(slider_label, "Brightness : %d", lcd.getBrightness());   

    lv_obj_t * slider = lv_slider_create(cont_2);
    lv_obj_center(slider);
    lv_obj_set_size(slider, LV_PCT(90), 20);
    lv_slider_set_range(slider, 50 , 255);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_TOP_MID, 0, 30);
    lv_bar_set_value(slider,lcd.getBrightness(),LV_ANIM_ON);

    // THEME Selection
    lv_obj_t *label = lv_label_create(cont_2);
    lv_label_set_text(label, "Theme : Dark");
    //lv_obj_set_size(label, LV_PCT(90), 20);
    lv_obj_align_to(label,slider,LV_ALIGN_OUT_TOP_MID,0,15);
    //lv_obj_center(slider);
    
    lv_obj_t *sw = lv_switch_create(cont_2);
    lv_obj_add_event_cb(sw, theme_switch_event_handler, LV_EVENT_ALL, label);
    lv_obj_align_to(label, sw, LV_ALIGN_OUT_TOP_MID, 0, 20);
    //lv_obj_align(sw,LV_ALIGN_RIGHT_MID,0,0);

    // Rotate to Portait/Landscape
    lv_obj_t *btn2 = lv_btn_create(cont_2);
    lv_obj_align(btn2, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(btn2, LV_SIZE_CONTENT, 30);
    lv_obj_add_event_cb(btn2, rotate_event_handler, LV_EVENT_ALL, NULL);
    lv_obj_t *lbl2 = lv_label_create(btn2);
    lv_label_set_text(lbl2, "Rotate to Landscape");
    //lv_obj_center(lbl2);
    lv_obj_align_to(btn2, sw, LV_ALIGN_OUT_BOTTOM_MID, 0, 15);
}

// Provision WIFI
static void tux_panel_wifi(lv_obj_t *parent)
{
    /******** PROVISION WIFI ********/
    island_wifi = tux_panel_create(parent, LV_SYMBOL_WIFI " WIFI STATUS", 270);
    lv_obj_add_style(island_wifi, &style_ui_island, 0);
    // tux_panel_set_title_color(island_wifi, lv_palette_main(LV_PALETTE_BLUE));

    // Get Content Area to add UI elements
    lv_obj_t *cont_1 = tux_panel_get_content(island_wifi);
    lv_obj_set_flex_flow(cont_1, LV_FLEX_FLOW_COLUMN_WRAP);
    lv_obj_set_flex_align(cont_1, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lbl_wifi_status = lv_label_create(cont_1);
    lv_obj_set_size(lbl_wifi_status, LV_SIZE_CONTENT, 30);
    lv_obj_align(lbl_wifi_status, LV_ALIGN_LEFT_MID, 0, 0);
    lv_label_set_text(lbl_wifi_status, "Waiting for IP");

    // Check for Updates button
    lv_obj_t *btn_unprov = lv_btn_create(cont_1);
    lv_obj_set_size(btn_unprov, LV_SIZE_CONTENT, 40);
    lv_obj_align(btn_unprov, LV_ALIGN_CENTER, 0, 0);
    lv_obj_t *lbl2 = lv_label_create(btn_unprov);
    lv_label_set_text(lbl2, "Reset Wi-Fi Settings");
    lv_obj_center(lbl2);    

    /* ESP QR CODE inserted here */
    qr_status_container = lv_obj_create(cont_1);
    lv_obj_set_size(qr_status_container, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(qr_status_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_ver(qr_status_container, 3, 0);
    lv_obj_set_style_border_width(qr_status_container, 0, 0);
    lv_obj_set_flex_flow(qr_status_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(qr_status_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_add_event_cb(btn_unprov, espwifi_event_handler, LV_EVENT_CLICKED, NULL);

    /* QR CODE */
    lv_color_t bg_color = lv_palette_lighten(LV_PALETTE_GREY, 4);
    lv_color_t fg_color = lv_palette_darken(LV_PALETTE_BLUE, 4);

    prov_qr = lv_qrcode_create(qr_status_container, 100, fg_color, bg_color);

    /* Set data - format of BLE provisioning data */
    // {"ver":"v1","name":"TUX_4AA440","pop":"abcd1234","transport":"ble"}
    const char *qrdata = "https://github.com/sukesh-ak/ESP32-TUX";
    lv_qrcode_update(prov_qr, qrdata, strlen(qrdata));

    /*Add a border with bg_color*/
    lv_obj_set_style_border_color(prov_qr, bg_color, 0);
    lv_obj_set_style_border_width(prov_qr, 5, 0);

    lbl_scan_status = lv_label_create(qr_status_container);
    lv_obj_set_size(lbl_scan_status, LV_SIZE_CONTENT, 30);
    lv_label_set_text(lbl_scan_status, "Scan to learn about ESP32-TUX");

}

static void tux_panel_ota(lv_obj_t *parent)
{
    /******** OTA UPDATES ********/
    island_ota = tux_panel_create(parent, LV_SYMBOL_DOWNLOAD " OTA UPDATES", 180);
    lv_obj_add_style(island_ota, &style_ui_island, 0);

    // Get Content Area to add UI elements
    lv_obj_t *cont_ota = tux_panel_get_content(island_ota);
    lv_obj_set_flex_flow(cont_ota, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont_ota, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Current Firmware version
    lbl_version = lv_label_create(cont_ota);
    lv_obj_set_size(lbl_version, LV_SIZE_CONTENT, 30);
    lv_obj_align(lbl_version, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text_fmt(lbl_version, "Firmware Version %s",get_firmware_version());

    // Check for Updates button
    lv_obj_t *btn_checkupdates = lv_btn_create(cont_ota);
    lv_obj_set_size(btn_checkupdates, LV_SIZE_CONTENT, 40);
    lv_obj_align(btn_checkupdates, LV_ALIGN_CENTER, 0, 0);
    lv_obj_t *lbl2 = lv_label_create(btn_checkupdates);
    lv_label_set_text(lbl2, "Check for Updates");
    lv_obj_center(lbl2);
    lv_obj_add_event_cb(btn_checkupdates, checkupdates_event_handler, LV_EVENT_ALL, NULL);

    lv_obj_t *esp_updatestatus = lv_obj_create(cont_ota);
    lv_obj_set_size(esp_updatestatus, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(esp_updatestatus, LV_OPA_10, 0);
    lv_obj_set_style_border_width(esp_updatestatus, 0, 0);

    lbl_update_status = lv_label_create(esp_updatestatus);
    lv_obj_set_style_text_color(lbl_update_status, lv_palette_main(LV_PALETTE_YELLOW), 0);
    lv_obj_align(lbl_update_status, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(lbl_update_status, "Click to check for updates");
}

static void tux_panel_devinfo(lv_obj_t *parent)
{
    island_devinfo = tux_panel_create(parent, LV_SYMBOL_TINT " DEVICE INFO", 200);
    lv_obj_add_style(island_devinfo, &style_ui_island, 0);

    // Get Content Area to add UI elements
    lv_obj_t *cont_devinfo = tux_panel_get_content(island_devinfo);

    lbl_device_info = lv_label_create(cont_devinfo);
    // Monoaspace font for alignment
    lv_obj_set_style_text_font(lbl_device_info,&font_robotomono_13,0); 
}

static void create_page_remote(lv_obj_t *parent)
{

    static lv_style_t style;
    lv_style_init(&style);

    /*Set a background color and a radius*/
    lv_style_set_radius(&style, 10);
    lv_style_set_bg_opa(&style, LV_OPA_80);
    // lv_style_set_bg_color(&style, lv_palette_lighten(LV_PALETTE_GREY, 1));

    /*Add a shadow*/
    lv_style_set_shadow_width(&style, 55);
    lv_style_set_shadow_color(&style, lv_palette_main(LV_PALETTE_BLUE));

    lv_obj_t * island_remote = tux_panel_create(parent, LV_SYMBOL_KEYBOARD " REMOTE", LV_PCT(100));
    lv_obj_add_style(island_remote, &style_ui_island, 0);

    // Get Content Area to add UI elements
    lv_obj_t *cont_remote = tux_panel_get_content(island_remote);

    lv_obj_set_flex_flow(cont_remote, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(cont_remote, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(cont_remote, 10, 0);
    lv_obj_set_style_pad_row(cont_remote, 10, 0);

    uint32_t i;
    for(i = 0; i <12; i++) {
        lv_obj_t * obj = lv_btn_create(cont_remote);
        lv_obj_add_style(obj, &style, LV_STATE_PRESSED);
        lv_obj_set_size(obj, 80, 80);
        
        lv_obj_t * label = lv_label_create(obj);
        lv_label_set_text_fmt(label, "%" LV_PRIu32, i);
        lv_obj_center(label);
    }

}

static void create_page_home(lv_obj_t *parent)
{
    /* HOME PAGE PANELS */
    tux_panel_clock_weather(parent);
    tux_panel_digital_input(parent);
    tux_panel_digital_output(parent);
    tux_panel_analog_input(parent);
    //tux_panel_devinfo(parent);  
}

static void create_page_settings(lv_obj_t *parent)
{
    /* SETTINGS PAGE PANELS */
    tux_panel_wifi(parent);
    tux_panel_config(parent);
}

static void create_page_updates(lv_obj_t *parent)
{
    /* OTA UPDATES PAGE PANELS */
    tux_panel_ota(parent);
    tux_panel_devinfo(parent);    
}

static void create_page_digitalinput(lv_obj_t *parent)
{
    /* DIGITAL INPUT PAGE */
    button_fullview_parent = parent; 
    tux_panel_di_fullview(parent);
}

static void create_page_digitaloutput(lv_obj_t *parent)
{
    /* DIGITAL INPUT PAGE */
    button_fullview_parent = parent; 
    tux_panel_do_fullview(parent);
}

static void create_page_analog(lv_obj_t *parent)
{
    /* DIGITAL INPUT PAGE */
    button_fullview_parent = parent; 
    tux_panel_ana_fullview(parent);
}

static void create_splash_screen()
{
    lv_obj_t * splash_screen = lv_scr_act();
    lv_obj_set_style_bg_color(splash_screen, lv_color_black(),0);
    lv_obj_t * splash_img = lv_img_create(splash_screen);
    lv_img_set_src(splash_img, "F:/bg/tux-logo.bin"); //&tux_logo);
    lv_obj_align(splash_img, LV_ALIGN_CENTER, 0, 0);

    //lv_scr_load_anim(splash_screen, LV_SCR_LOAD_ANIM_FADE_IN, 5000, 10, true);
    lv_scr_load(splash_screen);
}

static void show_ui()
{
    int i = 0;
    for (i = 0; i < 4; i++) {
        label_footer_text_di.push_back("");  // Push back an empty string
        label_footer_text_do.push_back("");  // Push back an empty string
        label_footer_text_ana.push_back("");  // Push back an empty string
        label_ana_values.push_back("");
        label_di_values.push_back("");
        label_do_values.push_back("");
    }

    // screen_container is the root of the UX
    screen_container = lv_obj_create(NULL);

    lv_obj_set_size(screen_container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_pad_all(screen_container, 0, 0);
    lv_obj_align(screen_container, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_border_width(screen_container, 0, 0);
    lv_obj_set_scrollbar_mode(screen_container, LV_SCROLLBAR_MODE_OFF);

    // Gradient / Image Background for screen container
    lv_obj_add_style(screen_container, &style_content_bg, 0);

    // HEADER & FOOTER
    create_header(screen_container);
    create_footer(screen_container);

    // CONTENT CONTAINER 
    content_container = lv_obj_create(screen_container);
    lv_obj_set_size(content_container, screen_w, screen_h - HEADER_HEIGHT - FOOTER_HEIGHT);
    lv_obj_align(content_container, LV_ALIGN_TOP_MID, 0, HEADER_HEIGHT);
    lv_obj_set_style_border_width(content_container, 0, 0);
    lv_obj_set_style_bg_opa(content_container, LV_OPA_TRANSP, 0);

    lv_obj_set_flex_flow(content_container, LV_FLEX_FLOW_COLUMN);

    // Show Home Page
    create_page_home(content_container);
    //create_page_settings(content_container);
    //create_page_remote(content_container);

    // Load main screen with animation
    //lv_scr_load(screen_container);
    lv_scr_load_anim(screen_container, LV_SCR_LOAD_ANIM_FADE_IN, 1000,100, true);

    // Status subscribers
    lv_msg_subsribe(MSG_WIFI_PROV_MODE, status_change_cb, NULL);    
    lv_msg_subsribe(MSG_WIFI_CONNECTED, status_change_cb, NULL);    
    lv_msg_subsribe(MSG_WIFI_DISCONNECTED, status_change_cb, NULL);    
    lv_msg_subsribe(MSG_OTA_STATUS, status_change_cb, NULL);    
    lv_msg_subsribe(MSG_SDCARD_STATUS, status_change_cb, NULL);  
    lv_msg_subsribe(MSG_BATTERY_STATUS, status_change_cb, NULL);  
    lv_msg_subsribe(MSG_DEVICE_INFO, status_change_cb, NULL);      

    // Send default page load notification => HOME
    lv_msg_send(MSG_PAGE_HOME,NULL);
}

static void rotate_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    lv_obj_t *label = lv_obj_get_child(btn, 0);

    if (code == LV_EVENT_CLICKED)
    {
        lvgl_acquire();

        if (lv_disp_get_rotation(disp) == LV_DISP_ROT_270)
            lv_disp_set_rotation(disp, LV_DISP_ROT_NONE);
        else
            lv_disp_set_rotation(disp, (lv_disp_rot_t)(lv_disp_get_rotation(disp) + 1));

        if (LV_HOR_RES > LV_VER_RES)
            lv_label_set_text(label, "Rotate to Portrait");
        else
            lv_label_set_text(label, "Rotate to Landscape");

        lvgl_release();

        // Update
        screen_h = lv_obj_get_height(lv_scr_act());
        screen_w = lv_obj_get_width(lv_scr_act());
        lv_obj_set_size(content_container, screen_w, screen_h - HEADER_HEIGHT - FOOTER_HEIGHT);

        // footer_message("%d,%d",screen_h,screen_w);
    }
}

static void theme_switch_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    lv_obj_t *udata = (lv_obj_t *)lv_event_get_user_data(e);

    if (code == LV_EVENT_VALUE_CHANGED)
    {
        LV_LOG_USER("State: %s\n", lv_obj_has_state(obj, LV_STATE_CHECKED) ? "On" : "Off");
        if (lv_obj_has_state(obj, LV_STATE_CHECKED))
        {
            switch_theme(false);
            lv_label_set_text(udata, "Theme : Light");

            // Pass the new theme info
            // ESP_ERROR_CHECK(esp_event_post(TUX_EVENTS, TUX_EVENT_THEME_CHANGED, NULL,NULL, portMAX_DELAY));
        }
        else
        {
            switch_theme(true);
            lv_label_set_text(udata, "Theme : Dark");
            
            // Pass the new theme info
            // ESP_ERROR_CHECK(esp_event_post(TUX_EVENTS, TUX_EVENT_THEME_CHANGED, NULL,NULL, portMAX_DELAY));
        }
    }
}

static void footer_message(const char *fmt, ...)
{
    char buffer[200];
    va_list args;
    va_start(args, fmt);
    vsprintf(buffer, fmt, args);
    lv_label_set_text(label_message, buffer);
    va_end(args);
}

static void home_clicked_eventhandler(lv_event_t *e)
{
    // footer_message("Home clicked!");
    //  Clean the content container first
    lv_obj_clean(content_container);
    create_page_home(content_container);
}

static void status_clicked_eventhandler(lv_event_t *e)
{
    // footer_message("Status icons touched but this is a very long message to show scroll animation!");
    //  Clean the content container first
    lv_obj_clean(content_container);
    create_page_settings(content_container);
    //create_page_remote(content_container);
}

void switch_theme(bool dark)
{
    if (dark)
    {
        theme_current = lv_theme_default_init(disp, lv_palette_main(LV_PALETTE_BLUE),
                                              lv_palette_main(LV_PALETTE_GREEN),
                                              1, /*Light or dark mode*/
                                              &lv_font_montserrat_14);
        bg_theme_color = lv_palette_darken(LV_PALETTE_GREY, 5);
        lv_disp_set_theme(disp, theme_current);
        //bg_theme_color = theme_current->flags & LV_USE_THEME_DEFAULT ? lv_palette_darken(LV_PALETTE_GREY, 5) : lv_palette_lighten(LV_PALETTE_GREY, 2);
        // lv_theme_set_apply_cb(theme_current, new_theme_apply_cb);

        lv_style_set_bg_color(&style_ui_island, bg_theme_color);
        //lv_style_set_bg_opa(&style_ui_island, LV_OPA_80);

        ESP_LOGI(TAG,"Dark theme set");
    }
    else
    {
        theme_current = lv_theme_default_init(disp,
                                              lv_palette_main(LV_PALETTE_BLUE),
                                              lv_palette_main(LV_PALETTE_RED),
                                              0, /*Light or dark mode*/
                                              &lv_font_montserrat_14);
        //bg_theme_color = lv_palette_lighten(LV_PALETTE_GREY, 5);    // #BFBFBD
        // bg_theme_color = lv_color_make(0,0,255); 
        bg_theme_color = lv_color_hex(0xBFBFBD); //383837


        lv_disp_set_theme(disp, theme_current);
        // lv_theme_set_apply_cb(theme_current, new_theme_apply_cb);
        lv_style_set_bg_color(&style_ui_island, bg_theme_color);
        ESP_LOGI(TAG,"Light theme set");        

    }
}

// /*Will be called when the styles of the base theme are already added
//   to add new styles*/
// static void new_theme_apply_cb(lv_theme_t * th, lv_obj_t * obj)
// {
//     LV_UNUSED(th);

//     if(lv_obj_check_type(obj, &tux_panel_class)) {
//         lv_obj_add_style(obj, &style_ui_island, 0);
//         //lv_style_set_bg_color(&style_ui_island,theme_current->color_primary);
//     }

// }

static void espwifi_event_handler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED)
    {
        bool provisioned = false;
        ESP_ERROR_CHECK(wifi_prov_mgr_is_provisioned(&provisioned));
        if (provisioned) {
            wifi_prov_mgr_reset_provisioning();     // reset wifi
            
            // Reset device to start provisioning
            lv_label_set_text(lbl_wifi_status, "Wi-Fi Disconnected!");
            lv_obj_set_style_text_color(lbl_wifi_status, lv_palette_main(LV_PALETTE_YELLOW), 0);
            lv_label_set_text(lbl_scan_status, "Restart device to provision WiFi.");
            lv_obj_add_state( btn, LV_STATE_DISABLED );  /* Disable */
        }
    }
}

inline void checkupdates_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED)
    {
        /*Get the first child of the button which is the label and change its text*/
        // Maybe disable the button and enable once completed
        //lv_obj_t *label = lv_obj_get_child(btn, 0);
        //lv_label_set_text_fmt(label, "Checking for updates...");
        LV_LOG_USER("Clicked");
        lv_msg_send(MSG_OTA_INITIATE,NULL);
    }
}

static const char* get_firmware_version()
{
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_app_desc_t running_app_info;
    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
        return fmt::format("{}",running_app_info.version).c_str();
    }
    return "0.0.0";
}

void datetime_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    //lv_event_get_target(e) => cont_datetime
    lv_msg_t * m = lv_event_get_msg(e);
    
    // Not necessary but if event target was button or so, then required
    if (code == LV_EVENT_MSG_RECEIVED)  
    {
        struct tm *dtinfo = (tm*)lv_msg_get_payload(m);
        // Date & Time formatted
        char strftime_buf[64];
        // strftime(strftime_buf, sizeof(strftime_buf), "%c %z", dtinfo);
        // ESP_LOGW(TAG,"Triggered:datetime_event_cb %s",strftime_buf);

        // Date formatted
        // strftime(strftime_buf, sizeof(strftime_buf), "%a, %e %b %Y", dtinfo);
        // lv_label_set_text_fmt(lbl_date,"%s",strftime_buf);
        
        // Day of week
        strftime(strftime_buf, sizeof(strftime_buf), "%a", dtinfo);
        // Convert to uppercase
        for (int i = 0; strftime_buf[i]; i++) {
            strftime_buf[i] = toupper((unsigned char)strftime_buf[i]);
        }
        lv_label_set_text_fmt(lbl_dayofweek,"%s",strftime_buf);

        // Day of month
        strftime(strftime_buf, sizeof(strftime_buf), "%e %b, %Y", dtinfo);
        lv_label_set_text_fmt(lbl_dayofmonth,"%s",strftime_buf);
        
        // Year
        // strftime(strftime_buf, sizeof(strftime_buf), "%Y", dtinfo);
        // lv_label_set_text_fmt(lbl_curryear,"%s",strftime_buf);

        // Time in 12hrs 
        strftime(strftime_buf, sizeof(strftime_buf), "%I:%M", dtinfo);
        lv_label_set_text_fmt(lbl_time, "%s", strftime_buf);        

        // 12hr clock AM/PM
        strftime(strftime_buf, sizeof(strftime_buf), "%p", dtinfo);
        lv_label_set_text_fmt(lbl_ampm, "%s", strftime_buf);        
    }
}

void weather_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_msg_t * m = lv_event_get_msg(e);
    
    // Not necessary but if event target was button or so, then required
    if (code == LV_EVENT_MSG_RECEIVED)  
    {
        OpenWeatherMap *e_owm = NULL;
        e_owm = (OpenWeatherMap*)lv_msg_get_payload(m);
        //ESP_LOGW(TAG,"weather_event_cb %s",e_owm->LocationName.c_str());

        // set this according to e_owm->WeatherIcon 
        set_weather_icon(e_owm->WeatherIcon);      

        lv_label_set_text(lbl_temp,fmt::format("{:.1f}°{}",e_owm->Temperature,e_owm->TemperatureUnit).c_str());
        lv_label_set_text(lbl_hl,fmt::format("H:{:.1f}° L:{:.1f}°",e_owm->TemperatureHigh,e_owm->TemperatureLow).c_str());
    }
}

static void footer_button_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    if(code == LV_EVENT_VALUE_CHANGED) {
        uint32_t page_id = lv_btnmatrix_get_selected_btn(obj);
        const char * txt = lv_btnmatrix_get_btn_text(obj, page_id);
        printf("[%ld] %s was pressed\n", page_id,txt);

        // Do not refresh the page if its not changed
        if (current_page != page_id) current_page = page_id;
        else return;    // Skip if no page change

        // HOME
        if (page_id==MSG_PAGE_HOME)  {
            lv_obj_clean(content_container);
            create_page_home(content_container);
            anim_move_left_x(content_container,screen_w,0,200);
            lv_msg_send(MSG_PAGE_HOME,NULL);
        } 
        // REMOTE
        else if (page_id == MSG_PAGE_REMOTE) {
            lv_obj_clean(content_container);
            create_page_remote(content_container);
            anim_move_left_x(content_container,screen_w,0,200);
            lv_msg_send(MSG_PAGE_REMOTE,NULL);
        }
        // SETTINGS
        else if (page_id == MSG_PAGE_SETTINGS) {
            lv_obj_clean(content_container);
            create_page_settings(content_container);
            anim_move_left_x(content_container,screen_w,0,200);
            lv_msg_send(MSG_PAGE_SETTINGS,NULL);
        }
        // OTA UPDATES
        else if (page_id == MSG_PAGE_OTA) {
            lv_obj_clean(content_container);
            create_page_updates(content_container);
            anim_move_left_x(content_container,screen_w,0,200);
            lv_msg_send(MSG_PAGE_OTA,NULL);
        }
    }
}

static void status_change_cb(void * s, lv_msg_t *m)
{
    LV_UNUSED(s);
    unsigned int msg_id = lv_msg_get_id(m);
    const char * msg_payload = (const char *)lv_msg_get_payload(m);
    const char * msg_data = (const char *)lv_msg_get_user_data(m);

    switch (msg_id)
    {
        case MSG_WIFI_PROV_MODE:
        {
            ESP_LOGW(TAG,"[%d] MSG_WIFI_PROV_MODE",msg_id);
            // Update QR code for PROV and wifi symbol to red?
            lv_style_set_text_color(&style_wifi, lv_palette_main(LV_PALETTE_GREY));
            lv_label_set_text(icon_wifi, LV_SYMBOL_WIFI);

            char qr_data[150] = {0};
            snprintf(qr_data,sizeof(qr_data),"%s",(const char*)lv_msg_get_payload(m));
            lv_qrcode_update(prov_qr, qr_data, strlen(qr_data));
            lv_label_set_text(lbl_scan_status, "Install 'ESP SoftAP Prov' App & Scan");
        }
        break;
        case MSG_WIFI_CONNECTED:
        {
            ESP_LOGW(TAG,"[%d] MSG_WIFI_CONNECTED",msg_id);
            lv_style_set_text_color(&style_wifi, lv_palette_main(LV_PALETTE_BLUE));
            lv_label_set_text(icon_wifi, LV_SYMBOL_WIFI);

            if (lv_msg_get_payload(m) != NULL) {
                char ip_data[20]={0};
                // IP address in the payload so display
                snprintf(ip_data,sizeof(ip_data),"%s",(const char*)lv_msg_get_payload(m));
                lv_label_set_text_fmt(lbl_wifi_status, "IP Address: %s",ip_data);
            }
        }
        break;
        case MSG_WIFI_DISCONNECTED:
        {
            ESP_LOGW(TAG,"[%d] MSG_WIFI_DISCONNECTED",msg_id);
            lv_style_set_text_color(&style_wifi, lv_palette_main(LV_PALETTE_GREY));
            lv_label_set_text(icon_wifi, LV_SYMBOL_WIFI);
        }
        break;
        case MSG_OTA_STATUS:
        {
            ESP_LOGW(TAG,"[%d] MSG_OTA_STATUS",msg_id);
            // Shows different status during OTA update
            char ota_data[150] = {0};
            snprintf(ota_data,sizeof(ota_data),"%s",(const char*)lv_msg_get_payload(m));
            lv_label_set_text(lbl_update_status, ota_data);
        }
        break;
        case MSG_SDCARD_STATUS:
        {
            bool sd_status = *(bool *)lv_msg_get_payload(m);
            ESP_LOGW(TAG,"[%d] MSG_SDCARD_STATUS %d",msg_id,sd_status);
            if (sd_status) {
                lv_style_set_text_color(&style_storage, lv_palette_main(LV_PALETTE_GREEN));
            } else {
                lv_style_set_text_color(&style_storage, lv_palette_main(LV_PALETTE_RED));
            }
        }
        break;
        case MSG_BATTERY_STATUS:
        {
            int battery_val = *(int *)lv_msg_get_payload(m);
            //ESP_LOGW(TAG,"[%d] MSG_BATTERY_STATUS %d",msg_id,battery_val);
            lv_update_battery(battery_val);
        }
        break;
        case MSG_DEVICE_INFO:
        {
            ESP_LOGW(TAG,"[%d] MSG_DEVICE_INFO",msg_id);
            char devinfo_data[300] = {0};
            snprintf(devinfo_data,sizeof(devinfo_data),"%s",(const char*)lv_msg_get_payload(m));
            lv_label_set_text(lbl_device_info,devinfo_data);
        }
        break;
    }
}

static void lv_update_battery(uint batval)
{
    if (batval < 20)
    {
        lv_style_set_text_color(&style_battery, lv_palette_main(LV_PALETTE_RED));
        lv_label_set_text(icon_battery, LV_SYMBOL_BATTERY_EMPTY);
    }
    else if (batval < 50)
    {
        lv_style_set_text_color(&style_battery, lv_palette_main(LV_PALETTE_RED));
        lv_label_set_text(icon_battery, LV_SYMBOL_BATTERY_1);
    }
    else if (batval < 70)
    {
        lv_style_set_text_color(&style_battery, lv_palette_main(LV_PALETTE_DEEP_ORANGE));
        lv_label_set_text(icon_battery, LV_SYMBOL_BATTERY_2);
    }
    else if (batval < 90)
    {
        lv_style_set_text_color(&style_battery, lv_palette_main(LV_PALETTE_GREEN));
        lv_label_set_text(icon_battery, LV_SYMBOL_BATTERY_3);
    }
    else
    {
        lv_style_set_text_color(&style_battery, lv_palette_main(LV_PALETTE_GREEN));
        lv_label_set_text(icon_battery, LV_SYMBOL_BATTERY_FULL);
    }
}

/********************** ANIMATIONS *********************/
void anim_move_left_x(lv_obj_t * TargetObject, int start_x, int end_x, int delay)
{
    lv_anim_t property_anim;
    lv_anim_init(&property_anim);
    lv_anim_set_time(&property_anim, 200);
    lv_anim_set_user_data(&property_anim, TargetObject);
    lv_anim_set_custom_exec_cb(&property_anim, tux_anim_callback_set_x);
    lv_anim_set_values(&property_anim, start_x, end_x);
    lv_anim_set_path_cb(&property_anim, lv_anim_path_overshoot);
    lv_anim_set_delay(&property_anim, delay + 0);
    lv_anim_set_playback_time(&property_anim, 0);
    lv_anim_set_playback_delay(&property_anim, 0);
    lv_anim_set_repeat_count(&property_anim, 0);
    lv_anim_set_repeat_delay(&property_anim, 0);
    lv_anim_set_early_apply(&property_anim, true);
    lv_anim_start(&property_anim);
}

void tux_anim_callback_set_x(lv_anim_t * a, int32_t v)
{
    lv_obj_set_x((lv_obj_t *)a->user_data, v);
}

void anim_move_left_y(lv_obj_t * TargetObject, int start_y, int end_y, int delay)
{
    lv_anim_t property_anim;
    lv_anim_init(&property_anim);
    lv_anim_set_time(&property_anim, 300);
    lv_anim_set_user_data(&property_anim, TargetObject);
    lv_anim_set_custom_exec_cb(&property_anim, tux_anim_callback_set_y);
    lv_anim_set_values(&property_anim, start_y, end_y);
    lv_anim_set_path_cb(&property_anim, lv_anim_path_overshoot);
    lv_anim_set_delay(&property_anim, delay + 0);
    lv_anim_set_playback_time(&property_anim, 0);
    lv_anim_set_playback_delay(&property_anim, 0);
    lv_anim_set_repeat_count(&property_anim, 0);
    lv_anim_set_repeat_delay(&property_anim, 0);
    lv_anim_set_early_apply(&property_anim, true);
    lv_anim_start(&property_anim);
}

void tux_anim_callback_set_y(lv_anim_t * a, int32_t v)
{
    lv_obj_set_y((lv_obj_t *)a->user_data, v);
}

void anim_fade_in(lv_obj_t * TargetObject, int delay)
{
    lv_anim_t property_anim;
    lv_anim_init(&property_anim);
    lv_anim_set_time(&property_anim, 3000);
    lv_anim_set_user_data(&property_anim, TargetObject);
    lv_anim_set_custom_exec_cb(&property_anim, tux_anim_callback_set_opacity);
    lv_anim_set_values(&property_anim, 0, 255);
    lv_anim_set_path_cb(&property_anim, lv_anim_path_ease_out);
    lv_anim_set_delay(&property_anim, delay + 0);
    lv_anim_set_playback_time(&property_anim, 0);
    lv_anim_set_playback_delay(&property_anim, 0);
    lv_anim_set_repeat_count(&property_anim, 0);
    lv_anim_set_repeat_delay(&property_anim, 0);
    lv_anim_set_early_apply(&property_anim, false);
    lv_anim_start(&property_anim);

}
void anim_fade_out(lv_obj_t * TargetObject, int delay)
{
    lv_anim_t property_anim;
    lv_anim_init(&property_anim);
    lv_anim_set_time(&property_anim, 1000);
    lv_anim_set_user_data(&property_anim, TargetObject);
    lv_anim_set_custom_exec_cb(&property_anim, tux_anim_callback_set_opacity);
    lv_anim_set_values(&property_anim, 255, 0);
    lv_anim_set_path_cb(&property_anim, lv_anim_path_ease_in_out);
    lv_anim_set_delay(&property_anim, delay + 0);
    lv_anim_set_playback_time(&property_anim, 0);
    lv_anim_set_playback_delay(&property_anim, 0);
    lv_anim_set_repeat_count(&property_anim, 0);
    lv_anim_set_repeat_delay(&property_anim, 0);
    lv_anim_set_early_apply(&property_anim, false);
    lv_anim_start(&property_anim);

}
void tux_anim_callback_set_opacity(lv_anim_t * a, int32_t v)
{
    lv_obj_set_style_opa((lv_obj_t *)a->user_data, v, 0);
}

static void set_weather_icon(string weatherIcon)
{
    /* 
        https://openweathermap.org/weather-conditions#Weather-Condition-Codes-2
        d = day / n = night
        01 - clear sky
        02 - few clouds
        03 - scattered clouds
        04 - broken clouds
        09 - shower rain
        10 - rain
        11 - thunderstorm
        13 - snow
        50 - mist
    */
    // lv_color_make(red, green, blue);

    if (weatherIcon.find('d') != std::string::npos) {
        // set daytime color
        // color = whitesmoke = lv_color_make(245, 245, 245)
        // Ideally it should change for each weather - light blue for rain etc...
        lv_obj_set_style_text_color(lbl_weathericon,lv_color_make(241, 235, 156),0); 
    } else {
        // set night time color
        lv_obj_set_style_text_color(lbl_weathericon,lv_palette_main(LV_PALETTE_BLUE_GREY),0);
    }

    if (weatherIcon.find("50") != std::string::npos) {     // mist - need icon
        lv_label_set_text(lbl_weathericon,FA_WEATHER_DROPLET);
        return;
    }
    
    if (weatherIcon.find("13") != std::string::npos) {     
        lv_label_set_text(lbl_weathericon,FA_WEATHER_SNOWFLAKES);
        return;
    }    

    if (weatherIcon.find("11") != std::string::npos) {     
        lv_label_set_text(lbl_weathericon,FA_WEATHER_CLOUD_SHOWERS_HEAVY);
        return;
    }    

    if (weatherIcon.find("10") != std::string::npos) {     
        lv_label_set_text(lbl_weathericon,FA_WEATHER_CLOUD_RAIN);
        return;
    }    

    if (weatherIcon.find("09") != std::string::npos) {     
        lv_label_set_text(lbl_weathericon,FA_WEATHER_CLOUD_RAIN);
        return;
    }    

    if (weatherIcon.find("04") != std::string::npos) {     
        lv_label_set_text(lbl_weathericon,FA_WEATHER_CLOUD);
        return;
    }   

    if (weatherIcon.find("03") != std::string::npos) {     
        lv_label_set_text(lbl_weathericon,FA_WEATHER_CLOUD);
        return;
    }

    if (weatherIcon.find("02") != std::string::npos) {     
        lv_label_set_text(lbl_weathericon,FA_WEATHER_CLOUD);
        return;
    }

    if (weatherIcon.find("01") != std::string::npos) {     
        lv_label_set_text(lbl_weathericon,FA_WEATHER_SUN);
        return;
    }


    // default
    lv_label_set_text(lbl_weathericon,FA_WEATHER_CLOUD_SHOWERS_HEAVY);
    lv_obj_set_style_text_color(lbl_weathericon,lv_palette_main(LV_PALETTE_BLUE_GREY),0); 


}

static void panel_di_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    ESP_LOGI(TAG,"DIGITAL INPUT CLICKED");

    if(code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG,"SETTINGS Page will open");
        lv_obj_clean(content_container);
        create_page_digitalinput(content_container);
        anim_move_left_x(content_container,screen_w,0,200);
        //lv_msg_send(MSG_PAGE_SETTINGS,NULL);
    }
}

static void tux_panel_di_fullview(lv_obj_t *parent)
{
    // For Title
    lv_obj_t *tux_di_fullview_title = tux_panel_create(parent, "", 50);
    lv_obj_add_style(tux_di_fullview_title, &style_ui_island, 0);
    lv_obj_t *digi_ip_title_panel = tux_panel_get_content(tux_di_fullview_title);

    lv_obj_t *title_label = lv_label_create(digi_ip_title_panel);
    lv_label_set_text(title_label, "Hall");
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_24, 0); // Set font size
    lv_obj_set_style_text_align(title_label, LV_TEXT_ALIGN_CENTER, 0); // Align text center
    lv_obj_align(title_label, LV_ALIGN_CENTER, 0, 0); // Position at the top middle
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFA500), 0); // Orange color

    lv_obj_t *cont = lv_obj_create(digi_ip_title_panel);  // Create container
    lv_obj_set_size(cont, 60, 40);  // Set the container size
    lv_obj_set_style_border_width(cont, 0, 0);  // Set border width
    lv_obj_set_style_border_color(cont, lv_color_hex(0xBB6D0A), 0);  // Set border color
    lv_obj_set_style_radius(cont, 10, 0);  // Rounded corners to form a circle shape
    // Make the container background transparent
    lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, 0);  // Set background opacity to transparent
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_align_to(cont, title_label, LV_ALIGN_OUT_RIGHT_MID, 5, 0); // Positioned to the right of title_label
    lv_obj_set_style_pad_left(cont, 30, 0);

    // Add the LV_SYMBOL_LEFT icon
    lv_obj_t *icon_label = lv_label_create(cont);
    lv_label_set_text(icon_label, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_font(icon_label, &lv_font_montserrat_16, 0);
    lv_obj_center(icon_label);  // Center align icon_label within cont


    // Enable the panel to be clickable
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    // Set up an event callback for the panel
    lv_obj_add_event_cb(cont, goback_di_event_handler, LV_EVENT_CLICKED, NULL);

    tux_di_fullview = tux_panel_create(parent, "", 340);
    lv_obj_add_style(tux_di_fullview, &style_ui_island, 0);

    // Adjust positions to create a thin gap between the panels
    lv_obj_align_to(tux_di_fullview, tux_di_fullview_title, LV_ALIGN_OUT_BOTTOM_MID, 0, 1); // Thin gap of 2 pixels

    // Get content area of the panel
    lv_obj_t *digi_ip_panel = tux_panel_get_content(tux_di_fullview);
    lv_obj_set_flex_flow(digi_ip_panel, LV_FLEX_FLOW_ROW_WRAP);  // Row wrapping for 2x2 layout
    lv_obj_set_flex_align(digi_ip_panel, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Adjust padding to ensure consistent spacing
    lv_obj_set_style_pad_left(digi_ip_panel, 20, 0);  // Left padding
    lv_obj_set_style_pad_right(digi_ip_panel, 10, 0); // Right padding
    lv_obj_set_style_pad_top(digi_ip_panel, 5, 0);    // Top padding
    lv_obj_set_style_pad_bottom(digi_ip_panel, 5, 0); // Bottom padding

    //lv_obj_set_style_pad_all(cont_digital_ipstatus, 5, 0);  // Padding between the containers (use 5px for gap)
    //lv_obj_set_style_pad_all(digi_ip_panel, 5, 0);  // Add padding to the container
    lv_obj_set_size(digi_ip_panel, 280, 340);  // Ensure digi_ip_panel fits the entire panel

    // Add row and column gaps between containers
    lv_obj_set_style_pad_row(digi_ip_panel, 20, 0);  // Vertical spacing between rows
    lv_obj_set_style_pad_column(digi_ip_panel, 20, 0);  // Horizontal spacing between columns

    // Button dimensions
    int spacing = 20;  // Spacing between buttons
    int button_width = (220 - spacing) / 2;  // 2 buttons per row
    int button_height = (280 - spacing) / 2;  // 2 rows

    // Create containers with circle and ON/OFF text for 4 items
    for (int i = 0; i < 4; i++) {
        int id = i+1;
        // Create a container for each digital input
        lv_obj_t *cont = lv_obj_create(digi_ip_panel);  // Create container
        lv_obj_set_size(cont, button_width, button_height);  // Set the container size
        lv_obj_set_style_border_width(cont, 1, 0);  // Set border width
        lv_obj_set_style_border_color(cont, lv_color_hex(0xBB6D0A), 0);  // Set border color
        lv_obj_set_style_radius(cont, 10, 0);  // Rounded corners to form a circle shape

        // Make the container background transparent
        lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, 0);  // Set background opacity to transparent

        // Create label for text
        lv_obj_t *label_footer = lv_label_create(cont);
        char buffer[16];  // Adjust the size based on your maximum expected text length
        snprintf(buffer, sizeof(buffer), "L%d", id);  // Format the text

        std::string text = label_footer_text_di.at(i);
        if(text == "")
        {
            lv_label_set_text(label_footer, buffer);  // Set the text for the label
            label_footer_text_di.at(i) = std::string(buffer);
        }
        else
        {
            lv_label_set_text(label_footer, text.c_str());  // Set the text for the label
        }
        lv_obj_align(label_footer, LV_ALIGN_BOTTOM_MID, 0, 0);  // Align the label to the bottom middle of the container

        // Create a circle inside the container
        lv_obj_t *circle = lv_obj_create(cont);  // Create a smaller container for the circle
        label_di_circle_list[i] = circle;
        lv_obj_set_size(circle, button_width * 0.6, button_width * 0.6);  // Adjust size to make it a circle
        lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);  // Make it a perfect circle
        lv_obj_set_style_bg_color(circle, lv_color_hex(0xBB6D0A), 0);  // Light blue background for the circle
        lv_obj_set_style_bg_opa(circle, LV_OPA_10, 0);  // Semi-transparent background for the circle (50% opacity)
        lv_obj_set_style_border_width(circle, 4, 0);  // Bold border for the circle
        lv_obj_set_style_border_color(circle, lv_color_hex(0x578133), 0);  // Blue border color
        lv_obj_set_style_shadow_width(circle, 10, 0);  // Add shadow for depth
        lv_obj_set_style_shadow_color(circle, lv_color_hex(0x8FB271), 0);  // Light shadow for a glowing effect
        lv_obj_center(circle);  // Center the circle inside the container
        lv_obj_set_scrollbar_mode(circle, LV_SCROLLBAR_MODE_OFF);

        // Create a label for the "ON" or "OFF" text inside the circle
        lv_obj_t *label = lv_label_create(circle);  // Create a label inside the circle
        //char label_text[10];
        //snprintf(label_text, sizeof(label_text), "OFF");  // Default text: "OFF"
        //lv_label_set_text(label, label_text);  // Set the label text
        
        label_di_obj_list[i] = label;
        lv_obj_set_style_text_color(label, lv_color_hex(0xb53e02), 0);  // Black text color
        lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);  // Slightly larger font for visibility
        lv_obj_center(label);  // Center the label inside the circle
        // Disable the scrollbar on the container
        lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);

        char label_text[10];
        snprintf(label_text, sizeof(label_text), "OFF");  // Default text: "OFF"
        std::string valDI = label_di_values.at(i);
        if(valDI == "")
        {
            lv_label_set_text(label, label_text);  // Set the text for the label
            label_di_values.at(i) = std::string(label_text);
            lv_obj_set_style_border_color(label_di_circle_list[i], lv_color_hex(0xb53e02), 0);
        }
        else
        {
            lv_label_set_text(label, valDI.c_str());  // Set the text for the label
            if(valDI == "OFF")
            {
                lv_obj_set_style_border_color(label_di_circle_list[i], lv_color_hex(0xb53e02), 0);
            }
            else
            {
                lv_obj_set_style_border_color(label_di_circle_list[i], lv_color_hex(0x578133), 0);
                lv_obj_set_style_text_color(label_di_obj_list[i], lv_color_hex(0x307A34), 0);
            }
        }

        

        // Enable container click
        lv_obj_add_flag(circle, LV_OBJ_FLAG_CLICKABLE);  // Make the container clickable
        // Add event callback to change text and color when clicked
        lv_obj_add_event_cb(circle, circle_event_handler, LV_EVENT_ALL, (void *)id);

        // Enable container click
        lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);  // Make the container clickable
        // Add event callback to change text and color when clicked
        lv_obj_add_event_cb(cont, button_click_event_handler, LV_EVENT_ALL, (void *)id);
    }

    lv_msg_send(MSG_PAGE_DI_VAL, NULL);
}


static void tux_panel_di_settings(lv_obj_t *parent, int button_id)
{
    which_button_di_settings = static_cast<lv_coord_t>(button_id);
    // For Title
    lv_obj_t *tux_settings_title = tux_panel_create(parent, "", 50);
    lv_obj_add_style(tux_settings_title, &style_ui_island, 0);
    lv_obj_t *digi_ip_title_panel = tux_panel_get_content(tux_settings_title);

    lv_obj_t *title_label = lv_label_create(digi_ip_title_panel);
    lv_label_set_text(title_label, "Digital Input");
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_24, 0); // Set font size
    lv_obj_set_style_text_align(title_label, LV_TEXT_ALIGN_CENTER, 0); // Align text center
    lv_obj_align(title_label, LV_ALIGN_CENTER, 0, 0); // Position at the top middle
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFA500), 0); // Orange color

    tux_settings_panel = tux_panel_create(parent, "", 340);
    lv_obj_add_style(tux_settings_panel, &style_ui_island, 0);
    lv_obj_set_size(tux_settings_panel, 280, 340);  // Set the container size
    // Adjust positions to create a thin gap between the panels
    lv_obj_align_to(tux_settings_panel, digi_ip_title_panel, LV_ALIGN_OUT_BOTTOM_MID, 0, 1); // Thin gap of 2 pixels

    // Get content area of the panel
    lv_obj_t *digi_ip_panel = tux_panel_get_content(tux_settings_panel);
    // Align digi_ip_panel to the top of tux_di_settings with a 1 or 2 pixel gap
    lv_obj_align_to(digi_ip_panel, tux_settings_panel, LV_ALIGN_OUT_TOP_MID, 0, 1);  // Adjust the vertical gap to 2 pixels

    lv_obj_set_flex_flow(digi_ip_panel, LV_FLEX_FLOW_ROW_WRAP);  // Row wrapping for 2x2 layout
    lv_obj_set_flex_align(digi_ip_panel, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    //lv_obj_set_size(digi_ip_panel, 280, 340);  // Ensure digi_ip_panel fits the entire panel

    // Button dimensions
    int spacing = 20;  // Spacing between buttons
    int button_width = 160;  // 2 buttons per row
    int button_height = 180;  // 2 rows

    // Create containers with circle and ON/OFF text for 4 items
    // Create a container for each digital input
    lv_obj_t *cont = lv_obj_create(digi_ip_panel);  // Create container
    lv_obj_set_size(cont, button_width, button_height);  // Set the container size
    lv_obj_set_style_border_width(cont, 1, 0);  // Set border width
    lv_obj_set_style_border_color(cont, lv_color_hex(0xBB6D0A), 0);  // Set border color
    lv_obj_set_style_radius(cont, 10, 0);  // Rounded corners to form a circle shape
    // Make the container background transparent
    lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, 0);  // Set background opacity to transparent
    // Disable the scrollbar on the container
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);
    // Align the container to the center of the digi_ip_panel
    lv_obj_align_to(cont, digi_ip_panel, LV_ALIGN_OUT_TOP_MID, 0, 3);  // Adjust the vertical gap to 2 pixels

    // Create a circle inside the container
    lv_obj_t *circle = lv_obj_create(cont);  // Create a smaller container for the circle
    lv_obj_set_size(circle, button_width * 0.6, button_width * 0.6);  // Adjust size to make it a circle
    lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);  // Make it a perfect circle
    lv_obj_set_style_bg_color(circle, lv_color_hex(0xBB6D0A), 0);  // Light blue background for the circle
    lv_obj_set_style_bg_opa(circle, LV_OPA_10, 0);  // Semi-transparent background for the circle (50% opacity)
    lv_obj_set_style_border_width(circle, 4, 0);  // Bold border for the circle
    lv_obj_set_style_border_color(circle, lv_color_hex(0x578133), 0);  // Blue border color
    lv_obj_set_style_shadow_width(circle, 10, 0);  // Add shadow for depth
    lv_obj_set_style_shadow_color(circle, lv_color_hex(0x8FB271), 0);  // Light shadow for a glowing effect
    lv_obj_center(circle);  // Center the circle inside the container
    lv_obj_set_scrollbar_mode(circle, LV_SCROLLBAR_MODE_OFF);
    
    // Create a label for the "ON" or "OFF" text inside the circle
    lv_obj_t *label = lv_label_create(circle);  // Create a label inside the circle
    char label_text[10];
    snprintf(label_text, sizeof(label_text), "OFF");  // Default text: "OFF"
    lv_label_set_text(label, label_text);  // Set the label text
    lv_obj_set_style_text_color(label, lv_color_hex(0x307A34), 0);  // Black text color
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);  // Slightly larger font for visibility
    lv_obj_center(label);  // Center the label inside the circle
    
    lv_obj_t *cont_diname = lv_obj_create(digi_ip_panel);  // Create container
    lv_obj_set_size(cont_diname, 160, 50);  // Set the container size
    lv_obj_set_style_border_width(cont_diname, 0, 0);  // Set border width
    //lv_obj_set_style_border_color(cont_diname, lv_color_hex(0xBB6D0A), 0);  // Set border color
    lv_obj_set_style_border_color(cont_diname, lv_color_hex(0x000000), 0);  // Set border color
    lv_obj_set_style_radius(cont_diname, 10, 0);  // Rounded corners to form a circle shape
    // Make the container background transparent
    lv_obj_set_style_bg_opa(cont_diname, LV_OPA_TRANSP, 0);  // Set background opacity to transparent
    // Disable the scrollbar on the container
    lv_obj_set_scrollbar_mode(cont_diname, LV_SCROLLBAR_MODE_OFF);
    // Align the container to the center of the digi_ip_panel
    lv_obj_align_to(cont_diname, cont, LV_ALIGN_OUT_TOP_MID, 0, 3);  // Adjust the vertical gap to 2 pixels

    // Create label for text
    label_footer = lv_label_create(cont_diname);
    //char buffer[16];  // Adjust the size based on your maximum expected text length

    std::string text = label_footer_text_di.at(button_id-1);

    //snprintf(buffer, sizeof(buffer), text.c_str());  // Format the text
    lv_label_set_text(label_footer, text.c_str());  // Set the text for the label
    // Center the label inside the container
    lv_obj_center(label_footer);  // Centers the label in the middle of cont_diname

    lv_obj_t *cont_diname_edit = lv_obj_create(digi_ip_panel);  // Create container
    lv_obj_set_size(cont_diname_edit, 260, 80);  // Set the container size
    lv_obj_set_style_border_width(cont_diname_edit, 0, 0);  // Set border width
    lv_obj_set_style_border_color(cont_diname_edit, lv_color_hex(0x000000), 0);  // Set border color
    lv_obj_set_style_radius(cont_diname_edit, 10, 0);  // Rounded corners to form a circle shape
    // Make the container background transparent
    lv_obj_set_style_bg_opa(cont_diname_edit, LV_OPA_TRANSP, 0);  // Set background opacity to transparent
    // Disable the scrollbar on the container
    lv_obj_set_scrollbar_mode(cont_diname_edit, LV_SCROLLBAR_MODE_OFF);
    // Align the container to the center of the digi_ip_panel
    lv_obj_align_to(cont_diname_edit, cont_diname, LV_ALIGN_OUT_TOP_MID, 0, 3);  // Adjust the vertical gap to 2 pixels

    // Create a text area for editing
    text_area_settings = lv_textarea_create(cont_diname_edit);
    lv_obj_set_size(text_area_settings, 160, 30);  // Set size of the text area
    lv_textarea_set_placeholder_text(text_area_settings, "Enter text");  // Set placeholder text
    lv_obj_set_scrollbar_mode(text_area_settings, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(text_area_settings, text_area_event_handler, LV_EVENT_FOCUSED, NULL);  // Attach event handler

    // Create the keyboard but keep it hidden initially
    keyboard = lv_keyboard_create(lv_scr_act());
    lv_obj_set_size(keyboard, LV_PCT(100), LV_PCT(40));  // Adjust size
    lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);   // Align to bottom
    lv_obj_add_event_cb(keyboard, keyboard_event_handler, LV_EVENT_CANCEL, NULL);  // Attach cancel handler
    lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);  // Hide it by default
    
    keyboard_text_area = lv_textarea_create(lv_scr_act());
    lv_obj_align_to(keyboard_text_area, keyboard, LV_ALIGN_TOP_MID, 0, -80);
    lv_obj_set_size(keyboard_text_area, lv_pct(80), 60);
    lv_obj_set_style_border_width(keyboard_text_area, 1, 0);  // Bold border for the circle
    lv_obj_set_style_border_color(keyboard_text_area, lv_color_hex(0xFFA500), 0); // Set border color (blue)
    // Set the font for the text area (you can replace `&lv_font_montserrat_16` with any other font)
    lv_obj_set_style_text_font(keyboard_text_area, &lv_font_montserrat_16, 0);
    lv_obj_add_state(keyboard_text_area, LV_STATE_FOCUSED);
    lv_obj_add_style(keyboard_text_area, &style_ui_island, 0);
    lv_obj_add_flag(keyboard_text_area, LV_OBJ_FLAG_HIDDEN);  // Hide it by default

    // Create an "OK" button
    lv_obj_t *ok_btn = lv_btn_create(cont_diname_edit);
    lv_obj_set_size(ok_btn, 50, 30);
    lv_obj_align_to(ok_btn, text_area_settings, LV_ALIGN_OUT_RIGHT_MID, 10, 0);  // Align the right side of ok_btn with the right side of text_area
    lv_obj_add_event_cb(ok_btn, ok_button_event_handler, LV_EVENT_CLICKED, text_area_settings);  // Pass text_area as user data

    // Add label to "OK" button
    lv_obj_t *btn_label = lv_label_create(ok_btn);
    lv_label_set_text(btn_label, "OK");
    lv_obj_center(btn_label);  // Center the label on the button
}

static void ok_button_event_handler(lv_event_t *e)
{
    lv_obj_t *ok_btn = lv_event_get_target(e);
    lv_obj_t *text_area = (lv_obj_t *)lv_event_get_user_data(e);  // Retrieve the text area object

    if (lv_event_get_code(e) == LV_EVENT_CLICKED)
    {
        const char *new_text = lv_textarea_get_text(text_area);  // Get the entered text
        lv_label_set_text(label_footer, new_text);              // Update the label_footer

        int idx = (int)which_button_di_settings - 1;
        label_footer_text_di.at(idx) = std::string(new_text);
        ESP_LOGI("FooterUpdate", "Label updated to: %s %d", new_text, which_button_di_settings);

        lv_obj_clean(content_container);
        tux_panel_di_fullview(content_container);
    }
}

// Example button click event handler (you can define your custom behavior)
static void button_click_event_handler(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    lv_obj_t *parent = lv_obj_get_parent(obj);  // Fetch the parent of the button
    int button_id = (int)lv_event_get_user_data(e);  // Retrieve button ID

    if (lv_event_get_code(e) == LV_EVENT_PRESSED)
    {
        // Change style on press for click effect
        lv_obj_set_style_bg_color(obj, lv_color_hex(0xBB6D0A), 0);  // Highlight with a different color
        lv_obj_set_style_bg_opa(obj, LV_OPA_50, 0);  // Make it slightly transparent

        ESP_LOGI(TAG, "button_click_event_handler %d", button_id );
        UPDATE_TASK_STATE = DATA_UPDATE_NONE;

    }
    else if (lv_event_get_code(e) == LV_EVENT_RELEASED || lv_event_get_code(e) == LV_EVENT_CLICKED)
    {
        ESP_LOGI(TAG, "button_click_event_handler %d", button_id );
        UPDATE_TASK_STATE = DATA_UPDATE_NONE;

        // Restore style on release
        lv_obj_set_style_bg_color(obj, lv_color_hex(0xBB6D0A), 0);  // Reset to original color
        lv_obj_set_style_bg_opa(obj, LV_OPA_10, 0);  // Reset opacity

        if (lv_event_get_code(e) == LV_EVENT_CLICKED)
        {
            ESP_LOGI(TAG, "Container %d clicked!", button_id );
            // Add your click logic here
            // Call another container to display Button
            //lv_obj_clean(parent);
            //tux_panel_di_settings(parent, button_id);
        }
    }
    else if (lv_event_get_code(e) == LV_EVENT_LONG_PRESSED)
    {
        ESP_LOGI(TAG, "button_click_event_handler %d", button_id );
        UPDATE_TASK_STATE = DATA_UPDATE_NONE;

        // Restore style on release
        lv_obj_set_style_bg_color(obj, lv_color_hex(0xBB6D0A), 0);  // Reset to original color
        lv_obj_set_style_bg_opa(obj, LV_OPA_10, 0);  // Reset opacity

        ESP_LOGI(TAG, "Container %d clicked!", button_id );
        // Add your click logic here
        // Call another container to display Button
        lv_obj_clean(parent);
        tux_panel_di_settings(parent, button_id);
    }
}

// Event handler for the circle
void circle_event_handler(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    lv_obj_t *circle = lv_event_get_target(e);

    if (lv_event_get_code(e) == LV_EVENT_PRESSED)
    {
        LV_LOG_USER("Circle LV_EVENT_PRESSED!");
        // Forward the click event to the parent
        lv_obj_t *parent = lv_obj_get_parent(circle);
        lv_event_send(parent, LV_EVENT_PRESSED, NULL);  // Send the clicked event to the parent
    }
    else if (lv_event_get_code(e) == LV_EVENT_RELEASED || lv_event_get_code(e) == LV_EVENT_CLICKED)
    {
        LV_LOG_USER("Circle LV_EVENT_RELEASED or LV_EVENT_CLICKED !");
        // Forward the click event to the parent
        lv_obj_t *parent = lv_obj_get_parent(circle);
        
        if (lv_event_get_code(e) == LV_EVENT_CLICKED)
        {
            ESP_LOGI(TAG, "Circle clicked!");
            // Add your click logic here
            lv_event_send(parent, LV_EVENT_CLICKED, NULL);  // Send the clicked event to the parent
        }
        
    }
    else if (lv_event_get_code(e) == LV_EVENT_LONG_PRESSED)
    {
        LV_LOG_USER("Circle LV_EVENT_RELEASED or LV_EVENT_CLICKED !");
        // Forward the click event to the parent
        lv_obj_t *parent = lv_obj_get_parent(circle);
        
        ESP_LOGI(TAG, "Circle clicked!");
        // Add your click logic here
        lv_event_send(parent, LV_EVENT_LONG_PRESSED, NULL);  // Send the clicked event to the parent
        
    }
}

// Event handler for text area
static void text_area_event_handler(lv_event_t *e)
{
    //lv_obj_clean(parent_di_settings);
    lv_obj_t *text_area = lv_event_get_target(e);

    // Show the keyboard when the text area is clicked
    if (lv_event_get_code(e) == LV_EVENT_FOCUSED)
    {
        if (!keyboard)
        {
            keyboard = lv_keyboard_create(lv_scr_act());  // Create the keyboard
            lv_obj_set_size(keyboard, LV_PCT(100), LV_PCT(40));  // Resize it to fit the screen
            lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);   // Align it to the bottom
        }

        lv_keyboard_set_textarea(keyboard, keyboard_text_area);  // Link the keyboard to the text area
        lv_obj_clear_flag(keyboard, LV_OBJ_FLAG_HIDDEN);  // Make the keyboard visible
        lv_obj_clear_flag(keyboard_text_area, LV_OBJ_FLAG_HIDDEN);  // Make the keyboard visible
    }
}

// Event handler for keyboard
static void keyboard_event_handler(lv_event_t *e)
{
    lv_obj_t *kb = lv_event_get_target(e);

    printf("LEY EVENT: %d", lv_event_get_code(e));

    if (lv_event_get_code(e) == LV_EVENT_CANCEL)
    {
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);  // Hide the keyboard on cancel
        lv_obj_add_flag(keyboard_text_area, LV_OBJ_FLAG_HIDDEN);  // Hide the keyboard on cancel
        lv_obj_t *textarea = lv_keyboard_get_textarea(kb);  // Get the linked text area
        if (textarea)
        {
            lv_obj_clear_state(textarea, LV_STATE_FOCUSED);  // Clear the focus state
        }

        lv_obj_clear_state(text_area_settings, LV_STATE_FOCUSED);  // Clear the focus state
        const char *text = lv_textarea_get_text(keyboard_text_area); // Get the content of keyboard_text_area
        lv_textarea_set_text(text_area_settings, text);
        // Clear the text content of the text area
        lv_textarea_set_text(keyboard_text_area, "");
        //tux_panel_di_settings(parent_di_settings);
    }
}

// static void keyboard_event_handler(lv_event_t *e)
// {
//     lv_obj_t *kb = lv_event_get_target(e);

//     // Check if the event is related to a key press (LV_EVENT_VALUE_CHANGED)
//     if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED)
//     {
//         //uint32_t key = lv_keyboard_get_active_btn(kb);  // Get the currently pressed key
//         uint32_t key = lv_event_get_param(e);  // Get the key that was pressed
//         // Check if the pressed key is the Enter key
//         if (key == LV_KEY_ENTER)
//         {
//             lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);  // Hide the keyboard

//             lv_obj_t *textarea = lv_keyboard_get_textarea(kb);  // Get the linked text area
//             if (textarea)
//             {
//                 lv_obj_clear_state(textarea, LV_STATE_FOCUSED);  // Remove focus from the text area
//             }
//         }
//     }
//     // Handle cancel event as well, in case of pressing cancel
//     else if (lv_event_get_code(e) == LV_EVENT_CANCEL)
//     {
//         lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);  // Hide the keyboard on cancel
//         lv_obj_t *textarea = lv_keyboard_get_textarea(kb);  // Get the linked text area
//         if (textarea)
//         {
//             lv_obj_clear_state(textarea, LV_STATE_FOCUSED);  // Remove focus from the text area
//         }
//     }
// }

static void goback_di_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    if(code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG,"goback_di_event_handler");
        UPDATE_TASK_STATE = DATA_UPDATE_NONE;
        lv_obj_clean(content_container);
        create_page_home(content_container);
    }
}

static void panel_do_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    ESP_LOGI(TAG,"DIGITAL OUTPUT CLICKED");

    if(code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG,"Digital Output SETTINGS Page will open");
        lv_obj_clean(content_container);
        create_page_digitaloutput(content_container);
        anim_move_left_x(content_container,screen_w,0,200);
        //lv_msg_send(MSG_PAGE_SETTINGS,NULL);
    }
}

static void tux_panel_do_fullview(lv_obj_t *parent)
{
    // For Title
    lv_obj_t *tux_fullview_title = tux_panel_create(parent, "", 50);
    lv_obj_add_style(tux_fullview_title, &style_ui_island, 0);
    lv_obj_t *digi_title_panel = tux_panel_get_content(tux_fullview_title);

    lv_obj_t *title_label = lv_label_create(digi_title_panel);
    lv_label_set_text(title_label, "Digital Output");
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_24, 0); // Set font size
    lv_obj_set_style_text_align(title_label, LV_TEXT_ALIGN_CENTER, 0); // Align text center
    lv_obj_align(title_label, LV_ALIGN_CENTER, 0, 0); // Position at the top middle
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFA500), 0); // Orange color

    lv_obj_t *cont = lv_obj_create(digi_title_panel);  // Create container
    lv_obj_set_size(cont, 60, 40);  // Set the container size
    lv_obj_set_style_border_width(cont, 0, 0);  // Set border width
    lv_obj_set_style_border_color(cont, lv_color_hex(0xBB6D0A), 0);  // Set border color
    lv_obj_set_style_radius(cont, 10, 0);  // Rounded corners to form a circle shape
    // Make the container background transparent
    lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, 0);  // Set background opacity to transparent
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_align_to(cont, title_label, LV_ALIGN_OUT_RIGHT_MID, 5, 0); // Positioned to the right of title_label
    lv_obj_set_style_pad_left(cont, 20, 0);

    // Add the LV_SYMBOL_LEFT icon
    lv_obj_t *icon_label = lv_label_create(cont);
    lv_label_set_text(icon_label, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_font(icon_label, &lv_font_montserrat_16, 0);
    lv_obj_center(icon_label);  // Center align icon_label within cont


    // Enable the panel to be clickable
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    // Set up an event callback for the panel
    lv_obj_add_event_cb(cont, goback_di_event_handler, LV_EVENT_CLICKED, NULL);

    lv_obj_t *tux_fullview = tux_panel_create(parent, "", 340);
    lv_obj_add_style(tux_fullview, &style_ui_island, 0);

    // Adjust positions to create a thin gap between the panels
    lv_obj_align_to(tux_fullview, tux_fullview_title, LV_ALIGN_OUT_BOTTOM_MID, 0, 1); // Thin gap of 2 pixels

    // Get content area of the panel
    lv_obj_t *digi_panel = tux_panel_get_content(tux_fullview);
    lv_obj_set_flex_flow(digi_panel, LV_FLEX_FLOW_ROW_WRAP);  // Row wrapping for 2x2 layout
    lv_obj_set_flex_align(digi_panel, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Adjust padding to ensure consistent spacing
    lv_obj_set_style_pad_left(digi_panel, 20, 0);  // Left padding
    lv_obj_set_style_pad_right(digi_panel, 10, 0); // Right padding
    lv_obj_set_style_pad_top(digi_panel, 5, 0);    // Top padding
    lv_obj_set_style_pad_bottom(digi_panel, 5, 0); // Bottom padding

    lv_obj_set_size(digi_panel, 280, 340);  // Ensure digi_panel fits the entire panel

    // Add row and column gaps between containers
    lv_obj_set_style_pad_row(digi_panel, 20, 0);  // Vertical spacing between rows
    lv_obj_set_style_pad_column(digi_panel, 20, 0);  // Horizontal spacing between columns

    // Button dimensions
    int spacing = 20;  // Spacing between buttons
    int button_width = (220 - spacing) / 2;  // 2 buttons per row
    int button_height = (280 - spacing) / 2;  // 2 rows

    // Create containers with circle and ON/OFF text for 4 items
    for (int i = 0; i < 4; i++) {
        int id = i+1;
        // Create a container for each digital input
        lv_obj_t *cont = lv_obj_create(digi_panel);  // Create container
        lv_obj_set_size(cont, button_width, button_height);  // Set the container size
        lv_obj_set_style_border_width(cont, 1, 0);  // Set border width
        lv_obj_set_style_border_color(cont, lv_color_hex(0xBB6D0A), 0);  // Set border color
        lv_obj_set_style_radius(cont, 10, 0);  // Rounded corners to form a circle shape

        // Make the container background transparent
        lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, 0);  // Set background opacity to transparent

        // Create label for text
        lv_obj_t *label_footer = lv_label_create(cont);
        char buffer[16];  // Adjust the size based on your maximum expected text length
        snprintf(buffer, sizeof(buffer), "DO-%d", id);  // Format the text

        std::string text = label_footer_text_do.at(i);
        if(text == "")
        {
            lv_label_set_text(label_footer, buffer);  // Set the text for the label
            label_footer_text_do.at(i) = std::string(buffer);
        }
        else
        {
            lv_label_set_text(label_footer, text.c_str());  // Set the text for the label
        }
        lv_obj_align(label_footer, LV_ALIGN_BOTTOM_MID, 0, 0);  // Align the label to the bottom middle of the container

        // Create a circle inside the container
        lv_obj_t *circle = lv_obj_create(cont);  // Create a smaller container for the circle
        label_do_circle_list[i] = circle;
        lv_obj_set_size(circle, button_width * 0.6, button_width * 0.6);  // Adjust size to make it a circle
        lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);  // Make it a perfect circle
        lv_obj_set_style_bg_color(circle, lv_color_hex(0xBB6D0A), 0);  // Light blue background for the circle
        lv_obj_set_style_bg_opa(circle, LV_OPA_10, 0);  // Semi-transparent background for the circle (50% opacity)
        lv_obj_set_style_border_width(circle, 4, 0);  // Bold border for the circle
        lv_obj_set_style_border_color(circle, lv_color_hex(0x578133), 0);  // Blue border color
        lv_obj_set_style_shadow_width(circle, 10, 0);  // Add shadow for depth
        lv_obj_set_style_shadow_color(circle, lv_color_hex(0x8FB271), 0);  // Light shadow for a glowing effect
        lv_obj_center(circle);  // Center the circle inside the container
        lv_obj_set_scrollbar_mode(circle, LV_SCROLLBAR_MODE_OFF);

        // Create a label for the "ON" or "OFF" text inside the circle
        lv_obj_t *label = lv_label_create(circle);  // Create a label inside the circle
        // char label_text[10];
        // snprintf(label_text, sizeof(label_text), "OFF");  // Default text: "OFF"
        // lv_label_set_text(label, label_text);  // Set the label text
        label_do_obj_list[i] = label;
        lv_obj_set_style_text_color(label, lv_color_hex(0xb53e02), 0);  // Black text color
        lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);  // Slightly larger font for visibility
        lv_obj_center(label);  // Center the label inside the circle
        // Disable the scrollbar on the container
        lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);

        char label_text[10];
        snprintf(label_text, sizeof(label_text), "OFF");  // Default text: "OFF"
        std::string valDO = label_do_values.at(i);
        if(valDO == "")
        {
            lv_label_set_text(label, label_text);  // Set the text for the label
            label_do_values.at(i) = std::string(label_text);
            lv_obj_set_style_border_color(label_do_circle_list[i], lv_color_hex(0xb53e02), 0);
        }
        else
        {
            lv_label_set_text(label, valDO.c_str());  // Set the text for the label
            if(valDO == "OFF")
            {
                lv_obj_set_style_border_color(label_do_circle_list[i], lv_color_hex(0xb53e02), 0);
            }
            else
            {
                lv_obj_set_style_border_color(label_do_circle_list[i], lv_color_hex(0x578133), 0);
                lv_obj_set_style_text_color(label_do_obj_list[i], lv_color_hex(0x307A34), 0);
            }
        }
        
        // Enable container click
        lv_obj_add_flag(circle, LV_OBJ_FLAG_CLICKABLE);  // Make the container clickable
        // Add event callback to change text and color when clicked
        lv_obj_add_event_cb(circle, circle_event_handler, LV_EVENT_ALL, (void *)id);

        // Enable container click
        lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);  // Make the container clickable
        // Add event callback to change text and color when clicked
        lv_obj_add_event_cb(cont, button_outp_click_event_handler, LV_EVENT_ALL, (void *)id);
    }

    lv_msg_send(MSG_PAGE_DO_VAL, NULL);
}

// Example button click event handler (you can define your custom behavior)
static void button_outp_click_event_handler(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    lv_obj_t *parent = lv_obj_get_parent(obj);  // Fetch the parent of the button
    int button_id = (int)lv_event_get_user_data(e);  // Retrieve button ID

    if (lv_event_get_code(e) == LV_EVENT_PRESSED)
    {
        ESP_LOGI(TAG, "button_outp_click_event_handler %d", button_id );
        UPDATE_TASK_STATE = DATA_UPDATE_NONE;

        // Change style on press for click effect
        lv_obj_set_style_bg_color(obj, lv_color_hex(0xBB6D0A), 0);  // Highlight with a different color
        lv_obj_set_style_bg_opa(obj, LV_OPA_50, 0);  // Make it slightly transparent
    }
    else if (lv_event_get_code(e) == LV_EVENT_RELEASED || lv_event_get_code(e) == LV_EVENT_CLICKED)
    {
        ESP_LOGI(TAG, "button_outp_click_event_handler %d", button_id );
        UPDATE_TASK_STATE = DATA_UPDATE_NONE;

        // Restore style on release
        lv_obj_set_style_bg_color(obj, lv_color_hex(0xBB6D0A), 0);  // Reset to original color
        lv_obj_set_style_bg_opa(obj, LV_OPA_10, 0);  // Reset opacity

        if (lv_event_get_code(e) == LV_EVENT_CLICKED)
        {
            ESP_LOGI(TAG, "Container %d clicked!", button_id );
            // Add your click logic here
            // Call another container to display Button
            lv_obj_clean(parent);
            tux_panel_do_settings(parent, button_id);
        }
    }
    else if (lv_event_get_code(e) == LV_EVENT_LONG_PRESSED)
    {
        ESP_LOGI(TAG, "button_outp_click_event_handler %d", button_id );
        UPDATE_TASK_STATE = DATA_UPDATE_NONE;

        // Restore style on release
        lv_obj_set_style_bg_color(obj, lv_color_hex(0xBB6D0A), 0);  // Reset to original color
        lv_obj_set_style_bg_opa(obj, LV_OPA_10, 0);  // Reset opacity

        //if (lv_event_get_code(e) == LV_EVENT_CLICKED)
        //{
        ESP_LOGI(TAG, "Container %d clicked!", button_id );
        // Add your click logic here
        // Call another container to display Button
        lv_obj_clean(parent);
        tux_panel_do_settings(parent, button_id);
        //}
    }
}

static void tux_panel_do_settings(lv_obj_t *parent, int button_id)
{
    //UPDATE_TASK_STATE = DATA_UPDATE_NONE;
    
    which_button_do_settings = static_cast<lv_coord_t>(button_id);
    // For Title
    lv_obj_t *tux_settings_title = tux_panel_create(parent, "", 50);
    lv_obj_add_style(tux_settings_title, &style_ui_island, 0);
    lv_obj_t *digi_title_panel = tux_panel_get_content(tux_settings_title);

    lv_obj_t *title_label = lv_label_create(digi_title_panel);
    lv_label_set_text(title_label, "Digital Output");
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_24, 0); // Set font size
    lv_obj_set_style_text_align(title_label, LV_TEXT_ALIGN_CENTER, 0); // Align text center
    lv_obj_align(title_label, LV_ALIGN_CENTER, 0, 0); // Position at the top middle
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFA500), 0); // Orange color

    lv_obj_t *tux_settings_panel = tux_panel_create(parent, "", 340);
    lv_obj_add_style(tux_settings_panel, &style_ui_island, 0);
    lv_obj_set_size(tux_settings_panel, 280, 340);  // Set the container size
    // Adjust positions to create a thin gap between the panels
    lv_obj_align_to(tux_settings_panel, digi_title_panel, LV_ALIGN_OUT_BOTTOM_MID, 0, 1); // Thin gap of 2 pixels

    // Get content area of the panel
    lv_obj_t *digi_panel = tux_panel_get_content(tux_settings_panel);
    // Align digi_ip_panel to the top of tux_di_settings with a 1 or 2 pixel gap
    lv_obj_align_to(digi_panel, tux_settings_panel, LV_ALIGN_OUT_TOP_MID, 0, 1);  // Adjust the vertical gap to 2 pixels

    lv_obj_set_flex_flow(digi_panel, LV_FLEX_FLOW_ROW_WRAP);  // Row wrapping for 2x2 layout
    lv_obj_set_flex_align(digi_panel, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Button dimensions
    int spacing = 20;  // Spacing between buttons
    int button_width = 160;  // 2 buttons per row
    int button_height = 180;  // 2 rows

    // Create containers with circle and ON/OFF text for 4 items
    // Create a container for each digital input
    lv_obj_t *cont = lv_obj_create(digi_panel);  // Create container
    lv_obj_set_size(cont, button_width, button_height);  // Set the container size
    lv_obj_set_style_border_width(cont, 1, 0);  // Set border width
    lv_obj_set_style_border_color(cont, lv_color_hex(0xBB6D0A), 0);  // Set border color
    lv_obj_set_style_radius(cont, 10, 0);  // Rounded corners to form a circle shape
    // Make the container background transparent
    lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, 0);  // Set background opacity to transparent
    // Disable the scrollbar on the container
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);
    // Align the container to the center of the digi_ip_panel
    lv_obj_align_to(cont, digi_panel, LV_ALIGN_OUT_TOP_MID, 0, 3);  // Adjust the vertical gap to 2 pixels

    // Create a circle inside the container
    lv_obj_t *circle = lv_obj_create(cont);  // Create a smaller container for the circle
    lv_obj_set_size(circle, button_width * 0.6, button_width * 0.6);  // Adjust size to make it a circle
    lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);  // Make it a perfect circle
    lv_obj_set_style_bg_color(circle, lv_color_hex(0xBB6D0A), 0);  // Light blue background for the circle
    lv_obj_set_style_bg_opa(circle, LV_OPA_10, 0);  // Semi-transparent background for the circle (50% opacity)
    lv_obj_set_style_border_width(circle, 4, 0);  // Bold border for the circle
    lv_obj_set_style_border_color(circle, lv_color_hex(0x578133), 0);  // Blue border color
    lv_obj_set_style_shadow_width(circle, 10, 0);  // Add shadow for depth
    lv_obj_set_style_shadow_color(circle, lv_color_hex(0x8FB271), 0);  // Light shadow for a glowing effect
    lv_obj_center(circle);  // Center the circle inside the container
    lv_obj_set_scrollbar_mode(circle, LV_SCROLLBAR_MODE_OFF);
    
    // Create a label for the "ON" or "OFF" text inside the circle
    lv_obj_t *label = lv_label_create(circle);  // Create a label inside the circle
    char label_text[10];
    snprintf(label_text, sizeof(label_text), "OFF");  // Default text: "OFF"
    lv_label_set_text(label, label_text);  // Set the label text
    lv_obj_set_style_text_color(label, lv_color_hex(0x307A34), 0);  // Black text color
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);  // Slightly larger font for visibility
    lv_obj_center(label);  // Center the label inside the circle
    
    lv_obj_t *cont_diginame = lv_obj_create(digi_panel);  // Create container
    lv_obj_set_size(cont_diginame, 160, 50);  // Set the container size
    lv_obj_set_style_border_width(cont_diginame, 0, 0);  // Set border width
    lv_obj_set_style_border_color(cont_diginame, lv_color_hex(0x000000), 0);  // Set border color
    lv_obj_set_style_radius(cont_diginame, 10, 0);  // Rounded corners to form a circle shape
    // Make the container background transparent
    lv_obj_set_style_bg_opa(cont_diginame, LV_OPA_TRANSP, 0);  // Set background opacity to transparent
    // Disable the scrollbar on the container
    lv_obj_set_scrollbar_mode(cont_diginame, LV_SCROLLBAR_MODE_OFF);
    // Align the container to the center of the digi_ip_panel
    lv_obj_align_to(cont_diginame, cont, LV_ALIGN_OUT_TOP_MID, 0, 3);  // Adjust the vertical gap to 2 pixels

    // Create label for text
    label_footer = lv_label_create(cont_diginame);
    //char buffer[16];  // Adjust the size based on your maximum expected text length

    std::string text = label_footer_text_do.at(button_id-1);

    //snprintf(buffer, sizeof(buffer), text.c_str());  // Format the text
    lv_label_set_text(label_footer, text.c_str());  // Set the text for the label
    // Center the label inside the container
    lv_obj_center(label_footer);  // Centers the label in the middle of cont_diginame

    lv_obj_t *cont_diginame_edit = lv_obj_create(digi_panel);  // Create container
    lv_obj_set_size(cont_diginame_edit, 260, 80);  // Set the container size
    lv_obj_set_style_border_width(cont_diginame_edit, 0, 0);  // Set border width
    lv_obj_set_style_border_color(cont_diginame_edit, lv_color_hex(0x000000), 0);  // Set border color
    lv_obj_set_style_radius(cont_diginame_edit, 10, 0);  // Rounded corners to form a circle shape
    // Make the container background transparent
    lv_obj_set_style_bg_opa(cont_diginame_edit, LV_OPA_TRANSP, 0);  // Set background opacity to transparent
    // Disable the scrollbar on the container
    lv_obj_set_scrollbar_mode(cont_diginame_edit, LV_SCROLLBAR_MODE_OFF);
    // Align the container to the center of the digi_ip_panel
    lv_obj_align_to(cont_diginame_edit, cont_diginame, LV_ALIGN_OUT_TOP_MID, 0, 3);  // Adjust the vertical gap to 2 pixels

    // Create a text area for editing
    text_area_settings = lv_textarea_create(cont_diginame_edit);
    lv_obj_set_size(text_area_settings, 160, 30);  // Set size of the text area
    lv_textarea_set_placeholder_text(text_area_settings, "Enter text");  // Set placeholder text
    lv_obj_set_scrollbar_mode(text_area_settings, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(text_area_settings, text_area_event_handler, LV_EVENT_FOCUSED, NULL);  // Attach event handler

    // Create the keyboard but keep it hidden initially
    keyboard = lv_keyboard_create(lv_scr_act());
    lv_obj_set_size(keyboard, LV_PCT(100), LV_PCT(40));  // Adjust size
    lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);   // Align to bottom
    lv_obj_add_event_cb(keyboard, keyboard_event_handler, LV_EVENT_CANCEL, NULL);  // Attach cancel handler
    lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);  // Hide it by default
    
    keyboard_text_area = lv_textarea_create(lv_scr_act());
    lv_obj_align_to(keyboard_text_area, keyboard, LV_ALIGN_TOP_MID, 0, -80);
    lv_obj_set_size(keyboard_text_area, lv_pct(80), 60);
    lv_obj_set_style_border_width(keyboard_text_area, 1, 0);  // Bold border for the circle
    lv_obj_set_style_border_color(keyboard_text_area, lv_color_hex(0xFFA500), 0); // Set border color (blue)
    // Set the font for the text area (you can replace `&lv_font_montserrat_16` with any other font)
    lv_obj_set_style_text_font(keyboard_text_area, &lv_font_montserrat_16, 0);
    lv_obj_add_state(keyboard_text_area, LV_STATE_FOCUSED);
    lv_obj_add_style(keyboard_text_area, &style_ui_island, 0);
    lv_obj_add_flag(keyboard_text_area, LV_OBJ_FLAG_HIDDEN);  // Hide it by default

    // Create an "OK" button
    lv_obj_t *ok_btn = lv_btn_create(cont_diginame_edit);
    lv_obj_set_size(ok_btn, 50, 30);
    lv_obj_align_to(ok_btn, text_area_settings, LV_ALIGN_OUT_RIGHT_MID, 10, 0);  // Align the right side of ok_btn with the right side of text_area
    lv_obj_add_event_cb(ok_btn, ok_button_outp_event_handler, LV_EVENT_CLICKED, text_area_settings);  // Pass text_area as user data

    // Add label to "OK" button
    lv_obj_t *btn_label = lv_label_create(ok_btn);
    lv_label_set_text(btn_label, "OK");
    lv_obj_center(btn_label);  // Center the label on the button
}

static void ok_button_outp_event_handler(lv_event_t *e)
{
    lv_obj_t *ok_btn = lv_event_get_target(e);
    lv_obj_t *text_area = (lv_obj_t *)lv_event_get_user_data(e);  // Retrieve the text area object

    if (lv_event_get_code(e) == LV_EVENT_CLICKED)
    {
        const char *new_text = lv_textarea_get_text(text_area);  // Get the entered text
        lv_label_set_text(label_footer, new_text);              // Update the label_footer

        int idx = (int)which_button_do_settings - 1;
        label_footer_text_do.at(idx) = std::string(new_text);
        ESP_LOGI("FooterUpdate", "Label updated to: %s %d", new_text, which_button_do_settings);

        lv_obj_clean(content_container);
        tux_panel_do_fullview(content_container);
    }
}


static void panel_ana_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    ESP_LOGI(TAG,"ANALOG INPUT CLICKED");

    if(code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG,"Analog SETTINGS Page will open");
        lv_obj_clean(content_container);
        create_page_analog(content_container);
        anim_move_left_x(content_container,screen_w,0,200);
        //lv_msg_send(MSG_PAGE_SETTINGS,NULL);
    }
}

static void tux_panel_ana_fullview(lv_obj_t *parent)
{
    // For Title
    lv_obj_t *tux_fullview_title = tux_panel_create(parent, "", 50);
    lv_obj_add_style(tux_fullview_title, &style_ui_island, 0);
    lv_obj_t *digi_title_panel = tux_panel_get_content(tux_fullview_title);

    lv_obj_t *title_label = lv_label_create(digi_title_panel);
    lv_label_set_text(title_label, "Analog Input");
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_24, 0); // Set font size
    lv_obj_set_style_text_align(title_label, LV_TEXT_ALIGN_CENTER, 0); // Align text center
    lv_obj_align(title_label, LV_ALIGN_CENTER, 0, 0); // Position at the top middle
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFA500), 0); // Orange color

    lv_obj_t *cont = lv_obj_create(digi_title_panel);  // Create container
    lv_obj_set_size(cont, 60, 40);  // Set the container size
    lv_obj_set_style_border_width(cont, 0, 0);  // Set border width
    lv_obj_set_style_border_color(cont, lv_color_hex(0xBB6D0A), 0);  // Set border color
    lv_obj_set_style_radius(cont, 10, 0);  // Rounded corners to form a circle shape
    // Make the container background transparent
    lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, 0);  // Set background opacity to transparent
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_align_to(cont, title_label, LV_ALIGN_OUT_RIGHT_MID, 5, 0); // Positioned to the right of title_label
    lv_obj_set_style_pad_left(cont, 20, 0);

    // Add the LV_SYMBOL_LEFT icon
    lv_obj_t *icon_label = lv_label_create(cont);
    lv_label_set_text(icon_label, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_font(icon_label, &lv_font_montserrat_16, 0);
    lv_obj_center(icon_label);  // Center align icon_label within cont


    // Enable the panel to be clickable
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    // Set up an event callback for the panel
    lv_obj_add_event_cb(cont, goback_di_event_handler, LV_EVENT_CLICKED, NULL);

    lv_obj_t *tux_fullview = tux_panel_create(parent, "", 340);
    lv_obj_add_style(tux_fullview, &style_ui_island, 0);

    // Adjust positions to create a thin gap between the panels
    lv_obj_align_to(tux_fullview, tux_fullview_title, LV_ALIGN_OUT_BOTTOM_MID, 0, 1); // Thin gap of 2 pixels

    // Get content area of the panel
    lv_obj_t *analog_panel = tux_panel_get_content(tux_fullview);
    lv_obj_set_flex_flow(analog_panel, LV_FLEX_FLOW_ROW_WRAP);  // Row wrapping for 2x2 layout
    lv_obj_set_flex_align(analog_panel, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Adjust padding to ensure consistent spacing
    lv_obj_set_style_pad_left(analog_panel, 20, 0);  // Left padding
    lv_obj_set_style_pad_right(analog_panel, 10, 0); // Right padding
    lv_obj_set_style_pad_top(analog_panel, 5, 0);    // Top padding
    lv_obj_set_style_pad_bottom(analog_panel, 5, 0); // Bottom padding

    lv_obj_set_size(analog_panel, 280, 340);  // Ensure analog_panel fits the entire panel

    // Add row and column gaps between containers
    lv_obj_set_style_pad_row(analog_panel, 20, 0);  // Vertical spacing between rows
    lv_obj_set_style_pad_column(analog_panel, 20, 0);  // Horizontal spacing between columns

    // Button dimensions
    int spacing = 20;  // Spacing between buttons
    int button_width = (220 - spacing) / 2;  // 2 buttons per row
    int button_height = (280 - spacing) / 2;  // 2 rows

    // Create containers with circle and ON/OFF text for 4 items
    for (int i = 0; i < 4; i++) {
        int id = i+1;
        // Create a container for each digital input
        lv_obj_t *cont = lv_obj_create(analog_panel);  // Create container
        lv_obj_set_size(cont, button_width, button_height);  // Set the container size
        lv_obj_set_style_border_width(cont, 1, 0);  // Set border width
        lv_obj_set_style_border_color(cont, lv_color_hex(0xBB6D0A), 0);  // Set border color
        lv_obj_set_style_radius(cont, 10, 0);  // Rounded corners to form a circle shape

        // Make the container background transparent
        lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, 0);  // Set background opacity to transparent

        // Create label for text
        lv_obj_t *label_footer = lv_label_create(cont);
        char buffer[16];  // Adjust the size based on your maximum expected text length
        snprintf(buffer, sizeof(buffer), "AI-%d", id);  // Format the text

        std::string text = label_footer_text_ana.at(i);
        if(text == "")
        {
            lv_label_set_text(label_footer, buffer);  // Set the text for the label
            label_footer_text_ana.at(i) = std::string(buffer);
        }
        else
        {
            lv_label_set_text(label_footer, text.c_str());  // Set the text for the label
        }
        lv_obj_align(label_footer, LV_ALIGN_BOTTOM_MID, 0, 0);  // Align the label to the bottom middle of the container
        // Disable the scrollbar on the container
        lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);

        /*Create an Arc*/
        lv_obj_t * arc = lv_arc_create(cont);
        arc_array_analog[i] = arc; 
        lv_arc_set_mode(arc, LV_ARC_MODE_NORMAL);
        lv_obj_set_size(arc, 70, 70);
        lv_arc_set_rotation(arc, 135);
        lv_arc_set_bg_angles(arc, 0, 270);
        lv_obj_set_style_arc_width(arc, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_arc_width(arc, 1, LV_PART_INDICATOR | LV_STATE_DEFAULT);
        lv_arc_set_value(arc, 10);
        lv_obj_center(arc);
        lv_obj_clear_flag(arc, LV_OBJ_FLAG_SCROLLABLE);
        //lv_obj_set_click(arc, false);
        lv_obj_remove_style(arc, NULL, LV_PART_KNOB);

        lv_obj_t *label_ana_val = lv_label_create(cont);
        label_ana_val_list[i] = label_ana_val;
        //lv_label_set_text(label_ana_val, "Value");  // Set the text for the label
        char abuffer[16];  // Adjust the size based on your maximum expected text length
        snprintf(abuffer, sizeof(buffer), "V.%d", id);  // Format the text
        std::string aval = label_ana_values.at(i);
        if(aval == "")
        {
            lv_label_set_text(label_ana_val, abuffer);  // Set the text for the label
            label_ana_values.at(i) = std::string(abuffer);
        }
        else
        {
            lv_label_set_text(label_ana_val, aval.c_str());  // Set the text for the label
        }

        // Optional: Avoid constraining label size unless necessary
        lv_obj_set_size(label_ana_val, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_center(label_ana_val);

        // Enable container click
        lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);  // Make the container clickable
        // Add event callback to change text and color when clicked
        lv_obj_add_event_cb(cont, button_ana_click_event_handler, LV_EVENT_ALL, (void *)id);
    }

    lv_msg_send(MSG_PAGE_ANALOG_VAL, NULL);
}

// Example button click event handler (you can define your custom behavior)
static void button_ana_click_event_handler(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    lv_obj_t *parent = lv_obj_get_parent(obj);  // Fetch the parent of the button
    int button_id = (int)lv_event_get_user_data(e);  // Retrieve button ID

    if (lv_event_get_code(e) == LV_EVENT_PRESSED)
    {
        ESP_LOGI(TAG, "button_ana_click_event_handler %d", button_id );
        UPDATE_TASK_STATE = DATA_UPDATE_NONE;

        // Change style on press for click effect
        lv_obj_set_style_bg_color(obj, lv_color_hex(0xBB6D0A), 0);  // Highlight with a different color
        lv_obj_set_style_bg_opa(obj, LV_OPA_50, 0);  // Make it slightly transparent
    }
    else if (lv_event_get_code(e) == LV_EVENT_RELEASED || lv_event_get_code(e) == LV_EVENT_CLICKED)
    {
        ESP_LOGI(TAG, "button_ana_click_event_handler %d", button_id );
        UPDATE_TASK_STATE = DATA_UPDATE_NONE;
        
        // Restore style on release
        lv_obj_set_style_bg_color(obj, lv_color_hex(0xBB6D0A), 0);  // Reset to original color
        lv_obj_set_style_bg_opa(obj, LV_OPA_10, 0);  // Reset opacity

        if (lv_event_get_code(e) == LV_EVENT_CLICKED)
        {
            ESP_LOGI(TAG, "Container %d clicked!", button_id );
            // Add your click logic here
            // Call another container to display Button
            lv_obj_clean(parent);
            tux_panel_ana_settings(parent, button_id);
        }
    }
    else if (lv_event_get_code(e) == LV_EVENT_LONG_PRESSED)
    {
        ESP_LOGI(TAG, "button_ana_click_event_handler %d", button_id );
        UPDATE_TASK_STATE = DATA_UPDATE_NONE;
        
        // Restore style on release
        lv_obj_set_style_bg_color(obj, lv_color_hex(0xBB6D0A), 0);  // Reset to original color
        lv_obj_set_style_bg_opa(obj, LV_OPA_10, 0);  // Reset opacity

        ESP_LOGI(TAG, "Container %d clicked!", button_id );
        // Add your click logic here
        // Call another container to display Button
        lv_obj_clean(parent);
        tux_panel_ana_settings(parent, button_id);
    }
}

static void tux_panel_ana_settings(lv_obj_t *parent, int button_id)
{
    which_button_ana_settings = static_cast<lv_coord_t>(button_id);
    // For Title
    lv_obj_t *tux_settings_title = tux_panel_create(parent, "", 50);
    lv_obj_add_style(tux_settings_title, &style_ui_island, 0);
    lv_obj_t *digi_title_panel = tux_panel_get_content(tux_settings_title);

    lv_obj_t *title_label = lv_label_create(digi_title_panel);
    lv_label_set_text(title_label, "Analog Input");
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_24, 0); // Set font size
    lv_obj_set_style_text_align(title_label, LV_TEXT_ALIGN_CENTER, 0); // Align text center
    lv_obj_align(title_label, LV_ALIGN_CENTER, 0, 0); // Position at the top middle
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFA500), 0); // Orange color

    lv_obj_t *tux_settings_panel = tux_panel_create(parent, "", 340);
    lv_obj_add_style(tux_settings_panel, &style_ui_island, 0);
    lv_obj_set_size(tux_settings_panel, 280, 340);  // Set the container size
    // Adjust positions to create a thin gap between the panels
    lv_obj_align_to(tux_settings_panel, digi_title_panel, LV_ALIGN_OUT_BOTTOM_MID, 0, 1); // Thin gap of 2 pixels

    // Get content area of the panel
    lv_obj_t *digi_panel = tux_panel_get_content(tux_settings_panel);
    // Align digi_ip_panel to the top of tux_di_settings with a 1 or 2 pixel gap
    lv_obj_align_to(digi_panel, tux_settings_panel, LV_ALIGN_OUT_TOP_MID, 0, 1);  // Adjust the vertical gap to 2 pixels

    lv_obj_set_flex_flow(digi_panel, LV_FLEX_FLOW_ROW_WRAP);  // Row wrapping for 2x2 layout
    lv_obj_set_flex_align(digi_panel, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Button dimensions
    int spacing = 20;  // Spacing between buttons
    int button_width = 160;  // 2 buttons per row
    int button_height = 180;  // 2 rows

    // Create a container for each digital input
    lv_obj_t *cont = lv_obj_create(digi_panel);  // Create container
    lv_obj_set_size(cont, button_width, button_height);  // Set the container size
    lv_obj_set_style_border_width(cont, 1, 0);  // Set border width
    lv_obj_set_style_border_color(cont, lv_color_hex(0xBB6D0A), 0);  // Set border color
    lv_obj_set_style_radius(cont, 10, 0);  // Rounded corners to form a circle shape
    // Make the container background transparent
    lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, 0);  // Set background opacity to transparent
    // Disable the scrollbar on the container
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);
    // Align the container to the center of the digi_ip_panel
    lv_obj_align_to(cont, digi_panel, LV_ALIGN_OUT_TOP_MID, 0, 3);  // Adjust the vertical gap to 2 pixels

    // Create a circle inside the container
    lv_obj_t *circle = lv_obj_create(cont);  // Create a smaller container for the circle
    lv_obj_set_size(circle, button_width * 0.6, button_width * 0.6);  // Adjust size to make it a circle
    lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);  // Make it a perfect circle
    lv_obj_set_style_bg_color(circle, lv_color_hex(0xBB6D0A), 0);  // Light blue background for the circle
    lv_obj_set_style_bg_opa(circle, LV_OPA_10, 0);  // Semi-transparent background for the circle (50% opacity)
    lv_obj_set_style_border_width(circle, 4, 0);  // Bold border for the circle
    lv_obj_set_style_border_color(circle, lv_color_hex(0x578133), 0);  // Blue border color
    lv_obj_set_style_shadow_width(circle, 10, 0);  // Add shadow for depth
    lv_obj_set_style_shadow_color(circle, lv_color_hex(0x8FB271), 0);  // Light shadow for a glowing effect
    lv_obj_center(circle);  // Center the circle inside the container
    lv_obj_set_scrollbar_mode(circle, LV_SCROLLBAR_MODE_OFF);
    
    // Create a label for the "ON" or "OFF" text inside the circle
    lv_obj_t *label = lv_label_create(circle);  // Create a label inside the circle
    char label_text[10];
    snprintf(label_text, sizeof(label_text), "OFF");  // Default text: "OFF"
    lv_label_set_text(label, label_text);  // Set the label text
    lv_obj_set_style_text_color(label, lv_color_hex(0x307A34), 0);  // Black text color
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);  // Slightly larger font for visibility
    lv_obj_center(label);  // Center the label inside the circle
    
    lv_obj_t *cont_diginame = lv_obj_create(digi_panel);  // Create container
    lv_obj_set_size(cont_diginame, 160, 50);  // Set the container size
    lv_obj_set_style_border_width(cont_diginame, 0, 0);  // Set border width
    lv_obj_set_style_border_color(cont_diginame, lv_color_hex(0x000000), 0);  // Set border color
    lv_obj_set_style_radius(cont_diginame, 10, 0);  // Rounded corners to form a circle shape
    // Make the container background transparent
    lv_obj_set_style_bg_opa(cont_diginame, LV_OPA_TRANSP, 0);  // Set background opacity to transparent
    // Disable the scrollbar on the container
    lv_obj_set_scrollbar_mode(cont_diginame, LV_SCROLLBAR_MODE_OFF);
    // Align the container to the center of the digi_ip_panel
    lv_obj_align_to(cont_diginame, cont, LV_ALIGN_OUT_TOP_MID, 0, 3);  // Adjust the vertical gap to 2 pixels

    // Create label for text
    label_footer = lv_label_create(cont_diginame);
    //char buffer[16];  // Adjust the size based on your maximum expected text length

    std::string text = label_footer_text_ana.at(button_id-1);

    //snprintf(buffer, sizeof(buffer), text.c_str());  // Format the text
    lv_label_set_text(label_footer, text.c_str());  // Set the text for the label
    // Center the label inside the container
    lv_obj_center(label_footer);  // Centers the label in the middle of cont_diginame

    lv_obj_t *cont_diginame_edit = lv_obj_create(digi_panel);  // Create container
    lv_obj_set_size(cont_diginame_edit, 260, 80);  // Set the container size
    lv_obj_set_style_border_width(cont_diginame_edit, 0, 0);  // Set border width
    lv_obj_set_style_border_color(cont_diginame_edit, lv_color_hex(0x000000), 0);  // Set border color
    lv_obj_set_style_radius(cont_diginame_edit, 10, 0);  // Rounded corners to form a circle shape
    // Make the container background transparent
    lv_obj_set_style_bg_opa(cont_diginame_edit, LV_OPA_TRANSP, 0);  // Set background opacity to transparent
    // Disable the scrollbar on the container
    lv_obj_set_scrollbar_mode(cont_diginame_edit, LV_SCROLLBAR_MODE_OFF);
    // Align the container to the center of the digi_ip_panel
    lv_obj_align_to(cont_diginame_edit, cont_diginame, LV_ALIGN_OUT_TOP_MID, 0, 3);  // Adjust the vertical gap to 2 pixels

    // Create a text area for editing
    text_area_settings = lv_textarea_create(cont_diginame_edit);
    lv_obj_set_size(text_area_settings, 160, 30);  // Set size of the text area
    lv_textarea_set_placeholder_text(text_area_settings, "Enter text");  // Set placeholder text
    lv_obj_set_scrollbar_mode(text_area_settings, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(text_area_settings, text_area_event_handler, LV_EVENT_FOCUSED, NULL);  // Attach event handler

    // Create the keyboard but keep it hidden initially
    keyboard = lv_keyboard_create(lv_scr_act());
    lv_obj_set_size(keyboard, LV_PCT(100), LV_PCT(40));  // Adjust size
    lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);   // Align to bottom
    lv_obj_add_event_cb(keyboard, keyboard_event_handler, LV_EVENT_CANCEL, NULL);  // Attach cancel handler
    lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);  // Hide it by default
    
    keyboard_text_area = lv_textarea_create(lv_scr_act());
    lv_obj_align_to(keyboard_text_area, keyboard, LV_ALIGN_TOP_MID, 0, -80);
    lv_obj_set_size(keyboard_text_area, lv_pct(80), 60);
    lv_obj_set_style_border_width(keyboard_text_area, 1, 0);  // Bold border for the circle
    lv_obj_set_style_border_color(keyboard_text_area, lv_color_hex(0xFFA500), 0); // Set border color (blue)
    // Set the font for the text area (you can replace `&lv_font_montserrat_16` with any other font)
    lv_obj_set_style_text_font(keyboard_text_area, &lv_font_montserrat_16, 0);
    lv_obj_add_state(keyboard_text_area, LV_STATE_FOCUSED);
    lv_obj_add_style(keyboard_text_area, &style_ui_island, 0);
    lv_obj_add_flag(keyboard_text_area, LV_OBJ_FLAG_HIDDEN);  // Hide it by default

    // Create an "OK" button
    lv_obj_t *ok_btn = lv_btn_create(cont_diginame_edit);
    lv_obj_set_size(ok_btn, 50, 30);
    lv_obj_align_to(ok_btn, text_area_settings, LV_ALIGN_OUT_RIGHT_MID, 10, 0);  // Align the right side of ok_btn with the right side of text_area
    lv_obj_add_event_cb(ok_btn, ok_button_ana_event_handler, LV_EVENT_CLICKED, text_area_settings);  // Pass text_area as user data

    // Add label to "OK" button
    lv_obj_t *btn_label = lv_label_create(ok_btn);
    lv_label_set_text(btn_label, "OK");
    lv_obj_center(btn_label);  // Center the label on the button
}

static void ok_button_ana_event_handler(lv_event_t *e)
{
    lv_obj_t *ok_btn = lv_event_get_target(e);
    lv_obj_t *text_area = (lv_obj_t *)lv_event_get_user_data(e);  // Retrieve the text area object

    if (lv_event_get_code(e) == LV_EVENT_CLICKED)
    {
        const char *new_text = lv_textarea_get_text(text_area);  // Get the entered text
        lv_label_set_text(label_footer, new_text);              // Update the label_footer

        int idx = (int)which_button_ana_settings - 1;
        label_footer_text_ana.at(idx) = std::string(new_text);
        ESP_LOGI("FooterUpdate", "Label updated to: %s %d", new_text, which_button_ana_settings);

        lv_obj_clean(content_container);
        tux_panel_ana_fullview(content_container);
    }
}