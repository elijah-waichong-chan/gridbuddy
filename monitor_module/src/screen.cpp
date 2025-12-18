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
lv_style_t style_text;
lv_style_t style_title;
lv_style_t style_number;
static lv_style_t style_icon;

static const lv_font_t * font_huge;
static const lv_font_t * font_large;
static const lv_font_t * font_medium;
static const lv_font_t * font_normal;

static lv_timer_t * auto_step_timer;
static lv_color_t original_screen_bg_color;

static lv_timer_t * meter2_timer;

lv_obj_t * first_value;
lv_obj_t * second_value;
lv_obj_t * third_value;
lv_obj_t * page_label;


/**********************
 *  GRID BUDDY VARIABLES
 **********************/
bool reset = false;

unsigned long pressStartTime = 0;  // Stores the time when the button was first pressed
bool buttonIsPressed = false;      // Flag to track the button press state
unsigned long pressDuration = 0;

int page = 1;

int hexGreen = 0x25D643;
int hexRed = 0xF12211;
int hexYellow = 0xDEE927;
int hexBlack = 0x000000;
int hexGray = 0x727B7F;
int hexBlue = 0x0082FC;
int hexWhite = 0xFFFFFF;
int hexOrange = 0xfcb103;

lv_obj_t *bt_icon = NULL;  // Global or static so you can reuse it

/**********************
 *  INITIALIZATION FUNCTION
 **********************/
