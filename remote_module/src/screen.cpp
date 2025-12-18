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
lv_style_t style_text_muted;
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

lv_obj_t * voltage_value;
lv_obj_t * current_value;
lv_obj_t * wattage_value;

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
  
  lv_style_init(&style_text_muted);
  lv_style_set_text_opa(&style_text_muted, LV_OPA_90);
  lv_style_set_text_font(&style_text_muted, font_medium);

  lv_style_init(&style_number);
  lv_style_set_text_font(&style_number, font_huge);
  lv_style_set_border_width(&style_number, 2);
  lv_style_set_border_color(&style_number, lv_color_hex(0x000000)); // Black border

  lv_style_init(&style_title);
  lv_style_set_text_font(&style_title, font_medium);

  lv_style_init(&style_icon);
  lv_style_set_text_color(&style_icon, lv_theme_get_color_primary(NULL));
  lv_style_set_text_font(&style_icon, font_normal);

  tv = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, tab_h);
  lv_obj_set_style_text_font(lv_scr_act(), font_large, 0);
  lv_obj_t * t1 = lv_tabview_add_tab(tv, "GridBuddy");

  Set_Backlight(100);

  
  Onboard_create(t1);
}

void screen_close(void)
{
  /*Delete all animation*/
  lv_anim_del(NULL, NULL);

  lv_timer_del(meter2_timer);
  meter2_timer = NULL;

  lv_obj_clean(lv_scr_act());

  lv_style_reset(&style_text_muted);
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

    // Create "Solar Input" label
    lv_obj_t * voltage_label = lv_label_create(parent);
    lv_label_set_text(voltage_label, "Solar Input");
    lv_obj_add_style(voltage_label, &style_text_muted, 0);
    lv_obj_set_grid_cell(voltage_label, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    // Create voltage value text area
    voltage_value = lv_textarea_create(parent);
    lv_textarea_set_one_line(voltage_value, true);
    lv_textarea_set_text(voltage_value, "-- V");
    lv_obj_add_style(voltage_value, &style_number, 0);
    lv_obj_set_grid_cell(voltage_value, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    // Create current value text area
    current_value = lv_textarea_create(parent);
    lv_textarea_set_one_line(current_value, true);
    lv_textarea_set_text(current_value, "-- A");
    lv_obj_add_style(current_value, &style_number, 0);
    lv_obj_set_grid_cell(current_value, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);

    // Create wattage value text area
    wattage_value = lv_textarea_create(parent);
    lv_textarea_set_one_line(wattage_value, true);
    lv_textarea_set_text(wattage_value, "-- W");
    lv_obj_add_style(wattage_value, &style_number, 0);
    lv_obj_set_grid_cell(wattage_value, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 3, 1);
  
  auto_step_timer = lv_timer_create(example1_increase_lvgl_tick, 100, NULL);
}

void IRAM_ATTR example1_increase_lvgl_tick(lv_timer_t * t)
{
  char buf[100]={0}; 
  
  snprintf(buf, sizeof(buf), "%.2f V", inputVoltage);
  lv_textarea_set_text(voltage_value, buf);

  if(inputVoltage <12)
  {
    lv_obj_set_style_bg_color(voltage_value, lv_color_hex(0xF1EE11), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else
  {
    lv_obj_set_style_bg_color(voltage_value, lv_color_hex(0x25D643), LV_PART_MAIN | LV_STATE_DEFAULT);
  }

    if(inputCurrent <0.1)
  {
    lv_obj_set_style_bg_color(current_value, lv_color_hex(0xF12211), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(wattage_value, lv_color_hex(0xF12211), LV_PART_MAIN | LV_STATE_DEFAULT);
    snprintf(buf, sizeof(buf), "-- A");
    lv_textarea_set_text(current_value, buf);
    snprintf(buf, sizeof(buf), "-- W");
    lv_textarea_set_text(wattage_value, buf);
  }
  else
  {
    lv_obj_set_style_bg_color(current_value, lv_color_hex(0x25D643), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(wattage_value, lv_color_hex(0x25D643), LV_PART_MAIN | LV_STATE_DEFAULT);
    snprintf(buf, sizeof(buf), "%.2f A", inputCurrent);
    lv_textarea_set_text(current_value, buf);
    snprintf(buf, sizeof(buf), "%.1f W", inputWattage);
    lv_textarea_set_text(wattage_value, buf);
  }
}
