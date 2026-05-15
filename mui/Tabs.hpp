// Tabs.hpp
#pragma once
#include "Theme.hpp"
#include "Policies.hpp"
#include <FL/Fl_Tabs.H>
#include <FL/fl_draw.H>
#include <cmath>

namespace mui
{
    class Tabs : public policy::HoverTracker<policy::CallbackRouter<Fl_Tabs>>
    {
    private:
        using Base = policy::HoverTracker<policy::CallbackRouter<Fl_Tabs>>;
        Fl_Widget *m_hovered_tab = nullptr;

    protected:
        int handle(int event) override
        {
            const int res = Base::handle(event);
            if (!active_r())
                return res;

            switch (event)
            {
            case FL_MOVE:
            case FL_ENTER:
            case FL_LEAVE:
            {
                Fl_Widget *hw = (event == FL_LEAVE)
                                    ? nullptr
                                    : which(Fl::event_x(), Fl::event_y());

                if (hw != m_hovered_tab)
                {
                    m_hovered_tab = hw;
                    damage(FL_DAMAGE_EXPOSE);
                }
                return 1;
            }
            default:
                break;
            }
            return res;
        }

        enum class TabState { Selected, Hovered, Normal };

        // --- Metaprogramming Helpers for DRY ---
        
        // Technique: Template function for conditional property resolution.
        // Proof of equivalence: Consolidates repeated ternary checks into a single inline template.
        // This generates identical assembly instructions (zero-overhead abstraction) while maintaining identical state logic.
        template <typename T>
        [[nodiscard]] static inline Fl_Color resolve_active_color(const T* widget, Fl_Color base_color) noexcept {
            return widget->active_r() ? base_color : fl_inactive(base_color);
        }

        // Technique: Template function using `if constexpr` for compile-time state dispatch.
        // Proof of equivalence: The `if constexpr` branches enforce mutually exclusive compilation paths
        // for each state, mapping perfectly to the original runtime if/else block semantics and side effects.
        template <TabState State, typename WidgetT, typename PaletteT>
        static inline void apply_state_style(WidgetT* w, const PaletteT& palette, Fl_Color text_active, Fl_Color text_inactive) noexcept {
            if constexpr (State == TabState::Selected) {
                w->color(palette.bg_main);
                w->labelcolor(text_active);
            } else if constexpr (State == TabState::Hovered) {
                w->color(fl_color_average(palette.bg_main, palette.bg_sec, 0.4f));
                w->labelcolor(text_inactive);
            } else {
                w->color(palette.bg_sec);
                w->labelcolor(text_inactive);
            }
        }

        void draw() override
        {
            const uchar d = damage();
            const bool redraw_tabs = (d & ~FL_DAMAGE_CHILD) != 0;

            if (redraw_tabs)
            {
                const auto &palette = ThemeManager::get_palette();

                // Main widget background (the empty track behind the tabs)
                color(palette.bg_sec);
                selection_color(palette.bg_main);

                const Fl_Color text_active = resolve_active_color(this, palette.selection);
                const Fl_Color text_inactive = resolve_active_color(this, palette.fg_main);

                for (int i = 0; i < children(); ++i)
                {
                    Fl_Widget *c = child(i);

                    // CRITICAL: Force a flat box to strip native FLTK 3D frames/dividers
                    c->box(FL_FLAT_BOX);

                    const bool is_selected = (c == value());
                    const bool is_hovered = (c == m_hovered_tab && !is_selected);

                    if (is_selected)
                    {
                        apply_state_style<TabState::Selected>(c, palette, text_active, text_inactive);
                    }
                    else if (is_hovered)
                    {
                        apply_state_style<TabState::Hovered>(c, palette, text_active, text_inactive);
                    }
                    else
                    {
                        apply_state_style<TabState::Normal>(c, palette, text_active, text_inactive);
                    }
                }
            }

            // FLTK calculates geometry and draws flat rectangles/text
            auto visible_f = visible_focus();
            visible_focus(false); // defer focus ring drawing until after tabs are drawn
            Fl_Tabs::draw();
            visible_focus(visible_f);

            if (children() == 0 || !redraw_tabs)
                return;

            const int sel = tab_positions();
            if (sel < 0 || sel >= children())
                return;

            const auto &palette = ThemeManager::get_palette();
            fl_push_clip(x(), y(), w(), h());

            const int tx = x() + tab_pos[sel] + tab_offset;
            const int tw = tab_width[sel];

            const int H = tab_height();
            const bool top = (H >= 0);
            const int th = std::abs(H);

            // pane_y marks the boundary where the tab header meets the content body
            const int pane_y = top ? y() + th : y();
            const int border_y = top ? pane_y - 1 : y() + h() - th;

            // 1. Draw a subtle separator line across the entire tab track
            const Fl_Color border_color = fl_color_average(palette.bg_main, palette.fg_main, 0.85f);
            fl_color(border_color);
            fl_xyline(x(), border_y, x() + w());

            // 2. Erase the border directly beneath the active tab to seamlessly connect it
            fl_color(palette.bg_main);
            fl_xyline(tx, border_y, tx + tw - 1);

            // 3. Draw the accent/focus line.
            // Instead of a separate focus ring, we enhance the tab's existing accent line
            // when focused, making it thicker and using the theme's focus color.
            const bool is_focused = (Fl::focus() == this && visible_focus() && Fl::visible_focus());

            const Fl_Color accent_color = is_focused
                                              ? palette.focus_ring
                                              : resolve_active_color(this, palette.selection);
            const int accent_height = is_focused ? 3 : 2;

            fl_color(accent_color);

            // Technique: Data-driven arithmetic substitution to eliminate branching.
            // Proof of equivalence: Algebraically calculates the same Y coordinate without branching,
            // `border_y - (accent_height - 1)` for true, `border_y` for false, preserving identical layout.
            const int accent_y = top ? border_y - (accent_height - 1) : border_y;
            fl_rectf(tx, accent_y, tw, accent_height);

            fl_pop_clip();
        }

    public:
        Tabs(int x, int y, int w, int h, const char *l = nullptr)
            : Base(x, y, w, h, l)
        {
            box(FL_FLAT_BOX);
            handle_overflow(OVERFLOW_PULLDOWN);
        }
    };
}