#pragma once
#include "Policies.hpp"
#include <FL/Fl_Spinner.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Repeat_Button.H>
#include <utility>

namespace mui
{
    class Spinner : public policy::HoverTracker<policy::CallbackRouter<Fl_Spinner>>
    {
    protected:
        void draw() override
        {
            const auto &palette = ThemeManager::get_palette();

            input_.color(palette.input_bg);
            input_.selection_color(palette.selection);
            input_.textcolor(palette.fg_main);

            if (damage() & ~FL_DAMAGE_CHILD)
                fl_draw_box(Theme::schemes::ROUNDED_INPUT_THIN_DOWN_BOX,
                            x(), y(), w(), h(), palette.input_bg);

            fl_push_clip(input_.x(), input_.y(), input_.w(), input_.h());
            draw_child(input_);
            fl_pop_clip();

            const int btn_x = up_button_.x();
            const int btn_y = up_button_.y();
            const int btn_w = up_button_.w();
            const int btn_h = up_button_.h() + down_button_.h();

            engine::WidgetState btn_state;
            btn_state.active = active_r();
            btn_state.hovered = is_hovered;
            btn_state.focused = (Fl::focus() == &up_button_ || Fl::focus() == &down_button_);
            btn_state.value = up_button_.value() || down_button_.value();

            engine::draw_button_right(btn_x, btn_y, btn_w, btn_h, btn_state, palette);

            fl_color(fl_color_average(palette.btn_frame.out_top, palette.input_bg, 0.5f));
            fl_line(btn_x, up_button_.y() + up_button_.h(), btn_x + btn_w - 1, up_button_.y() + up_button_.h());

            draw_arrow(up_button_, true, palette);
            draw_arrow(down_button_, false, palette);

            const bool is_focused = (Fl::focus() == this || contains(Fl::focus()));
            if (is_focused)
            {
                engine::draw_focus_ring(x(), y(), w(), h(),
                                        palette.focus_ring, palette.metrics.focus_ring_opacity,
                                        palette.metrics.focus_ring_width, palette.metrics.radius);

                // Draw separator line on focus, on the edge of the button
                fl_line_style(FL_SOLID, palette.metrics.focus_ring_width);
                fl_color(palette.focus_ring);
                const int sep_x = up_button_.x();
                fl_line(sep_x, y() + 2, sep_x, y() + h() - 2);
                fl_line_style(0);
            }
            if (this->is_hovered)
            {
                engine::draw_focus_ring(x() + 1, y() + 1, w() - 2, h() - 2, palette.selection,
                                        palette.metrics.focus_ring_opacity,
                                        palette.metrics.focus_ring_width,
                                        palette.metrics.radius);

                // Draw separator line on hover, in the gap
                fl_line_style(FL_SOLID, palette.metrics.focus_ring_width);
                fl_color(palette.selection);
                const int sep_x = up_button_.x();
                fl_line(sep_x - 1, y() + 2, sep_x - 1, y() + h() - 2);
                fl_line_style(0);
            }
        }

    private:
        void draw_arrow(const Fl_Repeat_Button &btn, bool is_up,
                        const ThemePalette &palette) const
        {
            const int cx = btn.x() + btn.w() / 2;
            const int cy = btn.y() + btn.h() / 2;
            const Fl_Color base = btn.value() ? palette.focus_ring : palette.spinner_arrow;
            const Fl_Color c = policy::resolve_color_active(this, base);
            core::draw_spinner_arrow(cx, cy, palette.metrics.spinner_arrow_size, is_up, c);
        }

    public:
        Spinner(int x, int y, int w, int h, const char *label = nullptr)
            : policy::HoverTracker<policy::CallbackRouter<Fl_Spinner>>(x, y, w, h, label)
        {
            box(FL_NO_BOX);
            input_.box(FL_FLAT_BOX);
            up_button_.box(FL_NO_BOX);
            down_button_.box(FL_NO_BOX);
            up_button_.label("");
            down_button_.label("");
            resize(x, y, w, h);
        }

        void resize(int X, int Y, int W, int H) override
        {
            Fl_Group::resize(X, Y, W, H);

            constexpr int wpad = 4, hpad = 2;
            constexpr int gap = 2;

            const int btn_w = 2 * H / 3;
            const int btn_x = X + W - btn_w;
            const int btn_h_half = H / 2;

            input_.resize(X + wpad, Y + hpad, W - btn_w - gap - wpad, H - hpad * 2);

            up_button_.resize(btn_x, Y, btn_w, btn_h_half);
            down_button_.resize(btn_x, Y + H - btn_h_half, btn_w, btn_h_half);
        }
    };
}
