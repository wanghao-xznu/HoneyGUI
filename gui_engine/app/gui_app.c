/*
 * File      : gui_app.c
 * This file is part of GUI Engine
 */
#include <guidef.h>
#include <gui_app.h>
#include <string.h>
#include <string.h>
#include <stdio.h>
#include "gui_obj.h"


gui_app_t *current_app;
gui_app_t *next_app;
static bool app_layer;
void gui_app_layer_top()
{
    app_layer = true;
}
void gui_app_layer_buttom()
{
    app_layer = false;
}
bool gui_app_get_layer()
{
    return app_layer;
}
gui_app_t *gui_current_app(void)
{
    return current_app;
}
gui_app_t *gui_next_app(void)
{
    return next_app;
}
void gui_app_exec(gui_app_t *app)
{
    gui_msg_t msg;
    msg.app = app;
    GUI_UNUSED(msg);
}



static void ctor(void *this)
{
    GUI_ASSERT(this != NULL);
}

static void dtor(void *this)
{
    GUI_ASSERT(this != NULL);
}

void gui_app_install(gui_app_t *app, void *ui_design, void *gui_app_entry)
{

    app->ctor = ctor;
    app->dtor = dtor;

    app->thread_entry = (void (*)(void *))gui_app_entry;
    app->ui_design = (void (*)(gui_app_t *))ui_design;
    ctor(app);

}

void gui_app_startup(gui_app_t *app)
{
    gui_list_init(&app->screen.child_list);
    gui_list_init(&app->screen.brother_list);
    app->screen.opacity_value = UINT8_MAX;
    app->ui_design(app);
    if (app->active_ms == 0)
    {
        app->active_ms = 10000;
    }
    gui_app_exec(app);
    if (app->thread_entry != NULL)
    {
        app->thread_id = gui_thread_create(app->screen.name,
                                           app->thread_entry, app,
                                           1024 * 3, 25);
    }
    if (current_app && !current_app->close_sync)
    {
        next_app = app;
        current_app->next = true;
    }
    else
    {
        current_app = app;
    }
    app->screen.opacity_value = UINT8_MAX;
    app->screen.create_done = true;

}

void gui_app_shutdown(gui_app_t *app)
{
    if (app->thread_entry != NULL)
    {
        gui_thread_delete(app->thread_id);
    }

    if (app->next)
    {
        app->close = true;
    }
    else
    {
        gui_tree_free(&app->screen);
        app->close_sync = true;
    }
}

void gui_app_uninstall(gui_app_t *app)
{
    gui_tree_free(&app->screen);
}

gui_app_t *gui_app_create(const char *app_name, void *ui_design, void *gui_app_entry)
{
    gui_app_t *app = gui_malloc(sizeof(gui_app_t));
    memset(app, 0, sizeof(gui_app_t));
    app->screen.name = app_name;

    gui_app_install(app, ui_design, gui_app_entry);

    return app;
}

void gui_app_delete(gui_app_t *app)
{
    gui_app_uninstall(app);
}

void gui_switch_app(gui_app_t *from, gui_app_t *to)
{
    gui_app_shutdown(from);

    gui_app_startup(to);
    gui_log("gui_switch_app from %s to %s\n", from->screen.name, to->screen.name);
}


