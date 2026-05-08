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
            if (damage() & ~FL_DAMAGE_CHILD)
            {
                fl_draw_box(Theme::schemes::ROUNDED_INPUT_THIN_DOWN_BOX, x(), y(), w(), h(), color());
            }

            Fl_Spinner::draw();

            const auto &palette = mui::ThemeManager::get_palette();

            int cx = up_button_.x() + up_button_.w() / 2;
            int cy = up_button_.y() + up_button_.h() / 2;
            Fl_Color c = up_button_.value() ? palette.selection : palette.spinner_arrow;
            core::draw_spinner_arrow(cx, cy, palette.metrics.spinner_arrow_size, true, active_r() ? c : fl_inactive(c));

            cx = down_button_.x() + down_button_.w() / 2;
            cy = down_button_.y() + down_button_.h() / 2;
            c = down_button_.value() ? palette.selection : palette.spinner_arrow;
            core::draw_spinner_arrow(cx, cy, palette.metrics.spinner_arrow_size, false, active_r() ? c : fl_inactive(c));
        }

    public:
        Spinner(int x, int y, int w, int h, const char *label = nullptr)
            : policy::HoverTracker<policy::CallbackRouter<Fl_Spinner>>(x, y, w, h, label)
        {
            box(FL_NO_BOX);
            color(mui::ThemeManager::get_palette().input_bg);
            selection_color(mui::ThemeManager::get_palette().selection);

            input_.box(FL_FLAT_BOX);
            input_.color(mui::ThemeManager::get_palette().input_bg);
            input_.selection_color(mui::ThemeManager::get_palette().selection);

            up_button_.box(FL_NO_BOX);
            down_button_.box(FL_NO_BOX);

            up_button_.color(mui::ThemeManager::get_palette().input_bg);
            down_button_.color(mui::ThemeManager::get_palette().input_bg);

            up_button_.label("");
            down_button_.label("");
            resize(x, y, w, h);
        }
        void resize(int X, int Y, int W, int H)
        {
            Fl_Group::resize(X, Y, W, H);
            int wpad = 4, hpad = 2;
            X += wpad;
            Y += hpad;
            W -= wpad << 1;
            H -= hpad << 1;
            input_.resize(X, Y, W - H / 2 - 2, H);
            up_button_.resize(X + W - H / 2 - 2, Y, H / 2 + 2, H / 2);
            down_button_.resize(X + W - H / 2 - 2, Y + H - H / 2,
                                H / 2 + 2, H / 2);
        }
    };
}