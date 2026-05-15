#pragma once
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Widget.H>
#include <algorithm>
#include <cstdint>
#include <cmath>

namespace mui
{
    namespace core
    {
        [[nodiscard]] inline double normalize_value(double value, double min, double max)
        {
            const double range = max - min;
            return (range != 0.0)
                       ? std::clamp((value - min) / range, 0.0, 1.0)
                       : 0.0;
        }

        enum class Rounding
        {
            None = 0,
            TopLeft = 1,
            TopRight = 2,
            BottomRight = 4,
            BottomLeft = 8,
            All = TopLeft | TopRight | BottomRight | BottomLeft,
            Left = TopLeft | BottomLeft,
            Right = TopRight | BottomRight,
            Top = TopLeft | TopRight,
            Bottom = BottomLeft | BottomRight
        };

        struct FrameColors
        {
            Fl_Color out_top, out_side, out_bot;
            Fl_Color in_top, in_side, in_bot;
            Fl_Color dot1, dot2;
            const char *classic_code;
        };

        [[nodiscard]] inline Fl_Color activated_color(Fl_Color c, bool active) noexcept
        {
            return active ? c : fl_inactive(c);
        }

        inline void rounded_rect_corners(int x, int y, int w, int h, int r, int corners)
        {
            if (r == 0 || corners == static_cast<int>(Rounding::None))
            {
                fl_rect(x, y, w, h);
                return;
            }
            int d = 2 * r;

            fl_xyline(x + ((corners & 1) ? r : 0), y, x + w - 1 - ((corners & 2) ? r : 0));
            fl_xyline(x + ((corners & 8) ? r : 0), y + h - 1, x + w - 1 - ((corners & 4) ? r : 0));
            fl_yxline(x, y + ((corners & 1) ? r : 0), y + h - 1 - ((corners & 8) ? r : 0));
            fl_yxline(x + w - 1, y + ((corners & 2) ? r : 0), y + h - 1 - ((corners & 4) ? r : 0));

            if (corners & 1)
                fl_arc(x, y, d, d, 90, 180);
            if (corners & 2)
                fl_arc(x + w - d, y, d, d, 0, 90);
            if (corners & 4)
                fl_arc(x + w - d, y + h - d, d, d, 270, 360);
            if (corners & 8)
                fl_arc(x, y + h - d, d, d, 180, 270);
        }

        inline void rounded_rectf_corners(int x, int y, int w, int h, int r, int corners)
        {
            if (r == 0 || corners == static_cast<int>(Rounding::None))
            {
                fl_rectf(x, y, w, h);
                return;
            }
            int d = 2 * r;
            fl_rectf(x + r, y, w - d, h);
            fl_rectf(x, y + r, w, h - d);

            if (corners & 1)
                fl_pie(x, y, d, d, 90, 180);
            else
                fl_rectf(x, y, r, r);
            if (corners & 2)
                fl_pie(x + w - d, y, d, d, 0, 90);
            else
                fl_rectf(x + w - r, y, r, r);
            if (corners & 4)
                fl_pie(x + w - d, y + h - d, d, d, 270, 360);
            else
                fl_rectf(x + w - r, y + h - r, r, r);
            if (corners & 8)
                fl_pie(x, y + h - d, d, d, 180, 270);
            else
                fl_rectf(x, y + h - r, r, r);
        }

        inline void draw_rounded_frame_h(int x, int y, int w, int h,
                                         const FrameColors &fc, int radius, bool active)
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

        inline void draw_rounded_frame_h_corners(int x, int y, int w, int h,
                                                 const FrameColors &fc, int radius, bool active, int corners)
        {
            if (fc.out_top)
            {
                fl_color(activated_color(fc.out_top, active));
                rounded_rect_corners(x, y, w, h, radius, corners);
            }
            if (fc.in_top)
            {
                fl_color(activated_color(fc.in_top, active));
                rounded_rect_corners(x + 1, y + 1, w - 2, h - 2, radius > 1 ? radius - 1 : 1, corners);
            }
        }

        inline void draw_solid_frame(int x, int y, int w, int h,
                                     const FrameColors &fc, bool active)
        {
            fl_color(activated_color(fc.out_top, active));
            fl_rect(x, y, w, h);
            if (fc.in_top != 0)
            {
                fl_color(activated_color(fc.in_top, active));
                fl_rect(x + 1, y + 1, w - 2, h - 2);
            }
        }

