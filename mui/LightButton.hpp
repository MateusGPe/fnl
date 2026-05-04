#pragma once
#include "Theme.hpp"
#include "Policies.hpp"
#include <FL/Fl_Light_Button.H>
#include <FL/fl_draw.H>

namespace mui
{
    class LightButton : public policy::HoverTracker<
                                    policy::CallbackRouter<Fl_Light_Button>>
    {
    protected:
        void draw() override
        {
            fl_push_clip(x(), y(), w(), h());
            Fl_Boxtype draw_b = value() ? down_box() : box();
            Fl_Color draw_c = value() ? selection_color() : color();

            // Apply procedural hover box if available, otherwise fallback to semantic color
            if (!value() && is_hovered && active_r())
            {
                Fl_Boxtype hover_b = this->resolve_hover_box(box());
                if (hover_b != box())
                {
                    draw_b = hover_b;
                }
                else
                {
                    // Fallback for flat semantic themes
                    draw_c = mui::ThemeManager::get_palette().bg_sec;
                }
            }

            // Explicit coordinate bounds maintained to prevent rendering artifacts
            draw_box(draw_b, x(), y(), w(), h(), draw_c);

            fl_color(active_r() ? labelcolor() : fl_inactive(labelcolor()));
            draw_label();
            fl_pop_clip();
        }

    public:
        template <typename... Args>
        LightButton(int x, int y, int w, int h, const char *l = nullptr, Args &&...args)
            : policy::HoverTracker<policy::CallbackRouter<Fl_Light_Button>>(x, y, w, h, l, std::forward<Args>(args)...)
        {
            align(FL_ALIGN_CENTER);
            box(FL_GTK_UP_BOX);
            down_box(Theme::schemes::BUTTON_DOWN_BOX);
            color(mui::ThemeManager::get_palette().bg_main);
            selection_color(mui::ThemeManager::get_palette().selection);
            labelcolor(mui::ThemeManager::get_palette().fg_main);
        }
    };
}
