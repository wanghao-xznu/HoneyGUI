#include "root_image/ui_resource.h"
#include "gui_curtainview.h"
#include "gui_curtain.h"
#include "gui_img.h"
#include "gui_text.h"
#include "gui_switch.h"
#include "gui_win.h"
#include "app_gui_main.h"
#include "gui_tabview.h"
#include "gui_obj.h"
#include "gui_img_scope.h"

#define MODE_SOURCE true
#define MODE_SINK false

extern gui_tabview_t *tabview_main;
bool cfg_mode = MODE_SOURCE;
gui_curtain_t *curtain_down_menu = NULL;
gui_img_t *img_base_menu = NULL;
gui_switch_t *switch_menu_bluetooth = NULL;
gui_switch_t *switch_menu_buds = NULL;
gui_switch_t *switch_menu_phone = NULL;
gui_switch_t *switch_menu_setting = NULL;
gui_win_t *win_menu_buds  = NULL;
gui_win_t *win_menu_phone = NULL;
gui_win_t *win_menu_setting = NULL;

//static void curtain_down_menu_update_cb(void *obj)
//{
//    gui_log("curtain_down_menu_update_cb\n");
//    if (cfg_mode == MODE_SOURCE)
//    {
//        switch_menu_buds->base.not_show = false;

//        switch_menu_phone->base.x = 51;
//        switch_menu_phone->base.y = 177;
//    }
//    else
//    {
//        switch_menu_buds->base.not_show = true;

//        switch_menu_phone->base.x = 177;
//        switch_menu_phone->base.y = 177;
//    }
//}

static void switch_menu_bluetooth_released_cb(void *obj, gui_event_t event)
{
    gui_log("switch_menu_bluetooth_released_cb\n");
    if (switch_menu_bluetooth->ifon)
    {
        switch_menu_buds->base.not_show = false;
        switch_menu_phone->base.not_show = false;
        switch_menu_setting->base.not_show = false;
    }
    else
    {
        switch_menu_buds->base.not_show = true;
        switch_menu_phone->base.not_show = true;
        switch_menu_setting->base.not_show = true;
    }

}

static void switch_menu_buds_released_cb(void *obj, gui_event_t event)
{
    gui_log("switch_menu_buds_released_cb\n");
    tabview_main->base.not_show = true;
    if (win_menu_buds == NULL)
    {
        gui_app_t *app = get_app_watch_ui();
        win_menu_buds = gui_win_create(&(app->screen), "win_menu_buds", 0, 0, 454, 454);
        extern void design_win_menu_buds(void *parent);
        design_win_menu_buds(win_menu_buds);
    }
}

static void switch_menu_phone_released_cb(void *obj, gui_event_t event)
{
    gui_log("switch_menu_phone_released_cb\n");
    tabview_main->base.not_show = true;
    if (win_menu_phone == NULL)
    {
        gui_app_t *app = get_app_watch_ui();
        win_menu_phone = gui_win_create(&(app->screen), "win_menu_phone", 0, 0, 454, 454);
        extern void design_win_menu_phone(void *parent);
        design_win_menu_phone(win_menu_phone);
    }
}

static void switch_menu_setting_released_cb(void *obj, gui_event_t event)
{
    gui_log("switch_menu_setting_released_cb\n");
    tabview_main->base.not_show = true;
    if (win_menu_setting == NULL)
    {
        gui_app_t *app = get_app_watch_ui();
        win_menu_setting = gui_win_create(&(app->screen), "win_menu_setting", 0, 0, 454, 454);
        extern void design_win_menu_setting(void *parent);
        design_win_menu_setting(win_menu_setting);
    }
}