        inline void draw_beveled_frame(int x, int y, int w, int h,
                                       const FrameColors &fc, bool active)
        {
            auto set = [&](Fl_Color c)
            { fl_color(activated_color(c, active)); };

            if (fc.out_top)
            {
                set(fc.out_top);
                fl_xyline(x + 2, y, x + w - 3);
            }
            if (fc.out_side)
            {
                set(fc.out_side);
                fl_yxline(x, y + 2, y + h - 3);
                fl_yxline(x + w - 1, y + 2, y + h - 3);
            }
            if (fc.out_bot)
            {
                set(fc.out_bot);
                fl_xyline(x + 2, y + h - 1, x + w - 3);
            }
            if (fc.in_top)
            {
                set(fc.in_top);
                fl_xyline(x + 2, y + 1, x + w - 3);
            }
            if (fc.in_side)
            {
                set(fc.in_side);
                fl_yxline(x + 1, y + 2, y + h - 3);
                fl_yxline(x + w - 2, y + 2, y + h - 3);
            }
            if (fc.in_bot)
            {
                set(fc.in_bot);
                fl_xyline(x + 2, y + h - 2, x + w - 3);
            }
            if (fc.dot1)
            {
                set(fc.dot1);
                fl_xyline(x, y + 1, x + 1, y);
                fl_yxline(x + w - 2, y, y + 1, x + w - 1);
            }
            if (fc.dot2)
            {
                set(fc.dot2);
                fl_xyline(x, y + h - 2, x + 1, y + h - 1);
                fl_yxline(x + w - 2, y + h - 1, y + h - 2, x + w - 1);
            }
        }

