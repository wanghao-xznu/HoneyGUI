#include "ezxml.h"
#include <gui_tabview.h>
#include <gui_img.h>
#include <gui_win.h>
#include <gui_text.h>
#include <gui_obj.h>
#include <gui_curtain.h>
#include "gui_button.h"
#include <gui_seekbar.h>
#include <gui_page.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include "gui_server.h"
#include <gui_img.h>
#include <gui_app.h>
#include <stdlib.h>
#include "acc_engine.h"
#include "gui_grid.h"
#include "gui_scroll_text.h"
#include "gui_switch.h"
#include "string.h"
#include "stdio.h"
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include "ezxml.h"
#ifdef __arm__
#include "romfs.h"
#else
#include <sys/stat.h>
#include <fcntl.h>
#endif

#if defined __WIN32
#include <dirent.h>
#else

#endif
//char *GUI_ROOT_FOLDER = GUI_ROOT_FOLDER;
struct widget_create
{
    char *name;
    obj_type_t type;
};

#define WIDGETS_NUM 20

struct widget_create widget[] =
{
    { "win", WINDOW }, //
    { "textbox", TEXTBOX },//
    { "img", IMAGE },//
    { "progressbar", PROGRESSBAR },
    { "seekbar", SEEKBAR },
    { "tabview", TABVIEW },//
    { "tab", TAB },
    { "curtainview", CURTAINVIEW },//
    { "curtain", CURTAIN },
    //{ "iconlist", ICONLIST },
    { "icon", ICON },//
    { "script", JAVASCRIPT },
    { "switch", CLICKSWITCH },
    { "page", PAGE },//
    //{ "pagebar", PAGEBAR },
    { "screen", SCREEN },
    { "grid",  GRID},
    {"radio", RADIO},
    {"appendTexts", RADIOSWITCH},
    {"arc", ARC},
    {"movie", MOVIE},
    {"animateTransform", MACRO_ANIMATETRANSFORM}
};
static void img_rotate_cb(gui_img_t *img)
{
    gui_img_translate(img, img->base.w / 2,
                      img->base.h);

    gui_img_rotation(img, 360 * img->animate->progress_percent, img->base.w / 2,
                     img->base.h);
}
gui_img_t *xml_gui_img_create_from_mem(void *parent,  const char *name, void *addr,
                                       int16_t x,
                                       int16_t y)
{
    if (addr == NULL)
    {
        addr = gui_get_file_address("app/system/resource/icMenuBird.bin");
    }

    return gui_img_create_from_mem(parent, name, addr,
                                   x,
                                   y, 0, 0);
}

gui_img_t *xml_gui_img_create(void *parent, const char *file, int16_t x, int16_t y)
{
    if (file == NULL)
    {
        file = gui_get_file_address("app/system/resource/icMenuBird.bin");
    }

    // check image size
    char *path = gui_malloc(strlen(file) + strlen(GUI_ROOT_FOLDER) + 1);
    sprintf(path, "%s%s", GUI_ROOT_FOLDER, file);
    gui_log("xml imgfile>%s \n", path);

    int fd = gui_fs_open(path,  0);
    if (fd == -1)
    {
        gui_log("open file fail !\n");
        return NULL;
    }
    int size = gui_fs_lseek(fd, 0, SEEK_END) - gui_fs_lseek(fd, 0, SEEK_SET);
    gui_log("img size: %d \n", size);
    gui_fs_close(fd);
    // gui_free(path);

    if (size < 100000)
    {
#ifdef __WIN32
        path = file;
#endif
        return parent = (void *)gui_img_create_from_fs(parent, path, x, y);
    }
    else
    {
        void *imgbuf = gui_get_file_address(file);
        return parent = (void *)gui_img_create_from_mem(parent, "image", imgbuf, x, y, 0, 0);
    }
}

char *get_space_string_head(const char *string)
{
    char *s = gui_strdup(string);
    size_t length = strlen(s);
    for (int i = length - 1; i >= 0; i--)
    {
        if (!((s[i] >= 8) && (s[i] <= 32)))
        {
            s[i + 1] = '\0';
            break;
        };
    }
    return s;
}
int line = 0;
char *js;

#define RGB888_RED      0x00ff0000
#define RGB888_GREEN    0x0000ff00
#define RGB888_BLUE     0x000000ff

#define RGB565_RED      0xf800
#define RGB565_GREEN    0x07e0
#define RGB565_BLUE     0x001f

unsigned short RGB888ToRGB565(unsigned int n888Color)
{
    unsigned short n565Color = 0;


    unsigned char cRed   = (n888Color & RGB888_RED)   >> 19;
    unsigned char cGreen = (n888Color & RGB888_GREEN) >> 10;
    unsigned char cBlue  = (n888Color & RGB888_BLUE)  >> 3;


    n565Color = (cRed << 11) + (cGreen << 5) + (cBlue << 0);
    return n565Color;
}
unsigned short string_rgb888_to_rgb565(const char *color)
{
    if (*color != '#')
    {
        gui_log("color format wrong");
        return 0;
    }
    uint32_t c = strtoul(color + 1, NULL, 16);
    gui_log("color%s:%x,%x\n", color, c, RGB888ToRGB565(c));
    return RGB888ToRGB565(c);
}
uint32_t string_rgb888(const char *color)
{
    if (*color != '#')
    {
        gui_log("color format wrong");
        return 0;
    }
    uint32_t c = strtoul(color + 1, NULL, 16);

    return (c);
}
static void sport_button_press_ani_cb(gui_button_t *button)
{
    float per = (button->animate->progress_percent);
    float alpha_to = 100;
    static float alpha_from = 0;
    static float from = 0;
    if (per == 0.0f)
    {
        from = ((gui_img_t *)(button->img))->scale_x;
        alpha_from = button->img->draw_img.opacity_value;
    }
    from = 1.0f;
    float scale = (-0.5f * from) * per + from;
    //circle_color_tab[1] = change_opacity_of_rgba(circle_color_tab[1],
    //                                             (uint8_t)(int)((alpha_to - alpha_from) * per + alpha_from));
    if (button->style == WIDGET_FADE || button->style == WIDGET_SCALE_FADE)
    {
        gui_img_set_opacity((void *)button->img,
                            (uint8_t)(int)((alpha_to - alpha_from) * per + alpha_from));
    }
    if (button->style == WIDGET_SCALE || button->style == WIDGET_SCALE_FADE)
    {
        gui_img_scale((void *)button->img, scale,  scale);
        //gui_img_translate((void *)button->img, 25.0f * (from - scale),  25.0f * (from - scale));
    }



    //gui_log("o:%f\n", 25.0f * (from - scale));
}

