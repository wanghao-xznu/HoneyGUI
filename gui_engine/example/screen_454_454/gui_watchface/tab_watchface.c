#include "app_gui_main.h"
#include "app_gui_perspective.h"
#include "root_image/ui_resource.h"
#include "gui_img.h"
#include "gui_curtainview.h"
#include <time.h>
#include <stdio.h>
#include "gui_text.h"
#include "gui_win.h"
#include "gui_api.h"

#define MY_PI   (180)
#define DOUBLE_CLICK_INTERVAL 600

void *gui_watchface_turn_around_timer = NULL;

gui_img_t *img_clock = NULL;
gui_curtainview_t *curtainview_menu = NULL;
gui_img_t *watchface_hand;
gui_img_t *watchface_hour;
gui_img_t *watchface_minute;
gui_img_t *watchface_second;
float angle_hour;
float angle_min;
float angle_sec;
//static uint16_t longtouch_count = 0;


static void win_switch_to_perspective_app_touch_cb(void *obj, gui_event_t event)
{
    static uint32_t first_click = 0;
    static uint32_t second_click = 0;
    static bool is_first_click = 1;
    switch (event)
    {
    // case GUI_EVENT_TOUCH_LONG:
    //     gui_log("win_switch_to_perspective_app_touch_cb event = %d longtouch_count = %d\n", event,
    //             longtouch_count);
    //     longtouch_count++;
    //     if (longtouch_count > 30)
    //     {
    //         longtouch_count = 0;
    //         gui_switch_app(get_app_watch_ui(), get_app_perspective_ui());
    //     }
    //     break;
    // case GUI_EVENT_TOUCH_RELEASED:
    //     longtouch_count = 0;
    //     break;
    case GUI_EVENT_TOUCH_CLICKED:
        if (is_first_click)
        {
            first_click = gui_ms_get();
            is_first_click = 0;
        }
        else
        {
            second_click = gui_ms_get();
            is_first_click = 1;
        }

        //gui_log("111first_click = %d, second_click = %d, is_first_click = %d\n", first_click, second_click,
        //        is_first_click);
        if (second_click - first_click < DOUBLE_CLICK_INTERVAL && second_click != 0)
        {
            gui_switch_app(get_app_watch_ui(), get_app_perspective_ui());
            is_first_click = 1;
            first_click = 0;
            second_click = 0;
        }
        else if (second_click - first_click >= DOUBLE_CLICK_INTERVAL && second_click != 0)
        {
            is_first_click = 0;
            first_click = second_click;
            second_click = 0;
        }
        //gui_log("222first_click = %d, second_click = %d, is_first_click = %d\n", first_click, second_click,
        //        is_first_click);
        break;
    default:
        break;

    }
}

static void img_animate_watchface_callback(void *obj)
{
//Get the current real-time time and print it
    struct tm *ptr;
    time_t lt;
    lt = time(NULL);
    ptr = localtime(&lt);

    uint8_t tm_sec = ptr->tm_sec;
    uint8_t tm_min = ptr->tm_min;
    uint8_t tm_hour = ptr->tm_hour;

    angle_hour = (tm_hour % 12) * MY_PI / 6 + tm_min * MY_PI / 360;
    angle_min  = tm_min * MY_PI / 30 + tm_sec * MY_PI / 1800;
    angle_sec  = tm_sec * MY_PI / 30;
    gui_img_translate(watchface_hour, watchface_hour->base.w / 2, watchface_hour->base.h);
    gui_img_translate(watchface_minute, watchface_minute->base.w / 2, watchface_minute->base.h);
    gui_img_translate(watchface_second, watchface_second->base.w / 2, watchface_second->base.h);

    gui_img_rotation(watchface_hour, angle_hour,  watchface_hour->base.w / 2,
                     watchface_hour->base.h);
    gui_img_rotation(watchface_minute, angle_min, watchface_minute->base.w / 2,
                     watchface_minute->base.h);
    gui_img_rotation(watchface_second, angle_sec, watchface_second->base.w / 2,
                     watchface_second->base.h);
}

void design_tab_watchface(void *parent)
{
    img_clock = gui_img_create_from_mem(parent, "img_clock", CLOCK_BASE_BIN, 0, 0, 454, 454);

    watchface_hour = gui_img_create_from_mem(parent, "hour", ICON_CLOCK_HOUR_HAND_BIN, 220, 129, 12,
                                             100);
    watchface_minute = gui_img_create_from_mem(parent, "minute", ICON_CLOCK_MINUTE_HAND_BIN, 220, 97,
                                               12,
                                               132);
    watchface_second = gui_img_create_from_mem(parent, "second", ICON_CLOCK_SECOND_HAND_BIN, 222, 81, 8,
                                               148);
    watchface_hand = gui_img_create_from_mem(parent, "hand", ICON_CLOCK_HAND_DOT_BIN, 218, 218, 0, 0);
    gui_img_set_quality(watchface_hour, 1);
    gui_img_set_quality(watchface_second, 1);
    gui_img_set_quality(watchface_minute, 1);

    //gui_img_set_animate(watchface_second, 1, -1, img_animate_watchface_callback, NULL);

    curtainview_menu = gui_curtainview_create(parent, "curtainview_menu", 0, 0, 454, 454);
    void design_curtain_menu(void *parent);
    design_curtain_menu(curtainview_menu);


    gui_win_t *win_switch_to_perspective_app = gui_win_create(parent, "win_switch_to_perspective_app",
                                                              0, 0, 454, 454);
    //gui_win_onLong(win_switch_to_perspective_app, win_switch_to_perspective_app_touch_cb, NULL);
    //gui_win_onRelease(win_switch_to_perspective_app, win_switch_to_perspective_app_touch_cb, NULL);
    gui_win_onClick(win_switch_to_perspective_app, win_switch_to_perspective_app_touch_cb, NULL);
}
