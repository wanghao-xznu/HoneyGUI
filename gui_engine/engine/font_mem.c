#include <string.h>
#include "draw_font.h"
#include "font_mem.h"

MEM_FONT_LIB font_lib_tab[10];
static uint8_t get_fontlib_name(uint8_t *dot_addr, uint8_t font_size)
{
    uint8_t tab_size = sizeof(font_lib_tab) / sizeof(MEM_FONT_LIB);
    if (dot_addr)
    {
        for (size_t i = 0; i < tab_size; i++)
        {
            if (font_lib_tab[i].bmp_addr == dot_addr || font_lib_tab[i].font_file == dot_addr)
            {
                return i;
            }
        }
    }
    else if (font_size)
    {
        for (size_t i = 0; i < tab_size; i++)
        {
            if (font_lib_tab[i].font_size == font_size)
            {
                return i;
            }
        }
    }
    gui_log("Can not match font file, use default \n");
    if (font_lib_tab[0].font_size == 0)
    {
        gui_log("There is no font file \n");
        GUI_ASSERT(font_lib_tab[0].font_size != 0)
    }
    return 0;
}
void gui_font_mem_load(gui_text_t *text, gui_rect_t *rect)
{
    uint8_t font_name = get_fontlib_name(text->path, text->font_height);
    uint32_t dot_offset = (uint32_t)(font_lib_tab[font_name].bmp_addr);
    uint32_t table_offset = (uint32_t)(font_lib_tab[font_name].table_addr);
    uint8_t rendor_mode = font_lib_tab[font_name].rendor_mode;
    uint8_t index_method = font_lib_tab[font_name].font_mode_detail.detail.index_method;
    mem_char_t *chr = gui_malloc(sizeof(mem_char_t) * text->len);
    if (chr == NULL)
    {
        GUI_ASSERT(NULL != NULL);
        return;
    }
    if (chr)
    {
        memset(chr, 0, sizeof(mem_char_t) * text->len);
    }
    text->data = chr;
    uint8_t aliened_font_size;
    aliened_font_size = text->font_height;
    if (text->font_height % 8 != 0)
    {
        aliened_font_size = 8 - text->font_height % 8 + text->font_height;
    }
    uint32_t font_area = aliened_font_size * text->font_height / 8 * rendor_mode + 4;
    uint16_t *p_buf = NULL;
    uint16_t unicode_len = 0;
    switch (text->charset)
    {
    case UTF_8_CHARSET:
        p_buf = gui_malloc(text->len * sizeof(uint16_t));
        if (p_buf == NULL)
        {
            GUI_ASSERT(NULL != NULL);
            return;
        }
        else
        {
            unicode_len = utf8_to_unicode(text->content, text->len, p_buf, text->len);
        }
        break;
    case UTF_16_CHARSET:
        unicode_len = text->len;
        p_buf = (uint16_t *)text->content;
        break;
    default:
        break;
    }
    uint32_t all_char_w = 0;
    uint32_t line_flag = 0;
    switch (index_method)
    {
    case 0: //address
        for (uint32_t i = 0; i < unicode_len; i++)
        {
            chr[i].unicode = p_buf[i];
            chr[i].w = aliened_font_size;
            chr[i].h = text->font_height;
            uint16_t offset = 0;
            if (chr[i].unicode == 0x20 || chr[i].unicode == 0x0D)
            {
                offset = *(uint16_t *)(0x20 * 2 + table_offset);
                if (offset == 0xFFFF) { continue; }
                chr[i].dot_addr = (uint8_t *)(offset * font_area + dot_offset + 4);
                chr[i].char_w = text->font_height / 2;
                chr[i].h = 1;
            }
            else if (chr[i].unicode == 0x0A)
            {
                line_flag ++;
                offset = *(uint16_t *)(0x20 * 2 + table_offset);
                if (offset == 0xFFFF) { continue; }
                chr[i].dot_addr = (uint8_t *)(offset * font_area + dot_offset + 4);
                chr[i].char_w = 0;
                chr[i].h = 0;
            }
            else
            {
                offset = *(uint16_t *)(chr[i].unicode * 2 + table_offset);
                if (offset == 0xFFFF) { continue; }
                chr[i].dot_addr = (uint8_t *)(offset * font_area + dot_offset + 4);
                chr[i].char_w = (int16_t)(*(chr[i].dot_addr - 2));
                chr[i].h = (int16_t)(*(chr[i].dot_addr - 1));
            }
            all_char_w += chr[i].char_w;
        }
        break;
    case 1: //offset
        {
            void *index_area_size_addr = ((uint8_t *)(font_lib_tab[font_name].font_file) + 9);
            uint32_t index_area_size = *(uint32_t *)index_area_size_addr;
            for (uint32_t i = 0; i < unicode_len; i++)
            {
                chr[i].unicode = p_buf[i];
                chr[i].w = aliened_font_size;
                chr[i].h = text->font_height;
                chr[i].char_w = text->font_height / 2;
                uint32_t index = 0;
                if (chr[i].unicode == 0x0A)
                {
                    line_flag ++;
                    chr[i].char_w = 0;
                    chr[i].h = 0;
                }
                else if (chr[i].unicode == 0x20)
                {
                    chr[i].char_w = text->font_height / 2;
                    chr[i].h = 1;
                }
                else
                {
                    for (; index < index_area_size / 2; index ++)
                    {
                        if (chr[i].unicode == *(uint16_t *)(table_offset + index * 2))
                        {
                            chr[i].dot_addr = (uint8_t *)(index * font_area + dot_offset + 4);
                            chr[i].char_w = (int16_t)(*(chr[i].dot_addr - 2));
                            chr[i].h = (int16_t)(*(chr[i].dot_addr - 1));
                            break;
                        }
                    }
                }
                all_char_w += chr[i].char_w;
            }
        }
        break;
    default:
        break;
    }

    text->font_len = unicode_len;
    switch (text->charset)
    {
    case UTF_8_CHARSET:
        gui_free(p_buf);
        break;
    case UTF_16_CHARSET:
        break;
    default:
        break;
    }

    switch (text->mode)
    {
    case LEFT:
    case CENTER:
    case RIGHT:
        {
            if (text->text_offset == 0)
            {
                text->text_offset = (text->base.w - all_char_w) / 2;
            }
            for (uint16_t i = 0; i < text->font_len; i++)
            {
                chr[i].y = rect->y1;
                if (i == 0)
                {
                    chr[i].x = rect->x1 + (text->base.w - all_char_w) / 2 * text->mode;
                }
                else
                {
                    chr[i].x = chr[i - 1].x + chr[i - 1].char_w;
                }
                if ((chr[i].x + chr[i].char_w) > rect->x2)
                {
                    text->font_len = i;
                    break;
                }
            }
            break;
        }
    case MUTI_LEFT:
    case MUTI_CENTER:
    case MUTI_RIGHT:
        {
            gui_text_line_t *line_buf;
            uint32_t line = 0;
            if (text->text_offset == 0)
            {
                text->text_offset = all_char_w / text->base.w + 1 + line_flag;
            }
            line_buf = gui_malloc((text->text_offset + 1) * sizeof(gui_text_line_t));
            memset(line_buf, 0, (text->text_offset + 1) * sizeof(gui_text_line_t));
            for (uint16_t i = 0; i < text->font_len; i++)
            {
                // gui_log("chr[i].y_bound is %d, chr[i].h_bound is %d\n",chr[i].y_bound,chr[i].h_bound);
                if (i == 0)
                {
                    chr[i].x = rect->x1;
                }
                else
                {
                    chr[i].x = chr[i - 1].x + chr[i - 1].char_w;
                }
                if ((chr[i].x + chr[i].char_w) > rect->x2 || chr[i - 1].unicode == 0x0A)
                {
                    line_buf[line].line_char = i - 1;
                    line_buf[line].line_dx = (text->base.w - chr[i].x + rect->x1) / 2 * (text->mode - 3);
                    line++;
                    chr[i].x = rect->x1;
                }
                chr[i].y = rect->y1 + line * chr[i].h;
                if (chr[i].y >= rect->y2)
                {
                    text->font_len = i;
                    break;
                }
            }
            line_buf[line].line_char = text->font_len - 1;
            line_buf[line].line_dx = (text->base.w - chr[text->font_len - 1].x + rect->x1 - chr[text->font_len -
                                      1].char_w) / 2 * (text->mode - 3);
            line = 0;
            for (uint16_t i = 0; i < text->font_len; i++)
            {
                chr[i].x += line_buf[line].line_dx;
                if (i > line_buf[line].line_char)
                {
                    line++;
                }
            }
            gui_free(line_buf);
            break;
        }
    case SCROLL_X:
        {
            if (text->text_offset == 0)
            {
                text->text_offset = all_char_w;
            }
            for (uint16_t i = 0; i < text->font_len; i++)
            {
                chr[i].y = rect->y1;
                if (i == 0)
                {
                    chr[i].x = rect->x1;
                }
                else
                {
                    chr[i].x = chr[i - 1].x + chr[i - 1].char_w;
                }
                if (chr[i].x > rect->xboundright)
                {
                    text->font_len = i;
                    break;
                }
            }
            break;
        }
    case SCROLL_Y:
        {
            uint32_t line = 0;
            for (uint16_t i = 0; i < text->font_len; i++)
            {
                // gui_log("chr[i].y_bound is %d, chr[i].h_bound is %d\n",chr[i].y_bound,chr[i].h_bound);
                if (i == 0)
                {
                    chr[i].x = rect->x1;
                }
                else
                {
                    chr[i].x = chr[i - 1].x + chr[i - 1].char_w;
                }
                if ((chr[i].x + chr[i].char_w) > rect->x2 || chr[i - 1].unicode == 0x0A)
                {
                    line++;
                    chr[i].x = rect->x1;
                }
                chr[i].y = rect->y1 + line * chr[i].h;
                if (text->text_offset != 0)
                {
                    if (chr[i].y >= rect->yboundbottom)
                    {
                        text->font_len = i;
                        break;
                    }
                }
            }
            if (text->text_offset == 0)
            {
                text->text_offset = line * text->font_height;
            }
            break;
        }
    default:
        break;
    }
}

