#include "draw_font.h"
#include "font_stb.h"
#define STB_TRUETYPE_IMPLEMENTATION
#define STB_REDUCE_MEMORY
// #define ALLOW_UNALIGNED_TRUETYPE
#include "stb_truetype.h"
#include "acc_engine.h"

static stbtt_fontinfo font;
void gui_font_stb_load(gui_text_t *text, gui_rect_t *rect)
{
    if (text->path)
    {
        gui_font_stb_init(text->path);
    }
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
    text->font_len = unicode_len;
    float all_char_w = 0;
    float scale = 0;
    int ch = 0, advance = 0, lsb = 0;
    int line_flag = 0;
    scale = stbtt_ScaleForPixelHeight(&font, text->font_height);
    while (ch < unicode_len)
    {
        stbtt_GetCodepointHMetrics(&font, p_buf[ch++], &advance, &lsb);
        all_char_w += advance * scale;
    }
    if (text->text_offset == 0)
    {
        switch (text->mode)
        {
        case LEFT:
        case CENTER:
        case RIGHT:
            text->text_offset = _UI_MAX((text->base.w - all_char_w) / 2, 0);
            break;
        case MUTI_LEFT:
        case MUTI_CENTER:
        case MUTI_RIGHT:
            text->text_offset = all_char_w / text->base.w + 1 + line_flag;
            break;
        case SCROLL_X:
            text->text_offset = (int)(all_char_w + 1);
            break;
        case SCROLL_Y:
            text->text_offset = (all_char_w / text->base.w + 1) * text->font_height;
        default:
            break;
        }
    }
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
}

void gui_font_stb_unload(gui_text_t *text)
{
    gui_free(((FONT_STB_SCREEN *)text->data)->buf);
    gui_free((FONT_STB_SCREEN *)text->data);
    text->data = NULL;
}
#ifndef RTK_GUI_FONT_ENABLE_TTF_SVG
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

