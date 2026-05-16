#pragma once
#include "Policies.hpp"

#include <FL/Fl_Value_Input.H>

namespace mui
{
    using namespace policy;
    using ValueInput_ = HoverTracker<CallbackRouter<Fl_Value_Input>>;

    class ValueInput : public ValueInput_
    {
    protected:
        void draw() override
        {
            const auto &palette = ThemeManager::get_palette();
            const int X = x(), Y = y(), W = w(), H = h();
            const bool is_focused = (Fl::focus() == this || Fl::focus() == &input) && visible_focus() && Fl::visible_focus();
            const int Xb = X + Fl::box_dx(box()), Yb = Y + Fl::box_dy(box()), Wb = W - Fl::box_dw(box()), Hb = H - Fl::box_dh(box());

            input.color(palette.input_bg);
            input.selection_color(palette.selection);
            input.textcolor(palette.fg_main);
            draw_box(box(), X, Y, W, H, color());

            if (is_focused)
            {
                fl_push_clip(Xb, Yb, Wb, Hb);
                static_cast<Fl_Widget *>(&input)->draw();
                fl_pop_clip();
            }
            else
            {
                char value_buf[64];
                format(value_buf);

                const double val = core::normalize_value(value(), minimum(), maximum());
                const int fill_w = static_cast<int>(val * Wb);
                const Fl_Color base_text_color = policy::resolve_color_active(this, textcolor());

                if (fill_w > (palette.metrics.radius<<1))
                {
                    const Fl_Color underlay_color = lerp_rgb(palette.inactive, palette.selection, palette.focus_ring, val);
                    const Fl_Color active_underlay_color = policy::resolve_color_active(this, underlay_color);

                    fl_color(active_underlay_color);
                    if (palette.metrics.radius > 0)
                        fl_rounded_rectf(Xb, Yb, fill_w, Hb, palette.metrics.radius);
                    else
                        fl_rectf(Xb, Yb, fill_w, Hb);

                    fl_push_clip(Xb, Yb, fill_w, Hb);
                    fl_color(fl_contrast(base_text_color, active_underlay_color));
                    fl_font(textfont(), textsize());
                    fl_draw(value_buf, Xb, Yb, Wb, Hb, FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
                    fl_pop_clip();
                }

                if (fill_w < Wb)
                {
                    fl_push_clip(Xb + fill_w, Yb, Wb - fill_w, Hb);
                    fl_color(base_text_color);
                    fl_font(textfont(), textsize());
                    fl_draw(value_buf, Xb, Yb, Wb, Hb, FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
                    fl_pop_clip();
                }
            }

            engine::draw_ring(this, is_hovered, is_focused);
        }

    public:
        template <typename... Args>
        explicit ValueInput(Args &&...args) : ValueInput_(std::forward<Args>(args)...)
        {
            box(Theme::schemes::ROUNDED_INPUT_THIN_DOWN_BOX);
            input.box(Theme::schemes::ROUNDED_INPUT_THIN_DOWN_BOX);
        }
    };
}