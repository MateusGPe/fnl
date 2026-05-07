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

        inline Fl_Color get_text_color(bool active, Fl_Color active_color, Fl_Color parent_bg_color, float disabled_factor)
        {
            Fl_Color contrasted = fl_contrast(active_color, parent_bg_color);
            return active ? contrasted : fl_color_average(contrasted, parent_bg_color, disabled_factor);
        }

        inline void draw_focus_ring(int x, int y, int w, int h, Fl_Color ring_color, float opacity, int width, int radius)
        {
            uint8_t r, g, b;
            Fl::get_color(ring_color, r, g, b);
            fl_color(rgba(r, g, b, static_cast<uint8_t>(opacity * 255)));
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

        inline void draw_frame(int x, int y, int w, int h, const core::FrameColors &fc, bool active, const ThemePalette &palette)
        {
            if (palette.metrics.radius > 0)
                core::draw_rounded_frame_h(x, y, w, h, fc, palette.metrics.radius, active);
            else
                core::draw_solid_frame(x, y, w, h, fc, active);
        }

        inline void draw_static_frame(int x, int y, int w, int h, const core::FrameColors &fc, bool active, const ThemePalette &palette)
        {
            if (palette.metrics.radius > 0)
                core::draw_rounded_frame_h(x, y, w, h, fc, palette.metrics.radius, active);
            else
                core::draw_solid_frame(x, y, w, h, fc, active);
        }

        inline void draw_box(int x, int y, int w, int h, const Gradient4 &grad, const core::FrameColors &fc, int inset, bool active, const ThemePalette &palette)
        {
            if (palette.metrics.radius > 0) {
                core::draw_rounded_gradient_box(x + inset, y + inset, w - (inset * 2), h - (inset * 2), grad.t_start, grad.t_end, grad.b_start, grad.b_end, palette.metrics.radius, active);
                core::draw_rounded_frame_h(x, y, w, h, fc, palette.metrics.radius, active);
            } else {
                core::draw_smart_gradient_4(x + inset, y + inset, w - (inset * 2), h - (inset * 2), grad.t_start, grad.t_end, grad.b_start, grad.b_end, active);
                core::draw_solid_frame(x, y, w, h, fc, active);
            }
        }

        inline void draw_button(int x, int y, int w, int h, const WidgetState &state, const ThemePalette &palette)
        {
            const Gradient4 &grad = (state.value != 0.0) ? palette.down_grad : (state.hovered ? palette.hover_grad : palette.btn_grad);
            draw_box(x, y, w, h, grad, palette.btn_frame, 2, state.active, palette);
            if (state.focused)
            {
                //draw_focus_ring(x, y, w, h, palette.bg_main, palette.metrics.focus_ring_opacity, palette.metrics.focus_ring_width, palette.metrics.radius);
            }
        }

        inline void draw_choice(int x, int y, int w, int h, const WidgetState &state, bool is_pressed, const ThemePalette &palette)
        {
            Fl_Color draw_c = is_pressed ? palette.bg_sec : palette.bg_main;
            const Gradient4 *grad = is_pressed ? &palette.down_grad : (state.hovered && state.active ? &palette.hover_grad : &palette.btn_grad);
            if (state.hovered && state.active && !is_pressed)
                draw_c = palette.bg_sec;

            draw_box(x, y, w, h, *grad, palette.btn_frame, 2, state.active, palette);

            int div_x = x + w - palette.metrics.choice_arrow_padding * 2 - palette.metrics.choice_arrow_size;
            fl_color(fl_color_average(palette.btn_frame.out_top, draw_c, 0.6f));
            fl_yxline(div_x, y + 4, y + h - 5);

            Fl_Color arrow_c = is_pressed ? palette.selection : (state.hovered ? palette.fg_main : palette.inactive);
            fl_color(get_text_color(state.active, arrow_c, draw_c, palette.metrics.widget_disabled_label_factor));

            int ax = div_x + palette.metrics.choice_arrow_padding + (palette.metrics.choice_arrow_size / 2);
            int ay = y + h / 2;
            int aw = palette.metrics.choice_arrow_size / 2;
            int ah = palette.metrics.choice_arrow_height / 2;

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