static void draw_arc(gui_canvas_t *c)
{

    nvgBeginPath(c->vg);
    nvgLineCap(c->vg, NVG_ROUND);
    /*can't get arc data*/
    //nvgArc(c->vg, this->cx, this->cy, this->r, this->a0, this->a1, NVG_CW);
    //nvgStrokeColor(c->vg, this->color);
    //nvgStrokeWidth(c->vg, this->width);
    nvgArc(c->vg, 100, 100, 20, 1, 2, NVG_CW);
    nvgStrokeColor(c->vg, nvgRGBA(255, 255, 255, 255));
    nvgStrokeWidth(c->vg, 3);
    nvgStroke(c->vg);

}
static void sport_button_release_ani_cb(gui_button_t *button)
{
    float per = (button->animate->progress_percent);
    float alpha_to = UINT8_MAX;
    static float alpha_from = 0;
    static float from = 0;
    if (per == 0.0f)
    {
        from = ((gui_img_t *)(button->img))->scale_x;
        alpha_from = button->img->draw_img.opacity_value;
    }
    //from = 1.25f;
    float scale = (1.0f - from) * per + from;
    //circle_color_tab[1] = change_opacity_of_rgba(circle_color_tab[1],
    //                                             (uint8_t)(int)((alpha_to - alpha_from) * per + alpha_from));


    //gui_img_translate((void *)button->img, 25.0f * (1.0f - scale),  25.0f * (1.0f - scale));
    if (button->style == WIDGET_FADE || button->style == WIDGET_SCALE_FADE)
    {
        gui_img_set_opacity((void *)button->img,
                            (uint8_t)(int)((alpha_to - alpha_from) * per + alpha_from));
    }
    if (button->style == WIDGET_SCALE || button->style == WIDGET_SCALE_FADE)
    {
        gui_img_scale((void *)button->img, scale,  scale);
    }
}
static void sport_button_press(gui_button_t *b)
{
    gui_button_api.set_animate(b, 200, 0, sport_button_press_ani_cb, b);
}
static void sport_button_release(gui_button_t *b)
{
    gui_button_api.set_animate(b, 200, 0, sport_button_release_ani_cb, b);
}
gui_obj_t *widget_create_handle(ezxml_t p, gui_obj_t *parent)
{
    char *name = p->name;
    gui_log("%s\n", name);
    for (size_t i = 0; i < WIDGETS_NUM; i++)
    {
        if (!strcmp(widget[i].name, name))
        {
            switch (widget[i].type)
            {
            case TEXTBOX:
                {
                    size_t i = 0;
                    int16_t x = 0;
                    int16_t y = 0;
                    int16_t w = gui_get_screen_width();
                    int16_t h = 50;
                    const char *text = "text";
#ifdef __WIN32
                    char *font = "app/system/resource/font/tangyuanti.ttf";
#elif defined RTL8772F
                    char *font = "app/system/resource/font/tangyuanti.ttf";
#else
                    char *font =
                        "app/system/resource/font/gbk_32_32_dot.bin;app/system/resource/font/gbk_unicode_table.bin";
#endif
                    size_t color = 0xffffffff;
                    int fontSize = 32;
                    int style = 0;
                    while (true)
                    {
                        if (!(p->attr[i]))
                        {
                            break;
                        }
                        //gui_log("p->attr[i]:%s\n", p->attr[i]);
                        if (!strcmp(p->attr[i], "x"))
                        {
                            x = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "y"))
                        {
                            y = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "w"))
                        {
                            uint32_t w_temp = atoi(p->attr[++i]);
                            if (w_temp)
                            {
                                w = w_temp;
                            }
                        }
                        else if (!strcmp(p->attr[i], "h"))
                        {
                            uint32_t w_temp = atoi(p->attr[++i]);
                            if (w_temp)
                            {
                                h = w_temp;
                            }
                        }
                        else if (!strcmp(p->attr[i], "text"))
                        {
                            text = p->attr[++i];
                        }
                        else if (!strcmp(p->attr[i], "font"))
                        {
                            font = p->attr[++i];
                        }
                        else if (!strcmp(p->attr[i], "fontSize"))
                        {
                            fontSize = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "color"))
                        {
                            color = string_rgb888(p->attr[++i]);
                            gui_log("color %s,%x\n", p->attr[i], color);
                        }
                        else if (!strcmp(p->attr[i], "mode"))
                        {
                            char *s = p->attr[++i];
                            if (!strcmp(p->attr[i], "truncate"))
                            {
                                style = 0;
                            }
                            else if (!strcmp(p->attr[i], "verticalscroll"))
                            {
                                style = SCROLL_Y;
                            }
                            else if (!strcmp(p->attr[i], "horizontalscroll"))
                            {
                                style = SCROLL_X;
                            }

                        }
                        i++;
                    }
                    //gui_log("x:%d,y:%d,w:%dh:%d,font:%s,text:%s,color:%x\n", x, y, w, h, font, text, color);
                    char *ptxt = get_space_string_head(p->txt);
                    //gui_log("p->txt2 = %s,\n", ptxt);

                    if (text && font)
                    {
                        gui_text_t *t = 0;
                        if (style == 0)
                        {
                            t = gui_text_create(parent, ptxt, x, y, gui_get_screen_width(), h);
                            gui_color_t color_temporary;
                            color_temporary.color.rgba_full = color;
                            gui_text_set(t, gui_strdup(text), "rtk_font_mem", color_temporary, strlen(text), 32);



                            {
                                char *font_type2 = NULL;
                                char *font_type = font;
                                if (strstr(font_type, ".bin") != NULL)
                                {
                                    if (strstr(font_type, ".bin;") != NULL)
                                    {
                                        font_type2 = "rtk_font_mem";
                                        char b[100] = {0};
                                        strncpy(b, font_type, strstr(font_type, ".bin;") - font_type + strlen(".bin"));
                                        void *addr1 = gui_get_file_address(b);
                                        memset(b, 0, sizeof(b));
                                        char *a = font_type;
                                        strncpy(b, strstr(a, ".bin;") + strlen(".bin;"), strlen(a) - (strstr(a,
                                                                                                             ".bin;") - a + strlen(".bin;")));
                                        void *addr2 = gui_get_file_address(b);
                                        gui_set_font_mem_resourse(fontSize, addr1,  addr2);
                                        t->font_height = fontSize;
                                        t->path = 0;
                                        t->font_type = GUI_FONT_SOURCE_BMP;
                                    }
                                    else
                                    {
                                        font_type2 = "rtk_font_mem";
                                        void *addr1 = gui_get_file_address(font_type);
                                        gui_font_mem_init(addr1);
                                        t->font_height = fontSize;
                                        t->path = 0;
                                        t->font_type = GUI_FONT_SOURCE_BMP;
                                        gui_text_type_set(t, addr1);
                                        gui_text_mode_set(t, LEFT);
                                        // t->font_height = fontSize;
                                        //t->path = 0;
                                    }





                                }
                                else if ((strstr(font_type, ".ttf") != NULL) || (strstr(font_type, ".TTF") != NULL))
                                {
#ifdef __WIN32
                                    font_type2 = "rtk_font_stb";
                                    t->path = gui_get_file_address(font);
                                    t->font_height = fontSize;
                                    t->font_type = GUI_FONT_SOURCE_TTF;
#elif defined RTL8772F
                                    font_type2 = "rtk_font_stb";
                                    t->path = gui_get_file_address(font);
                                    t->font_height = fontSize;
                                    t->font_type = GUI_FONT_SOURCE_TTF;
#else
                                    font_type =
                                        "app/system/resource/font/gbk_32_32_dot.bin;app/system/resource/font/gbk_unicode_table.bin";
                                    {
                                        font_type2 = "rtk_font_mem";
                                        char b[100] = {0};
                                        strncpy(b, font_type, strstr(font_type, ".bin;") - font_type + strlen(".bin"));
                                        void *addr1 = gui_get_file_address(b);
                                        memset(b, 0, sizeof(b));
                                        char *a = font_type;
                                        strncpy(b, strstr(a, ".bin;") + strlen(".bin;"), strlen(a) - (strstr(a,
                                                                                                             ".bin;") - a + strlen(".bin;")));
                                        void *addr2 = gui_get_file_address(b);
                                        gui_set_font_mem_resourse(fontSize, addr1,  addr2);
                                        t->path = 0;
                                        t->font_type = GUI_FONT_SOURCE_BMP;
                                    }
#endif
                                }

                            }
                        }
                        else
                        {
                            gui_scroll_text_t *scroll_text = gui_scrolltext_create(parent,  ptxt, x, y,
                                                                                   strlen(text) * (fontSize / 4), h);
                            gui_scrolltext_scroll_set(scroll_text, style, 100, 0, 3000, 1500000);
                            gui_color_t color_temporary;
                            color_temporary.color.rgba_full = color;
                            gui_scrolltext_text_set(scroll_text, gui_strdup(text), "rtk_font_stb", color_temporary,
                                                    strlen(text),
                                                    fontSize);
                            t = (void *)scroll_text;
                        }




                        parent = (void *)t;
                    }
                }
                break;
            case IMAGE:
                {
                    size_t i = 0;
                    int16_t x = 0;
                    int16_t y = 0;
                    int16_t w = 0;
                    int16_t h = 0;
                    char *file = NULL;
                    float scalex = 1;
                    float scaley = 1;
                    float angle = 0;
                    int mode = 0;
                    while (true)
                    {
                        if (!(p->attr[i]))
                        {
                            break;
                        }
                        //gui_log("p->attr[i]:%s,\n", p->attr[i]);
                        if (!strcmp(p->attr[i], "x"))
                        {
                            x = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "y"))
                        {
                            y = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "w"))
                        {
                            w = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "h"))
                        {
                            h = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "scaleX"))
                        {
                            scalex = atof(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "scaleY"))
                        {
                            scaley = atof(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "rotationAngle"))
                        {
                            angle = atof(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "mode"))
                        {
                            char *s = p->attr[++i];
                            if (!strcmp(p->attr[i], "normal"))
                            {
                                mode = IMG_FILTER_BLACK;
                            }
                            else if (!strcmp(p->attr[i], "blendSrcBlack"))
                            {
                                mode = IMG_BYPASS_MODE;
                            }

                        }
                        else if (!strcmp(p->attr[i], "file"))
                        {
                            char *filetemp = p->attr[++i];
                            if (strlen(filetemp) == 0)
                            {
                                file = NULL;
                            }
                            else
                            {
                                file = gui_strdup(filetemp);
                            }
                        }
                        i++;
                    }
                    char *ptxt = get_space_string_head(p->txt);
                    //gui_log("x:%d,y:%d,w:%dh:%d,file:%s\n", x, y, w, h, file);
                    if (file)
                    {
                        void *imgbuf = gui_get_file_address(file);
                        parent = (void *)xml_gui_img_create_from_mem(parent, gui_strdup(p->txt), imgbuf, x, y);
                        gui_img_set_mode((gui_img_t *)parent, IMG_SRC_OVER_MODE);
                        if (scalex != 1 || scaley != 1)
                        {
                            gui_img_scale((void *)parent, scalex, scaley);
                        }
                        if (angle != 0)
                        {
                            gui_img_get_height((void *)parent);
                            gui_img_rotation((void *)parent, angle, parent->w / 2, parent->h / 2);
                        }
                    }
                }
                break;
            case WINDOW:
                {
                    size_t i = 0;
                    int16_t x = 0;
                    int16_t y = 0;
                    int16_t w = 0;
                    int16_t h = 0;
                    bool hide = false;
                    while (true)
                    {
                        if (!(p->attr[i]))
                        {
                            break;
                        }
                        //gui_log("p->attr[i]:%s\n", p->attr[i]);
                        if (!strcmp(p->attr[i], "x"))
                        {
                            x = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "y"))
                        {
                            y = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "w"))
                        {
                            w = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "h"))
                        {
                            h = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "hidden"))
                        {
                            hide = true;
                        }
                        i++;
                    }
                    char *ptxt = get_space_string_head(p->txt);
                    //gui_log("x:%d,y:%d,w:%dh:%d\n", x, y, w, h);
                    {
                        parent = (void *)gui_win_create(parent, ptxt, x, y, w, h);
                        if (hide)
                        {
                            parent->not_show = 1;
                        }


                    }
                }
                break;
            /*case CLICKSWITCH:
            {
                size_t i = 0;
                int16_t x = 0;
                int16_t y = 0;
                while (true)
                {
                    if (!(p->attr[i]))
                    {
                        break;
                    }
                    //gui_log("p->attr[i]:%s\n", p->attr[i]);
                    if (!strcmp(p->attr[i], "x"))
                    {
                        x = atoi(p->attr[++i]);
                    }
                    else if (!strcmp(p->attr[i], "y"))
                    {
                        y = atoi(p->attr[++i]);
                    }
                    i++;
                }
                char *ptxt = get_space_string_head(p->txt);
                //gui_log("x:%d,y:%d,w:%dh:%d\n", x, y, w, h);
                {
                    parent = gui_switch_create(parent, ptxt, x, y,0,0);
                }
            }
            break;*/
            case TABVIEW:
                {
                    size_t i = 0;
                    int16_t x = 0;
                    int16_t y = 0;
                    int16_t w = 0;
                    int16_t h = 0;
                    int style = 0;
                    while (true)
                    {
                        if (!(p->attr[i]))
                        {
                            break;
                        }
                        //gui_log("p->attr[i]:%s\n", p->attr[i]);
                        if (!strcmp(p->attr[i], "x"))
                        {
                            x = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "y"))
                        {
                            y = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "w"))
                        {
                            w = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "h"))
                        {
                            h = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "transition"))
                        {
                            char *s = p->attr[++i];
                            if (!strcmp(p->attr[i], "normal"))
                            {
                                style = CLASSIC;
                            }
                            else if (!strcmp(p->attr[i], "fade"))
                            {
                                style = FADE;
                            }
                            else if (!strcmp(p->attr[i], "scale"))
                            {
                                style = REDUCTION;
                            }
                            else if (!strcmp(p->attr[i], "fadeScale"))
                            {
                                style = REDUCTION_FADE;
                            }

                        }
                        i++;
                    }
                    char *ptxt = get_space_string_head(p->txt);
                    parent = (void *)gui_tabview_create(parent, ptxt, x, y, w, h);
                    gui_tabview_loop((void *)parent, true);
                }
                break;
            case ARC:
                {
                    size_t i = 0;
                    int16_t cx = 100;
                    int16_t cy = 100;
                    int16_t r = 100;
                    int16_t stroke_width = 10;
                    int cap = 0;
                    uint32_t fill = 0xff0000ff;
                    uint32_t stroke = 0xff0000ff;
                    float sd = 0;
                    float ed = 100;
                    while (true)
                    {
                        if (!(p->attr[i]))
                        {
                            break;
                        }
                        //gui_log("p->attr[i]:%s\n", p->attr[i]);
                        if (!strcmp(p->attr[i], "cx") || !strcmp(p->attr[i], "centralX"))
                        {
                            cx = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "cy") || !strcmp(p->attr[i], "centralY"))
                        {
                            cy = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "r") || !strcmp(p->attr[i], "radius"))
                        {
                            r = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "startDegree"))
                        {
                            sd = atof(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "endDegree"))
                        {
                            ed = atof(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "strokeWidth") || !strcmp(p->attr[i], "stroke-width"))
                        {
                            stroke_width = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "stroke-linecap") || !strcmp(p->attr[i], "capMode"))
                        {
                            char *s = p->attr[++i];
                            if (!strcmp(p->attr[i], "butt"))
                            {
                                cap = CAP_BUTT;
                            }
                            else if (!strcmp(p->attr[i], "round"))
                            {
                                cap = CAP_ROUND;
                            }
                            else if (!strcmp(p->attr[i], "square"))
                            {
                                cap = CAP_SQUARE;
                            }


                        }
                        else if (!strcmp(p->attr[i], "fill"))
                        {
                            fill = string_rgb888(p->attr[++i]);
                            gui_log("color %s,%x\n", p->attr[i], fill);
                        }
                        else if (!strcmp(p->attr[i], "stroke") || !strcmp(p->attr[i], "color"))
                        {
                            stroke = string_rgb888(p->attr[++i]);
                            gui_log("color %s,%x\n", p->attr[i], stroke);
                        }
                        i++;
                    }
                    char *ptxt = get_space_string_head(p->txt);
                    parent = (void *)gui_canvas_create(parent, ptxt, 0, 0, 454, 454, 0);
                    gui_canvas_set_canvas_cb((void *)parent, draw_arc);
                    /*GUI_TYPE(gui_canvas_t, parent)->draw = draw_arc;
                    GUI_TYPE(gui_canvas_t, parent)->cx = cx;
                    GUI_TYPE(gui_canvas_t, parent)->cy = cy;
                    GUI_TYPE(gui_canvas_t, parent)->r = r;
                    GUI_TYPE(gui_canvas_t, parent)->sw = stroke_width;
                    GUI_TYPE(gui_canvas_t, parent)->s = stroke;
                    GUI_TYPE(gui_canvas_t, parent)->ed = ed;
                    GUI_TYPE(gui_canvas_t, parent)->sd = sd;*/


                }
                break;
            case PROGRESSBAR:
                {
                    size_t i = 0;
                    int16_t x = 0;
                    int16_t y = 0;
                    int16_t w = 0;
                    int16_t h = 0;
                    uint32_t color = 0xffffffaa;
                    uint32_t highlightColor = 0xffffffff;
                    bool vh = false;
                    bool canvas = false;
                    char *picture = "app/system/resource/Progress bar_full.bin";
                    while (true)
                    {
                        if (!(p->attr[i]))
                        {
                            break;
                        }
                        //gui_log("p->attr[i]:%s\n", p->attr[i]);
                        if (!strcmp(p->attr[i], "x"))
                        {
                            x = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "y"))
                        {
                            y = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "w"))
                        {
                            w = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "h"))
                        {
                            h = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "canvasOrPicture"))
                        {
                            if (!strcmp(p->attr[++i], "canvas"))
                            {
                                canvas = true;
                            }
                            else
                            {
                                canvas = false;
                            }

                        }
                        else if (!strcmp(p->attr[i], "picture"))
                        {
                            picture = gui_strdup(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "color"))
                        {
                            color = string_rgb888(p->attr[++i]);
                            gui_log("color %s,%x\n", p->attr[i], color);
                        }
                        else if (!strcmp(p->attr[i], "highlightColor"))
                        {
                            highlightColor = string_rgb888(p->attr[++i]);
                            gui_log("color %s,%x\n", p->attr[i], highlightColor);
                        }
                        else if (!strcmp(p->attr[i], "orientation"))
                        {
                            char *s = p->attr[++i];
                            if (!strcmp(p->attr[i], "vertical"))
                            {
                                vh = true;
                            }
                            else if (!strcmp(p->attr[i], "V"))
                            {
                                vh = true;
                            }
                            else if (!strcmp(p->attr[i], "horizontal"))
                            {
                                vh = false;
                            }
                            else if (!strcmp(p->attr[i], "H"))
                            {
                                vh = false;
                            }

                        }
                        i++;
                    }
                    char *ptxt = get_space_string_head(p->txt);
                    if (canvas == true)
                    {
#if 0
                        if (vh)
                        {
                            //parent = (void *)gui_progressbar_v_create(parent, ptxt, x, y, w, h);
                        }
                        else
                        {
                            //parent = (void *)gui_progressbar_create(parent, ptxt, x, y, w, h);
                        }

                        GUI_TYPE(gui_progressbar_t, parent)->color = color;
                        GUI_TYPE(gui_progressbar_t, parent)->color_hl = highlightColor;
#endif
                    }
                    else
                    {
                        if (vh)
                        {
                            parent = (void *)gui_progressbar_img_v_create(parent, gui_get_file_address(picture), x, y);
                        }
                        else
                        {
                            parent = (void *)gui_progressbar_img_h_create(parent, gui_get_file_address(picture), x, y);
                        }
                    }
                    parent->name = ptxt;




                }
                break;
            case SEEKBAR:
                {
                    size_t i = 0;
                    int16_t x = 0;
                    int16_t y = 0;
                    int16_t w = 0;
                    int16_t h = 0;
                    uint32_t color = 0xffffffaa;
                    uint32_t highlightColor = 0xffffffff;
                    bool canvas = false;
                    bool arc = false;
                    char *picture = "app/system/resource/Progress bar_full.bin";
                    char *folder = NULL;
                    bool vh = false;
                    int16_t cx = 0;
                    int16_t cy = 0;

                    float sd = 0;
                    float ed = 0;
                    while (true)
                    {
                        if (!(p->attr[i]))
                        {
                            break;
                        }
                        //gui_log("p->attr[i]:%s\n", p->attr[i]);
                        if (!strcmp(p->attr[i], "x"))
                        {
                            x = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "y"))
                        {
                            y = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "w"))
                        {
                            w = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "h"))
                        {
                            h = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "canvasOrPicture"))
                        {
                            if (!strcmp(p->attr[++i], "canvas"))
                            {
                                canvas = true;
                            }
                            else
                            {
                                canvas = false;
                            }

                        }
                        else if (!strcmp(p->attr[i], "folder"))
                        {
                            folder = gui_strdup(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "picture"))
                        {
                            picture = gui_strdup(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "color"))
                        {
                            color = string_rgb888(p->attr[++i]);
                            gui_log("color %s,%x\n", p->attr[i], color);
                        }
                        else if (!strcmp(p->attr[i], "highlightColor"))
                        {
                            highlightColor = string_rgb888(p->attr[++i]);
                            gui_log("color %s,%x\n", p->attr[i], highlightColor);
                        }
                        else if (!strcmp(p->attr[i], "orientation"))
                        {
                            char *s = p->attr[++i];
                            if (!strcmp(p->attr[i], "vertical"))
                            {
                                vh = true;
                            }
                            else if (!strcmp(p->attr[i], "V"))
                            {
                                vh = true;
                            }
                            else if (!strcmp(p->attr[i], "horizontal"))
                            {
                                vh = false;
                            }
                            else if (!strcmp(p->attr[i], "H"))
                            {
                                vh = false;
                            }
                            else if (!strcmp(p->attr[i], "arc"))
                            {
                                arc = true;
                            }

                        }
                        else if (!strcmp(p->attr[i], "cx") || !strcmp(p->attr[i], "centralX"))
                        {
                            cx = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "cy") || !strcmp(p->attr[i], "centralY"))
                        {
                            cy = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "startDegree"))
                        {
                            sd = atof(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "endDegree"))
                        {
                            ed = atof(p->attr[++i]);
                        }
                        i++;
                    }
                    char *ptxt = get_space_string_head(p->txt);
                    if (canvas)
                    {
#if 0
                        /* code */


                        if (vh)
                        {
                            //parent = (void *)gui_seekbar_create(parent, ptxt, x, y, w, h);
                        }
                        else
                        {
                            //parent = (void *)gui_seekbar_h_create(parent, ptxt, x, y, w, h);
                        }

                        GUI_TYPE(gui_progressbar_t, parent)->color = color;
                        GUI_TYPE(gui_progressbar_t, parent)->color_hl = highlightColor;
#endif
                    }
                    else if (!folder)
                    {

                        if (vh)
                        {
                            parent = (void *)gui_seekbar_create_img_v(parent, gui_get_file_address(picture), x, y);
                        }
                        else
                        {
                            parent = (void *)gui_seekbar_create_img_h(parent, gui_get_file_address(picture), x, y);
                        }
                    }
                    else if (folder)
                    {
                        int file_count = 0;

                        {
                            DIR *dir = 0;
                            struct dirent *entry;
                            char *path = gui_malloc(strlen(folder) + strlen(GUI_ROOT_FOLDER) + 1);
                            sprintf(path, "%s%s", GUI_ROOT_FOLDER, folder);
                            if ((dir = opendir(path)) == NULL)
                            {
                                gui_free(path);
                                //perror("opendir() failed"); return;
                            }
                            gui_free(path);
                            while ((entry = readdir(dir)) != NULL)
                            {
                                file_count++;
                            }
                            closedir(dir);
                        }
                        void **image_array = gui_malloc(file_count * sizeof(void *));
                        {
                            DIR *dir = 0;
                            struct dirent *entry;
                            char *path = gui_malloc(strlen(folder) + strlen(GUI_ROOT_FOLDER) + 1);
                            sprintf(path, "%s%s", GUI_ROOT_FOLDER, folder);
                            if ((dir = opendir(path)) == NULL)
                            {
                                gui_free(path);
                                //perror("opendir() failed"); return;
                            }

                            int count = 0;
                            while ((entry = readdir(dir)) != NULL)
                            {
                                if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
                                {
                                    char *path2 = gui_malloc(strlen(entry->d_name) + strlen(folder) + 2);
                                    sprintf(path2, "%s/%s", folder, entry->d_name);
                                    image_array[count++] = gui_get_file_address(path2);
                                }

                            }
                            gui_free(path);
                            closedir(dir);
                        }
                        if (arc)
                        {
                            parent = (void *)gui_seekbar_create_movie_arc(parent, image_array, file_count, x, y,
                                                                          cx, cy, 100, 100, sd, ed);
                            gui_img_set_mode(GUI_TYPE(gui_img_t, GUI_TYPE(gui_seekbar_t, parent)->base.c), IMG_SRC_OVER_MODE);

                        }
                        else
                        {
                            if (vh)
                            {
                                parent = (void *)gui_seekbar_create_movie_v(parent, image_array, file_count, x, y);
                            }
                            else
                            {
                                parent = (void *)gui_seekbar_create_movie_h(parent, image_array, file_count, x, y);
                            }
                        }


                    }

                    parent->name = ptxt;



                }
                break;
            /*case PROGRESSBAR:
            {
                size_t i = 0;
                int16_t x = 0;
                int16_t y = 0;
                int16_t w = 0;
                int16_t h = 0;
                while (true)
                {
                    if (!(p->attr[i]))
                    {
                        break;
                    }
                    //gui_log("p->attr[i]:%s\n", p->attr[i]);
                    if (!strcmp(p->attr[i], "x"))
                    {
                        x = atoi(p->attr[++i]);
                    }
                    else if (!strcmp(p->attr[i], "y"))
                    {
                        y = atoi(p->attr[++i]);
                    }
                    else if (!strcmp(p->attr[i], "w"))
                    {
                        w = atoi(p->attr[++i]);
                    }
                    else if (!strcmp(p->attr[i], "h"))
                    {
                        h = atoi(p->attr[++i]);
                    }
                    i++;
                }
                char *ptxt = get_space_string_head(p->txt);
                parent = gui_progressbar_create(parent, ptxt, x, y, w, h);
            }
            break;
            case SEEKBAR:
            {
                size_t i = 0;
                int16_t x = 0;
                int16_t y = 0;
                int16_t w = 0;
                int16_t h = 0;
                while (true)
                {
                    if (!(p->attr[i]))
                    {
                        break;
                    }
                    //gui_log("p->attr[i]:%s\n", p->attr[i]);
                    if (!strcmp(p->attr[i], "x"))
                    {
                        x = atoi(p->attr[++i]);
                    }
                    else if (!strcmp(p->attr[i], "y"))
                    {
                        y = atoi(p->attr[++i]);
                    }
                    else if (!strcmp(p->attr[i], "w"))
                    {
                        w = atoi(p->attr[++i]);
                    }
                    else if (!strcmp(p->attr[i], "h"))
                    {
                        h = atoi(p->attr[++i]);
                    }
                    i++;
                }
                char *ptxt = get_space_string_head(p->txt);
                parent = gui_seekbar_create(parent, ptxt, x, y, w, h);
            }
            break;*/
            case TAB:
                {
                    size_t i = 0;
                    int16_t x = 0;
                    int16_t y = 0;
                    int16_t w = 0;
                    int16_t h = 0;
                    int16_t idx = 0;
                    int16_t idy = 0;
                    while (true)
                    {
                        if (!(p->attr[i]))
                        {
                            break;
                        }
                        //gui_log("p->attr[i]:%s\n", p->attr[i]);
                        if (!strcmp(p->attr[i], "x"))
                        {
                            x = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "y"))
                        {
                            y = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "w"))
                        {
                            w = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "h"))
                        {
                            h = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "idx"))
                        {
                            idx = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "idy"))
                        {
                            idy = atoi(p->attr[++i]);
                        }
                        i++;
                    }
                    char *ptxt = get_space_string_head(p->txt);
                    //gui_log("x:%d,y:%d,w:%dh:%d,idx:%d,idy:%d\n", x, y, w, h, idx, idy);
                    parent = (void *)gui_tab_create(parent, ptxt, x, y, w, h, idx, idy);
                }
                break;
            case CURTAINVIEW:
                {
                    size_t i = 0;
                    int16_t x = 0;
                    int16_t y = 0;
                    int16_t w = 0;
                    int16_t h = 0;
                    int style = 0;
                    while (true)
                    {
                        if (!(p->attr[i]))
                        {
                            break;
                        }
                        //gui_log("p->attr[i]:%s\n", p->attr[i]);
                        if (!strcmp(p->attr[i], "x"))
                        {
                            x = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "y"))
                        {
                            y = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "w"))
                        {
                            w = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "h"))
                        {
                            h = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "transition"))
                        {
                            char *s = p->attr[++i];
                            if (!strcmp(p->attr[i], "normal"))
                            {
                                style = WIDGET_CLASSIC;
                            }
                            else if (!strcmp(p->attr[i], "fade"))
                            {
                                style = WIDGET_FADE;
                            }
                            else if (!strcmp(p->attr[i], "scale"))
                            {
                                style = WIDGET_SCALE;
                            }
                            else if (!strcmp(p->attr[i], "fadeScale"))
                            {
                                style = WIDGET_SCALE_FADE;
                            }

                        }
                        i++;
                    }
                    char *ptxt = get_space_string_head(p->txt);
                    //gui_log("x:%d,y:%d,w:%dh:%d,scope:%f\n", x, y, w, h);
                    parent = (void *)gui_curtainview_create(parent, ptxt, x, y, w, h);
                    GUI_TYPE(gui_curtainview_t, parent)->style = style;
                }
                break;
            case PAGE:
                {
                    size_t i = 0;
                    int16_t x = 0;
                    int16_t y = 0;
                    int16_t w = 0;
                    int16_t h = 0;
                    while (true)
                    {
                        if (!(p->attr[i]))
                        {
                            break;
                        }
                        //gui_log("p->attr[i]:%s\n", p->attr[i]);
                        if (!strcmp(p->attr[i], "x"))
                        {
                            x = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "y"))
                        {
                            y = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "w"))
                        {
                            w = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "h"))
                        {
                            h = atoi(p->attr[++i]);
                        }
                        i++;
                    }
                    char *ptxt = get_space_string_head(p->txt);
                    //gui_log("x:%d,y:%d,w:%dh:%d,scope:%f\n", x, y, w, h);
                    parent = (void *)gui_page_create(parent, ptxt, x, y, w, h);
                }
                break;
            case GRID:
                {
                    size_t i = 0;
                    int16_t x = 0;
                    int16_t y = 0;
                    int16_t rowNumber = 4;
                    int16_t colNumber = 4;
                    int16_t rowGap = 100;
                    int16_t colGap = 100;
                    while (true)
                    {
                        if (!(p->attr[i]))
                        {
                            break;
                        }
                        //gui_log("p->attr[i]:%s\n", p->attr[i]);
                        if (!strcmp(p->attr[i], "x"))
                        {
                            x = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "y"))
                        {
                            y = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "rowNumber"))
                        {
                            rowNumber = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "colNumber"))
                        {
                            colNumber = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "rowGap"))
                        {
                            rowGap = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "colGap"))
                        {
                            colGap = atoi(p->attr[++i]);
                        }
                        i++;
                    }
                    char *ptxt = get_space_string_head(p->txt);
                    //gui_log("x:%d,y:%d,w:%dh:%d,scope:%f\n", x, y, w, h);
                    parent = (void *)gui_grid_create(parent, x, y, colNumber, rowNumber, colGap, rowGap);
                    parent->name = ptxt;
                }
                break;
            case SCREEN:
                {
                    size_t i = 0;
                    int16_t w = 0;
                    int16_t h = 0;
                    while (true)
                    {
                        if (!(p->attr[i]))
                        {
                            break;
                        }
                        //gui_log("p->attr[i]:%s\n", p->attr[i]);
                        else if (!strcmp(p->attr[i], "w"))
                        {
                            w = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "h"))
                        {
                            h = atoi(p->attr[++i]);
                        }
                        i++;
                    }
                }
                break;
            /*case PAGEBAR:
            {
                size_t i = 0;
                int16_t x = 0;
                int16_t y = 0;
                int16_t w = 0;
                int16_t h = 0;
                BAR_LAYOUT_T bar_layout = UP;
                while (true)
                {
                    if (!(p->attr[i]))
                    {
                        break;
                    }
                    //gui_log("p->attr[i]:%s\n", p->attr[i]);
                    if (!strcmp(p->attr[i], "x"))
                    {
                        x = atoi(p->attr[++i]);
                    }
                    else if (!strcmp(p->attr[i], "y"))
                    {
                        y = atoi(p->attr[++i]);
                    }
                    else if (!strcmp(p->attr[i], "w"))
                    {
                        w = atoi(p->attr[++i]);
                    }
                    else if (!strcmp(p->attr[i], "h"))
                    {
                        h = atoi(p->attr[++i]);
                    }
                    else if (!strcmp(p->attr[i], "orientation"))
                    {
                        const char *o = p->attr[++i];
                        if (!strcmp(o, "MIDDLE"))
                            bar_layout = MIDDLE;
                        else if (!strcmp(o, "UP"))
                        {
                            bar_layout = UP;
                        }
                        else if (!strcmp(o, "DOWN"))
                        {
                            bar_layout = DOWN;
                        }
                        else if (!strcmp(o, "LEFT"))
                        {
                            bar_layout = LEFT;
                        }
                        else if (!strcmp(o, "RIGHT"))
                        {
                            bar_layout = RIGHT;
                        }
                    }
                    i++;
                }
                char *ptxt = get_space_string_head(p->txt);
                //gui_log("x:%d,y:%d,w:%dh:%d,scope:%f\n", x, y, w, h);
                //parent = gui_pagebar_create(parent, ptxt, x, y, w, h, bar_layout);
            }
            break;*/
            case CURTAIN:
                {
                    size_t i = 0;
                    int16_t x = 0;
                    int16_t y = 0;
                    int16_t w = 0;
                    int16_t h = 0;
                    float scope = 1.0f;
                    gui_curtain_enum_t orientation = CURTAIN_MIDDLE;
                    int style = 0;
                    while (true)
                    {
                        if (!(p->attr[i]))
                        {
                            break;
                        }
                        //gui_log("p->attr[i]:%s\n", p->attr[i]);
                        if (!strcmp(p->attr[i], "x"))
                        {
                            x = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "y"))
                        {
                            y = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "w"))
                        {
                            w = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "h"))
                        {
                            h = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "scope"))
                        {
                            scope = atof(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "orientation"))
                        {
                            const char *o = p->attr[++i];
                            if (!strcmp(o, "middle"))
                            {
                                orientation = CURTAIN_MIDDLE;
                            }
                            else if (!strcmp(o, "up"))
                            {
                                orientation = CURTAIN_UP;
                            }
                            else if (!strcmp(o, "down"))
                            {
                                orientation = CURTAIN_DOWN;
                            }
                            else if (!strcmp(o, "left"))
                            {
                                orientation = CURTAIN_LEFT;
                            }
                            else if (!strcmp(o, "right"))
                            {
                                orientation = CURTAIN_RIGHT;
                            }
                        }
                        else if (!strcmp(p->attr[i], "transition"))
                        {
                            char *s = p->attr[++i];
                            if (!strcmp(p->attr[i], "normal"))
                            {
                                style = WIDGET_CLASSIC;
                            }
                            else if (!strcmp(p->attr[i], "fade"))
                            {
                                style = WIDGET_FADE;
                            }
                            else if (!strcmp(p->attr[i], "scale"))
                            {
                                style = WIDGET_SCALE;
                            }
                            else if (!strcmp(p->attr[i], "fadeScale"))
                            {
                                style = WIDGET_SCALE_FADE;
                            }

                        }
                        i++;
                    }
                    char *ptxt = get_space_string_head(p->txt);
                    //gui_log("x:%d,y:%d,w:%dh:%d,orientation:%d\n", x, y, w, h, orientation);
                    parent = (void *)gui_curtain_create(parent, ptxt, x, y, w, h, orientation, scope);
                }
                break;
            /*case ICONLIST:
            {
                size_t i = 0;
                int16_t x = 0;
                int16_t y = 0;
                int16_t w = 320;
                int16_t h = gui_get_screen_height();
                uint32_t space_between_icons = 10;
                uint32_t space_in_icon = 2;
                void *back_img_src = NULL;
                void *back_img_highlight_src = NULL;
                char *font_type = "rtk_font_fs32";
                uint32_t font_color = 0XFFFF;
                uint32_t font_size = 40;
                ICON_LAYOUT_T icon_layout = RIGHT;
                while (true)
                {
                    if (!(p->attr[i]))
                    {
                        break;
                    }
                    //gui_log("p->attr[i]:%s\n", p->attr[i]);
                    if (!strcmp(p->attr[i], "x"))
                    {
                        x = atoi(p->attr[++i]);
                    }
                    else if (!strcmp(p->attr[i], "y"))
                    {
                        y = atoi(p->attr[++i]);
                    }
                    else if (!strcmp(p->attr[i], "w"))
                    {
                        uint32_t w_temp = atoi(p->attr[++i]);
                        if (w_temp)
                        {
                            w = w_temp;
                        }

                    }
                    else if (!strcmp(p->attr[i], "h"))
                    {
                        uint32_t w_temp = atoi(p->attr[++i]);
                        if (w_temp)
                        {
                            h = w_temp;
                        }
                    }
                    else if (!strcmp(p->attr[i], "font"))
                    {
                        font_type = gui_strdup(p->attr[++i]);
                    }
                    else if (!strcmp(p->attr[i], "fontColor"))
                    {
                        font_color = string_rgb888_to_rgb565(p->attr[++i]);
                    }
                    else if (!strcmp(p->attr[i], "fontSize"))
                    {
                        font_size = atoi(p->attr[++i]);
                    }
                    else if (!strcmp(p->attr[i], "spaceBetweenIcons"))
                    {
                        space_between_icons = atoi(p->attr[++i]);
                    }
                    else if (!strcmp(p->attr[i], "spaceInIcon"))
                    {
                        space_in_icon = atoi(p->attr[++i]);
                    }
                    else if (!strcmp(p->attr[i], "backImg"))
                    {
                        back_img_src = gui_strdup(p->attr[++i]);
                    }
                    else if (!strcmp(p->attr[i], "backImgHighlight"))
                    {
                        back_img_highlight_src = gui_strdup(p->attr[++i]);
                    }
                    else if (!strcmp(p->attr[i], "orientation"))
                    {
                        const char *o = p->attr[++i];
                        if (!strcmp(o, "MIDDLE"))
                            icon_layout = CURTAIN_MIDDLE;
                        else if (!strcmp(o, "UP"))
                        {
                            icon_layout = CURTAIN_UP;
                        }
                        else if (!strcmp(o, "DOWN"))
                        {
                            icon_layout = CURTAIN_DOWN;
                        }
                        else if (!strcmp(o, "LEFT"))
                        {
                            icon_layout = CURTAIN_LEFT;
                        }
                        else if (!strcmp(o, "RIGHT"))
                        {
                            icon_layout = CURTAIN_RIGHT;
                        }
                    }
                    i++;
                }
                //gui_log("x:%d,y:%d,w:%dh:%d\n", x, y, w, h);
                char *ptxt = get_space_string_head(p->txt);
                parent = gui_iconlist_create_theme1(parent, ptxt, x, y, w, h, NULL, 0);
                gui_iconlist_theme_1_t *il = (gui_iconlist_theme_1_t *)parent;
                il->font_color = font_color;
                il->font_size = font_size;
                il->font_type = font_type;
                il->icon_gap = space_between_icons;
                il->icon_layout = icon_layout;
                il->gap_in_icon = space_in_icon;
                if (back_img_src)
                {
                    il->back_img_src = back_img_src;
                }
                if (back_img_highlight_src)
                {
                    il->back_img_highlight_src = back_img_highlight_src;
                }


            }
            break;*/
            case ICON:
                {
                    size_t i = 0;
                    int16_t x = 0;
                    int16_t y = 0;
                    int16_t w = 0;
                    int16_t h = 0;
                    char *font_type = "app/system/resource/font/tangyuanti.ttf";
                    char *text = NULL;
                    int text_x = 0;
                    int text_y = 0;
                    uint32_t font_color = 0Xf0f0;
                    uint32_t font_size = 32;
                    int picture_x = 0;
                    int picture_y = 0;
                    int transition = 0;
                    char *picture = NULL;
                    char *hl_picture = NULL;
                    int style = 0;
                    while (true)
                    {
                        if (!(p->attr[i]))
                        {
                            break;
                        }
                        //gui_log("p->attr[i]:%s,\n", p->attr[i]);
                        if (!strcmp(p->attr[i], "x"))
                        {
                            x = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "y"))
                        {
                            y = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "w"))
                        {
                            w = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "h"))
                        {
                            h = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "font"))
                        {
                            font_type = gui_strdup(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "picture"))
                        {
                            picture = gui_strdup(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "highlightPicture"))
                        {
                            hl_picture = gui_strdup(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "fontColor"))
                        {
                            //font_color = string_rgb888_to_rgb565(p->attr[++i]);
                            font_color = string_rgb888(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "fontSize"))
                        {
                            font_size = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "text"))
                        {
                            text = gui_strdup(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "textX"))
                        {
                            text_x = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "textY"))
                        {
                            text_y = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "pictureX"))
                        {
                            picture_x = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "pictureY"))
                        {
                            picture_y = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "mode"))
                        {
                            char *s = p->attr[++i];
                            if (!strcmp(p->attr[i], "normal"))
                            {
                                style = WIDGET_CLASSIC;
                            }
                            else if (!strcmp(p->attr[i], "fade"))
                            {
                                style = WIDGET_FADE;
                            }
                            else if (!strcmp(p->attr[i], "scale"))
                            {
                                style = WIDGET_SCALE;
                            }
                            else if (!strcmp(p->attr[i], "fadeScale"))
                            {
                                style = WIDGET_SCALE_FADE;
                            }

                        }
                        i++;
                    }
                    void *img1;
                    void *img2;
                    {
                        img1 = gui_get_file_address(picture);
                    }
                    {
                        img2 = gui_get_file_address(hl_picture);;
                    }
                    char *ptxt = get_space_string_head(p->txt);
                    //font_size = 32;
                    parent = (void *)gui_button_create(parent, x, y, w, h, img1, img2, text, 0, 0);
                    GUI_TYPE(gui_button_t, parent)->style = style;
                    parent->name = ptxt;
                    gui_button_img_move((void *)parent, picture_x, picture_y);
                    gui_button_text_move((void *)parent, text_x, text_y);

                    gui_color_t color_temporary;
                    color_temporary.color.rgba_full = font_color;
                    GUI_TYPE(gui_button_t, parent)->text->color = color_temporary;
                    if (style)
                    {
                        gui_button_api.onPress((void *)parent, sport_button_press, parent);
                        gui_button_api.onRelease((void *)parent, sport_button_release, parent);
                    }

                    {
                        char *font_type2 = NULL;
                        if (strstr(font_type, ".bin") != NULL)
                        {
                            if (strstr(font_type, ".bin;") != NULL)
                            {
                                font_type2 = "rtk_font_mem";
                                char b[100] = {0};
                                strncpy(b, font_type, strstr(font_type, ".bin;") - font_type + strlen(".bin"));
                                void *addr1 = gui_get_file_address(b);
                                memset(b, 0, sizeof(b));
                                char *a = font_type;
                                strncpy(b, strstr(a, ".bin;") + strlen(".bin;"), strlen(a) - (strstr(a,
                                                                                                     ".bin;") - a + strlen(".bin;")));
                                void *addr2 = gui_get_file_address(b);
                                gui_set_font_mem_resourse(font_size, addr1,  addr2);
                                GUI_TYPE(gui_button_t, parent)->text->font_height = font_size ;
                                GUI_TYPE(gui_button_t, parent)->text->path = 0;
                            }
                            else
                            {
                                font_type2 = "rtk_font_mem";
                                void *addr1 = gui_get_file_address(font_type);
                                gui_font_mem_init(addr1);
                                GUI_TYPE(gui_button_t, parent)->text->font_height = font_size;
                                GUI_TYPE(gui_button_t, parent)->text->path = 0;
                                gui_text_type_set(GUI_TYPE(gui_button_t, parent)->text, addr1);
                                gui_text_mode_set(GUI_TYPE(gui_button_t, parent)->text, LEFT);
                                // t->font_height = fontSize;
                                //t->path = 0;
                            }
                        }
                        else if ((strstr(font_type, ".ttf") != NULL) || (strstr(font_type, ".TTF") != NULL))
                        {
#ifdef __WIN32
                            font_type2 = "rtk_font_stb";
                            GUI_TYPE(gui_button_t, parent)->text->path = gui_get_file_address(font_type);
                            GUI_TYPE(gui_button_t, parent)->text->font_type = GUI_FONT_SOURCE_TTF;
#elif defined RTL8772F
                            font_type2 = "rtk_font_stb";
                            GUI_TYPE(gui_button_t, parent)->text->path = gui_get_file_address(font_type);
                            GUI_TYPE(gui_button_t, parent)->text->font_type = GUI_FONT_SOURCE_TTF;
#else
                            font_type =
                                "app/system/resource/font/gbk_32_32_dot.bin;app/system/resource/font/gbk_unicode_table.bin";
                            {
                                font_type2 = "rtk_font_mem";
                                char b[100] = {0};
                                strncpy(b, font_type, strstr(font_type, ".bin;") - font_type + strlen(".bin"));
                                void *addr1 = gui_get_file_address(b);
                                memset(b, 0, sizeof(b));
                                char *a = font_type;
                                strncpy(b, strstr(a, ".bin;") + strlen(".bin;"), strlen(a) - (strstr(a,
                                                                                                     ".bin;") - a + strlen(".bin;")));
                                void *addr2 = gui_get_file_address(b);
                                gui_set_font_mem_resourse(32, addr1,  addr2);
                                GUI_TYPE(gui_button_t, parent)->text->path = 0;
                                GUI_TYPE(gui_button_t, parent)->text->font_type = GUI_FONT_SOURCE_BMP;
                            }
#endif
                        }
                    }
                    break;
                }
            case RADIO:
                {
                    size_t i = 0;
                    int16_t x = 0;
                    int16_t y = 0;
                    int16_t w = 0;
                    int16_t h = 0;
                    char *font_type = "rtk_font_fs32";
                    char *text = NULL;
                    int text_x = 0;
                    int text_y = 0;
                    uint32_t font_color = 0Xf0f0;
                    uint32_t font_size = 40;
                    int picture_x = 0;
                    int picture_y = 0;
                    int transition = 0;
                    char *picture = NULL;
                    char *hl_picture = NULL;
                    int style = 0;
                    while (true)
                    {
                        if (!(p->attr[i]))
                        {
                            break;
                        }
                        //gui_log("p->attr[i]:%s,\n", p->attr[i]);
                        if (!strcmp(p->attr[i], "x"))
                        {
                            x = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "y"))
                        {
                            y = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "w"))
                        {
                            w = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "h"))
                        {
                            h = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "picture"))
                        {
                            picture = gui_strdup(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "highlightPicture"))
                        {
                            hl_picture = gui_strdup(p->attr[++i]);
                        }

                        i++;
                    }
                    void *img1;
                    void *img2;
                    {
                        img1 = gui_get_file_address(picture);
                    }
                    {
                        img2 = gui_get_file_address(hl_picture);;
                    }
                    //parent = (void *)gui_radio_create(parent, x, y, w, h, img1, img2);
                    //parent->name = get_space_string_head(p->txt);
                    //parent = gui_win_create(parent, "1", 0,0,0,0);
                }
                break;
            case CLICKSWITCH:
                {
                    size_t i = 0;
                    int16_t x = 0;
                    int16_t y = 0;
                    int16_t w = 0;
                    int16_t h = 0;
                    char *picture = NULL;
                    char *hl_picture = NULL;
                    char *pictureHl = NULL;
                    char *hl_pictureHl = NULL;
                    int picture_x = 0;
                    int picture_y = 0;
                    while (true)
                    {
                        if (!(p->attr[i]))
                        {
                            break;
                        }
                        //gui_log("p->attr[i]:%s,\n", p->attr[i]);
                        if (!strcmp(p->attr[i], "x"))
                        {
                            x = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "y"))
                        {
                            y = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "w"))
                        {
                            w = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "h"))
                        {
                            h = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "picture"))
                        {
                            picture = gui_strdup(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "highlightPicture"))
                        {
                            hl_picture = gui_strdup(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "clickedPicture"))
                        {
                            pictureHl = gui_strdup(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "clickedHighlightPicture"))
                        {
                            hl_pictureHl = gui_strdup(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "pictureX"))
                        {
                            picture_x = atoi(p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "pictureY"))
                        {
                            picture_y = atoi(p->attr[++i]);
                        }
                        i++;
                    }
                    void *img1;
                    void *img2;
                    {
                        img1 = gui_get_file_address(picture);
                    }
                    {
                        img2 = gui_get_file_address(hl_picture);
                    }
                    if (!img1)
                    {
                        img1 = img2;
                    }
                    if (!img2)
                    {
                        img2 = img1;
                    }
                    if (!img2 && !img1)
                    {
                        img1 = gui_get_file_address("app/system/resource/switchOff.bin");
                        img2 = gui_get_file_address("app/system/resource/switchOn.bin");
                    }


                    parent = (void *)gui_switch_create(parent, x, y, w, h, img1, img2);
                    parent->name = get_space_string_head(p->txt);
                    GUI_TYPE(gui_switch_t, parent)->on_hl_pic_addr = gui_get_file_address(hl_pictureHl);
                    GUI_TYPE(gui_switch_t, parent)->off_hl_pic_addr = gui_get_file_address(pictureHl);
                    GUI_TYPE(gui_switch_t, parent)->switch_picture->base.x = picture_x;
                    GUI_TYPE(gui_switch_t, parent)->switch_picture->base.y = picture_y;
                }
                break;
            // case MOVIE:
            //     {
            //         size_t i = 0;
            //         int16_t x = 0;
            //         int16_t y = 0;
            //         char *folder = NULL;
            //         uint32_t dur = 1000;
            //         bool reverse = false;
            //         int repeat = -1;
            //         while (true)
            //         {
            //             if (!(p->attr[i]))
            //             {
            //                 break;
            //             }
            //             //gui_log("p->attr[i]:%s,\n", p->attr[i]);
            //             if (!strcmp(p->attr[i], "x"))
            //             {
            //                 x = atoi(p->attr[++i]);
            //             }
            //             else if (!strcmp(p->attr[i], "y"))
            //             {
            //                 y = atoi(p->attr[++i]);
            //             }
            //             else if (!strcmp(p->attr[i], "folder"))
            //             {
            //                 folder = gui_strdup(p->attr[++i]);
            //             }
            //             else if (!strcmp(p->attr[i], "dur"))
            //             {
            //                 char *d = p->attr[++i];
            //                 dur = atoi(d);
            //                 if (strstr(d, "ms") != NULL)
            //                 {
            //                     (void)(dur);
            //                 }
            //                 else if (strstr(d, "h") != NULL)
            //                 {
            //                     dur = dur * 60 * 60 * 1000;
            //                 }
            //                 else if (strstr(d, "s") != NULL)
            //                 {
            //                     dur = dur * 1000;
            //                 }
            //             }
            //             else if (!strcmp(p->attr[i], "repeatCount"))
            //             {

            //                 char *r = p->attr[++i];
            //                 if (strstr(r, "indefinite") != NULL)
            //                 {
            //                     repeat = -1;
            //                 }
            //                 else
            //                 {
            //                     repeat = atoi(r);
            //                 }
            //             }
            //             else if (!strcmp(p->attr[i], "reverse"))
            //             {
            //                 char *r = p->attr[++i];
            //                 if (strstr(r, "false") != NULL)
            //                 {
            //                     reverse = false;
            //                 }
            //                 else if (strstr(r, "true") != NULL)
            //                 {
            //                     reverse = true;
            //                 }
            //             }

            //             i++;
            //         }
            //         int file_count = 0;
            //         {
            //             DIR *dir = 0;
            //             struct dirent *entry;
            //             char *path = gui_malloc(strlen(folder) + strlen(GUI_ROOT_FOLDER) + 1);
            //             sprintf(path, "%s%s", GUI_ROOT_FOLDER, folder);
            //             if ((dir = opendir(path)) == NULL)
            //             {
            //                 gui_free(path);
            //                 //perror("opendir() failed"); return;
            //             }
            //             gui_free(path);
            //             while ((entry = readdir(dir)) != NULL)
            //             {
            //                 file_count++;
            //             }
            //             closedir(dir);
            //         }
            //         void **image_array = gui_malloc(file_count * sizeof(void *));
            //         {
            //             DIR *dir = 0;
            //             struct dirent *entry;
            //             char *path = gui_malloc(strlen(folder) + strlen(GUI_ROOT_FOLDER) + 1);
            //             sprintf(path, "%s%s", GUI_ROOT_FOLDER, folder);
            //             if ((dir = opendir(path)) == NULL)
            //             {
            //                 gui_free(path);
            //                 //perror("opendir() failed"); return;
            //             }

            //             int count = 0;
            //             while ((entry = readdir(dir)) != NULL)
            //             {
            //                 if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
            //                 {
            //                     char *path2 = gui_malloc(strlen(entry->d_name) + strlen(folder) + 2);
            //                     sprintf(path2, "%s/%s", folder, entry->d_name);
            //                     image_array[count++] = gui_get_file_address(path2);
            //                 }

            //             }
            //             gui_free(path);
            //             closedir(dir);
            //         }
            //         uint32_t duration_time_ms = 0;
            //         if (repeat == -1)
            //         {
            //             duration_time_ms = UINT32_MAX;
            //         }
            //         else
            //         {
            //             duration_time_ms = repeat * dur;
            //         }


            //         parent = (void *)gui_dynamic_create_from_mem(parent, "dm", image_array, x, y, file_count - 2,
            //                                                      dur / (file_count - 5),
            //                                                      INT32_MAX);
            //     }
            //     break;
            case JAVASCRIPT:
                {
                    if (!strcmp(p->attr[0], "file"))
                    {
                        js = gui_strdup(p->attr[1]);
                    }

                }
                break;
            case MACRO_ANIMATETRANSFORM:
                {
                    char *type = 0;
                    char *from = 0;
                    char *to = 0;
                    char *dur = 0;
                    char *repeatCount = 0;
                    size_t i = 0;
                    while (true)
                    {
                        //gui_log("p->attr[i]:%x\n",p->attr[i]);
                        if (!(p->attr[i]))
                        {
                            break;
                        }
                        //gui_log("p->attr[i]:%s,\n", p->attr[i]);
                        if (!strcmp(p->attr[i], "type"))
                        {
                            type = (p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "from"))
                        {
                            from = (p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "to"))
                        {
                            to = (p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "dur"))
                        {
                            dur = (p->attr[++i]);
                        }
                        else if (!strcmp(p->attr[i], "repeatCount"))
                        {
                            repeatCount = (p->attr[++i]);
                        }
                        i++;
                    }
                    if (type && from && to && dur && repeatCount)
                    {
                        int dur_num = 0;
                        int from_num[3];
                        int to_num[3];
                        int repeat_num = 0;
                        if (!strcmp(type, "rotate"))
                        {
                            {
                                //from
                                int from_length = strlen(from);
                                int index[4];
                                index[0] = 0;
                                int idcount = 1;
                                for (size_t i = 0; i < from_length + 1; i++)
                                {
                                    if (from[i] == ' ' || from[i] == '\0')
                                    {
                                        index[idcount] = i;
                                        int num_length = index[idcount] - index[idcount - 1] + 1;
                                        char num_char[num_length + 1];
                                        num_char[num_length] = '\0';
                                        memcpy(num_char, from + index[idcount - 1], num_length);
                                        from_num[idcount - 1] = atoi(num_char);
                                        idcount++;
                                    }

                                }
                                gui_log("p->attr[i]:%x\n", p->attr[i]);
                            }
                            {
                                //to
                                int from_length = strlen(to);
                                int index[4];
                                index[0] = 0;

                                int idcount = 1;
                                for (size_t i = 0; i < from_length + 1; i++)
                                {
                                    if (to[i] == ' ' || to[i] == '\0')
                                    {
                                        index[idcount] = i;
                                        int num_length = index[idcount] - index[idcount - 1] + 1;
                                        char num_char[num_length + 1];
                                        num_char[num_length] = '\0';
                                        memcpy(num_char, to + index[idcount - 1], num_length);
                                        to_num[idcount - 1] = atoi(num_char);
                                        idcount++;
                                    }

                                }
                                gui_log("p->attr[i]:%x\n", p->attr[i]);
                            }
                            {
                                //dur
                                int from_length = strlen(dur);
                                int ms = 0;
                                for (size_t i = 0; i < from_length + 1; i++)
                                {
                                    if (dur[i] == 'm')
                                    {
                                        ms = 1;
                                        break;
                                    }

                                }
                                dur_num = atoi(dur);
                                if (!ms)
                                {
                                    dur_num = dur_num * 1000;
                                }

                                gui_log("p->attr[i]:%x\n", p->attr[i]);
                            }
                            {
                                //repeatCount
                                int repeatCount_num = 0;
                                if (!strcmp(repeatCount, "indefinite"))
                                {
                                    repeatCount_num = -1;
                                }
                                else
                                {
                                    repeatCount_num = atoi(repeatCount);
                                }
                                repeat_num = repeatCount_num;

                                gui_log("p->attr[i]:%x\n", p->attr[i]);
                            }
                        }
                        {
                            gui_img_set_animate((gui_img_t *)parent, dur_num, repeat_num, img_rotate_cb, parent);
                        }


                    }
                }
                break;
            default:
                break;
            }
        }
    }
    return parent;


}
void foreach_create(ezxml_t p, gui_obj_t *parent)
{
    ezxml_t i;
    for (i = p; i != NULL; i = i->ordered)
    {
        foreach_create(i->child, widget_create_handle(i, parent));
    }
}
void foreach_scan(ezxml_t p, const char *element, ezxml_t *target)
{
    ezxml_t i;
    for (i = p; i != NULL; i = i->ordered)
    {
        if (strcmp(i->name, "screen") == 0)
        {

            *target = i;
            return;
        }
        foreach_scan(i->child, element, target);
    }
}
void foreach_scan_with_content(ezxml_t p, const char *element, ezxml_t *target, const char *content)
{
    ezxml_t i;
    for (i = p; i != NULL; i = i->ordered)
    {
        if (strncmp(i->name, element, strlen(element)) == 0 &&
            strncmp(i->txt, content, strlen(content)) == 0)
        {

            *target = i;
            return;
        }
        foreach_scan_with_content(i->child, element, target, content);
    }
}
void level_scan(ezxml_t p, char **pic, char **text)
{
    ezxml_t i;
    for (i = p; i != NULL; i = i->ordered)
    {

        if (strcmp(i->name, "app") == 0)
        {

            *text = get_space_string_head(i->txt); gui_log("level:%s\n", *text);
            size_t j = 0;
            while (true)
            {
                if (!(i->attr[j]))
                {
                    break;
                }
                //gui_log("p->attr[i]:%s\n", p->attr[i]);
                if (!strcmp(i->attr[j], "iconPicture"))
                {
                    *pic = gui_strdup(i->attr[++j]);
                    break;
                }

                j++;
            }
            break;
        }
    }
}
void foreach_count(ezxml_t p, size_t *widget_count)
{
    ezxml_t i;
    for (i = p; i != NULL; i = i->ordered)
    {
        (*widget_count)++;
        foreach_count(i->child, widget_count);
    }
}
//#include "gui_server.h"
/*void app_init(void *p)
{
    gui_app_t *app = p;
    gui_list_init(&(app->screen->child_list));
    gui_list_init(&(app->screen->brother_list));

    ezxml_t f1 = ezxml_parse_file(app->xml);
    js = NULL;
    foreach_create(f1, app->screen);
    ezxml_free(f1);
    gui_return_create(app->screen);
    app->inited = true;
    js_run_file_on_server(js, p);

    gui_exec(app->screen);
    while (1)
    {
        gui_thread_mdelay(1000000);
        //handle_launcher_msg();
    }
}*/

