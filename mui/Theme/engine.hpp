// Theme/engine.hpp
#pragma once
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <algorithm>

#include "data.hpp"
#include "primitives.hpp"

namespace mui
{
    namespace Theme
    {
        struct CornerRadii
        {
            int top_left = 0;
            int top_right = 0;
            int bottom_right = 0;
            int bottom_left = 0;
        };
    }

    namespace engine
    {
        struct WidgetState
        {
            bool active = true;
            bool hovered = false;
            bool focused = false;
            double value = 0.0;
        };

        // ---------------------------------------------------------
        // The OOP Interface (Replaces ThemeDrawInterface struct)
        // ---------------------------------------------------------
        class IThemeRenderer
        {
        public:
            virtual ~IThemeRenderer() = default;

            // Core Box & Frame Primitives
            virtual void draw_frame(int x, int y, int w, int h, const core::FrameColors &fc, bool active, const ThemePalette &palette) const = 0;
            virtual void draw_static_frame(int x, int y, int w, int h, const core::FrameColors &fc, bool active, const ThemePalette &palette) const = 0;
            virtual void draw_box(int x, int y, int w, int h, const Gradient4 &grad, const core::FrameColors &fc, int inset, bool active, const ThemePalette &palette) const = 0;

            // Widget Primitives
            virtual void draw_button(int x, int y, int w, int h, const WidgetState &state, const ThemePalette &palette) const = 0;
            virtual void draw_checkbox(int x, int y, int size, const WidgetState &state, const ThemePalette &palette) const = 0;
            virtual void draw_radio_button(int x, int y, int size, const WidgetState &state, const ThemePalette &palette) const = 0;
            virtual void draw_slider(int x, int y, int w, int h, const WidgetState &state, const ThemePalette &palette) const = 0;
            virtual void draw_spinner(int x, int y, int w, int h, const WidgetState &state, const ThemePalette &palette) const = 0;
            virtual void draw_choice(int x, int y, int w, int h, const WidgetState &state, bool is_pressed, const ThemePalette &palette) const = 0;
            virtual void draw_tabs(int x, int y, int w, int h, const WidgetState &state, const ThemePalette &palette) const = 0;
            virtual void draw_tab_item(int x, int y, int w, int h, const WidgetState &state, const ThemePalette &palette) const = 0;
            virtual void draw_progress(int x, int y, int w, int h, const WidgetState &state, const ThemePalette &palette) const = 0;
            virtual void draw_image_viewer(int x, int y, int w, int h, const WidgetState &state, const ThemePalette &palette) const = 0;
            virtual void draw_toggle(int x, int y, int w, int h, const WidgetState &state, const ThemePalette &palette) const = 0;
            virtual void draw_output(int x, int y, int w, int h, const WidgetState &state, const ThemePalette &palette) const = 0;
            virtual void draw_multiline_output(int x, int y, int w, int h, const WidgetState &state, const ThemePalette &palette) const = 0;
            virtual void draw_menubar(int x, int y, int w, int h, const WidgetState &state, const ThemePalette &palette) const = 0;
            virtual void draw_value_slider(int x, int y, int w, int h, const WidgetState &state, const ThemePalette &palette) const = 0;
            virtual void draw_input(int x, int y, int w, int h, const WidgetState &state, const ThemePalette &palette) const = 0;
            virtual void draw_counter(int x, int y, int w, int h, const WidgetState &state, const ThemePalette &palette) const = 0;
        };

        // ---------------------------------------------------------
        // Base Renderer (Implements common logic for DRYness)
        // ---------------------------------------------------------
        class BaseRenderer : public IThemeRenderer
        {
        protected:
            // Encapsulated Utilities
            struct Rgb
            {
                uint8_t r, g, b;
            };

            static Fl_Color rgba_to_fl_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0)
            {
                return ((r & 0xff) << 24) | ((g & 0xff) << 16) | ((b & 0xff) << 8) | (a & 0xff);
            }

            static Rgb get_rgb(Fl_Color col)
            {
                uint8_t r, g, b;
                Fl::get_color(col, r, g, b);
                return {r, g, b};
            }

