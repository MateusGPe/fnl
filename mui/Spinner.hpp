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

            color(palette.input_bg);
            selection_color(palette.selection);
            labelcolor(palette.fg_main);
            input_.color(palette.input_bg);
            input_.selection_color(palette.selection);
            input_.textcolor(palette.fg_main);
            up_button_.color(palette.input_bg);
            down_button_.color(palette.input_bg);

            if (damage() & ~FL_DAMAGE_CHILD)
                fl_draw_box(Theme::schemes::ROUNDED_INPUT_THIN_DOWN_BOX,
                            x(), y(), w(), h(), color());

            Fl_Spinner::draw();

            draw_arrow(up_button_, true, palette);
            draw_arrow(down_button_, false, palette);

            if (Fl::focus() == this || contains(Fl::focus()))
            {
                engine::draw_focus_ring(x(), y(), w(), h(),
                                        palette.focus_ring, palette.metrics.focus_ring_opacity,
                                        palette.metrics.focus_ring_width, palette.metrics.radius);
            }
        }

    private:
        void draw_arrow(const Fl_Repeat_Button &btn, bool is_up,
                        const ThemePalette &palette) const
        {
            const int cx = btn.x() + btn.w() / 2;
            const int cy = btn.y() + btn.h() / 2;
            const Fl_Color base = btn.value() ? palette.focus_ring : palette.spinner_arrow;
            const Fl_Color c = active_r() ? base : fl_inactive(base);
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
            const int ix = X + wpad;
            const int iy = Y + hpad;
            const int iw = W - wpad * 2;
            const int ih = H - hpad * 2;

            const int btn_w = ih / 2 + 2;

            input_.resize(ix, iy, iw - btn_w - 2, ih);
            up_button_.resize(ix + iw - btn_w - 2, iy, btn_w, ih / 2);
            down_button_.resize(ix + iw - btn_w - 2, iy + ih - ih / 2, btn_w, ih / 2);
        }
    };
}
