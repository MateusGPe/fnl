#pragma once
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Widget.H>
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>
#include <unordered_map>

namespace mui
{
    namespace core
    {
        inline Fl_Color activated_color(Fl_Color c, bool active)
        {
            return active ? c : fl_inactive(c);
        }
        inline void draw_rounded_gradient_box(int x, int y, int w, int h, Fl_Color t_start, Fl_Color t_end, Fl_Color b_start, Fl_Color b_end, int radius, bool active)
        {
            if (t_start == 0 && t_end == 0 && b_start == 0 && b_end == 0)
                return;

            if (t_start == t_end && b_start == b_end && t_start == b_start)
            {
                fl_color(activated_color(t_start, active));
                fl_rounded_rectf(x, y, w, h, radius);
                return;
            }

            Fl_Color avg = fl_color_average(t_start, b_end, 0.5f);
            fl_color(activated_color(avg, active));
            fl_rounded_rectf(x, y, w, h, radius);
        }

        struct FrameColors
        {
            Fl_Color out_top, out_side, out_bot;
            Fl_Color in_top, in_side, in_bot;
            Fl_Color dot1, dot2;
            const char *classic_code;
        };

        inline void draw_rounded_frame_h(int x, int y, int w, int h, const FrameColors &fc, int radius, bool active)
        {
            if (fc.out_top)
            {
                fl_color(activated_color(fc.out_top, active));
                fl_rounded_rect(x, y, w, h, radius);
            }
            if (fc.in_top)
            {
                fl_color(activated_color(fc.in_top, active));
                fl_rounded_rect(x + 1, y + 1, w - 2, h - 2, radius > 1 ? radius - 1 : 1);
            }
        }

        inline void draw_smart_gradient_4(
            int x, int y, int w, int h,
            Fl_Color t_start, Fl_Color t_end,
            Fl_Color b_start, Fl_Color b_end, bool active)
        {
            if (t_start == 0 && t_end == 0 && b_start == 0 && b_end == 0)
                return;
            if (t_start == t_end && b_start == b_end && t_start == b_start)
            {
                fl_color(activated_color(t_start, active));
                fl_rectf(x, y, w, h);
                return;
            }
            if (w >= h)
            {
                int half_h = h / 2;
                for (int i = 0; i < half_h; ++i)
                {
                    float weight = 1.0f - (float)i / (float)(half_h > 0 ? half_h : 1);
                    fl_color(activated_color(fl_color_average(t_start, t_end, weight), active));
                    fl_xyline(x, y + i, x + w - 1);
                }
                for (int i = 0; i < h - half_h; ++i)
                {
                    float weight = 1.0f - (float)i / (float)((h - half_h) > 0 ? (h - half_h) : 1);
                    fl_color(activated_color(fl_color_average(b_start, b_end, weight), active));
                    fl_xyline(x, y + half_h + i, x + w - 1);
                }
            }
            else
            {
                int half_w = w / 2;
                for (int i = 0; i < half_w; ++i)
                {
                    float weight = 1.0f - (float)i / (float)(half_w > 0 ? half_w : 1);
                    fl_color(activated_color(fl_color_average(t_start, t_end, weight), active));
                    fl_yxline(x + i, y, y + h - 1);
                }
                for (int i = 0; i < w - half_w; ++i)
                {
                    float weight = 1.0f - (float)i / (float)((w - half_w) > 0 ? (w - half_w) : 1);
                    fl_color(activated_color(fl_color_average(b_start, b_end, weight), active));
                    fl_yxline(x + half_w + i, y, y + h - 1);
                }
            }
        }

        inline void draw_beveled_frame(int x, int y, int w, int h, const FrameColors &fc, bool active)
        {
            if (fc.out_top)
            {
                fl_color(activated_color(fc.out_top, active));
                fl_xyline(x + 2, y, x + w - 3);
            }
            if (fc.out_side)
            {
                fl_color(activated_color(fc.out_side, active));
                fl_yxline(x, y + 2, y + h - 3);
                fl_yxline(x + w - 1, y + 2, y + h - 3);
            }
            if (fc.out_bot)
            {
                fl_color(activated_color(fc.out_bot, active));
                fl_xyline(x + 2, y + h - 1, x + w - 3);
            }
            if (fc.in_top)
            {
                fl_color(activated_color(fc.in_top, active));
                fl_xyline(x + 2, y + 1, x + w - 3);
            }
            if (fc.in_side)
            {
                fl_color(activated_color(fc.in_side, active));
                fl_yxline(x + 1, y + 2, y + h - 3);
                fl_yxline(x + w - 2, y + 2, y + h - 3);
            }
            if (fc.in_bot)
            {
                fl_color(activated_color(fc.in_bot, active));
                fl_xyline(x + 2, y + h - 2, x + w - 3);
            }
            if (fc.dot1)
            {
                fl_color(activated_color(fc.dot1, active));
                fl_xyline(x, y + 1, x + 1, y);
                fl_yxline(x + w - 2, y, y + 1, x + w - 1);
            }
            if (fc.dot2)
            {
                fl_color(activated_color(fc.dot2, active));
                fl_xyline(x, y + h - 2, x + 1, y + h - 1);
                fl_yxline(x + w - 2, y + h - 1, y + h - 2, x + w - 1);
            }
        }

