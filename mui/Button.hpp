#pragma once
#include "Policies.hpp"
#include <FL/Fl_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Radio_Button.H>
#include <FL/fl_draw.H>
#include "Theme.hpp"

#include <type_traits>
#include <utility>

namespace mui
{
    using namespace policy;

    template <typename FlBase>
    class StandardButtonDraw : public FlBase
    {
    protected:
        void draw() override
        {
            const auto &palette = ThemeManager::get_palette();

            this->color(palette.bg_main);
            this->selection_color(palette.selection);
            this->labelcolor(palette.fg_main);

            fl_push_clip(this->x(), this->y(), this->w(), this->h());

            Fl_Boxtype draw_b = this->value() ? this->down_box() : this->box();
            Fl_Color draw_c = this->value() ? this->selection_color() : this->color();

            if constexpr (requires { this->is_hovered; })
            {
                if (!this->value() && this->is_hovered && this->active_r())
                {
                    const Fl_Boxtype hb = this->resolve_hover_box(this->box());
                    if (hb != this->box())
                        draw_b = hb;
                }
            }

            fl_draw_box(FL_FLAT_BOX, this->x(), this->y(), this->w(), this->h(), palette.bg_main);
            fl_draw_box(draw_b, this->x(), this->y(), this->w(), this->h(), draw_c);

            if constexpr (requires { this->is_hovered; })
            {
                if constexpr (std::is_base_of_v<Fl_Light_Button, FlBase> ||
                              std::is_base_of_v<Fl_Radio_Button, FlBase>)
                {
                    if (this->is_hovered && this->value())
                    {
                        unsigned char r, g, b;
                        Fl::get_color(palette.focus_ring, r, g, b);
                        fl_color(rgba(r, g, b,
                                      static_cast<uint8_t>(palette.metrics.focus_ring_opacity * 255.0f)));
                        fl_line_style(FL_SOLID, palette.metrics.focus_ring_width);
                        if (palette.metrics.radius > 0)
                            fl_rounded_rect(this->x(), this->y(), this->w(), this->h(), palette.metrics.radius);
                        else
                            fl_rect(this->x(), this->y(), this->w(), this->h());
                        fl_line_style(0);
                    }
                }
            }

            fl_color(this->active_r() ? this->labelcolor()
                                      : fl_inactive(this->labelcolor()));
            this->draw_label();

            fl_pop_clip();
        }

    public:
        template <typename... Args>
        explicit StandardButtonDraw(Args &&...args)
            : FlBase(std::forward<Args>(args)...) {}

        template <typename T, void (T::*Method)()>
        StandardButtonDraw &on_click(T *instance)
        {
            this->template bind_callback<T, Method>(instance);
            return *this;
        }

        StandardButtonDraw &on_click(std::function<void()> handler)
        {
            this->bind_callback(std::move(handler));
            return *this;
        }
    };

    using Button = StandardButtonDraw<HoverTracker<AutoThemed<CallbackRouter<Fl_Button>>>>;
    using LightButton = StandardButtonDraw<HoverTracker<AutoThemed<CallbackRouter<Fl_Light_Button>>>>;
    using RadioButton = StandardButtonDraw<HoverTracker<AutoThemed<CallbackRouter<Fl_Radio_Button>>>>;
    using ReturnButton = StandardButtonDraw<HoverTracker<AutoThemed<CallbackRouter<Fl_Return_Button>>>>;

}