        inline void draw_smart_gradient_4(int x, int y, int w, int h,
                                          Fl_Color t_start, Fl_Color t_end,
                                          Fl_Color b_start, Fl_Color b_end,
                                          bool active)
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
                const int half_h = h / 2;
                for (int i = 0; i < half_h; ++i)
                {
                    float t = 1.0f - static_cast<float>(i) / static_cast<float>(half_h > 0 ? half_h : 1);
                    fl_color(activated_color(fl_color_average(t_start, t_end, t), active));
                    fl_xyline(x, y + i, x + w - 1);
                }
                for (int i = 0; i < h - half_h; ++i)
                {
                    float t = 1.0f - static_cast<float>(i) / static_cast<float>((h - half_h) > 0 ? (h - half_h) : 1);
                    fl_color(activated_color(fl_color_average(b_start, b_end, t), active));
                    fl_xyline(x, y + half_h + i, x + w - 1);
                }
            }
            else
            {
                const int half_w = w / 2;
                for (int i = 0; i < half_w; ++i)
                {
                    float t = 1.0f - static_cast<float>(i) / static_cast<float>(half_w > 0 ? half_w : 1);
                    fl_color(activated_color(fl_color_average(t_start, t_end, t), active));
                    fl_yxline(x + i, y, y + h - 1);
                }
                for (int i = 0; i < w - half_w; ++i)
                {
                    float t = 1.0f - static_cast<float>(i) / static_cast<float>((w - half_w) > 0 ? (w - half_w) : 1);
                    fl_color(activated_color(fl_color_average(b_start, b_end, t), active));
                    fl_yxline(x + half_w + i, y, y + h - 1);
                }
            }
        }

        inline void draw_rounded_gradient_box(int x, int y, int w, int h,
                                              Fl_Color t_start, Fl_Color t_end,
                                              Fl_Color b_start, Fl_Color b_end,
                                              int radius, bool active)
        {
            if (t_start == 0 && t_end == 0 && b_start == 0 && b_end == 0)
                return;

            if (radius <= 1)
            {
                draw_smart_gradient_4(x, y, w, h, t_start, t_end, b_start, b_end, active);
                return;
            }

            if (t_start == t_end && b_start == b_end && t_start == b_start)
            {
                fl_color(activated_color(t_start, active));
                fl_rounded_rectf(x, y, w, h, radius);
                return;
            }

            radius = std::min({radius, w / 2, h / 2});
            if (radius <= 0)
            {
                draw_smart_gradient_4(x, y, w, h, t_start, t_end, b_start, b_end, active);
                return;
            }

            const int half_h = h / 2;
            for (int i = 0; i < h; ++i)
            {
                Fl_Color c;
                if (i < half_h)
                {
                    float t = 1.0f - static_cast<float>(i) / static_cast<float>(half_h > 0 ? half_h : 1);
                    c = fl_color_average(t_start, t_end, t);
                }
                else
                {
                    float t = 1.0f - static_cast<float>(i - half_h) / static_cast<float>((h - half_h) > 0 ? (h - half_h) : 1);
                    c = fl_color_average(b_start, b_end, t);
                }
                fl_color(activated_color(c, active));

                int dx = 0;
                if (i < radius)
                {
                    double y_rel = i + 0.5;
                    dx = static_cast<int>(std::round(radius - std::sqrt(2.0 * radius * y_rel - y_rel * y_rel)));
                }
                else if (i >= h - radius)
                {
                    double y_rel = (h - 1 - i) + 0.5;
                    dx = static_cast<int>(std::round(radius - std::sqrt(2.0 * radius * y_rel - y_rel * y_rel)));
                }
                fl_xyline(x + dx, y + i, x + w - 1 - dx);
            }
        }

        inline void draw_rounded_gradient_box_corners(int x, int y, int w, int h,
                                                      Fl_Color t_start, Fl_Color t_end,
                                                      Fl_Color b_start, Fl_Color b_end,
                                                      int radius, bool active, int corners)
        {
            if (t_start == 0 && t_end == 0 && b_start == 0 && b_end == 0)
                return;

            if (radius <= 1)
            {
                draw_smart_gradient_4(x, y, w, h, t_start, t_end, b_start, b_end, active);
                return;
            }

            if (t_start == t_end && b_start == b_end && t_start == b_start)
            {
                fl_color(activated_color(t_start, active));
                rounded_rectf_corners(x, y, w, h, radius, corners);
                return;
            }

            radius = std::min({radius, w / 2, h / 2});
            if (radius <= 0)
            {
                draw_smart_gradient_4(x, y, w, h, t_start, t_end, b_start, b_end, active);
                return;
            }

            const int half_h = h / 2;
            for (int i = 0; i < h; ++i)
            {
                Fl_Color c;
                if (i < half_h)
                {
                    float t = 1.0f - static_cast<float>(i) / static_cast<float>(half_h > 0 ? half_h : 1);
                    c = fl_color_average(t_start, t_end, t);
                }
                else
                {
                    float t = 1.0f - static_cast<float>(i - half_h) / static_cast<float>((h - half_h) > 0 ? (h - half_h) : 1);
                    c = fl_color_average(b_start, b_end, t);
                }
                fl_color(activated_color(c, active));

                int dx_left = 0, dx_right = 0;
                if (i < radius)
                {
                    double y_rel = i + 0.5;
                    int dx = static_cast<int>(std::round(radius - std::sqrt(2.0 * radius * y_rel - y_rel * y_rel)));
                    if (corners & 1) dx_left = dx;
                    if (corners & 2) dx_right = dx;
                }
                else if (i >= h - radius)
                {
                    double y_rel = (h - 1 - i) + 0.5;
                    int dx = static_cast<int>(std::round(radius - std::sqrt(2.0 * radius * y_rel - y_rel * y_rel)));
                    if (corners & 8) dx_left = dx;
                    if (corners & 4) dx_right = dx;
                }
                fl_xyline(x + dx_left, y + i, x + w - 1 - dx_right);
            }
        }

        inline void draw_slider_track_split(int start_coord, int mid_coord, int end_coord,
                                            int cross_coord, int track_thickness,
                                            Fl_Color filled_color, Fl_Color empty_color,
                                            bool is_horiz)
        {
            fl_line_style(FL_SOLID | FL_CAP_ROUND, track_thickness, nullptr);
            if (is_horiz)
            {
                fl_color(empty_color);
                fl_line(mid_coord, cross_coord, end_coord, cross_coord);
                fl_color(filled_color);
                fl_line(start_coord, cross_coord, mid_coord, cross_coord);
            }
            else
            {
                fl_color(empty_color);
                fl_line(cross_coord, start_coord, cross_coord, mid_coord);
                fl_color(filled_color);
                fl_line(cross_coord, mid_coord, cross_coord, end_coord);
            }
            fl_line_style(0);
        }

        inline void calculate_slider_thumb_position_and_draw_track(
            int slider_area_x, int slider_area_y, int slider_area_w, int slider_area_h,
            int track_area_x, int track_area_y, int track_area_w, int track_area_h,
            double val,
            bool is_horiz,
            int track_thickness,
            Fl_Color filled_color,
            Fl_Color empty_color,
            int &out_cx, int &out_cy)
        {
            if (is_horiz)
            {
                out_cx = track_area_x + static_cast<int>(val * track_area_w);
                out_cy = slider_area_y + slider_area_h / 2;
                draw_slider_track_split(
                    track_area_x, out_cx, track_area_x + track_area_w, out_cy,
                    track_thickness, filled_color, empty_color, true);
            }
            else
            {
                out_cx = slider_area_x + slider_area_w / 2;
                out_cy = track_area_y + static_cast<int>((1.0 - val) * track_area_h);
                draw_slider_track_split(
                    track_area_y, out_cy, track_area_y + track_area_h, out_cx,
                    track_thickness, filled_color, empty_color, false);
            }
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

        inline void draw_spinner_separators(int x, int y, int w, int h,
                                            int btn_w, int inset, Fl_Color color)
        {
            const int btn_x = x + w - btn_w;
            fl_color(color);
            fl_line(btn_x, y + inset, btn_x, y + h - inset - 1);
            fl_line(btn_x, y + h / 2, x + w - inset, y + h / 2);
        }

        inline void draw_spinner_arrow(int cx, int cy, int size, bool is_up, Fl_Color color)
        {
            const int h_size = size >> 1;
            const int q_size = size >> 2;
            fl_color(color);
            fl_polygon(
                cx - h_size, cy + (is_up ? q_size : -q_size),
                cx, cy + (is_up ? -q_size : q_size),
                cx + h_size, cy + (is_up ? q_size : -q_size));
        }

        inline void draw_radio_dot(int cx, int cy, int dot_size, Fl_Color dot_color)
        {
            fl_color(dot_color);
            fl_pie(cx - dot_size / 2, cy - dot_size / 2, dot_size, dot_size, 0, 360);
        }

    }
}