        inline void draw_solid_frame(int x, int y, int w, int h, const FrameColors &fc, bool active)
        {
            fl_color(activated_color(fc.out_top, active));
            fl_rect(x, y, w, h);
            if (fc.in_top != 0)
            {
                fl_color(activated_color(fc.in_top, active));
                fl_rect(x + 1, y + 1, w - 2, h - 2);
            }
        }

        inline void draw_classic_frame(int x, int y, int w, int h, const FrameColors &fc, bool active)
        {
            if (fc.classic_code)
                fl_frame2(fc.classic_code, x, y, w, h);
        }

        inline void draw_rounded_arcs_frame(int x, int y, int w, int h, const FrameColors &fc, bool active)
        {
            fl_color(activated_color(fc.out_top, active));
            fl_xyline(x + 3, y, x + w - 4);
            fl_color(activated_color(fc.out_side, active));
            fl_yxline(x, y + 3, y + h - 4);
            fl_yxline(x + w - 1, y + 3, y + h - 4);
            fl_color(activated_color(fc.out_bot, active));
            fl_xyline(x + 3, y + h - 1, x + w - 4);
            fl_color(activated_color(fc.in_top, active));
            fl_xyline(x + 3, y + 1, x + w - 4);
            fl_color(activated_color(fc.in_side, active));
            fl_yxline(x + 1, y + 3, y + h / 2 - 1);
            fl_yxline(x + w - 2, y + 3, y + h / 2 - 1);
            fl_color(activated_color(fc.in_bot, active));
            fl_yxline(x + 1, y + h / 2, y + h - 4);
            fl_yxline(x + w - 2, y + h / 2, y + h - 4);
            if (fc.dot1)
            {
                fl_color(activated_color(fc.dot1, active));
                fl_xyline(x + 3, y + h - 2, x + w - 4);
            }
            fl_color(activated_color(fc.dot2, active));
            fl_arc(x, y, 8, 8, 90.0, 180.0);
            fl_arc(x, y + h - 8, 8, 8, 180.0, 270.0);
            fl_arc(x + w - 8, y + h - 8, 8, 8, 270.0, 360.0);
            fl_arc(x + w - 8, y, 8, 8, 0.0, 90.0);
        }

        inline void draw_winxp_frame(int x, int y, int w, int h, const FrameColors &fc, bool active)
        {
            if (fc.out_top)
            {
                fl_color(activated_color(fc.out_top, active));
                fl_xyline(x + 2, y, x + w - 3);
                fl_xyline(x + 2, y + h - 1, x + w - 3);
            }
            if (fc.out_side)
            {
                fl_color(activated_color(fc.out_side, active));
                fl_yxline(x, y + 2, y + h - 3);
                fl_yxline(x + w - 1, y + 2, y + h - 3);
            }

            if (fc.dot1)
            {
                fl_color(activated_color(fc.dot1, active));
                fl_point(x + 1, y + 1);
                fl_point(x + w - 2, y + 1);
                fl_point(x + 1, y + h - 2);
                fl_point(x + w - 2, y + h - 2);
            }

            if (fc.in_top)
            {
                fl_color(activated_color(fc.in_top, active));
                fl_xyline(x + 2, y + 1, x + w - 3); // Top inner
                fl_yxline(x + 1, y + 2, y + h - 3); // Left inner
            }
            if (fc.in_bot)
            {
                fl_color(activated_color(fc.in_bot, active));
                fl_xyline(x + 2, y + h - 2, x + w - 3); // Bottom inner
                fl_yxline(x + w - 2, y + 2, y + h - 3); // Right inner
            }
        }

