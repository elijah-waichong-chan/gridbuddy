#include "screen.h"

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    DISP_SMALL,
    DISP_MEDIUM,
    DISP_LARGE,
} disp_size_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void Onboard_create(lv_obj_t * parent);
static void color_changer_create(lv_obj_t * parent);

void IRAM_ATTR example1_increase_lvgl_tick(lv_timer_t * t);

/**********************
 *  STATIC VARIABLES
 **********************/
static disp_size_t disp_size;

static lv_obj_t * tv;
lv_style_t style_title;
lv_style_t style_number;
lv_style_t style_text;
static lv_style_t style_icon;

static lv_timer_t * auto_step_timer;
static lv_color_t original_screen_bg_color;

static lv_timer_t * meter2_timer;


lv_obj_t * page_label;

lv_obj_t * relay1_textArea;
lv_obj_t * relay2_textArea;
lv_obj_t * relay3_textArea;
lv_obj_t * relay4_textArea;
lv_obj_t * wattage_textArea;
lv_obj_t * battery_textarea;
lv_obj_t * timeLeft_textarea;
lv_obj_t * trend_symbol;
lv_obj_t * timeLeft_label;


/**********************
 *  GRID BUDDY VARIABLES
 **********************/
int page = 1;
bool reset = false;
bool wirelessConnection = false;

unsigned long pressStartTime = 0;  // Stores the time when the button was first pressed
bool buttonIsPressed = false;      // Flag to track the button press state
unsigned long pressDuration = 0;

int hours=0;
int minutes=0;

int hexGreen = 0x25D643;
int hexRed = 0xF12211;
int hexYellow = 0xDEE927;
int hexBlack = 0x000000;
int hexGray = 0x727B7F;
int hexBlue = 0x0082FC;
int hexWhite = 0xFFFFFF;

bool isConnected=false;

lv_obj_t *bt_icon = NULL;  // Global or static so you can reuse it

/**********************
 *  INITIALIZATION FUNCTION
 **********************/
void startScreen(void){

  disp_size = DISP_SMALL;                            
  lv_obj_clean(lv_scr_act());                   
  
  lv_coord_t tab_h;
  tab_h = 45;
  
  lv_style_init(&style_text);
  lv_style_set_text_font(&style_text, &lv_font_montserrat_20);
  lv_style_set_border_width(&style_text, 2);
  lv_style_set_border_color(&style_text, lv_color_hex(hexBlack)); // Black border

  lv_style_init(&style_number);
  lv_style_set_text_font(&style_number, &lv_font_montserrat_20);
  lv_style_set_border_width(&style_number, 2);
  lv_style_set_border_color(&style_number, lv_color_hex(hexBlack)); // Black border
  
  lv_style_init(&style_title);
  lv_style_set_text_font(&style_title, &lv_font_montserrat_16);

  lv_style_init(&style_icon);
  lv_style_set_text_color(&style_icon, lv_theme_get_color_primary(NULL));
  lv_style_set_text_font(&style_icon, &lv_font_montserrat_16);

  tv = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, tab_h);
  lv_obj_set_style_text_font(lv_scr_act(), &lv_font_montserrat_20, 0);
  lv_obj_t * t1 = lv_tabview_add_tab(tv, "GridBuddy");
  lv_obj_set_scrollbar_mode(t1, LV_SCROLLBAR_MODE_OFF);
  Onboard_create(t1);

  Set_Backlight(100);
}

void screen_close(void)
{
  /*Delete all animation*/
  lv_anim_del(NULL, NULL);

  lv_timer_del(meter2_timer);
  meter2_timer = NULL;

  lv_obj_clean(lv_scr_act());

  lv_style_reset(&style_text);
  lv_style_reset(&style_title);
  lv_style_reset(&style_icon);
}


/**********************
*   STATIC FUNCTIONS
**********************/

