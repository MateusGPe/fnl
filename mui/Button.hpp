#pragma once
#include "Policies.hpp"
#include <FL/Fl_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Radio_Button.H>
#include <utility>
#include <type_traits>
#include <FL/fl_draw.H>
#include "Theme.hpp"
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
            const bool has_focus = Fl::focus() == this;
            auto palette = mui::ThemeManager::get_palette();

            if constexpr (requires { this->is_hovered; })
            {
                if (!this->value() && this->is_hovered && this->active_r())
                {
                    Fl_Boxtype hover_b = this->resolve_hover_box(this->box());
                    if (hover_b != this->box())
                    {
                        draw_b = hover_b;
                    }
                }
            }

            fl_draw_box(FL_FLAT_BOX, this->x(), this->y(), this->w(), this->h(), palette.bg_main);
            fl_draw_box(draw_b, this->x(), this->y(), this->w(), this->h(), draw_c);

            if constexpr (requires { this->is_hovered; })
            {
                if constexpr (std::is_base_of_v<Fl_Light_Button, FlBase> || std::is_base_of_v<Fl_Radio_Button, FlBase>)
                {
                    if (this->is_hovered && this->value())
                    {
                        uint8_t r, g, b;
                        Fl::get_color(palette.selection, r, g, b);
                        fl_color(rgba(r, g, b, static_cast<uint8_t>(palette.metrics.focus_ring_opacity * 255)));

                        fl_line_style(FL_SOLID, palette.metrics.focus_ring_width);
                        if (palette.metrics.radius > 0)
                        {
                            fl_rounded_rect(this->x(), this->y(), this->w(), this->h(), palette.metrics.radius);
                        }
                        else
                        {
                            fl_rect(this->x(), this->y(), this->w(), this->h());
                        }
                        fl_line_style(0);
                    }
                }
            }
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
    using RadioButton = StandardButtonDraw<HoverTracker<AutoThemed<CallbackRouter<Fl_Radio_Button>>>>;
    using ReturnButton = StandardButtonDraw<HoverTracker<AutoThemed<CallbackRouter<Fl_Return_Button>>>>;
}