void design_curtain_menu(void *parent)
{
    gui_curtainview_t *ct = (gui_curtainview_t *)parent;
    curtain_down_menu = gui_curtain_create(ct, "curtain_down_menu", 0, 0, 454, 454, CURTAIN_DOWN, 1.0f);
    img_base_menu = gui_img_create_from_mem(curtain_down_menu, "", WATCH_BASE_GREY_70_ALPHA_BIN, 0, 0,
                                            454, 454);
    img_base_menu->draw_img.blend_mode = IMG_BYPASS_MODE;
    img_base_menu->draw_img.opacity_value = 128;

    switch_menu_bluetooth = gui_switch_create(img_base_menu, 177, 38, 100, 100, ICON_BLUETOOTH_OFF_BIN,
                                              ICON_BLUETOOTH_ON_BIN);
    switch_menu_bluetooth->off_hl_pic_addr = ICON_BLUETOOTH_TOUCH_BIN;
    switch_menu_bluetooth->on_hl_pic_addr = ICON_BLUETOOTH_TOUCH_BIN;
    gui_obj_add_event_cb(switch_menu_bluetooth, (gui_event_cb_t)switch_menu_bluetooth_released_cb,
                         GUI_EVENT_1, NULL);
    gui_obj_add_event_cb(switch_menu_bluetooth, (gui_event_cb_t)switch_menu_bluetooth_released_cb,
                         GUI_EVENT_2, NULL);

    switch_menu_buds = gui_switch_create(img_base_menu, 300, 177, 100, 100, ICON_BUDS_OFF_BIN,
                                         ICON_BUDS_OFF_BIN);//on_bin by BT event ICON_BUDS_ON_BIN
    switch_menu_buds->off_hl_pic_addr = ICON_BUDS_TOUCH_BIN;
    switch_menu_buds->on_hl_pic_addr = ICON_BUDS_TOUCH_BIN;
    gui_obj_add_event_cb(switch_menu_buds, (gui_event_cb_t)switch_menu_buds_released_cb, GUI_EVENT_1,
                         NULL);
    gui_obj_add_event_cb(switch_menu_buds, (gui_event_cb_t)switch_menu_buds_released_cb, GUI_EVENT_2,
                         NULL);

    switch_menu_phone = gui_switch_create(img_base_menu, 51, 177, 100, 100, ICON_PHONE_OFF_BIN,
                                          ICON_PHONE_OFF_BIN);
    switch_menu_phone->off_hl_pic_addr = ICON_PHONE_TOUCH_BIN;
    switch_menu_phone->on_hl_pic_addr = ICON_PHONE_TOUCH_BIN;
    gui_obj_add_event_cb(switch_menu_phone, (gui_event_cb_t)switch_menu_phone_released_cb, GUI_EVENT_1,
                         NULL);
    gui_obj_add_event_cb(switch_menu_phone, (gui_event_cb_t)switch_menu_phone_released_cb, GUI_EVENT_2,
                         NULL);

    switch_menu_setting = gui_switch_create(img_base_menu, 177, 327, 100, 100, ICON_MORE_OFF_BIN,
                                            ICON_MORE_OFF_BIN);
    switch_menu_setting->off_hl_pic_addr = ICON_MORE_TOUCH_BIN;
    switch_menu_setting->on_hl_pic_addr = ICON_MORE_TOUCH_BIN;
    gui_obj_add_event_cb(switch_menu_setting, (gui_event_cb_t)switch_menu_setting_released_cb,
                         GUI_EVENT_1, NULL);
    gui_obj_add_event_cb(switch_menu_setting, (gui_event_cb_t)switch_menu_setting_released_cb,
                         GUI_EVENT_2, NULL);

    switch_menu_buds->base.not_show = true;
    switch_menu_phone->base.not_show = true;
    switch_menu_setting->base.not_show = true;


    gui_curtain_t *curtain_left_menu = gui_curtain_create(ct, "curtain_left_menu", 0, 0, 454, 454,
                                                          CURTAIN_LEFT, 1.0f);
    gui_img_create_from_mem(curtain_left_menu, "", WATCH_BASE_GREY_70_ALPHA_BIN, 0, 0, 454, 454);
}