void create_tree(gui_app_t *app)
{
    ezxml_t f1 = ezxml_parse_file(app->xml);
    js = NULL;
    foreach_create(f1, &app->screen);
#ifdef ENABLE_RTK_GUI_SCRIPT_AS_A_APP
    extern void js_run_file(const char *file, gui_app_t  *app);
    if (js)
    {
        char *path = gui_malloc(strlen(js) + strlen(GUI_ROOT_FOLDER) + 1);

        sprintf(path, "%s%s", GUI_ROOT_FOLDER, js); gui_log("js_path:%s", path);
        js_run_file(path, app);

        gui_free(path);
    }
#endif
}

const char *get_child_ele_attr(char *xml, char *ele, char *txt, char *chile_ele, char *attr)
{
    ezxml_t f1 = ezxml_parse_file(xml);
    ezxml_t t;
    foreach_scan_with_content(f1, ele, &t, txt);
    ezxml_t title = ezxml_get(t, chile_ele, -1);
    return ezxml_attr(title, attr);
}
ezxml_t get_child_ele(char *xml, char *ele, char *txt, char *chile_ele)
{
    ezxml_t f1 = ezxml_parse_file(xml);
    ezxml_t t;
    foreach_scan_with_content(f1, ele, &t, txt);
    ezxml_t title = ezxml_get(t, chile_ele, -1);
    return title;
}
const char *get_tag_by_widget_type(int type)
{
    if (type == BUTTON)
    {
        type = ICON;
    }

    for (size_t i = 0; i < sizeof(widget) / sizeof(struct widget_create); i++)
    {
        if (widget[i].type == type)
        {
            return widget[i].name;
        }
    }
    return NULL;
}
void get_app(gui_app_t *app, char *pic, char *text)
{
    ezxml_t f1 = ezxml_parse_file(app->xml);


    level_scan(f1, (void *)pic, (void *)text);
    ezxml_free(f1);
}
void level_scan_width_and_hight(ezxml_t p, int *width, int *hight)
{
    ezxml_t i;
    for (i = p; i != NULL; i = i->ordered)
    {
        //gui_log("%s\n",i->child->next->name);
        if (strcmp(i->name, "app") == 0)
        {
            ezxml_t title = ezxml_get(i, "screen", -1);
//gui_log("%s,%s,%s,%s\n",title->name, title->attr[0], title->attr[1],title->attr[2]);

            size_t j = 0;
            while (true)
            {
                if (!(title->attr[j]))
                {
                    break;
                }
                //gui_log("p->attr[i]:%s\n", p->attr[i]);
                if (!strcmp(title->attr[j], "w"))
                {
                    *width = atoi(title->attr[++j]); gui_log("widgt:%d\n", *width);
                }
                else if (!strcmp(title->attr[j], "h"))
                {
                    *hight = atoi(title->attr[++j]); gui_log("hight:%d\n", *hight);
                }

                j++;
            }
            break;
        }
    }
}
void get_app_by_file(char *xml, char *pic, char *text)
{
    ezxml_t f1 = ezxml_parse_file(xml);


    level_scan(f1, (void *)pic, (void *)text);
    ezxml_free(f1);
}
void get_screen_size(char *xml, int *widgt, int *hight)
{
    ezxml_t f1 = ezxml_parse_file(xml);


    level_scan_width_and_hight(f1, widgt, hight);
    ezxml_free(f1);
}
void get_system_screen(int *w, int *h)
{
    char *ezxmlpath = "app/system/launcher.xml";
    char *path = gui_malloc(strlen(ezxmlpath) + strlen(GUI_ROOT_FOLDER) + 1);
    sprintf(path, "%s%s", GUI_ROOT_FOLDER, ezxmlpath);
    ezxml_t f1 =
        ezxml_parse_file(path);
    gui_free(path);
    ezxml_t t;
    foreach_scan(f1, "screen", &t);
    size_t i = 0;

    while (true)
    {
        if (!(t->attr[i]))
        {
            break;
        }
        //gui_log("p->attr[i]:%s\n", p->attr[i]);
        else if (!strcmp(t->attr[i], "w"))
        {
            *w = atoi(t->attr[++i]);
        }
        else if (!strcmp(t->attr[i], "h"))
        {
            *h = atoi(t->attr[++i]);
        }
        i++;
    }
    ezxml_free(f1);
}
/*void album_startup(void *this)
{
    gui_app_startup_base(this);
}
static void album_close(void *this)
{
    gui_app_close_base(this);
}
void album_ctor(void *this)
{
    gui_app_ctor_base(this, "album", app_init, album_startup, album_close);
}*/
#include <gui_api.h>