void gui_font_mem_unload(gui_text_t *text)
{
    gui_free(text->data);
    return;
}
// Fast RGB565 pixel blending
// Found in a pull request for the Adafruit framebuffer library. Clever!
// https://github.com/tricorderproject/arducordermini/pull/1/files#diff-d22a481ade4dbb4e41acc4d7c77f683d
gui_inline uint16_t alphaBlendRGB565(uint32_t fg, uint32_t bg, uint8_t alpha)
{
    // Alpha converted from [0..255] to [0..31]
    alpha = (alpha + 4) >> 3;
    // Converts  0000000000000000rrrrrggggggbbbbb
    //     into  00000gggggg00000rrrrr000000bbbbb
    // with mask 00000111111000001111100000011111
    // This is useful because it makes space for a parallel fixed-point multiply
    // bg = (bg | (bg << 16)) & 0b00000111111000001111100000011111;
    // fg = (fg | (fg << 16)) & 0b00000111111000001111100000011111;
    bg = (bg | (bg << 16)) & 0x7e0f81f;
    fg = (fg | (fg << 16)) & 0x7e0f81f;
    // This implements the linear interpolation formula: result = bg * (1.0 - alpha) + fg * alpha
    // This can be factorized into: result = bg + (fg - bg) * alpha
    // alpha is in Q1.5 format, so 0.0 is represented by 0, and 1.0 is represented by 32
    uint32_t result = (fg - bg) * alpha; // parallel fixed-point multiply of all components
    result >>= 5;
    result += bg;
    // result &= 0b00000111111000001111100000011111; // mask out fractional parts
    result &= 0x7e0f81f;
    return (uint16_t)((result >> 16) | result); // contract result
}
gui_inline uint32_t alphaBlendRGBA(gui_color_t fg, uint32_t bg, uint8_t alpha)
{
    uint32_t mix;
    uint8_t back_a = 0xff - alpha;
#if defined(_WIN32)
    mix = 0xff000000;
    mix += ((bg >> 16 & 0xff) * back_a + fg.color.rgba.r * alpha) / 0xff << 16;
    mix += ((bg >>  8 & 0xff) * back_a + fg.color.rgba.g * alpha) / 0xff <<  8;
    mix += ((bg >>  0 & 0xff) * back_a + fg.color.rgba.b * alpha) / 0xff <<  0;
#else
    mix = 0x000000ff;
    mix += ((bg >> 24 & 0xff) * back_a + fg.color.rgba.r * alpha) / 0xff << 24;
    mix += ((bg >> 16 & 0xff) * back_a + fg.color.rgba.g * alpha) / 0xff << 16;
    mix += ((bg >>  8 & 0xff) * back_a + fg.color.rgba.b * alpha) / 0xff <<  8;
#endif
    return mix;
}
gui_inline uint16_t rgba2565(gui_color_t rgba)
{
    uint16_t red = rgba.color.rgba.r * 0x1f / 0xff << 11;
    uint16_t gre = rgba.color.rgba.g * 0x3f / 0xff << 5;
    uint16_t blu = rgba.color.rgba.b * 0x1f / 0xff;
    return red + gre + blu;
}
static void rtk_draw_unicode(int dx, mem_char_t *chr, gui_color_t color, uint8_t rendor_mode,
                             gui_rect_t *rect)
{
    if (chr->dot_addr == NULL)
    {
        return;
    }
    uint8_t *dots = chr->dot_addr;
    gui_dispdev_t *dc = gui_get_dc();
    int font_x = chr->x + dx;
    int font_y = chr->y;
    int font_w = chr->w;
    int font_h = chr->h;
    int x_start = _UI_MAX(_UI_MAX(font_x, rect->xboundleft), 0);
    int x_end;
    if (rect->xboundright != 0)
    {
        x_end = _UI_MIN(_UI_MIN(font_x + chr->char_w, dc->fb_width), rect->xboundright);
    }
    else
    {
        x_end = _UI_MIN(font_x + chr->char_w, dc->fb_width);
    }
    int y_start = _UI_MAX(dc->section.y1, _UI_MAX(font_y, rect->yboundtop));
    int y_end;
    if (rect->yboundbottom != 0)
    {
        y_end = _UI_MIN(_UI_MIN(dc->section.y2, font_y + font_h), rect->yboundbottom);
    }
    else
    {
        y_end = _UI_MIN(dc->section.y2, font_y + font_h);
    }
    if ((x_start >= x_end) || (y_start >= y_end))
    {
        return;
    }
    uint8_t dc_bytes_per_pixel = dc->bit_depth >> 3;
    switch (rendor_mode)
    {
    case 2:
        if (dc_bytes_per_pixel == 2)
        {
            uint16_t *writebuf = (uint16_t *)dc->frame_buf;
            uint16_t color_back;
            for (uint32_t i = y_start; i < y_end; i++)
            {
                int write_off = (i - dc->section.y1) * dc->fb_width ;
                for (uint32_t j = x_start; j < x_end; j++)
                {
                    uint8_t alpha = dots[(i - font_y) * (font_w / 4) + (j - font_x) / 4] >>
                                    ((j - font_x) % 4 * 2);
                    if (alpha != 0)
                    {
                        alpha = alpha & 0x03;
                        alpha *= 85;
                        alpha = color.color.rgba.a * alpha / 0xff;
                        color_back = writebuf[write_off + j];
                        writebuf[write_off + j] = alphaBlendRGB565(rgba2565(color), color_back, alpha);
                    }
                }
            }
        }
        else if (dc_bytes_per_pixel == 3)
        {
            uint8_t *writebuf = (uint8_t *)dc->frame_buf;
            uint8_t color_back[3];
            for (uint32_t i = y_start; i < y_end; i++)
            {
                int write_off = (i - dc->section.y1) * dc->fb_width ;
                for (uint32_t j = x_start; j < x_end; j++)
                {
                    uint8_t alpha = dots[(i - font_y) * (font_w / 4) + (j - font_x) / 4] >>
                                    ((j - font_x) % 4 * 2);
                    if (alpha != 0)
                    {
                        alpha = alpha & 0x03;
                        alpha *= 85;
                        alpha = color.color.rgba.a * alpha / 0xff;
                        color_back[0] = writebuf[write_off * 3 + j * 3 + 2];
                        color_back[1] = writebuf[write_off * 3 + j * 3 + 1];
                        color_back[2] = writebuf[write_off * 3 + j * 3 + 0];
                        writebuf[write_off * 3 + j * 3 + 0] = (color.color.rgba.b * alpha + color_back[2] *
                                                               (0xff - alpha)) / 0xff;
                        writebuf[write_off * 3 + j * 3 + 1] = (color.color.rgba.g * alpha + color_back[1] *
                                                               (0xff - alpha)) / 0xff;
                        writebuf[write_off * 3 + j * 3 + 2] = (color.color.rgba.r * alpha + color_back[0] *
                                                               (0xff - alpha)) / 0xff;
                    }
                }
            }
        }
        else if (dc_bytes_per_pixel == 4)
        {
            uint32_t *writebuf = (uint32_t *)dc->frame_buf;
            uint32_t color_back;
            for (uint32_t i = y_start; i < y_end; i++)
            {
                int write_off = (i - dc->section.y1) * dc->fb_width ;
                for (uint32_t j = x_start; j < x_end; j++)
                {
                    uint8_t alpha = dots[(i - font_y) * (font_w / 4) + (j - font_x) / 4] >>
                                    ((j - font_x) % 4 * 2);
                    if (alpha != 0)
                    {
                        alpha = alpha & 0x03;
                        alpha *= 85;
                        alpha = color.color.rgba.a * alpha / 0xff;
                        color_back = writebuf[write_off + j];
                        writebuf[write_off + j] = alphaBlendRGBA(color, color_back, alpha);
                    }
                }
            }
        }
        break;
    case 4:
        if (dc_bytes_per_pixel == 2)
        {
            uint16_t *writebuf = (uint16_t *)dc->frame_buf;
            uint16_t color_back;
            for (uint32_t i = y_start; i < y_end; i++)
            {
                int write_off = (i - dc->section.y1) * dc->fb_width ;
                for (uint32_t j = x_start; j < x_end; j++)
                {
                    uint8_t alpha = dots[(i - font_y) * (font_w / 2) + (j - font_x) / 2] >>
                                    (4 - (j - font_x) % 2 * 4);
                    if (alpha != 0)
                    {
                        alpha = alpha & 0x0f;
                        alpha *= 17;
                        alpha = color.color.rgba.a * alpha / 0xff;
                        color_back = writebuf[write_off + j];
                        writebuf[write_off + j] = alphaBlendRGB565(rgba2565(color), color_back, alpha);
                    }
                }
            }
        }
        else if (dc_bytes_per_pixel == 3)
        {
            uint8_t *writebuf = (uint8_t *)dc->frame_buf;
            uint8_t color_back[3];
            for (uint32_t i = y_start; i < y_end; i++)
            {
                int write_off = (i - dc->section.y1) * dc->fb_width ;
                for (uint32_t j = x_start; j < x_end; j++)
                {
                    uint8_t alpha = (dots[(i - font_y) * (font_w / 2) + (j - font_x) / 2] >>
                                     (4 - (j - font_x) % 2 * 4));
                    if (alpha != 0)
                    {
                        alpha = alpha & 0x0f;
                        alpha *= 17;
                        alpha = color.color.rgba.a * alpha / 0xff;
                        color_back[0] = writebuf[write_off * 3 + j * 3 + 2];
                        color_back[1] = writebuf[write_off * 3 + j * 3 + 1];
                        color_back[2] = writebuf[write_off * 3 + j * 3 + 0];
                        writebuf[write_off * 3 + j * 3 + 0] = (color.color.rgba.b * alpha + color_back[2] *
                                                               (0xff - alpha)) / 0xff;
                        writebuf[write_off * 3 + j * 3 + 1] = (color.color.rgba.g * alpha + color_back[1] *
                                                               (0xff - alpha)) / 0xff;
                        writebuf[write_off * 3 + j * 3 + 2] = (color.color.rgba.r * alpha + color_back[0] *
                                                               (0xff - alpha)) / 0xff;
                    }
                }
            }
        }
        else if (dc_bytes_per_pixel == 4)
        {
            uint32_t *writebuf = (uint32_t *)dc->frame_buf;
            uint32_t color_back;
            for (uint32_t i = y_start; i < y_end; i++)
            {
                int write_off = (i - dc->section.y1) * dc->fb_width ;
                for (uint32_t j = x_start; j < x_end; j++)
                {
                    uint8_t alpha = (dots[(i - font_y) * (font_w / 2) + (j - font_x) / 2] >>
                                     (4 - (j - font_x) % 2 * 4));
                    if (alpha != 0)
                    {
                        alpha = alpha & 0x0f;
                        alpha *= 17;
                        alpha = color.color.rgba.a * alpha / 0xff;
                        color_back = writebuf[write_off + j];
                        writebuf[write_off + j] = alphaBlendRGBA(color, color_back, alpha);
                    }
                }
            }
        }
        break;
    case 8:
        if (dc_bytes_per_pixel == 2)
        {
            uint16_t *writebuf = (uint16_t *)dc->frame_buf;
            uint16_t color_back;
            uint16_t color_output = rgba2565(color);
            for (uint32_t i = y_start; i < y_end; i++)
            {
                int write_off = (i - dc->section.y1) * dc->fb_width;
                int dots_off = (i - font_y) * font_w - font_x;
                for (uint32_t j = x_start; j < x_end; j++)
                {
                    uint8_t alpha = dots[dots_off + j];
                    if (alpha != 0)
                    {
                        alpha = color.color.rgba.a * alpha / 0xff;
                        color_back = writebuf[write_off + j];
                        writebuf[write_off + j] = alphaBlendRGB565(color_output, color_back, alpha);
                    }
                }
            }
        }
        else if (dc_bytes_per_pixel == 3)
        {
            uint8_t *writebuf = (uint8_t *)dc->frame_buf;
            uint8_t color_back[3];
            for (uint32_t i = y_start; i < y_end; i++)
            {
                int write_off = (i - dc->section.y1) * dc->fb_width ;
                int dots_off = (i - font_y) * font_w - font_x;
                for (uint32_t j = x_start; j < x_end; j++)
                {
                    uint8_t alpha = dots[dots_off + j];
                    if (alpha != 0)
                    {
                        alpha = color.color.rgba.a * alpha / 0xff;
                        color_back[0] = writebuf[write_off * 3 + j * 3 + 2];
                        color_back[1] = writebuf[write_off * 3 + j * 3 + 1];
                        color_back[2] = writebuf[write_off * 3 + j * 3 + 0];
                        writebuf[write_off * 3 + j * 3 + 0] = (color.color.rgba.b * alpha + color_back[2] *
                                                               (0xff - alpha)) / 0xff;
                        writebuf[write_off * 3 + j * 3 + 1] = (color.color.rgba.g * alpha + color_back[1] *
                                                               (0xff - alpha)) / 0xff;
                        writebuf[write_off * 3 + j * 3 + 2] = (color.color.rgba.r * alpha + color_back[0] *
                                                               (0xff - alpha)) / 0xff;
                    }
                }
            }
        }
        else if (dc_bytes_per_pixel == 4)
        {
            uint32_t *writebuf = (uint32_t *)dc->frame_buf;
            uint32_t color_back;
            for (uint32_t i = y_start; i < y_end; i++)
            {
                int write_off = (i - dc->section.y1) * dc->fb_width ;
                int dots_off = (i - font_y) * font_w - font_x;
                for (uint32_t j = x_start; j < x_end; j++)
                {
                    uint8_t alpha = dots[dots_off + j];
                    if (alpha != 0)
                    {
                        color_back = writebuf[write_off + j];
                        alpha = color.color.rgba.a * alpha / 0xff;
                        writebuf[write_off + j] = alphaBlendRGBA(color, color_back, alpha);
                    }
                }
            }
        }
        break;
    case 1:
#if 1 //0 - Stable slow processing , 1 - Fast processing in tests
        if (dc_bytes_per_pixel == 2)
        {
            uint16_t *writebuf = (uint16_t *)dc->frame_buf;
            uint16_t color_output = rgba2565(color);

            uint32_t *p_dot32 = (uint32_t *)(dots + (y_start - font_y) * (font_w / 8));
            int write_off = (y_start - dc->section.y1) * dc->fb_width;
            for (uint32_t i = y_start; i < y_end; i++, write_off += dc->fb_width)
            {
                uint8_t *temp_p = (uint8_t *)p_dot32;
                uint8_t temp;
                p_dot32 += font_w / 32;
                if (x_start > font_x)
                {
                    temp_p += (x_start - font_x) / 8;
                    temp = *temp_p;
                    uint8_t skip_bits = (x_start - font_x) % 8;
                    temp &= ~(((1 << skip_bits) - 1) << (8 - skip_bits));
                }
                else
                {
                    temp = *temp_p;
                }

                uint32_t scan_end = font_x + font_w - (font_x + font_w - x_end) / 8 * 8;

                for (uint32_t j = x_start; j < scan_end;)
                {
                    if (temp)
                    {
                        uint16_t offset = write_off + font_x + (j - font_x) / 8 * 8;

                        if (temp & 0x0F)
                        {
                            if (temp & 0x01)
                            {
                                writebuf[offset + 7] = color_output;
                            }
                            if (temp & 0x02)
                            {
                                writebuf[offset + 6] = color_output;
                            }
                            if (temp & 0x04)
                            {
                                writebuf[offset + 5] = color_output;
                            }
                            if (temp & 0x08)
                            {
                                writebuf[offset + 4] = color_output;
                            }
                        }
                        if (temp & 0xF0)
                        {
                            if (temp & 0x20)
                            {
                                writebuf[offset + 2] = color_output;
                            }
                            if (temp & 0x10)
                            {
                                writebuf[offset + 3] = color_output;
                            }
                            if (temp & 0x40)
                            {
                                writebuf[offset + 1] = color_output;
                            }
                            if (temp & 0x80)
                            {
                                writebuf[offset] = color_output;
                            }
                        }
                    }
                    temp_p++;
                    temp = *temp_p;
                    j += 8;
                }
            }
        }
#else
        if (dc_bytes_per_pixel == 2)
        {
            uint16_t *writebuf = (uint16_t *)dc->frame_buf;
            uint16_t color_output = rgba2565(color);
            for (uint32_t i = y_start; i < y_end; i++)
            {
                int write_off = (i - dc->section.y1) * dc->fb_width;
                int dots_off = (i - font_y) * (font_w / 8);
                for (uint32_t j = x_start; j < x_end; j++)
                {
                    if ((dots[dots_off + (j - font_x) / 8] >> (7 - (j - font_x) % 8)) & 0x01)
                    {
                        writebuf[write_off + j] = color_output;
                    }
                }
            }
        }
#endif
        else if (dc_bytes_per_pixel == 3)
        {
            uint8_t *writebuf = (uint8_t *)dc->frame_buf;
            uint8_t color_back[3];
            uint8_t alpha = color.color.rgba.a;
            for (uint32_t i = y_start; i < y_end; i++)
            {
                int write_off = (i - dc->section.y1) * dc->fb_width ;
                int dots_off = (i - font_y) * (font_w / 8);
                for (uint32_t j = x_start; j < x_end; j++)
                {
                    if ((dots[dots_off + (j - font_x) / 8] >> (7 - (j - font_x) % 8)) & 0x01)
                    {
                        color_back[0] = writebuf[write_off * 3 + j * 3 + 2];
                        color_back[1] = writebuf[write_off * 3 + j * 3 + 1];
                        color_back[2] = writebuf[write_off * 3 + j * 3 + 0];
                        writebuf[write_off * 3 + j * 3 + 0] = (color.color.rgba.b * alpha + color_back[2] *
                                                               (0xff - alpha)) / 0xff;
                        writebuf[write_off * 3 + j * 3 + 1] = (color.color.rgba.g * alpha + color_back[1] *
                                                               (0xff - alpha)) / 0xff;
                        writebuf[write_off * 3 + j * 3 + 2] = (color.color.rgba.r * alpha + color_back[0] *
                                                               (0xff - alpha)) / 0xff;
                    }
                }
            }
        }
        else if (dc_bytes_per_pixel == 4)
        {
            uint32_t *writebuf = (uint32_t *)dc->frame_buf;
            uint32_t color_back;
            for (uint32_t i = y_start; i < y_end; i++)
            {
                int write_off = (i - dc->section.y1) * dc->fb_width ;
                int dots_off = (i - font_y) * (font_w / 8);
                for (uint32_t j = x_start; j < x_end; j++)
                {
                    if ((dots[dots_off + (j - font_x) / 8] >> (7 - (j - font_x) % 8)) & 0x01)
                    {
                        color_back = writebuf[write_off + j];
                        writebuf[write_off + j] = alphaBlendRGBA(color, color_back, color.color.rgba.a);
                    }
                }
            }
        }
        break;
    default:
        break;
    }
}
void gui_font_mem_draw(gui_text_t *text, gui_rect_t *rect)
{
    mem_char_t *chr = text->data;
    uint8_t font_name = get_fontlib_name(text->path, text->font_height);
    uint8_t rendor_mode = font_lib_tab[font_name].rendor_mode;
    for (uint16_t i = 0; i < text->font_len; i++)
    {
        rtk_draw_unicode(0, chr + i, text->color, rendor_mode, rect);
    }
}