static bool creat_stb_screen(gui_text_t *text, gui_rect_t *rect, FONT_STB_SCREEN *screen)
{
    if (text->mode == LEFT || text->mode == CENTER || text->mode == RIGHT)
    {
        screen->width = _UI_MIN(rect->x2 - rect->x1, _UI_MAX(text->base.w - text->text_offset * 2, 0));
        screen->height = _UI_MIN(text->font_height, rect->y2 - rect->y1);
    }
    else if (text->mode == MUTI_LEFT || text->mode == MUTI_CENTER || text->mode == MUTI_RIGHT)
    {
        screen->width =  rect->x2 - rect->x1;
        screen->height = rect->y2 - rect->y1;
    }
    else if (text->mode == SCROLL_X)
    {
        screen->width =  rect->x2 - rect->x1;
        screen->height = rect->y2 - rect->y1;
    }
    else if (text->mode == SCROLL_Y)
    {
        screen->width =  rect->x2 - rect->x1;
        screen->height = rect->y2 - rect->y1;
    }
    if (screen->width == 0 || screen->height == 0)
    {
        return false;
    }
    GUI_ASSERT(screen->width > 0);
    GUI_ASSERT(screen->height > 0);
    uint8_t *buffer = gui_malloc((screen->width) * (screen->height) * sizeof(uint8_t));
    if (buffer == NULL)
    {
        GUI_ASSERT(NULL != NULL);
        return false;
    }
    memset(buffer, 0, (screen->width) * (screen->height) * sizeof(uint8_t));
    // gui_log("creat_stb_screen width %d , height %d \n", screen->width, screen->height);
    screen->buf = buffer;
    return true;
}
static void font_stb_draw_bitmap(gui_text_t *text, FONT_STB_SCREEN *stb_screen,
                                 gui_rect_t *rect)
{
    gui_dispdev_t *dc = gui_get_dc();
    uint8_t *dots = stb_screen->buf;
    uint8_t dc_bytes_per_pixel = dc->bit_depth >> 3;
    int offset = 0;
    switch (text->mode)
    {
    case LEFT:
    case CENTER:
    case RIGHT:
        offset = text->text_offset * text->mode;
        break;
    case MUTI_LEFT:
    case MUTI_CENTER:
    case MUTI_RIGHT:
        break;
    case SCROLL_X:
        break;
    case SCROLL_Y:
    default:
        break;
    }

    int font_x = rect->x1 + offset;
    int font_y = rect->y1;
    int font_w = stb_screen->width;
    int font_h = stb_screen->height;
    int x_start = _UI_MAX(_UI_MAX(font_x, rect->xboundleft), 0);
    int x_end;
    if (rect->xboundright != 0)
    {
        x_end = _UI_MIN(_UI_MIN(font_x + font_w, dc->fb_width), rect->xboundright);
    }
    else
    {
        x_end = _UI_MIN(font_x + font_w, dc->fb_width);
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

    if (dc_bytes_per_pixel == 4)
    {
        uint32_t *writebuf = (uint32_t *)dc->frame_buf;
        uint32_t color_back;
        for (uint32_t i = y_start; i < y_end; i++)
        {
            int write_off = (i - dc->section.y1) * dc->fb_width ;
            for (uint32_t j = x_start; j < x_end; j++)
            {
                uint8_t alpha = dots[(i - font_y) * font_w + (j - font_x)];
                if (alpha != 0)
                {
                    color_back = writebuf[write_off + j];
                    alpha = text->color.color.rgba.a * alpha / 0xff;
                    writebuf[write_off + j] = alphaBlendRGBA(text->color, color_back, alpha);
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
                uint8_t alpha = dots[(i - font_y) * font_w + (j - font_x)];
                if (alpha != 0)
                {
                    alpha = text->color.color.rgba.a * alpha / 0xff;
                    color_back[0] = writebuf[write_off * 3 + j * 3 + 2];
                    color_back[1] = writebuf[write_off * 3 + j * 3 + 1];
                    color_back[2] = writebuf[write_off * 3 + j * 3 + 0];
                    writebuf[write_off * 3 + j * 3 + 0] = (text->color.color.rgba.b * alpha + color_back[2] *
                                                           (0xff - alpha)) / 0xff;
                    writebuf[write_off * 3 + j * 3 + 1] = (text->color.color.rgba.g * alpha + color_back[1] *
                                                           (0xff - alpha)) / 0xff;
                    writebuf[write_off * 3 + j * 3 + 2] = (text->color.color.rgba.r * alpha + color_back[0] *
                                                           (0xff - alpha)) / 0xff;
                }
            }
        }
    }
    else if (dc_bytes_per_pixel == 2)
    {
        uint16_t *writebuf = (uint16_t *)dc->frame_buf;
        uint16_t color_back;
        for (uint32_t i = y_start; i < y_end; i++)
        {
            int write_off = (i - dc->section.y1) * dc->fb_width ;
            for (uint32_t j = x_start; j < x_end; j++)
            {
                uint8_t alpha = dots[(i - font_y) * font_w + (j - font_x)];
                if (alpha != 0)
                {
                    alpha = text->color.color.rgba.a * alpha / 0xff;
                    color_back = writebuf[write_off + j];
                    writebuf[write_off + j] = alphaBlendRGB565(rgba2565(text->color), color_back, alpha);
                }
            }
        }
    }
}
#endif // !RTK_GUI_FONT_ENABLE_TTF_SVG
#include "nanosvg.h"
//#include "nanosvgrast.h"
#ifdef RTK_GUI_FONT_ENABLE_TTF_SVG
static void stb_add_path(NSVGshape *shape, stbtt_vertex *stbVertex, int line_count)
{
    NSVGpath *path = NULL;
    path = gui_malloc(sizeof(NSVGpath));
    if (path == NULL)
    {
        GUI_ASSERT(NULL != NULL);
    }
    memset(path, 0, sizeof(NSVGpath));

    path->closed = 1;
    path->npts = (line_count) * 3 + 1;

    path->pts = (float *)gui_malloc(path->npts * 2 * sizeof(float));
    memset(path->pts, 0, path->npts * 2 * sizeof(float));
    if (path->pts == NULL)
    {
        GUI_ASSERT(NULL != NULL);
    }

    if (shape->paths)
    {
        path->next = shape->paths;
    }
    shape->paths = path;
}
#endif
void gui_font_stb_draw(gui_text_t *text, gui_rect_t *rect)
{
    int ascent = 0, descent = 0, lineGap = 0, ch = 0, line_num = 0;
    float scale = 0, xpos = 0, ypos = 0, baseline = 0;
    scale = stbtt_ScaleForPixelHeight(&font, text->font_height);
    stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);
    baseline = ascent * scale;
    // baseline = 900 * scale;
    gui_dispdev_t *dc = gui_get_dc();
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

#ifdef RTK_GUI_FONT_ENABLE_TTF_SVG
    while (ch < unicode_len)
    {
        int glyph_index = stbtt_FindGlyphIndex(&font, p_buf[ch]);
        int advance, lsb, x0, y0, x1, y1;
        stbtt_GetGlyphHMetrics(&font, glyph_index, &advance, &lsb);
        // gui_log("unicode : %c\n", p_buf[ch]);
        if (text->mode == LEFT || text->mode == CENTER || text->mode == RIGHT)
        {
            if (xpos + advance * scale > text->base.w)
            {
                break;
            }
        }
        else if (text->mode == MUTI_LEFT || text->mode == MUTI_CENTER || text->mode == MUTI_RIGHT)
        {
            if (xpos + advance * scale > text->base.w)
            {
                line_num ++;
                xpos = 0;
            }
            if ((line_num + 1) *  scale * (ascent - descent + lineGap) > text->base.h)
            {
                break;
            }
        }
        else if (text->mode == SCROLL_X)
        {
            if (rect->x1 + xpos + advance * scale >= text->base.dx + text->base.w)
            {
                break;
            }
            if (rect->x1 + xpos < text->base.dx)
            {
                xpos += (advance * scale);
                ch++;
                continue;
            }
        }
        else if (text->mode == SCROLL_Y)
        {
            if (xpos + advance * scale > text->base.w)
            {
                line_num ++;
                xpos = 0;
            }
        }
        ypos = line_num * scale * (ascent - descent + lineGap);
        stbtt_GetGlyphBox(&font, glyph_index, &x0, &y0, &x1, &y1);
        // stbtt_GetGlyphBitmapBoxSubpixel(&font, glyph_index, scale, scale, 0, 0, &x0, &y0, &x1, &y1);
        stbtt_GetGlyphBitmapBox(&font, glyph_index, scale, scale, &x0, &y0, &x1, &y1);
        stbtt_vertex *stbVertex = NULL;

        int verCount = 0;
        verCount = stbtt_GetGlyphShape(&font, glyph_index, &stbVertex);

//start fill svg
        NSVGimage *font_svg = gui_malloc(sizeof(NSVGimage));
        if (font_svg == NULL)
        {
            GUI_ASSERT(NULL != NULL);
        }
        memset(font_svg, 0, sizeof(NSVGimage));
        font_svg->height = baseline - y0;
        font_svg->width = x1;

        NSVGshape *font_shape = gui_malloc(sizeof(NSVGshape));
        if (font_shape == NULL)
        {
            GUI_ASSERT(NULL != NULL);
        }
        memset(font_shape, 0, sizeof(NSVGshape));
        font_shape->opacity = 1;
        font_shape->fill.type = 1;
        font_shape->fill.d.color = text->color.color.rgba_full;
        font_shape->flags = 1;
        // font_shape->bounds[0] = 0;
        // font_shape->bounds[1] = 0;
        // font_shape->bounds[2] = 20;
        // font_shape->bounds[3] = 20;
        font_svg->shapes = font_shape;

        int path_count = 1, path_npts = 1;
        for (int i = 0; i < verCount; i++)
        {
            switch (stbVertex[i].type)
            {
            case 1:
                path_count++;
                break;
            case 2:
            case 3:
                path_npts += 3;
                break;
            default:
                break;
            }
        }

        int *path_num = gui_malloc(path_count * sizeof(int));
        int cur_path = 0;
        for (int i = 0; i < verCount; i++)
        {
            if (stbVertex[i].type == 1)
            {
                path_num[cur_path++] = i;
            }
        }
        path_num[cur_path] = verCount - 1;

        int  pts_count = 0;
        float px, py, cx, cy;
        for (int i = 0; i < cur_path; i++)
        {
            for (int j = path_num[i]; j <= path_num[i + 1]; j++)
            {
                px = (float)stbVertex[j].x * scale;
                py = baseline - ((float)stbVertex[j].y * scale);
                switch (stbVertex[j].type)
                {
                case 1:
                    stb_add_path(font_shape, &stbVertex[path_num[i]], path_num[i + 1] - path_num[i]);
                    pts_count = 0;
                    font_shape->paths->pts[pts_count++] = px;
                    font_shape->paths->pts[pts_count++] = py;
                    break;
                case 2:
                    font_shape->paths->pts[pts_count++] = px;
                    font_shape->paths->pts[pts_count++] = py;
                    font_shape->paths->pts[pts_count++] = px;
                    font_shape->paths->pts[pts_count++] = py;
                    font_shape->paths->pts[pts_count++] = px;
                    font_shape->paths->pts[pts_count++] = py;
                    break;
                case 3:
                    cx = (float)stbVertex[j].cx * scale;
                    cy = baseline - ((float)stbVertex[j].cy * scale);
                    font_shape->paths->pts[pts_count++] = cx;
                    font_shape->paths->pts[pts_count++] = cy;
                    font_shape->paths->pts[pts_count++] = cx;
                    font_shape->paths->pts[pts_count++] = cy;
                    font_shape->paths->pts[pts_count++] = px;
                    font_shape->paths->pts[pts_count++] = py;
                    break;
                default:
                    break;
                }
            }
        }
        gui_free(path_num);

//svg fill ok
        // gui_log("fill ok stb \n");
        int offset = 0;
        switch (text->mode)
        {
        case LEFT:
        case CENTER:
        case RIGHT:
            offset = text->text_offset * text->mode;
            break;
        case MUTI_LEFT:
        case MUTI_CENTER:
        case MUTI_RIGHT:
            break;
        case SCROLL_X:
            break;
        case SCROLL_Y:
        default:
            break;
        }
        // gui_log("draw %c , ch : %d , xpos : %f , y pos : %f \n",p_buf[ch],ch,xpos,ypos);
        gui_get_acc()->draw_svg(font_svg, 1, dc, rect->x1 + xpos + offset, rect->y1 + ypos, 1, 0, 0, 0);
        extern void nsvgDelete(NSVGimage * image);
        nsvgDelete(font_svg);
        stbtt_FreeShape(&font, stbVertex);

        xpos += (advance * scale);
        ++ch;
    }
#else
    if (text->data == NULL)
    {
        FONT_STB_SCREEN *stb_screen = gui_malloc(sizeof(FONT_STB_SCREEN));
        bool flag = creat_stb_screen(text, rect, stb_screen);
        text->data = stb_screen;
        if (flag == false)
        {
            return;
        }
        while (ch < unicode_len)
        {
            int glyph_index = stbtt_FindGlyphIndex(&font, p_buf[ch]);
            int advance, lsb, x0, y0, x1, y1;
            float x_shift = xpos - (float) floor(xpos);
            stbtt_GetGlyphHMetrics(&font, glyph_index, &advance, &lsb);
            stbtt_GetGlyphBitmapBoxSubpixel(&font, glyph_index, scale, scale, x_shift, 0, &x0, &y0, &x1, &y1);
            if (text->mode == LEFT || text->mode == CENTER || text->mode == RIGHT)
            {
                if (xpos + advance * scale > stb_screen->width)
                {
                    break;
                }
            }
            else if (text->mode == MUTI_LEFT || text->mode == MUTI_CENTER || text->mode == MUTI_RIGHT)
            {
                if (xpos + advance * scale > stb_screen->width)
                {
                    line_num ++;
                    xpos = 0;
                }
                if ((line_num + 1) *  scale * (ascent - descent + lineGap) > stb_screen->height)
                {
                    break;
                }
            }
            else if (text->mode == SCROLL_X)
            {
                xpos += (advance * scale);
                ch++;
                continue;
            }
            else if (text->mode == SCROLL_Y)
            {
                if (xpos + advance * scale > stb_screen->width)
                {
                    line_num ++;
                    xpos = 0;
                }
            }
            // gui_log("unicode %x \n",p_buf[ch]);
            ypos = line_num * scale * (ascent - descent + lineGap);
            /*If a non-standard ttf font is used, the header information of the stb_screen memory may be lost because the drawing is out of bounds, triggering a free problem.*/
            stbtt_MakeCodepointBitmapSubpixel(&font,
                                              &stb_screen->buf[((int)baseline + y0 + (int)ypos)*stb_screen->width + (int) xpos + x0],
                                              x1 - x0, y1 - y0, stb_screen->width, scale, scale, x_shift, 0, p_buf[ch]);
            // gui_log("draw %c , ch : %d , xpos : %f , y pos : %f \n",p_buf[ch],ch,xpos,ypos);
            ++ch;
            xpos += (advance * scale);
        }
    }
    font_stb_draw_bitmap(text, text->data, rect);
#endif
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
}

void gui_font_stb_init(void *font_ttf_addr)
{
    stbtt_InitFont(&font, font_ttf_addr, 0);
}