static void Onboard_create(lv_obj_t * parent)
{
  static lv_coord_t grid_main_col_dsc[] = {LV_GRID_FR(1),LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST}; // One flexible column
  static lv_coord_t grid_main_row_dsc[] = {
      37,
      LV_GRID_CONTENT, // Row for Buttons
      LV_GRID_CONTENT, // Row for Buttons
      40,
      LV_GRID_CONTENT, // Row for others
      LV_GRID_CONTENT, // Row for others
      LV_GRID_CONTENT, // Row for others
      LV_GRID_TEMPLATE_LAST
    };

  // Apply the grid layout to the parent container
  lv_obj_set_grid_dsc_array(parent, grid_main_col_dsc, grid_main_row_dsc);
  lv_obj_set_layout(parent, LV_LAYOUT_GRID); // Enable grid layout

  // FIRST ROW //
  // Left Text Area
  relay1_textArea = lv_textarea_create(parent);
  lv_textarea_set_one_line(relay1_textArea, true);
  lv_textarea_set_text(relay1_textArea, "Auto");
  lv_obj_add_style(relay1_textArea, &style_text, 0);
  lv_obj_set_scrollbar_mode(relay1_textArea, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_style_text_align(relay1_textArea, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_grid_cell(relay1_textArea, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

  // Right Text Area
  relay2_textArea = lv_textarea_create(parent);
  lv_textarea_set_one_line(relay2_textArea, true);
  lv_textarea_set_text(relay2_textArea, "Auto");
  lv_obj_add_style(relay2_textArea, &style_text, 0);
  lv_obj_set_scrollbar_mode(relay2_textArea, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_style_text_align(relay2_textArea, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_grid_cell(relay2_textArea, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

  // SECOND ROW //
  // Left Text Area
  relay3_textArea = lv_textarea_create(parent);
  lv_textarea_set_one_line(relay3_textArea, true);
  lv_textarea_set_text(relay3_textArea, "Auto");
  lv_obj_add_style(relay3_textArea, &style_text, 0);
  lv_obj_set_scrollbar_mode(relay3_textArea, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_style_text_align(relay3_textArea, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_grid_cell(relay3_textArea, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);

  // Right Text Area
  relay4_textArea = lv_textarea_create(parent);
  lv_textarea_set_one_line(relay4_textArea, true);
  lv_textarea_set_text(relay4_textArea, "Auto");
  lv_obj_add_style(relay4_textArea, &style_text, 0);
  lv_obj_set_scrollbar_mode(relay4_textArea, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_style_text_align(relay4_textArea, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_grid_cell(relay4_textArea, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);

  // POWER ROW //
  //Create a horizontal flex container
  lv_obj_t *wattage_container = lv_obj_create(parent);
  lv_obj_remove_style_all(wattage_container);  // optional: make it transparent
  lv_obj_set_flex_flow(wattage_container, LV_FLEX_FLOW_ROW);
  lv_obj_set_style_flex_cross_place(wattage_container, LV_FLEX_ALIGN_CENTER, 0);
  lv_obj_set_style_pad_gap(wattage_container, 6, 0); // space between icon and text
  lv_obj_set_size(wattage_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT); // auto-sizing
  lv_obj_set_grid_cell(wattage_container, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_CENTER, 2, 1);
  // Add the power icon
  lv_obj_t *power_label = lv_label_create(wattage_container);
  lv_label_set_text(power_label, LV_SYMBOL_CHARGE);
  lv_obj_set_style_text_color(power_label, lv_color_hex(hexYellow), 0);
  lv_obj_set_style_text_font(power_label, &lv_font_montserrat_28, 0);  // optional font
  // Add the text area next to it
  wattage_textArea = lv_textarea_create(wattage_container);
  lv_obj_set_width(wattage_textArea, 110);  // Or whatever size looks good
  lv_obj_add_style(wattage_textArea, &style_text, 0);
  lv_textarea_set_one_line(wattage_textArea, true);
  lv_textarea_set_text(wattage_textArea, "-- W");
  lv_obj_set_scrollbar_mode(wattage_textArea, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_style_text_align(wattage_textArea, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_set_style_flex_grow(power_label, 1, LV_PART_MAIN);

  // BATTERY ROW //
  //Create a horizontal flex container
  lv_obj_t *battery_container = lv_obj_create(parent);
  lv_obj_remove_style_all(battery_container);  // optional: make it transparent
  lv_obj_set_flex_flow(battery_container, LV_FLEX_FLOW_ROW);
  lv_obj_set_style_flex_cross_place(battery_container, LV_FLEX_ALIGN_CENTER, 0);
  lv_obj_set_style_pad_gap(battery_container, 6, 0); // space between icon and text
  lv_obj_set_size(battery_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT); // auto-sizing
  lv_obj_set_grid_cell(battery_container, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_CENTER, 3, 1);
  // Add the battery icon
  lv_obj_t *battery_label = lv_label_create(battery_container);
  lv_label_set_text(battery_label, LV_SYMBOL_BATTERY_2);
  lv_obj_set_style_text_font(battery_label, &lv_font_montserrat_24, 0);  // optional font
  // Add the text area next to it
  battery_textarea = lv_textarea_create(battery_container);
  lv_obj_set_width(battery_textarea, 85);  // Or whatever size looks good
  lv_obj_add_style(battery_textarea, &style_text, 0);
  lv_textarea_set_one_line(battery_textarea, true);
  lv_textarea_set_text(battery_textarea, "-- %");
  lv_obj_set_scrollbar_mode(battery_textarea, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_style_text_align(battery_textarea, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  // Add trend symbol
  trend_symbol = lv_label_create(battery_container);
  lv_label_set_text(trend_symbol, LV_SYMBOL_MINUS); //trend symbol
  lv_obj_set_style_text_font(trend_symbol, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(trend_symbol, lv_color_hex(hexGray), LV_PART_MAIN | LV_STATE_DEFAULT);
  // Flush
  lv_obj_set_style_flex_grow(battery_label, 1, LV_PART_MAIN);

  // TIME ROW //
  //Create a horizontal flex container
  lv_obj_t *timeLeft_container = lv_obj_create(parent);
  lv_obj_remove_style_all(timeLeft_container);  // optional: make it transparent
  lv_obj_set_flex_flow(timeLeft_container, LV_FLEX_FLOW_ROW);
  lv_obj_set_style_flex_cross_place(timeLeft_container, LV_FLEX_ALIGN_CENTER, 0);
  lv_obj_set_style_pad_gap(timeLeft_container, 6, 0); // space between icon and text
  lv_obj_set_size(timeLeft_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT); // auto-sizing
  lv_obj_set_grid_cell(timeLeft_container, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_CENTER, 4, 1);
  // Add the empty battery icon
  timeLeft_label = lv_label_create(timeLeft_container);
  lv_label_set_text(timeLeft_label, LV_SYMBOL_BATTERY_EMPTY);
  lv_obj_set_style_text_font(timeLeft_label, &lv_font_montserrat_24, 0);  // optional font
  // Add the text area next to it
  timeLeft_textarea = lv_textarea_create(timeLeft_container);
  lv_obj_set_width(timeLeft_textarea, 110);  // Or whatever size looks good
  lv_obj_add_style(timeLeft_textarea, &style_text, 0);
  lv_textarea_set_one_line(timeLeft_textarea, true);
  lv_textarea_set_text(timeLeft_textarea, "--h --m");
  lv_obj_set_scrollbar_mode(timeLeft_textarea, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_style_text_align(timeLeft_textarea, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  // Flush
  lv_obj_set_style_flex_grow(timeLeft_label, 1, LV_PART_MAIN);
  
  auto_step_timer = lv_timer_create(example1_increase_lvgl_tick, 100, NULL);
}

/**********************
 *  SCREEN UPDATE HELPER FUNCTION
 **********************/
void update_outlet_status(lv_obj_t *textArea, bool status, uint32_t hexRed, uint32_t hexGreen) {
    lv_color_t color = lv_color_hex(status ? hexGreen : hexRed);
    lv_obj_set_style_bg_color(textArea, color, LV_PART_MAIN | LV_STATE_DEFAULT);
}

void update_relay_mode(lv_obj_t *textArea, int mode) {
    const char* mode_text = "Unknown";
    switch (mode) {
        case 0: mode_text = "Off"; break;
        case 1: mode_text = "On"; break;
        case 2: mode_text = "Auto"; break;
    }
    lv_textarea_set_text(textArea, mode_text);
}

void update_wireless_icon(unsigned long messageTime) {
    static bool wirelessConnection = false;
    unsigned long timeSinceLastMessage = millis() - messageTime;

    // Check connection status
    isConnected = (timeSinceLastMessage <= 3000);

    // If status changed, update icon
    if (isConnected && !wirelessConnection) {
        // Connection restored → show icon
        if (bt_icon == NULL) {
            bt_icon = lv_label_create(lv_scr_act());
            lv_label_set_text(bt_icon, LV_SYMBOL_BLUETOOTH);
            lv_obj_set_style_text_font(bt_icon, &lv_font_montserrat_20, 0);
            lv_obj_set_style_text_color(bt_icon, lv_color_hex(hexBlue), 0);
            lv_obj_align(bt_icon, LV_ALIGN_TOP_RIGHT, -5, 12);
        } else {
            lv_obj_clear_flag(bt_icon, LV_OBJ_FLAG_HIDDEN);
        }
    } 
    else if (!isConnected && wirelessConnection) {
        // Connection lost → hide icon
        if (bt_icon != NULL) {
            lv_obj_add_flag(bt_icon, LV_OBJ_FLAG_HIDDEN);
        }
    }

    wirelessConnection = isConnected;
}

/**********************
 *  SCREEN UPDATE FUNCTION
 **********************/
void IRAM_ATTR example1_increase_lvgl_tick(lv_timer_t * t)
{
  char buf[100]={0};
  if(isConnected == false){
    snprintf(buf, sizeof(buf), "-- W");
    lv_textarea_set_text(wattage_textArea, buf);
    snprintf(buf, sizeof(buf), "-- %%");
    lv_textarea_set_text(battery_textarea, buf);
    lv_obj_set_style_bg_color(battery_textarea, lv_color_hex(hexWhite), LV_PART_MAIN | LV_STATE_DEFAULT);
    snprintf(buf, sizeof(buf), "--h --m");
    lv_textarea_set_text(timeLeft_textarea, buf);
    lv_label_set_text(trend_symbol, LV_SYMBOL_MINUS); //trend symbol
    lv_obj_set_style_text_color(trend_symbol, lv_color_hex(hexGray), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else{
    snprintf(buf, sizeof(buf), "%.1f W", dischargeWattage);
    lv_textarea_set_text(wattage_textArea, buf);
    snprintf(buf, sizeof(buf), "%.1f %%", batteryPercentage); /////////////////////////////////////////////HEREEEEEE
    lv_textarea_set_text(battery_textarea, buf);

    if(chargeWattage == dischargeWattage)
    {
      snprintf(buf, sizeof(buf), "- h -- m");
      lv_textarea_set_text(timeLeft_textarea, buf);
    }
    else if(chargeWattage < dischargeWattage){
      hours = (int)batteryTimeLeft_h;
      minutes = (int)((batteryTimeLeft_h - hours) * 60);
      snprintf(buf, sizeof(buf), "%d h %02d m", hours, minutes);
      lv_textarea_set_text(timeLeft_textarea, buf);
      lv_obj_set_style_bg_color(timeLeft_textarea, lv_color_hex(hexRed), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_label_set_text(timeLeft_label, LV_SYMBOL_BATTERY_EMPTY);
    }
    else if(chargeWattage > dischargeWattage){
      hours = (int)batteryTimeLeft_h;
      minutes = (int)((batteryTimeLeft_h - hours) * 60);
      snprintf(buf, sizeof(buf), "%d h %02d m", hours, minutes);
      lv_textarea_set_text(timeLeft_textarea, buf);
      lv_obj_set_style_bg_color(timeLeft_textarea, lv_color_hex(hexGreen), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_label_set_text(timeLeft_label, LV_SYMBOL_BATTERY_FULL);
    }

    if(batteryPercentage>80){
      lv_obj_set_style_bg_color(battery_textarea, lv_color_hex(hexGreen), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else if(batteryPercentage>80){
      lv_obj_set_style_bg_color(battery_textarea, lv_color_hex(hexYellow), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else{
      lv_obj_set_style_bg_color(battery_textarea, lv_color_hex(hexRed), LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    if(netCharge_Ah>0){
      lv_label_set_text(trend_symbol, LV_SYMBOL_UP); //trend symbol
      lv_obj_set_style_text_color(trend_symbol, lv_color_hex(hexGreen), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else if(netCharge_Ah<0){
      lv_label_set_text(trend_symbol, LV_SYMBOL_DOWN); //trend symbol
      lv_obj_set_style_text_color(trend_symbol, lv_color_hex(hexRed), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else{
      lv_label_set_text(trend_symbol, LV_SYMBOL_MINUS); //trend symbol
      lv_obj_set_style_text_color(trend_symbol, lv_color_hex(hexGray), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
  }

  update_wireless_icon(messageTime);

  update_outlet_status(relay1_textArea, outlet1_status, hexRed, hexGreen);
  update_outlet_status(relay2_textArea, outlet2_status, hexRed, hexGreen);
  update_outlet_status(relay3_textArea, outlet3_status, hexRed, hexGreen);
  update_outlet_status(relay4_textArea, outlet4_status, hexRed, hexGreen);

  update_relay_mode(relay1_textArea, relay1_mode);
  update_relay_mode(relay2_textArea, relay2_mode);
  update_relay_mode(relay3_textArea, relay3_mode);
  update_relay_mode(relay4_textArea, relay4_mode);

  
}

void ButtonAction(void)
{

}