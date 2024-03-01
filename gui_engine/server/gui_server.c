/**
 * @file gui_server.c
 * @author howie_wang (howie_wang@realtek.com.cn)
 * @brief
 * @version 0.1
 * @date 2023-12-12
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <guidef.h>
#include <draw_img.h>
#include <draw_font.h>
#include <gui_fb.h>
#include <gui_server.h>
#include <gui_obj.h>
#include <tp_algo.h>
#include <kb_algo.h>
#include <wheel_algo.h>
#include <string.h>
#include <gui_app.h>
#include "acc_engine.h"

static void *gui_server_mq = NULL;
static void (*gui_task_ext_execution_hook)(void) = NULL;
bool GUI_SERVER_ALLOW_DLPS = false;


static void gui_server_msg_handler(gui_msg_t *msg)
{

}
void gui_task_ext_execution_sethook(void (*hook)(void))
{
    gui_task_ext_execution_hook = hook;
}

bool send_msg_to_gui_server(gui_msg_t *msg)
{
    if (gui_server_mq != NULL)
    {
        if (msg == NULL)
        {
            gui_log("msg == NULL\n");
        }

        gui_mq_send(gui_server_mq, msg, sizeof(gui_msg_t), 0);
        return true;
    }
    else
    {
        return false;
    }
}

static uint32_t daemon_start_ms = 0;
static uint32_t daemon_cnt = 0;

/**
 * @brief
 *
 * @param parameter
 */
static void gui_server_entry(void *parameter)
{
#if defined ENABLE_RTK_GUI_SCRIPT_AS_A_APP
    extern void js_init(void);
    js_init();
#endif
    gui_mq_create(&gui_server_mq, "gui_svr_mq", sizeof(gui_msg_t), 16);
    while (1)
    {
        gui_app_t *app = gui_current_app();
        gui_app_t *app_next = gui_next_app();
        while (app == NULL)
        {
            gui_thread_mdelay(1000);
            app = gui_current_app();
            gui_log("!!! GUI APP NOT READY! \n");
        }

        gui_obj_t *screen = &app->screen;
        GUI_ASSERT(screen != NULL);

        /*exe some app action, like kick watchdog*/
        if (gui_task_ext_execution_hook != NULL)
        {
            gui_task_ext_execution_hook();
        }

        tp_algo_process(touchpad_get_data());

        kb_algo_process(kb_get_data());

        wheel_algo_process(wheel_get_data());

        if (app->lvgl == true)
        {
            continue;
        }
        bool layer = gui_app_get_layer();
        if (layer)
        {
            if (app_next && (&(app_next->screen)))
            {
                if (app->close)
                {
                    extern gui_app_t *next_app;
                    extern gui_app_t *current_app;
                    current_app->close = false;
                    gui_list_remove(&(app->screen.brother_list));
                    app->screen.parent = 0;
                    gui_tree_free(&((gui_app_t *)app)->screen);
                    current_app = next_app;
                    next_app = 0;
                    app = current_app;
                }
                else
                {
                    gui_list_insert_before(&((screen)->child_list), &(app_next->screen.brother_list));
                    app_next->screen.parent = screen;
                }
            }
            screen = &app->screen;
        }
        else
        {
            if (app_next && (&(app_next->screen)))
            {
                if (app->close)
                {
                    extern gui_app_t *next_app;
                    extern gui_app_t *current_app;
                    current_app->close = false;
                    gui_list_remove(&(app->screen.brother_list));
                    app->screen.parent = 0;
                    gui_tree_free(&((gui_app_t *)app)->screen);
                    current_app = next_app;
                    next_app = 0;
                    app = current_app;
                }
                else
                {

                    gui_list_insert_before(&((app_next->screen).child_list), &(screen->brother_list));
                    screen->parent = &(app_next->screen);
                    screen = &(app_next->screen);
                }
            }

        }



        gui_fb_disp(screen);
        app_next = gui_next_app();
        if (layer)
        {
            if (app_next && (&(app_next->screen)))
            {
                gui_list_remove(&(app_next->screen.brother_list));
            }
        }
        else
        {
            if (app_next && (&(app_next->screen)))
            {
                gui_list_remove(&(app->screen.brother_list));
            }
        }


#ifdef _WIN32
        gui_thread_mdelay(17);
#endif
        daemon_cnt++;
        if (daemon_cnt == 1)
        {
            daemon_start_ms = gui_ms_get();
        }

        touch_info_t *tp = tp_get_info();

        if (tp->pressed == true || tp->pressing == true)
        {
            daemon_start_ms = gui_ms_get();
        }

#if defined __WIN32
        continue;
#endif

        if ((gui_ms_get() - daemon_start_ms) > app->active_ms)
        {
            gui_log("line %d, aemon_start_ms time = %dms, current = %dms, app->active_ms = %dms \n",
                    __LINE__, daemon_start_ms, gui_ms_get(), app->active_ms);
            gui_msg_t msg;
            gui_display_off();
            GUI_SERVER_ALLOW_DLPS = true;
            if (true == gui_mq_recv(gui_server_mq, &msg, sizeof(gui_msg_t), 0xFFFFFFFF))
            {
                gui_server_msg_handler(&msg);
            }
            GUI_SERVER_ALLOW_DLPS = false;
            gui_display_on();
            gui_fb_change();
            daemon_cnt = 0;
            daemon_start_ms = 0;
        }
    }
}

bool gui_server_dlps_check(void)
{
    return GUI_SERVER_ALLOW_DLPS;
}

/**
 * @brief
 *
 * @return int
 */
int gui_server_init(void)
{
    extern void gui_port_dc_init(void);
    extern void gui_port_indev_init(void);
    extern void gui_port_os_init(void);
    extern void gui_port_fs_init(void);
    gui_port_os_init();
    gui_port_indev_init();
    gui_port_dc_init();
    gui_port_fs_init();

    gui_acc_init();

    gui_fb_change();

    gui_thread_create(GUI_SERVER_THREAD_NAME,
                      gui_server_entry, NULL,
                      1024 * 10,
                      15);
    return 0;
}