        inline void draw_winxp_box(int x, int y, int w, int h, Fl_Color t_start, Fl_Color t_end, Fl_Color b_start, Fl_Color b_end, bool active)
        {
            if (t_start == 0 && t_end == 0 && b_start == 0 && b_end == 0)
                return;

            int fill_x = x + 2;
            int fill_y = y + 2;
            int fill_w = w - 4;
            int fill_h = h - 4;

            if (fill_h <= 0 || fill_w <= 0)
                return;

            int half_h = fill_h / 2;
            for (int i = 0; i < fill_h; ++i)
            {
                int current_y = fill_y + i;

                Fl_Color c;
                if (i < half_h)
                {
                    float weight = 1.0f - (float)i / (float)(half_h > 0 ? half_h : 1);
                    c = fl_color_average(t_start, t_end, weight);
                }
                else
                {
                    float weight = 1.0f - (float)(i - half_h) / (float)((fill_h - half_h) > 0 ? (fill_h - half_h) : 1);
                    c = fl_color_average(b_start, b_end, weight);
                }

                fl_color(activated_color(c, active));
                fl_xyline(fill_x, current_y, fill_x + fill_w - 1);
            }
        }
        /*
                inline void draw_checkbox_box(int x, int y, int size, bool is_checked, Fl_Color bg, Fl_Color border_color)
                {
                    fl_color(is_checked ? bg : border_color);
                    if (!is_checked)
                    {
                        fl_rounded_rect(x, y, size, size, mui::current_palette->metrics.checkbox_corner_radius);
                        fl_color(bg);
                        fl_rounded_rectf(x + mui::current_palette->metrics.checkbox_border_width, y + mui::current_palette->metrics.checkbox_border_width, size - (mui::current_palette->metrics.checkbox_border_width * 2), size - (mui::current_palette->metrics.checkbox_border_width * 2), mui::current_palette->metrics.checkbox_corner_radius - mui::current_palette->metrics.checkbox_border_width);
                    }
                    else
                    {
                        fl_rounded_rectf(x, y, size, size, mui::current_palette->metrics.checkbox_corner_radius);
                    }
                }

                inline void draw_checkbox_focus(int x, int y, int size, Fl_Color focus_color)
                {
                    fl_color(focus_color);
                    fl_line_style(FL_SOLID, mui::current_palette->metrics.checkbox_focus_width);
                    fl_rounded_rect(x - mui::current_palette->metrics.checkbox_focus_padding, y - mui::current_palette->metrics.checkbox_focus_padding, size + (mui::current_palette->metrics.checkbox_focus_padding * 2), size + (mui::current_palette->metrics.checkbox_focus_padding * 2), mui::current_palette->metrics.checkbox_corner_radius + 1);
                    fl_line_style(0);
                }

                inline void draw_checkbox_tick(int x, int y, int size, Fl_Color tick_color)
                {
                    fl_color(tick_color);
                    fl_line_style(FL_SOLID, mui::current_palette->metrics.checkbox_tick_width);
                    fl_begin_line();
                    fl_vertex(x + mui::current_palette->metrics.checkbox_tick_padding, y + size / 2);
                    fl_vertex(x + 8, y + size - 5);
                    fl_vertex(x + size - mui::current_palette->metrics.checkbox_tick_padding, y + mui::current_palette->metrics.checkbox_tick_padding);
                    fl_end_line();
                    fl_line_style(0);
                }

                inline void draw_radio_focus(int cx, int cy, int size, Fl_Color focus_color)
                {
                    fl_color(focus_color);
                    int focus_size = size + (mui::current_palette->metrics.radio_focus_padding * 2);
                    fl_pie(cx - focus_size / 2, cy - focus_size / 2, focus_size, focus_size, 0, 360);
                }

                inline void draw_radio_border(int x, int y, int size, Fl_Color border_color)
                {
                    fl_color(border_color);
                    fl_line_style(FL_SOLID, mui::current_palette->metrics.radio_border_width);
                    fl_arc(x, y, size, size, 0, 360);
                    fl_line_style(0);
                } */

        inline void draw_radio_dot(int cx, int cy, int dot_size, Fl_Color dot_color)
        {
            fl_color(dot_color);
            fl_pie(cx - dot_size / 2, cy - dot_size / 2, dot_size, dot_size, 0, 360);
        }

        inline void draw_slider_track_split(int start_coord, int mid_coord, int end_coord, int cross_coord, int track_thickness, Fl_Color filled_color, Fl_Color empty_color, bool is_horiz)
        {
            fl_line_style(FL_SOLID, track_thickness, 0);
            if (is_horiz)
            {
                fl_color(empty_color);
                fl_line(mid_coord, cross_coord, end_coord, cross_coord);
                fl_color(filled_color);
                fl_line(start_coord, cross_coord, mid_coord, cross_coord);
            }
            else // vertical
            {
                fl_color(empty_color);
                fl_line(cross_coord, start_coord, cross_coord, mid_coord);
                fl_color(filled_color);
                fl_line(cross_coord, mid_coord, cross_coord, end_coord);
            }
            fl_line_style(0);
        }

        inline void draw_slider_thumb(int cx, int cy, int size, Fl_Color color)
        {
            fl_color(color);
            fl_pie(cx - size / 2, cy - size / 2, size, size, 0, 360);
        }

        inline void draw_slider_halo(int cx, int cy, int size, Fl_Color color)
        {
            fl_color(color);
            fl_pie(cx - size / 2, cy - size / 2, size, size, 0, 360);
        }

        inline void draw_spinner_separators(int x, int y, int w, int h, int btn_w, int inset, Fl_Color color)
        {
            int btn_x = x + w - btn_w;
            fl_color(color);
            fl_line(btn_x, y + inset, btn_x, y + h - inset - 1);
            fl_line(btn_x, y + h / 2, x + w - inset, y + h / 2);
        }

        inline void draw_spinner_arrow(int cx, int cy, int size, bool is_up, Fl_Color color)
        {
            const int h_size = size >> 1, q_size = size >> 2;
            fl_color(color);
            fl_polygon(cx - h_size, cy + (is_up ? q_size : -q_size), cx, cy + (is_up ? -q_size : q_size), cx + h_size, cy + (is_up ? q_size : -q_size));
        }
    }
}