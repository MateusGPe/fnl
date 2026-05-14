#pragma once
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <algorithm>

#include "data.hpp"
#include "primitives.hpp"

namespace mui
{
    namespace engine
    {
        struct WidgetState
        {
            bool active = true;
            bool hovered = false;
            bool focused = false;
            double value = 0.0;
        };

        [[nodiscard]] inline Fl_Color get_text_color(bool active,
                                                     Fl_Color active_color,
                                                     Fl_Color parent_bg,
                                                     float disabled_factor) noexcept
        {
            const Fl_Color contrasted = fl_contrast(active_color, parent_bg);
            return active ? contrasted
                          : fl_color_average(contrasted, parent_bg, disabled_factor);
        }

        inline void unpack_rgb(Fl_Color c,
                               unsigned char &r, unsigned char &g, unsigned char &b) noexcept
        {
            r = static_cast<unsigned char>((c >> 24) & 0xFF);
            g = static_cast<unsigned char>((c >> 16) & 0xFF);
            b = static_cast<unsigned char>((c >> 8) & 0xFF);
        }

        inline void draw_focus_ring(int x, int y, int w, int h,
                                    Fl_Color ring_color, float opacity,
                                    int width, int radius)
        {
            unsigned char r, g, b;
            Fl::get_color(ring_color, r, g, b);
            fl_color(rgba(r, g, b, static_cast<uint8_t>(opacity * 255.0f)));

            if (width > 0)
            {
                fl_line_style(FL_SOLID, width);
                if (radius > 0)
                    fl_rounded_rect(x, y, w, h, radius);
                else
                    fl_rect(x, y, w, h);
                fl_line_style(0);
            }
            else
            {
                if (radius >= w / 2)
                    fl_pie(x, y, w, h, 0, 360);
                else if (radius > 0)
                    fl_rounded_rectf(x, y, w, h, radius);
                else
                    fl_rectf(x, y, w, h);
            }
        }

        inline void draw_frame(int x, int y, int w, int h,
                               const core::FrameColors &fc,
                               bool active, const ThemePalette &palette)
        {
            if (palette.metrics.radius > 0)
                core::draw_rounded_frame_h(x, y, w, h, fc, palette.metrics.radius, active);
            else
                core::draw_solid_frame(x, y, w, h, fc, active);
        }

        inline void draw_static_frame(int x, int y, int w, int h,
                                      const core::FrameColors &fc,
                                      bool active, const ThemePalette &palette)
        {
            draw_frame(x, y, w, h, fc, active, palette);
        }

        inline void draw_box(int x, int y, int w, int h,
                             const Gradient4 &grad, const core::FrameColors &fc,
                             int inset, bool active, const ThemePalette &palette)
        {
            if (palette.metrics.radius > 0)
            {
                if (fc.out_top)
                {
                    fl_color(core::activated_color(fc.out_top, active));
                    fl_rounded_rectf(x, y, w, h, palette.metrics.radius);
                }
                core::draw_rounded_gradient_box(
                    x + inset, y + inset, w - inset * 2, h - inset * 2,
                    grad.t_start, grad.t_end, grad.b_start, grad.b_end,
                    std::max(0, palette.metrics.radius - inset), active);
                core::draw_rounded_frame_h(x, y, w, h, fc, palette.metrics.radius, active);
            }
            else
            {
                core::draw_smart_gradient_4(
                    x + inset, y + inset, w - inset * 2, h - inset * 2,
                    grad.t_start, grad.t_end, grad.b_start, grad.b_end, active);
                core::draw_solid_frame(x, y, w, h, fc, active);
            }
        }