char *get_app_name(const char *xml)
{
    ezxml_t f1 = ezxml_parse_file(xml);
    GUI_ASSERT(f1);
    if (!strcmp("app", f1->name))
    {
        char *s = gui_strdup(f1->txt);
        ezxml_free(f1);
        return s;
    }
    return NULL;
}
void create_tree_nest(char *xml, void *obj)
{
    ezxml_t f1 = ezxml_parse_file(xml);
    foreach_create(f1, obj);
}
/*static gui_list_t app_list;
static void screen_backfunc(gui_app_t *app)
{
    if (app->installed)
    {
        gui_app_close_base(app);
        memset(app, 0, sizeof(gui_app_t));
        gui_free(app);
    }
    open_xml_app("app/system/launcher.xml");
}
static void set_sreeen_back_function(gui_app_t *app, void *backfunc)
{
    app->screen->w = gui_get_screen_width();
    app->screen->h = gui_get_screen_height();
    gui_cb_set_with_param(app->screen, backfunc, RIGHT_SLIDE_CB, app);
}
static void screen_prepare(gui_obj_t *obj, struct gui_dispdev *dc, gui_rect_t *rect,
                             touch_info_t *tp)
{
    if (obj->callback.link_cb)
    {
        if ((tp->type == TOUCH_RIGHT_SLIDE && (tp->x>0&&tp->x<20))|| (tp->type == TOUCH_LEFT_SLIDE&& (tp->x>(gui_get_screen_width()-20)&&tp->x<gui_get_screen_width())))
        {
                gui_msg_t msg;
                msg.type = GUI_SRV_CB;
                msg.cb = obj->callback.link_cb;
                msg.u.payload = obj->callback.param;
                send_msg_to_gui_server(&msg);
        }

    }

}
void open_xml_app(const char *xml)
{
    gui_app_t *app = gui_malloc(sizeof(gui_app_t));

    memset(app, 0, sizeof(gui_app_t));
    char *app_name = NULL;
    bool startup = false;
    gui_log("ezxml_parse_file:%s\n",xml);
    ezxml_t f1 = ezxml_parse_file(xml);
    GUI_ASSERT(f1);
    if (!strcmp("app", f1->name))
    {
        app_name = get_space_string_head(f1->txt);
        size_t i = 0;
        while (true)
        {
            if (!(f1->attr[i]))
            {
                break;
            }
            //gui_log("p->attr[i]:%s\n", f1->attr[i]);
            //gui_log("p->attr[i+1]:%s\n", f1->attr[i + 1]);
            if (!strcmp(f1->attr[i], "startup"))
            {
                if (!strcmp(f1->attr[++i], "true"))
                {
                    startup = true;
                    break;
                }
            }
            i++;
        }
        ezxml_free(f1);
    }
    //gui_log("startup:%d\n", startup);
    gui_app_ctor_base(app, app_name, app_init, gui_app_startup_base, gui_app_close_base);
    //gui_list_append(&app_list, &(app->list_node));
    app->xml = xml;
    //if (startup)
    {
        gui_app_startup_base(app);
    }

}
*/