void gui_set_font_mem_resourse(unsigned char font_size, void *font_bitmap_addr,
                               void *font_table_addr)
{
    int i = 0;
    for (; i < sizeof(font_lib_tab) / sizeof(MEM_FONT_LIB); i++)
    {
        if (font_lib_tab[i].bmp_addr == 0 && font_lib_tab[i].font_size == 0 &&
            font_lib_tab[i].table_addr == 0)
        {
            break;
        }
        if (font_lib_tab[i].bmp_addr == font_bitmap_addr)
        {
            break;
        }
    }
    if (i >= sizeof(font_lib_tab) / sizeof(MEM_FONT_LIB))
    {
        return;
    }
    font_lib_tab[i].font_file = font_bitmap_addr;
    font_lib_tab[i].bmp_addr = font_bitmap_addr;
    font_lib_tab[i].font_size = font_size;
    font_lib_tab[i].table_addr = font_table_addr;
    font_lib_tab[i].rendor_mode = 1;
    font_lib_tab[i].font_mode_detail.value = 0;
}

void gui_font_mem_init(uint8_t *font_bin_addr)
{
    int i = 0;
    if (!font_bin_addr)
    {
        return;
    }
    uint8_t font_file_flag = * (uint8_t *)(font_bin_addr + 1);
    if (font_file_flag == 0)
    {
        return;
    }
    for (; i < sizeof(font_lib_tab) / sizeof(MEM_FONT_LIB); i++)
    {
        if (font_lib_tab[i].bmp_addr == 0)
        {
            break;
        }
        if (font_lib_tab[i].font_file == font_bin_addr)
        {
            break;
        }
    }
    if (i >= sizeof(font_lib_tab) / sizeof(MEM_FONT_LIB))
    {
        return;
    }

    uint8_t head_length = * (uint8_t *)(font_bin_addr);
    void *index_area_size_addr = (uint8_t *)(font_bin_addr + 9);
    uint32_t index_area_size = *(uint32_t *)index_area_size_addr;

    font_lib_tab[i].font_file = font_bin_addr;
    font_lib_tab[i].font_size = * (uint8_t *)(font_bin_addr + 6);
    font_lib_tab[i].rendor_mode = * (uint8_t *)(font_bin_addr + 7);
    font_lib_tab[i].table_addr = (uint8_t *)(font_bin_addr + head_length);
    font_lib_tab[i].bmp_addr = (uint8_t *)(font_bin_addr + index_area_size + head_length);
    font_lib_tab[i].font_mode_detail.value =  * (uint8_t *)(font_bin_addr + 8);
}

