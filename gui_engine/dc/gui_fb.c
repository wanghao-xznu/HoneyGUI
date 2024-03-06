/**
 * @file gui_fb.c
 * @author howie_wang (howie_wang@realtek.com.cn)
 * @brief
 * @version 0.1
 * @date 2023-10-16
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <string.h>
#include <guidef.h>
#include <gui_matrix.h>
#include <gui_fb.h>
#include <gui_obj.h>
#include "gui_server.h"




static bool obj_is_active(gui_obj_t *obj)
{

    gui_point_t p[4] =
    {
        {0, 0, 1},
        {obj->w, 0, 1},
        {0, obj->h, 1},
        {obj->w, obj->h, 1}
    };

    float x_min = 0.0f;
    float x_max = 0.0f;
    float y_min = 0.0f;
    float y_max = 0.0f;


    matrix_multiply_point(obj->matrix, p);
    x_min = p->p[0];
    x_max = p->p[0];
    y_min = p->p[1];
    y_max = p->p[1];
    matrix_multiply_point(obj->matrix, p + 1);
    matrix_multiply_point(obj->matrix, p + 2);
    matrix_multiply_point(obj->matrix, p + 3);

    for (uint32_t i = 1; i < 3; i++)
    {
        gui_point_t *point = p + i;
        if (point->p[0] < x_min)
        {
            x_min = point->p[0];
        }
    }

    for (uint32_t i = 1; i < 3; i++)
    {
        gui_point_t *point = p + i;
        if (point->p[0] > x_max)
        {
            x_max = point->p[0];
        }
    }

    for (uint32_t i = 1; i < 3; i++)
    {
        gui_point_t *point = p + i;
        if (point->p[1] < y_min)
        {
            y_min = point->p[1];
        }
    }

    for (uint32_t i = 1; i < 3; i++)
    {
        gui_point_t *point = p + i;
        if (point->p[1] > y_max)
        {
            y_max = point->p[1];
        }
    }

    if ((x_min < (int)gui_get_screen_width()) && (x_max > 0) && \
        (y_min < (int)gui_get_screen_height()) && (y_max > 0))
    {
        obj->active = true;
        return true;
    }

    return false;
}

static void obj_input_prepare(gui_obj_t *obj)
{
    gui_list_t *node = NULL;
    gui_list_for_each(node, &obj->child_list)
    {
        gui_obj_t *obj = gui_list_entry(node, gui_obj_t, brother_list);

        if (obj->obj_input_prepare != NULL)
        {
            obj->obj_input_prepare(obj);
        }

        obj_input_prepare(obj);
    }
}

static void obj_draw_prepare(gui_obj_t *object)
{
    GUI_ASSERT(object->name != NULL);
    gui_list_t *node = NULL;

    gui_list_for_each(node, &object->child_list)
    {
        GUI_ASSERT(node != NULL);
        gui_obj_t *obj = gui_list_entry(node, gui_obj_t, brother_list);

        if (!(obj->create_done))
        {
            continue;
        }
        if (obj->not_show)
        {
            continue;
        }

        obj->opacity_value = obj->parent->opacity_value;

        if ((obj->matrix != NULL) && (obj->parent->matrix))
        {
            memcpy(obj->matrix, obj->parent->matrix, sizeof(gui_matrix_t));
            matrix_translate(obj->x, obj->y, obj->matrix);
        }

        if (obj->obj_prepare != NULL)
        {
            obj->obj_prepare(obj);
        }
        if (obj->not_show)
        {
            continue;
        }
        if (obj_is_active(obj) == false)
        {
            continue;
        }
        obj_draw_prepare(obj);
    }
}

static void obj_draw_scan(gui_obj_t *obj)
{
    gui_list_t *node = NULL;
    struct gui_dispdev *dc = gui_get_dc();
    gui_list_for_each(node, &obj->child_list)
    {
        gui_obj_t *obj = gui_list_entry(node, gui_obj_t, brother_list);
        if (obj->active)
        {
            dc->section.y1 = dc->section_count * dc->fb_height;
            dc->section.y2 = _UI_MIN(dc->section.y1 + dc->fb_height, dc->screen_height);
            if (obj->obj_draw != NULL)
            {
                obj->obj_draw(obj);
            }

        }
        obj_draw_scan(obj);
    }
}


#define MAX_EVENT_CNT   10
static gui_event_cb_t event_cb[MAX_EVENT_CNT];
static gui_event_t event_code[MAX_EVENT_CNT];
static void *event_obj[MAX_EVENT_CNT];
static uint8_t event_cnt = 0;

static void obj_draw_end(gui_obj_t *obj)
{
    gui_list_t *node = NULL;
    gui_list_for_each(node, &obj->child_list)
    {
        gui_obj_t *obj = gui_list_entry(node, gui_obj_t, brother_list);
        if (obj->obj_end != NULL)
        {
            obj->obj_end(obj);
        }
        if (obj->active)
        {
            for (uint32_t i = 0; i < obj->event_dsc_cnt; i++)
            {
                gui_event_dsc_t *event_dsc = obj->event_dsc + i;
                if (event_dsc->filter == event_dsc->event_code)
                {
                    event_cb[event_cnt] = event_dsc->event_cb;
                    event_code[event_cnt] = event_dsc->event_code;
                    event_obj[event_cnt] = obj;
                    event_dsc->event_code = GUI_EVENT_INVALIDE;
                    event_cnt++;
                }
            }
        }

        matrix_identity(obj->matrix);
        obj->active = false;
        obj_draw_end(obj);
    }
}

static void gui_fb_draw(gui_obj_t *root)
{
    gui_dispdev_t *dc = gui_get_dc();
    uint32_t active_line = dc->driver_ic_active_width;
    uint32_t hfp_line = dc->driver_ic_hfp;
    uint32_t hbp_line = dc->driver_ic_hbp;
    uint32_t fps = dc->driver_ic_fps;
    uint32_t line_time = 1000000 / (fps * (active_line + hfp_line + hbp_line));

    uint32_t read_time = 0;
    uint32_t write_time = 0;

    if (dc->reset_lcd_timer != NULL)
    {
        dc->reset_lcd_timer();
    }

    if (dc->lcd_te_wait != NULL)
    {
        dc->lcd_te_wait();
    }


    if (dc->type == DC_RAMLESS)
    {
        uint32_t section_count = dc->screen_height / dc->fb_height + ((dc->screen_height % dc->fb_height) ?
                                                                      1 : 0);
        gui_flash_boost();
        for (uint32_t i = 0; i < section_count; i++)
        {
            if (i % 2 == 0)
            {
                dc->frame_buf = dc->disp_buf_1;
            }
            else
            {
                dc->frame_buf = dc->disp_buf_2;
            }
            if (dc->frame_buf)
            {
                memset(dc->frame_buf, 0x0, dc->fb_height * dc->fb_width * dc->bit_depth >> 3);
            }
            dc->section_count = i;

            obj_draw_scan(root);
            if (dc->get_lcd_us != NULL)
            {
                read_time = line_time * (i * dc->fb_height + hfp_line);
                write_time = dc->get_lcd_us();
                while (write_time < read_time)
                {
                    write_time = dc->get_lcd_us();
                }
                //gui_log("read = %dus, write = %d us\n", read_time, write_time);
            }

            dc->lcd_update(dc);
        }

        gui_flash_boost_disable();
    }
    else if (dc->type == DC_SINGLE)
    {
        memset(dc->frame_buf, 0x00, dc->fb_height * dc->fb_width * dc->bit_depth >> 3);

#if 0
        uint32_t start_tick = rt_tick_get_millisecond();
        obj_draw_scan(root);
        uint32_t end_tick = rt_tick_get_millisecond();
        gui_log("draw time :%dms \n", end_tick - start_tick);
        uint32_t lcd_start_tick = rt_tick_get_millisecond();
        dc->lcd_update(dc);
        uint32_t lcd_end_tick = rt_tick_get_millisecond();
        gui_log("lcd time :%dms \n", lcd_end_tick - lcd_start_tick);
#else
        obj_draw_scan(root);
        dc->lcd_update(dc);
#endif
    }
}

static bool fb_change = false;
void gui_fb_change(void)
{
    fb_change = true;
}
static uint32_t spf = 5;
uint32_t gui_spf()
{
    return spf;
}
void gui_fb_disp(gui_obj_t *root)
{
    if (root == NULL)
    {
        GUI_ASSERT(NULL != NULL);
        return;
    }

    gui_dispdev_t *dc = gui_get_dc();
    if ((dc->frame_buf == NULL) && (dc->disp_buf_1 == NULL) && (dc->disp_buf_2 == NULL))
    {
        GUI_ASSERT(NULL != NULL);
        return;
    }

    obj_input_prepare(root);
    obj_draw_prepare(root);

    static int last_ms;
    if (fb_change == true)
    {
        gui_fb_draw(root);
        fb_change = false;
        {

            int ms = gui_ms_get();
            if (last_ms)
            {
                spf = ms - last_ms;
                //gui_log("spf:%d ms", spf);
            }
            else { spf = 0; }
            last_ms = ms;
        }
    }
    else
    {
        gui_thread_mdelay(17);
        last_ms = gui_ms_get();
    }


    obj_draw_end(root);
    for (uint8_t i = 0; i < event_cnt; i++)
    {
        event_cb[i](event_obj[i], event_code[i]);
    }
    event_cnt = 0;

}