void startScreen(void){

  disp_size = DISP_SMALL;                            

  font_huge = LV_FONT_DEFAULT;  
  font_large = LV_FONT_DEFAULT;   
  font_medium = LV_FONT_DEFAULT;                          
  font_normal = LV_FONT_DEFAULT;                         
  
  lv_coord_t tab_h;
  tab_h = 45;

  #if LV_FONT_MONTSERRAT_34
    font_huge     = &lv_font_montserrat_34;
  #else
    LV_LOG_WARN("LV_FONT_MONTSERRAT_34 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.");
  #endif

  #if LV_FONT_MONTSERRAT_24
    font_large     = &lv_font_montserrat_24;
  #else
    LV_LOG_WARN("LV_FONT_MONTSERRAT_24 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.");
  #endif

  #if LV_FONT_MONTSERRAT_18
    font_medium     = &lv_font_montserrat_18;
  #else
    LV_LOG_WARN("LV_FONT_MONTSERRAT_18 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.");
  #endif

  #if LV_FONT_MONTSERRAT_12
    font_normal    = &lv_font_montserrat_12;
  #else
    LV_LOG_WARN("LV_FONT_MONTSERRAT_12 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.");
  #endif

  lv_style_init(&style_title);
  lv_style_set_text_font(&style_title, &lv_font_montserrat_16);

  lv_style_init(&style_text);
  lv_style_set_text_font(&style_title, &lv_font_montserrat_16);

  lv_style_init(&style_number);
  lv_style_set_text_font(&style_number, &lv_font_montserrat_30);
  lv_style_set_border_width(&style_number, 2);
  lv_style_set_border_color(&style_number, lv_color_hex(hexBlack)); // Black border
  
  lv_style_init(&style_icon);
  lv_style_set_text_color(&style_icon, lv_theme_get_color_primary(NULL));
  lv_style_set_text_font(&style_icon, &lv_font_montserrat_16);

  tv = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, tab_h);
  lv_obj_set_style_text_font(lv_scr_act(), &lv_font_montserrat_20, 0);
  lv_obj_t * t1 = lv_tabview_add_tab(tv, "GridBuddy");
  lv_obj_set_scrollbar_mode(t1, LV_SCROLLBAR_MODE_OFF);
  Onboard_create(t1);

  pinMode(0, INPUT);
  pinMode(17, INPUT_PULLDOWN);

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
  // Define a single-column grid for the parent
    static lv_coord_t grid_main_col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST}; // One flexible column
    static lv_coord_t grid_main_row_dsc[] = {
        LV_GRID_CONTENT, // Row for "Solar Input" label
        LV_GRID_CONTENT, // Row for voltage value text area
        LV_GRID_CONTENT, // Row for current value text area
        LV_GRID_CONTENT, // Row for wattage value text area
        LV_GRID_TEMPLATE_LAST
    };

    // Apply the grid layout to the parent container
    lv_obj_set_grid_dsc_array(parent, grid_main_col_dsc, grid_main_row_dsc);
    lv_obj_set_layout(parent, LV_LAYOUT_GRID); // Enable grid layout
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);

    // Create "Solar Input" label
    page_label = lv_label_create(parent);
    lv_label_set_text(page_label, "Solar Input");
    lv_obj_add_style(page_label, &style_text, 0);
    lv_obj_set_grid_cell(page_label, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    // Create voltage value text area
    first_value = lv_textarea_create(parent);
    lv_textarea_set_one_line(first_value, true);
    lv_textarea_set_text(first_value, "-- V");
    lv_obj_add_style(first_value, &style_number, 0);
    lv_obj_set_scrollbar_mode(first_value, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_grid_cell(first_value, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    // Create current value text area
    second_value = lv_textarea_create(parent);
    lv_textarea_set_one_line(second_value, true);
    lv_textarea_set_text(second_value, "-- A");
    lv_obj_add_style(second_value, &style_number, 0);
    lv_obj_set_scrollbar_mode(second_value, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_grid_cell(second_value, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);

    // Create wattage value text area
    third_value = lv_textarea_create(parent);
    lv_textarea_set_one_line(third_value, true);
    lv_textarea_set_text(third_value, "-- W");
    lv_obj_add_style(third_value, &style_number, 0);
    lv_obj_set_scrollbar_mode(third_value, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_grid_cell(third_value, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 3, 1);
  
  auto_step_timer = lv_timer_create(example1_increase_lvgl_tick, 100, NULL);
}

/**********************
 *  SCREEN UPDATE HELPER FUNCTION
 **********************/
void update_wireless_icon(unsigned long messageTime) {
    static bool wirelessConnection = false;
    unsigned long timeSinceLastMessage = millis() - messageTime;

    // Check connection status
    bool isConnected = (timeSinceLastMessage <= 5000);

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

  update_wireless_icon(messageTime);

  char buf[100]={0}; 
  if(reset==true)
  {
    snprintf(buf, sizeof(buf), "Reset!");
    lv_textarea_set_text(second_value, buf);
  }
  else if(page==1)
  {
    lv_label_set_text(page_label, "Solar Input");
    //Solar Input Information Page 
    snprintf(buf, sizeof(buf), "%.2f V", inputVoltage);
    lv_textarea_set_text(first_value, buf);

    if(inputVoltage <12)
    {
      lv_obj_set_style_bg_color(first_value, lv_color_hex(hexYellow), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else
    {
      lv_obj_set_style_bg_color(first_value, lv_color_hex(hexGreen), LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    if(inputCurrent ==0)
    {
      lv_obj_set_style_bg_color(second_value, lv_color_hex(hexWhite), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_bg_color(third_value, lv_color_hex(hexWhite), LV_PART_MAIN | LV_STATE_DEFAULT);
      snprintf(buf, sizeof(buf), "-- A");
      lv_textarea_set_text(second_value, buf);
      snprintf(buf, sizeof(buf), "-- W");
      lv_textarea_set_text(third_value, buf);
    }
    else
    {
      lv_obj_set_style_bg_color(second_value, lv_color_hex(hexGreen), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_bg_color(third_value, lv_color_hex(hexGreen), LV_PART_MAIN | LV_STATE_DEFAULT);
      snprintf(buf, sizeof(buf), "%.2f A", inputCurrent);
      lv_textarea_set_text(second_value, buf);
      snprintf(buf, sizeof(buf), "%.1f W", inputWattage);
      lv_textarea_set_text(third_value, buf);
    }
  }
  else if(page==2)
  {
    lv_label_set_text(page_label, "Charge Info");
    //Charging Information Page 
    snprintf(buf, sizeof(buf), "%.1f A", chargeCurrent);
    lv_textarea_set_text(first_value, buf); 
    snprintf(buf, sizeof(buf), "%.2f Ah", totalCharge_Ah);
    lv_textarea_set_text(second_value, buf);
    lv_obj_set_style_bg_color(second_value, lv_color_hex(hexWhite), LV_PART_MAIN | LV_STATE_DEFAULT);
    snprintf(buf, sizeof(buf), "%.1f %%", batteryPercentage);
    lv_textarea_set_text(third_value, buf);

    if(chargeCurrent == 0)
    {
      lv_obj_set_style_bg_color(first_value, lv_color_hex(hexWhite), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else
    {
      lv_obj_set_style_bg_color(first_value, lv_color_hex(hexGreen), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    if(batteryPercentage < 25)
    {
      lv_obj_set_style_bg_color(third_value, lv_color_hex(hexRed), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else if(batteryPercentage < 50)
    {
      lv_obj_set_style_bg_color(third_value, lv_color_hex(hexOrange), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else if(batteryPercentage < 75)
    {
      lv_obj_set_style_bg_color(third_value, lv_color_hex(hexYellow), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else
    {
      lv_obj_set_style_bg_color(third_value, lv_color_hex(hexGreen), LV_PART_MAIN | LV_STATE_DEFAULT);
    }

  }
  else if(page==3)
  {
    lv_label_set_text(page_label, "Discharge Info");
    //Charging Information Page 
    snprintf(buf, sizeof(buf), "%.1f A", dischargeCurrent);
    lv_textarea_set_text(first_value, buf);
    snprintf(buf, sizeof(buf), "%.0f W", dischargeWattage);
    lv_textarea_set_text(second_value, buf);
    lv_obj_set_style_bg_color(second_value, lv_color_hex(hexWhite), LV_PART_MAIN | LV_STATE_DEFAULT);
    snprintf(buf, sizeof(buf), "%.1f %%", batteryPercentage);
    lv_textarea_set_text(third_value, buf);

    if(dischargeCurrent == 0)
    {
      lv_obj_set_style_bg_color(first_value, lv_color_hex(hexWhite), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else
    {
      lv_obj_set_style_bg_color(first_value, lv_color_hex(hexRed), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    if(batteryPercentage < 25)
    {
      lv_obj_set_style_bg_color(third_value, lv_color_hex(hexRed), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else if(batteryPercentage < 50)
    {
      lv_obj_set_style_bg_color(third_value, lv_color_hex(hexOrange), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else if(batteryPercentage < 75)
    {
      lv_obj_set_style_bg_color(third_value, lv_color_hex(hexYellow), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else
    {
      lv_obj_set_style_bg_color(third_value, lv_color_hex(hexGreen), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
  }
  else if(page==4)
  {
    lv_label_set_text(page_label, "Battery Info");
    //Charging Information Page 
    snprintf(buf, sizeof(buf), "%.1f W", chargeWattage);
    lv_textarea_set_text(first_value, buf);
    snprintf(buf, sizeof(buf), "%.1f W", dischargeWattage);
    lv_textarea_set_text(second_value, buf);
    snprintf(buf, sizeof(buf), "%.1f %%", batteryPercentage);
    lv_textarea_set_text(third_value, buf);

    if(chargeWattage >0){
      lv_obj_set_style_bg_color(first_value, lv_color_hex(0x25D643), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else if (chargeWattage == 0){
      lv_obj_set_style_bg_color(first_value, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    if(dischargeWattage >0){
      lv_obj_set_style_bg_color(second_value, lv_color_hex(0x25D643), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else if (dischargeWattage == 0){
      lv_obj_set_style_bg_color(second_value, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    if(netChange_Ah >0){
      lv_obj_set_style_bg_color(third_value, lv_color_hex(0x25D643), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else if (netChange_Ah <0){
      lv_obj_set_style_bg_color(third_value, lv_color_hex(0xF12211), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else if (netChange_Ah == 0){
      lv_obj_set_style_bg_color(third_value, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    }

  }
}

void ButtonAction(void)
{
  // Reset Button
  if (digitalRead(0) == LOW && !buttonIsPressed) {
    buttonIsPressed = true;           // Set flag: button is now pressed
    pressStartTime = millis();        // Record the time when the press started
  }
  if(buttonIsPressed==true){
    pressDuration = millis() - pressStartTime;
    // If the button was held for at least 1.5 seconds (1500 milliseconds)
    if (pressDuration >= 1500) {
      reset=true;
    }
  }
  if (digitalRead(0) == HIGH && buttonIsPressed) {
    buttonIsPressed = false;          // Reset flag on button release
  }

  if (digitalRead(17) == HIGH)
  {
    page = (page % 4) + 1;
    delay(100);
  }
}