        inline void draw_box_corners(int x, int y, int w, int h,
                                     const Gradient4 &grad, const core::FrameColors &fc,
                                     int inset, bool active, const ThemePalette &palette,
                                     int corners)
        {
            if (palette.metrics.radius > 0)
            {
                if (fc.out_top)
                {
                    fl_color(core::activated_color(fc.out_top, active));
                    core::rounded_rectf_corners(x, y, w, h, palette.metrics.radius, corners);
                }
                core::draw_rounded_gradient_box_corners(
                    x + inset, y + inset, w - inset * 2, h - inset * 2,
                    grad.t_start, grad.t_end, grad.b_start, grad.b_end,
                    std::max(0, palette.metrics.radius - inset), active, corners);
                core::draw_rounded_frame_h_corners(x, y, w, h, fc, palette.metrics.radius, active, corners);
            }
            else
            {
                core::draw_smart_gradient_4(
                    x + inset, y + inset, w - inset * 2, h - inset * 2,
                    grad.t_start, grad.t_end, grad.b_start, grad.b_end, active);
                core::draw_solid_frame(x, y, w, h, fc, active);
            }
        }

        inline void draw_button(int x, int y, int w, int h,
                                const WidgetState &state, const ThemePalette &palette)
        {
            const Gradient4 &grad = (state.value != 0.0)
                                        ? palette.down_grad
                                        : (state.hovered ? palette.hover_grad : palette.btn_grad);
            draw_box(x, y, w, h, grad, palette.btn_frame, 2, state.active, palette);
        }

        inline void draw_button_left(int x, int y, int w, int h,
                                     const WidgetState &state, const ThemePalette &palette)
        {
            const Gradient4 &grad = (state.value != 0.0)
                                        ? palette.down_grad
                                        : (state.hovered ? palette.hover_grad : palette.btn_grad);
            draw_box_corners(x, y, w, h, grad, palette.btn_frame, 2, state.active, palette, static_cast<int>(core::Rounding::Left));
        }

        inline void draw_button_middle(int x, int y, int w, int h,
                                       const WidgetState &state, const ThemePalette &palette)
        {
            const Gradient4 &grad = (state.value != 0.0)
                                        ? palette.down_grad
                                        : (state.hovered ? palette.hover_grad : palette.btn_grad);
            draw_box_corners(x, y, w, h, grad, palette.btn_frame, 2, state.active, palette, static_cast<int>(core::Rounding::None));
        }

        inline void draw_button_right(int x, int y, int w, int h,
                                      const WidgetState &state, const ThemePalette &palette)
        {
            const Gradient4 &grad = (state.value != 0.0)
                                        ? palette.down_grad
                                        : (state.hovered ? palette.hover_grad : palette.btn_grad);
            draw_box_corners(x, y, w, h, grad, palette.btn_frame, 2, state.active, palette, static_cast<int>(core::Rounding::Right));
        }

        inline void draw_choice(int x, int y, int w, int h,
                                const WidgetState &state, bool is_pressed,
                                const ThemePalette &palette)
        {
            const bool hovering = state.hovered && state.active && !is_pressed;
            const Gradient4 &grad = is_pressed
                                        ? palette.down_grad
                                        : (hovering ? palette.hover_grad : palette.btn_grad);
            draw_box(x, y, w, h, grad, palette.btn_frame, 2, state.active, palette);

            const int div_x = x + w - palette.metrics.choice_arrow_padding * 2 - palette.metrics.choice_arrow_size;
            fl_color(fl_color_average(palette.btn_frame.out_top,
                                      is_pressed ? palette.bg_sec : palette.bg_main, 0.6f));
            fl_yxline(div_x, y + 4, y + h - 5);

            const Fl_Color arrow_c = is_pressed      ? palette.selection
                                     : state.hovered ? palette.fg_main
                                                     : palette.inactive;
            fl_color(get_text_color(state.active, arrow_c,
                                    is_pressed ? palette.bg_sec : palette.bg_main,
                                    palette.metrics.widget_disabled_label_factor));

            const int ax = div_x + palette.metrics.choice_arrow_padding + palette.metrics.choice_arrow_size / 2;
            const int ay = y + h / 2;
            const int aw = palette.metrics.choice_arrow_size / 2;
            const int ah = palette.metrics.choice_arrow_height / 2;

            fl_line_style(FL_SOLID, palette.metrics.choice_arrow_thickness);
            fl_begin_line();
            fl_vertex(ax - aw, ay - ah);
            fl_vertex(ax, ay + ah);
            fl_vertex(ax + aw, ay - ah);
            fl_end_line();
            fl_line_style(0);
        }
    }
}