            static Fl_Color get_text_color(bool active, Fl_Color active_color, Fl_Color parent_bg_color, float disabled_factor)
            {
                Fl_Color contrasted = fl_contrast(active_color, parent_bg_color);
                return active ? contrasted : fl_color_average(contrasted, parent_bg_color, disabled_factor);
            }

            void draw_focus_ring(int x, int y, int w, int h, Fl_Color ring_color, float opacity, int width, int radius) const
            {
                auto [r, g, b] = get_rgb(ring_color);
                fl_color(rgba_to_fl_color(r, g, b, static_cast<uint8_t>(opacity * 255)));
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

        public:
            void draw_button(int x, int y, int w, int h, const WidgetState &state, const ThemePalette &palette) const override
            {
                const Gradient4 &grad = (state.value != 0.0) ? palette.down_grad : (state.hovered ? palette.hover_grad : palette.btn_grad);
                this->draw_box(x, y, w, h, grad, palette.btn_frame, 2, state.active, palette);
                if (state.focused)
                {
                    draw_focus_ring(x, y, w, h, palette.bg_main, palette.metrics.focus_ring_opacity, palette.metrics.focus_ring_width, palette.metrics.radius);
                }
            }

            void draw_checkbox(int x, int y, int size, const WidgetState &state, const ThemePalette &palette) const override
            {
                Fl_Color bg = (state.value != 0.0) ? palette.selection : palette.input_bg;
                if (!state.active)
                    bg = fl_color_average(bg, palette.bg_main, 0.5f);

                fl_color(bg);
                fl_rounded_rectf(x, y, size, size, palette.metrics.checkbox_corner_radius);

                if (state.hovered && state.active)
                {
                    auto [r, g, b] = get_rgb(palette.selection);
                    fl_color(rgba_to_fl_color(r, g, b, static_cast<uint8_t>(palette.metrics.state_hover_opacity * 255)));
                    fl_rounded_rectf(x, y, size, size, palette.metrics.checkbox_corner_radius);
                }

                fl_color((state.value != 0.0) ? bg : palette.toggle_border);
                fl_rounded_rect(x, y, size, size, palette.metrics.checkbox_corner_radius);

                if (state.focused)
                {
                    int pad = palette.metrics.checkbox_focus_padding;
                    draw_focus_ring(x - pad, y - pad, size + pad * 2, size + pad * 2, palette.focus_ring, palette.metrics.focus_ring_opacity, palette.metrics.checkbox_focus_width, palette.metrics.checkbox_corner_radius + 1);
                }

                if (state.value != 0.0)
                {
                    Fl_Color tick_c = state.active ? palette.toggle_tick : fl_color_average(palette.toggle_tick, palette.bg_main, palette.metrics.widget_disabled_label_factor);
                    fl_color(tick_c);
                    fl_line_style(FL_SOLID, palette.metrics.checkbox_tick_width);
                    fl_begin_line();
                    fl_vertex(x + palette.metrics.checkbox_tick_padding, y + size / 2);
                    fl_vertex(x + size / 2 - 1, y + size - palette.metrics.checkbox_tick_padding);
                    fl_vertex(x + size - palette.metrics.checkbox_tick_padding, y + palette.metrics.checkbox_tick_padding);
                    fl_end_line();
                    fl_line_style(0);
                }
            }

            void draw_radio_button(int x, int y, int size, const WidgetState &state, const ThemePalette &palette) const override
            {
                fl_color(palette.input_bg);
                fl_pie(x, y, size, size, 0, 360);

                if (state.hovered && state.active)
                {
                    auto [r, g, b] = get_rgb(palette.selection);
                    fl_color(rgba_to_fl_color(r, g, b, static_cast<uint8_t>(palette.metrics.state_hover_opacity * 255)));
                    fl_pie(x, y, size, size, 0, 360);
                }

                fl_color(state.value != 0.0 ? palette.selection : palette.toggle_border);
                fl_line_style(FL_SOLID, palette.metrics.radio_border_width);
                fl_arc(x, y, size, size, 0, 360);
                fl_line_style(0);

                if (state.focused)
                {
                    int f_size = size + (palette.metrics.radio_focus_padding * 2);
                    draw_focus_ring(x + size / 2 - f_size / 2, y + size / 2 - f_size / 2, f_size, f_size, palette.focus_ring, palette.metrics.focus_ring_opacity, 0, f_size / 2);
                }

                if (state.value != 0.0)
                {
                    Fl_Color tick_c = state.active ? palette.toggle_tick : fl_color_average(palette.toggle_tick, palette.bg_main, palette.metrics.widget_disabled_label_factor);
                    fl_color(tick_c);
                    int d_size = palette.metrics.radio_dot_size;
                    fl_pie(x + (size - d_size) / 2, y + (size - d_size) / 2, d_size, d_size, 0, 360);
                }
            }

            void draw_slider(int x, int y, int w, int h, const WidgetState &state, const ThemePalette &palette) const override
            {
                bool is_horiz = (w > h);
                int t_size = palette.metrics.slider_thumb_size;
                int halo = palette.metrics.slider_thumb_focus_halo_size;
                int bleed = (halo > t_size) ? (halo - t_size) / 2 : 0;
                int track_h = palette.metrics.slider_track_height;

                int tx, ty, tw, th, cx, cy;

                if (is_horiz)
                {
                    tx = x + bleed;
                    ty = y + (h - track_h) / 2;
                    tw = w - (bleed * 2);
                    th = track_h;
                    cx = tx + static_cast<int>(state.value * (tw - t_size)) + t_size / 2;
                    cy = y + h / 2;
                }
                else
                {
                    tx = x + (w - track_h) / 2;
                    ty = y + bleed;
                    tw = track_h;
                    th = h - (bleed * 2);
                    cx = x + w / 2;
                    cy = ty + static_cast<int>((1.0 - state.value) * (th - t_size)) + t_size / 2;
                }

                // Track
                fl_color(palette.slider_track);
                fl_rounded_rectf(tx, ty, tw, th, track_h / 2);
                fl_color(palette.selection);
                if (is_horiz)
                {
                    fl_push_clip(tx, ty, cx - tx, th);
                    fl_rounded_rectf(tx, ty, tw, th, track_h / 2);
                    fl_pop_clip();
                }
                else
                {
                    fl_push_clip(tx, cy, tw, (ty + th) - cy);
                    fl_rounded_rectf(tx, ty, tw, th, track_h / 2);
                    fl_pop_clip();
                }

                // Halo
                if (state.focused || state.hovered)
                {
                    float opacity = state.focused ? palette.metrics.focus_ring_opacity : palette.metrics.slider_thumb_hover_halo_opacity;
                    draw_focus_ring(cx - halo / 2, cy - halo / 2, halo, halo, palette.focus_ring, opacity, 0, halo / 2);
                }

                // Thumb
                fl_color(palette.selection);
                fl_pie(cx - t_size / 2, cy - t_size / 2, t_size, t_size, 0, 360);
                if (palette.style != BoxStyle::Solid)
                {
                    fl_color(palette.hover_grad.t_start);
                    fl_arc(cx - t_size / 2, cy - t_size / 2, t_size, t_size, 45, 225);
                }
            }

            void draw_spinner(int x, int y, int w, int h, const WidgetState &state, const ThemePalette &palette) const override
            {
                Fl_Color bg_col = (state.hovered && !state.focused) ? palette.bg_sec : palette.bg_main;
                fl_color(bg_col);
                fl_rounded_rectf(x, y, w, h, palette.metrics.radius);

                Fl_Color border_col = state.focused ? palette.selection : (state.hovered ? palette.selection : palette.btn_frame.out_top);
                int border_w = state.focused ? palette.metrics.input_focused_border_width : palette.metrics.input_default_border_width;
                fl_color(border_col);
                fl_line_style(FL_SOLID, border_w);
                int inset = state.focused ? palette.metrics.input_focused_inset : 0;
                fl_rounded_rect(x + inset, y + inset, w - (inset * 2), h - (inset * 2), palette.metrics.radius);
                fl_line_style(0);

                const int btn_w = palette.metrics.spinner_button_width;
                int client_inset = state.focused ? (inset + 1) : border_w;
                int btn_x = x + w - btn_w;

                fl_color(palette.btn_frame.out_top);
                fl_line(btn_x, y + client_inset, btn_x, y + h - client_inset - 1);
                fl_line(btn_x, y + h / 2, x + w - client_inset, y + h / 2);

                const int mid_x = x + w - btn_w / 2;
                const int up_cy = y + h / 4;
                const int down_cy = y + h * 3 / 4;
                Fl_Color arrow_col = state.active ? palette.spinner_arrow : fl_color_average(palette.inactive, bg_col, 0.5);

                fl_color(arrow_col);
                int s_size = palette.metrics.spinner_arrow_size / 2;
                fl_line_style(FL_SOLID, palette.metrics.spinner_arrow_thickness);
                fl_begin_line();
                fl_vertex(mid_x - s_size, up_cy + s_size / 2);
                fl_vertex(mid_x, up_cy - s_size / 2);
                fl_vertex(mid_x + s_size, up_cy + s_size / 2);
                fl_end_line();
                fl_begin_line();
                fl_vertex(mid_x - s_size, down_cy - s_size / 2);
                fl_vertex(mid_x, down_cy + s_size / 2);
                fl_vertex(mid_x + s_size, down_cy - s_size / 2);
                fl_end_line();
                fl_line_style(0);
            }

            void draw_choice(int x, int y, int w, int h, const WidgetState &state, bool is_pressed, const ThemePalette &palette) const override
            {
                Fl_Color draw_c = is_pressed ? palette.bg_sec : palette.bg_main;
                const Gradient4 *grad = is_pressed ? &palette.down_grad : (state.hovered && state.active ? &palette.hover_grad : &palette.btn_grad);
                if (state.hovered && state.active && !is_pressed)
                    draw_c = palette.bg_sec;

                this->draw_box(x, y, w, h, *grad, palette.btn_frame, 2, state.active, palette);

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

            void draw_tabs(int x, int y, int w, int h, const WidgetState &state, const ThemePalette &palette) const override
            {
                int th = palette.metrics.tab_height;
                fl_color(palette.btn_frame.out_top);
                fl_line_style(0);
                fl_line(x, y + th - 1, x + w, y + th - 1);
            }

            void draw_tab_item(int x, int y, int w, int h, const WidgetState &state, const ThemePalette &palette) const override
            {
                fl_color(palette.bg_main);
                fl_rectf(x, y, w, h);
                if (state.value != 0.0)
                {
                    fl_color(palette.selection);
                    fl_line_style(FL_SOLID, palette.metrics.tab_active_line_thickness);
                    fl_line(x, y + h - 1, x + w, y + h - 1);
                    fl_line_style(0);
                }
            }

            void draw_progress(int x, int y, int w, int h, const WidgetState &state, const ThemePalette &palette) const override
            {
                int bar_h = palette.metrics.progress_height;
                int bar_y = y + (h - bar_h) / 2;
                fl_color(palette.bg_sec);
                fl_rounded_rectf(x, bar_y, w, bar_h, palette.metrics.progress_corner_radius);

                int fill_w = static_cast<int>(state.value * w);
                if (fill_w > 0)
                {
                    fl_color(palette.selection);
                    fl_rounded_rectf(x, bar_y, fill_w, bar_h, palette.metrics.progress_corner_radius);
                }
            }

            void draw_toggle(int x, int y, int w, int h, const WidgetState &state, const ThemePalette &palette) const override
            {
                int t_h = std::min(palette.metrics.toggle_track_height, h);
                int t_y = y + (h - t_h) / 2;
                int radius = t_h / 2;

                Fl_Color t_base = state.value != 0 ? palette.selection : palette.bg_sec;
                fl_color(state.hovered && state.active ? fl_color_average(t_base, FL_WHITE, palette.metrics.toggle_track_hover_factor) : t_base);
                fl_rounded_rectf(x, t_y, w, t_h, radius);

                fl_color(state.hovered && state.active ? fl_color_average(palette.btn_frame.out_top, FL_BLACK, palette.metrics.toggle_outline_hover_factor) : palette.btn_frame.out_top);
                fl_rounded_rect(x, t_y, w, t_h, radius);

                int knob_size = std::min(palette.metrics.toggle_knob_size, h);
                int cx = (state.value != 0) ? (x + w - radius) : (x + radius);
                int cy = y + h / 2;

                if (state.focused)
                {
                    int fp = palette.metrics.toggle_focus_padding;
                    draw_focus_ring(cx - (knob_size / 2 + fp), cy - (knob_size / 2 + fp), knob_size + fp * 2, knob_size + fp * 2, palette.selection, palette.metrics.focus_ring_opacity, 0, (knob_size + fp * 2) / 2);
                }

                Fl_Color thumb_c = state.value != 0 ? palette.selection : palette.bg_main;
                fl_color(state.hovered && state.active ? fl_color_average(thumb_c, FL_WHITE, palette.metrics.toggle_thumb_hover_factor) : thumb_c);
                fl_pie(cx - knob_size / 2, cy - knob_size / 2, knob_size, knob_size, 0, 360);
            }

            void draw_input(int x, int y, int w, int h, const WidgetState &state, const ThemePalette &palette) const override
            {
                Fl_Color bg_col = (state.hovered && !state.focused) ? palette.bg_sec : palette.input_bg;
                fl_color(bg_col);
                fl_rounded_rectf(x, y, w, h, palette.metrics.radius);

                Fl_Color border_col = state.focused ? palette.selection : (state.hovered ? palette.selection : palette.btn_frame.out_top);
                int border_w = state.focused ? palette.metrics.input_focused_border_width : palette.metrics.input_default_border_width;
                int inset = state.focused ? palette.metrics.input_focused_inset : 0;

                fl_color(border_col);
                fl_line_style(FL_SOLID, border_w);
                fl_rounded_rect(x + inset, y + inset, w - (inset * 2), h - (inset * 2), palette.metrics.radius);
                fl_line_style(0);
            }

            void draw_output(int x, int y, int w, int h, const WidgetState &state, const ThemePalette &palette) const override
            {
                this->draw_box(x, y, w, h, palette.btn_grad, palette.btn_frame, 2, state.active, palette);
            }
            void draw_multiline_output(int x, int y, int w, int h, const WidgetState &state, const ThemePalette &palette) const override
            {
                this->draw_output(x, y, w, h, state, palette);
            }
            void draw_menubar(int x, int y, int w, int h, const WidgetState &state, const ThemePalette &palette) const override
            {
                this->draw_box(x, y, w, h, palette.def_btn_grad, palette.def_btn_frame, 2, state.active, palette);
            }
            void draw_image_viewer(int, int, int, int, const WidgetState &, const ThemePalette &) const override {}
            void draw_value_slider(int, int, int, int, const WidgetState &, const ThemePalette &) const override {}
            void draw_counter(int, int, int, int, const WidgetState &, const ThemePalette &) const override {}
        };

        // ---------------------------------------------------------
        // Modern Style Renderers
        // ---------------------------------------------------------
        class SolidRenderer : public BaseRenderer
        {
        public:
            void draw_frame(int x, int y, int w, int h, const core::FrameColors &fc, bool active, const ThemePalette &) const override
            {
                core::draw_solid_frame(x, y, w, h, fc, active);
            }
            void draw_static_frame(int x, int y, int w, int h, const core::FrameColors &fc, bool active, const ThemePalette &) const override
            {
                if (fc.in_top == 0)
                    core::draw_solid_frame(x, y, w, h, fc, active);
                else
                    core::draw_beveled_frame(x, y, w, h, fc, active);
            }
            void draw_box(int x, int y, int w, int h, const Gradient4 &grad, const core::FrameColors &fc, int inset, bool active, const ThemePalette &) const override
            {
                core::draw_smart_gradient_4(x + inset, y + inset, w - (inset * 2), h - (inset * 2), grad.t_start, grad.t_end, grad.b_start, grad.b_end, active);
                core::draw_solid_frame(x, y, w, h, fc, active);
            }
        };

        class BeveledRenderer : public BaseRenderer
        {
        public:
            void draw_frame(int x, int y, int w, int h, const core::FrameColors &fc, bool active, const ThemePalette &) const override
            {
                core::draw_beveled_frame(x, y, w, h, fc, active);
            }
            void draw_static_frame(int x, int y, int w, int h, const core::FrameColors &fc, bool active, const ThemePalette &) const override
            {
                if (fc.in_top == 0)
                    core::draw_solid_frame(x, y, w, h, fc, active);
                else
                    core::draw_beveled_frame(x, y, w, h, fc, active);
            }
            void draw_box(int x, int y, int w, int h, const Gradient4 &grad, const core::FrameColors &fc, int inset, bool active, const ThemePalette &) const override
            {
                core::draw_smart_gradient_4(x + inset, y + inset, w - (inset * 2), h - (inset * 2), grad.t_start, grad.t_end, grad.b_start, grad.b_end, active);
                core::draw_beveled_frame(x, y, w, h, fc, active);
            }
        };

        class RoundedArcsRenderer : public BaseRenderer
        {
        public:
            void draw_frame(int x, int y, int w, int h, const core::FrameColors &fc, bool active, const ThemePalette &) const override
            {
                core::draw_rounded_arcs_frame(x, y, w, h, fc, active);
            }
            void draw_static_frame(int x, int y, int w, int h, const core::FrameColors &fc, bool active, const ThemePalette &) const override
            {
                if (fc.in_top == 0)
                    core::draw_solid_frame(x, y, w, h, fc, active);
                else
                    core::draw_beveled_frame(x, y, w, h, fc, active);
            }
            void draw_box(int x, int y, int w, int h, const Gradient4 &grad, const core::FrameColors &fc, int inset, bool active, const ThemePalette &) const override
            {
                core::draw_smart_gradient_4(x + inset, y + inset, w - (inset * 2), h - (inset * 2), grad.t_start, grad.t_end, grad.b_start, grad.b_end, active);
                core::draw_rounded_arcs_frame(x, y, w, h, fc, active);
            }
        };

        // ---------------------------------------------------------
        // Legacy Style Renderers (Classic Windows 95 & XP)
        // ---------------------------------------------------------
        class ClassicRenderer : public BaseRenderer
        {
        public:
            void draw_frame(int x, int y, int w, int h, const core::FrameColors &fc, bool active, const ThemePalette &) const override
            {
                core::draw_classic_frame(x, y, w, h, fc, active);
            }
            void draw_static_frame(int x, int y, int w, int h, const core::FrameColors &fc, bool active, const ThemePalette &) const override
            {
                core::draw_classic_frame(x, y, w, h, fc, active);
            }
            void draw_box(int x, int y, int w, int h, const Gradient4 &grad, const core::FrameColors &fc, int inset, bool active, const ThemePalette &palette) const override
            {
                fl_color(core::activated_color(palette.bg_main, active));
                fl_rectf(x + inset, y + inset, w - (inset * 2), h - (inset * 2));
                core::draw_classic_frame(x, y, w, h, fc, active);
            }

            void draw_checkbox(int x, int y, int size, const WidgetState &state, const ThemePalette &palette) const override
            {
                fl_color(state.value != 0.0 ? palette.selection : palette.input_bg);
                fl_rectf(x + 2, y + 2, size - 4, size - 4);
                fl_color(palette.btn_frame.in_side ? palette.btn_frame.in_side : palette.toggle_border);
                fl_xyline(x, y, x + size - 2);
                fl_yxline(x, y, y + size - 2);
                fl_color(FL_WHITE);
                fl_xyline(x, y + size - 1, x + size - 1);
                fl_yxline(x + size - 1, y, y + size - 1);
                fl_color(FL_BLACK);
                fl_xyline(x + 1, y + 1, x + size - 3);
                fl_yxline(x + 1, y + 1, y + size - 3);
                fl_color(fl_color_average(palette.bg_main, FL_BLACK, 0.2f));
                fl_xyline(x + 1, y + size - 2, x + size - 2);
                fl_yxline(x + size - 2, y + 1, y + size - 2);

                if (state.focused)
                {
                    fl_color(palette.focus_ring);
                    fl_line_style(FL_DOT, 1);
                    fl_rect(x - 2, y - 2, size + 4, size + 4);
                    fl_line_style(0);
                }
                if (state.value != 0.0)
                {
                    fl_color(FL_BLACK);
                    fl_line_style(FL_SOLID, palette.metrics.checkbox_tick_width);
                    fl_begin_line();
                    fl_vertex(x + 3, y + size / 2);
                    fl_vertex(x + size / 2, y + size - 4);
                    fl_vertex(x + size - 3, y + 3);
                    fl_end_line();
                    fl_line_style(0);
                }
            }

            void draw_radio_button(int x, int y, int size, const WidgetState &state, const ThemePalette &palette) const override
            {
                fl_color(palette.input_bg);
                fl_pie(x + 2, y + 2, size - 4, size - 4, 0, 360);
                fl_color(palette.btn_frame.in_side ? palette.btn_frame.in_side : palette.toggle_border);
                fl_arc(x, y, size, size, 45, 225);
                fl_color(FL_WHITE);
                fl_arc(x, y, size, size, 225, 405);
                fl_color(FL_BLACK);
                fl_arc(x + 1, y + 1, size - 2, size - 2, 45, 225);
                fl_color(fl_color_average(palette.bg_main, FL_BLACK, 0.2f));
                fl_arc(x + 1, y + 1, size - 2, size - 2, 225, 405);

                if (state.focused)
                {
                    fl_color(palette.focus_ring);
                    fl_line_style(FL_DOT, 1);
                    fl_rect(x - 2, y - 2, size + 4, size + 4);
                    fl_line_style(0);
                }
                if (state.value != 0.0)
                {
                    fl_color(FL_BLACK);
                    int d_size = palette.metrics.radio_dot_size;
                    fl_pie(x + (size - d_size) / 2, y + (size - d_size) / 2, d_size, d_size, 0, 360);
                }
            }

            void draw_slider(int x, int y, int w, int h, const WidgetState &state, const ThemePalette &palette) const override
            {
                bool is_horiz = (w > h);
                int t_size = palette.metrics.slider_thumb_size;
                int track_h = palette.metrics.slider_track_height;
                int tx, ty, tw, th, t_x, t_y;

                if (is_horiz)
                {
                    tx = x;
                    ty = y + (h - track_h) / 2;
                    tw = w;
                    th = track_h;
                    t_x = tx + static_cast<int>(state.value * (tw - t_size));
                    t_y = y + h / 2 - t_size / 2;
                }
                else
                {
                    tx = x + (w - track_h) / 2;
                    ty = y;
                    tw = track_h;
                    th = h;
                    t_x = x + w / 2 - t_size / 2;
                    t_y = ty + static_cast<int>((1.0 - state.value) * (th - t_size));
                }

                // Track
                fl_color(palette.slider_track);
                fl_rectf(tx, ty, tw, th);
                fl_color(palette.btn_frame.in_side);
                fl_xyline(tx, ty, tx + tw - 1);
                fl_yxline(tx, ty, ty + th - 1);
                fl_color(FL_WHITE);
                fl_xyline(tx, ty + th, tx + tw);
                fl_yxline(tx + tw, ty, ty + th);

                // Thumb
                this->draw_box(t_x, t_y, t_size, t_size, palette.btn_grad, palette.btn_frame, 1, state.active, palette);
            }

            void draw_spinner(int x, int y, int w, int h, const WidgetState &state, const ThemePalette &palette) const override
            {
                BaseRenderer::draw_spinner(x, y, w, h, state, palette); // Call base for box, but override arrow drawing internally if needed, or rely on generalized drawing.
            }
        };

        class WinXPRenderer : public BaseRenderer
        {
        public:
            void draw_frame(int x, int y, int w, int h, const core::FrameColors &fc, bool active, const ThemePalette &) const override
            {
                core::draw_winxp_frame(x, y, w, h, fc, active);
            }
            void draw_static_frame(int x, int y, int w, int h, const core::FrameColors &fc, bool active, const ThemePalette &) const override
            {
                if (fc.in_top == 0)
                    core::draw_solid_frame(x, y, w, h, fc, active);
                else
                    core::draw_winxp_frame(x, y, w, h, fc, active);
            }
            void draw_box(int x, int y, int w, int h, const Gradient4 &grad, const core::FrameColors &fc, int inset, bool active, const ThemePalette &) const override
            {
                core::draw_winxp_box(x, y, w, h, grad.t_start, grad.t_end, grad.b_start, grad.b_end, active);
                core::draw_winxp_frame(x, y, w, h, fc, active);
            }

            void draw_checkbox(int x, int y, int size, const WidgetState &state, const ThemePalette &palette) const override
            {
                Fl_Color xp_border = state.active ? (state.hovered ? fl_color_average(palette.toggle_border, FL_WHITE, 0.8f) : palette.toggle_border)
                                                  : fl_color_average(palette.toggle_border, palette.bg_main, 0.6f);
                fl_color(palette.input_bg);
                fl_rectf(x + 1, y + 1, size - 2, size - 2);
                fl_color(xp_border);
                fl_rect(x, y, size, size);
                fl_color(fl_color_average(palette.bg_main, FL_BLACK, 0.1f));
                fl_xyline(x + 1, y + 1, x + size - 2);
                fl_yxline(x + 1, y + 1, y + size - 2);

                if (state.focused)
                {
                    fl_color(palette.focus_ring);
                    fl_line_style(FL_DOT, 1);
                    fl_rect(x - 2, y - 2, size + 4, size + 4);
                    fl_line_style(0);
                }
                if (state.value != 0.0)
                {
                    fl_color(FL_BLACK); // WinXP tick
                    fl_line_style(FL_SOLID, palette.metrics.checkbox_tick_width);
                    fl_begin_line();
                    fl_vertex(x + 3, y + size / 2);
                    fl_vertex(x + size / 2, y + size - 4);
                    fl_vertex(x + size - 3, y + 3);
                    fl_end_line();
                    fl_line_style(0);
                }
            }

            void draw_radio_button(int x, int y, int size, const WidgetState &state, const ThemePalette &palette) const override
            {
                Fl_Color xp_border = state.active ? (state.hovered ? fl_color_average(palette.toggle_border, FL_WHITE, 0.8f) : palette.toggle_border)
                                                  : fl_color_average(palette.toggle_border, palette.bg_main, 0.6f);
                fl_color(palette.input_bg);
                fl_pie(x + 1, y + 1, size - 2, size - 2, 0, 360);
                fl_color(xp_border);
                fl_arc(x, y, size, size, 0, 360);

                if (state.focused)
                {
                    fl_color(palette.focus_ring);
                    fl_line_style(FL_DOT, 1);
                    fl_rect(x - 2, y - 2, size + 4, size + 4);
                    fl_line_style(0);
                }
                if (state.value != 0.0)
                {
                    fl_color(state.active ? FL_BLACK : fl_color_average(FL_BLACK, palette.bg_main, 0.5f));
                    int d_size = palette.metrics.radio_dot_size;
                    fl_pie(x + (size - d_size) / 2, y + (size - d_size) / 2, d_size, d_size, 0, 360);
                }
            }

            void draw_slider(int x, int y, int w, int h, const WidgetState &state, const ThemePalette &palette) const override
            {
                bool is_horiz = (w > h);
                int t_size = palette.metrics.slider_thumb_size;
                int track_h = palette.metrics.slider_track_height;
                int tx, ty, tw, th, t_x, t_y;

                if (is_horiz)
                {
                    tx = x;
                    ty = y + (h - track_h) / 2;
                    tw = w;
                    th = track_h;
                    t_x = tx + static_cast<int>(state.value * (tw - t_size));
                    t_y = y + h / 2 - t_size / 2;
                }
                else
                {
                    tx = x + (w - track_h) / 2;
                    ty = y;
                    tw = track_h;
                    th = h;
                    t_x = x + w / 2 - t_size / 2;
                    t_y = ty + static_cast<int>((1.0 - state.value) * (th - t_size));
                }

                // Track
                fl_color(FL_WHITE);
                fl_rectf(tx + 1, ty + 1, tw - 2, th - 2);
                fl_color(palette.slider_track);
                fl_rect(tx, ty, tw, th);

                // Thumb
                this->draw_box(t_x, t_y, t_size, t_size, palette.btn_grad, palette.btn_frame, 1, state.active, palette);
            }
        };

    } // namespace engine
} // namespace mui
