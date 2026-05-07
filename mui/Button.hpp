#pragma once
#include "Policies.hpp"
#include <FL/Fl_Button.H>
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
            fl_push_clip(this->x(), this->y(), this->w(), this->h());

            Fl_Boxtype draw_b = this->value() ? this->down_box() : this->box();
            Fl_Color draw_c = this->value() ? this->selection_color() : this->color();

            if constexpr (requires { this->is_hovered; })
            {
                if (!this->value() && this->is_hovered && this->active_r())
                {
                    Fl_Boxtype hover_b = this->resolve_hover_box(this->box());
                    if (hover_b != this->box())
                    {
                        draw_b = hover_b;
                    }
                    else
                    {
                        draw_c = mui::ThemeManager::get_palette().bg_sec;
                    }
                }
            }

            fl_draw_box(draw_b, this->x(), this->y(), this->w(), this->h(), draw_c);

            fl_color(this->active_r() ? this->labelcolor() : fl_inactive(this->labelcolor()));
            this->draw_label();

            fl_pop_clip();
        }

    public:
        template <typename... Args>
        StandardButtonDraw(Args &&...args) : FlBase(std::forward<Args>(args)...) {}
    };

    using Button = StandardButtonDraw<HoverTracker<AutoThemed<CallbackRouter<Fl_Button>>>>;
    using LightButton = StandardButtonDraw<HoverTracker<AutoThemed<CallbackRouter<Fl_Light_Button>>>>;
    using ReturnButton = StandardButtonDraw<HoverTracker<AutoThemed<CallbackRouter<Fl_Return_Button>>>>;
}