uint32_t gui_get_mem_utf8_char_width(void *content, void *font_bin_addr)
{
    return gui_get_mem_char_width(content, font_bin_addr, UTF_8_CHARSET);
}

uint32_t gui_get_mem_char_width(void *content, void *font_bin_addr, TEXT_CHARSET charset)
{
    GUI_FONT_HEAD *font = (GUI_FONT_HEAD *)font_bin_addr;
    uint32_t string_len = strlen(content);
    uint32_t table_offset = (uint32_t)((uint8_t *)font_bin_addr + font->head_length);
    uint32_t dot_offset = table_offset + font->index_area_size;

    uint8_t aliened_font_size = font->font_size;
    if (font->font_size % 8 != 0)
    {
        aliened_font_size = 8 - font->font_size % 8 + font->font_size;
    }
    uint32_t font_area = aliened_font_size * font->font_size / 8 * font->rendor_mode + 4;

    uint16_t *unicode_buffer = NULL;
    uint16_t unicode_len = 0;
    switch (charset)
    {
    case UTF_8_CHARSET:
        unicode_buffer = gui_malloc(string_len * sizeof(uint16_t));
        if (unicode_buffer == NULL)
        {
            GUI_ASSERT(NULL != NULL);
            return 0;
        }
        else
        {
            unicode_len = utf8_to_unicode(content, string_len, unicode_buffer, string_len);
        }
        break;
    case UTF_16_CHARSET:
        unicode_len = string_len;
        unicode_buffer = (uint16_t *)content;
        break;
    default:
        break;
    }
    uint32_t all_char_w = 0;
    uint32_t line_flag = 0;
    switch (font->font_mode_detail.detail.index_method)
    {
    case 0: //address
        for (uint32_t i = 0; i < unicode_len; i++)
        {
            uint16_t offset = 0;
            uint16_t char_w = 0;
            if (unicode_buffer[i] == 0x20 || unicode_buffer[i] == 0x0D)
            {
                char_w = font->font_size / 2;
            }
            else if (unicode_buffer[i] == 0x0A)
            {
                line_flag ++;
                char_w = 0;
            }
            else
            {
                offset = *(uint16_t *)(unicode_buffer[i] * 2 + table_offset);
                if (offset == 0xFFFF) { continue; }
                uint8_t *dot_addr = (uint8_t *)(offset * font_area + dot_offset + 4);
                char_w = (int16_t)(*(dot_addr - 2));
            }
            all_char_w += char_w;
        }
        break;
    case 1: //offset
        for (uint32_t i = 0; i < unicode_len; i++)
        {
            uint16_t char_w = 0;
            uint32_t index = 0;
            if (unicode_buffer[i] == 0x0A)
            {
                line_flag ++;
                char_w = 0;
            }
            else if (unicode_buffer[i] == 0x20)
            {
                char_w = font->font_size / 2;
            }
            else
            {
                for (; index < font->index_area_size / 2; index ++)
                {
                    if (unicode_buffer[i] == *(uint16_t *)(table_offset + index * 2))
                    {
                        uint8_t *dot_addr = (uint8_t *)(index * font_area + dot_offset + 4);
                        char_w = (int16_t)(*(dot_addr - 2));
                    }
                }
            }
            all_char_w += char_w;
        }
        break;
    default:
        break;
    }
    return all_char_w;
}