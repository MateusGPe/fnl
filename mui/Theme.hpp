#pragma once

#include <FL/Enumerations.H>
#include <FL/Fl.H>
#include <FL/Fl_Tooltip.H>
#include <memory>
#include <mutex>

#include "Theme/data.hpp"
#include "Theme/engine.hpp"
#include "Theme/primitives.hpp"

namespace mui
{
    namespace Theme::schemes
    {
        inline constexpr Fl_Boxtype BG_BOX = static_cast<Fl_Boxtype>(FL_FREE_BOXTYPE);
        inline constexpr Fl_Boxtype BG_DOWN_BOX = static_cast<Fl_Boxtype>(FL_FREE_BOXTYPE + 1);
        inline constexpr Fl_Boxtype TOOLBAR_FRAME = static_cast<Fl_Boxtype>(FL_FREE_BOXTYPE + 2);
        inline constexpr Fl_Boxtype BUTTON_DOWN_BOX = static_cast<Fl_Boxtype>(FL_FREE_BOXTYPE + 3);
        inline constexpr Fl_Boxtype MENU_POPUP_BOX = static_cast<Fl_Boxtype>(FL_FREE_BOXTYPE + 4);
        inline constexpr Fl_Boxtype TEXT_INSET_BOX = static_cast<Fl_Boxtype>(FL_FREE_BOXTYPE + 5);
        inline constexpr Fl_Boxtype ROUNDED_BUTTON_UP_BOX = static_cast<Fl_Boxtype>(FL_FREE_BOXTYPE + 10);
        inline constexpr Fl_Boxtype ROUNDED_BUTTON_UP_FRAME = static_cast<Fl_Boxtype>(FL_FREE_BOXTYPE + 11);
        inline constexpr Fl_Boxtype ROUNDED_HOVERED_UP_BOX = static_cast<Fl_Boxtype>(FL_FREE_BOXTYPE + 12);
        inline constexpr Fl_Boxtype ROUNDED_HOVERED_UP_FRAME = static_cast<Fl_Boxtype>(FL_FREE_BOXTYPE + 13);
        inline constexpr Fl_Boxtype ROUNDED_DEPRESSED_DOWN_BOX = static_cast<Fl_Boxtype>(FL_FREE_BOXTYPE + 14);
        inline constexpr Fl_Boxtype ROUNDED_DEPRESSED_DOWN_FRAME = static_cast<Fl_Boxtype>(FL_FREE_BOXTYPE + 15);
        inline constexpr Fl_Boxtype ROUNDED_INPUT_THIN_DOWN_BOX = static_cast<Fl_Boxtype>(FL_FREE_BOXTYPE + 16);
        inline constexpr Fl_Boxtype ROUNDED_INPUT_THIN_DOWN_FRAME = static_cast<Fl_Boxtype>(FL_FREE_BOXTYPE + 17);
        inline constexpr Fl_Boxtype ROUNDED_PANEL_THIN_UP_BOX = static_cast<Fl_Boxtype>(FL_FREE_BOXTYPE + 18);
        inline constexpr Fl_Boxtype ROUNDED_PANEL_THIN_UP_FRAME = static_cast<Fl_Boxtype>(FL_FREE_BOXTYPE + 19);
    }

    class ThemeManager
    {
    public:
        ThemeManager() = delete;

        [[nodiscard]] static ThemePalette &get_palette() noexcept
        {
            return *current_palette_;
        }

        static void apply_theme(ThemePalette &palette)
        {
            current_palette_ = &palette;
            register_boxtypes();
            apply_fltk_globals(palette);
        }

        static void set_palette(const ThemePalette &palette)
        {
            *current_palette_ = palette;
            apply_fltk_globals(*current_palette_);
        }

        static void register_boxtypes();

    private:
        static inline ThemePalette *current_palette_ = &BLUE_LIGHT_PALETTE;

        static void apply_fltk_globals(const ThemePalette &p)
        {
            Fl::scrollbar_size(15);

            auto set_bg = [](Fl_Color c)
            {
                unsigned char r, g, b;
                engine::unpack_rgb(c, r, g, b);
                Fl::background(r, g, b);
            };
            auto set_bg2 = [](Fl_Color c)
            {
                unsigned char r, g, b;
                engine::unpack_rgb(c, r, g, b);
                Fl::background2(r, g, b);
            };
            auto set_fg = [](Fl_Color c)
            {
                unsigned char r, g, b;
                engine::unpack_rgb(c, r, g, b);
                Fl::foreground(r, g, b);
            };

            set_bg(p.bg_main);
            set_bg2(p.bg_sec);
            set_fg(p.fg_main);

            Fl::set_color(FL_INACTIVE_COLOR, p.inactive);
            Fl::set_color(FL_SELECTION_COLOR, p.selection);
            Fl::set_color(FL_FREE_COLOR, p.free_bg);

            Fl_Tooltip::color(p.tt_bg);
            Fl_Tooltip::textcolor(p.tt_fg);

            Fl::redraw();
        }
    };

    namespace engine
    {
        template <auto GradPtr, auto FramePtr>
        inline void tpl_box(int x, int y, int w, int h, Fl_Color)
        {
            draw_box(x, y, w, h,
                     ThemeManager::get_palette().*GradPtr,
                     ThemeManager::get_palette().*FramePtr,
                     2, Fl::draw_box_active(), ThemeManager::get_palette());
        }

        template <auto FramePtr>
        inline void tpl_frame(int x, int y, int w, int h, Fl_Color)
        {
            draw_frame(x, y, w, h,
                       ThemeManager::get_palette().*FramePtr,
                       Fl::draw_box_active(), ThemeManager::get_palette());
        }

        template <auto FramePtr>
        inline void tpl_static_frame(int x, int y, int w, int h, Fl_Color)
        {
            draw_static_frame(x, y, w, h,
                              ThemeManager::get_palette().*FramePtr,
                              Fl::draw_box_active(), ThemeManager::get_palette());
        }

        template <auto FramePtr>
        inline void tpl_flat_box(int x, int y, int w, int h, Fl_Color c)
        {
            const Gradient4 fill{c, c, c, c};
            draw_box(x, y, w, h, fill,
                     ThemeManager::get_palette().*FramePtr,
                     1, Fl::draw_box_active(), ThemeManager::get_palette());
        }

        template <auto FramePtr>
        inline void tpl_input_box(int x, int y, int w, int h, Fl_Color c)
        {
            const bool active = Fl::draw_box_active();
            const int radius = ThemeManager::get_palette().metrics.radius;
            fl_color(core::activated_color(c, active));
            if (radius > 0)
                fl_rounded_rectf(x, y, w, h, radius);
            else
                fl_rectf(x, y, w, h);
            draw_static_frame(x, y, w, h,
                              ThemeManager::get_palette().*FramePtr,
                              active, ThemeManager::get_palette());
        }

        template <auto FramePtr>
        inline void tpl_thin_down_box(int x, int y, int w, int h, Fl_Color c)
        {

            tpl_input_box<FramePtr>(x, y, w, h, c);
        }

        inline void smart_down_frame(int x, int y, int w, int h, Fl_Color c)
        {
            if (c == FL_BACKGROUND2_COLOR)
                tpl_static_frame<&ThemePalette::input_frame>(x, y, w, h, c);
            else
                tpl_frame<&ThemePalette::down_frame>(x, y, w, h, c);
        }

        inline void smart_down_box(int x, int y, int w, int h, Fl_Color c)
        {
            if (c == FL_BACKGROUND2_COLOR)
                tpl_input_box<&ThemePalette::input_frame>(x, y, w, h, c);
            else
                tpl_box<&ThemePalette::down_grad, &ThemePalette::down_frame>(x, y, w, h, c);
        }

        inline void radio_round_down_box(int x, int y, int w, int h, Fl_Color c)
        {
            const bool active = Fl::draw_box_active();
            fl_color(core::activated_color(c, active));
            fl_pie(x, y, w, h, 0.0, 360.0);
            fl_color(core::activated_color(ThemeManager::get_palette().input_frame.out_top, active));
            fl_arc(x, y, w, h, 0.0, 360.0);
        }
    }

    inline void ThemeManager::register_boxtypes()
    {
        static std::once_flag s_flag;
        std::call_once(
            s_flag,
            []
            {
                Fl::scheme("gtk+");

                Fl::set_boxtype(FL_GTK_UP_BOX, engine::tpl_box<&ThemePalette::btn_grad, &ThemePalette::btn_frame>, 2, 2, 4, 4);
                Fl::set_boxtype(FL_GTK_UP_FRAME, engine::tpl_frame<&ThemePalette::btn_frame>, 2, 2, 4, 4);
                Fl::set_boxtype(FL_PLASTIC_UP_BOX, engine::tpl_box<&ThemePalette::hover_grad, &ThemePalette::hover_frame>, 2, 2, 4, 4);
                Fl::set_boxtype(FL_PLASTIC_UP_FRAME, engine::tpl_frame<&ThemePalette::hover_frame>, 2, 2, 4, 4);
                Fl::set_boxtype(FL_PLASTIC_DOWN_BOX, engine::tpl_box<&ThemePalette::down_grad, &ThemePalette::down_frame>, 2, 2, 4, 4);
                Fl::set_boxtype(FL_PLASTIC_DOWN_FRAME, engine::tpl_frame<&ThemePalette::down_frame>, 2, 2, 4, 4);
                Fl::set_boxtype(FL_DIAMOND_UP_BOX, engine::tpl_box<&ThemePalette::def_btn_grad, &ThemePalette::def_btn_frame>, 2, 2, 4, 4);
                Fl::set_boxtype(FL_PLASTIC_THIN_UP_BOX, engine::tpl_box<&ThemePalette::hover_grad, &ThemePalette::hover_frame>, 2, 2, 4, 4);
                Fl::set_boxtype(FL_DIAMOND_DOWN_BOX, engine::tpl_box<&ThemePalette::down_grad, &ThemePalette::down_frame>, 2, 2, 4, 4);
                Fl::set_boxtype(FL_GTK_DOWN_BOX, engine::smart_down_box, 2, 2, 4, 4);
                Fl::set_boxtype(FL_GTK_DOWN_FRAME, engine::smart_down_frame, 2, 2, 4, 4);
                Fl::set_boxtype(FL_PLASTIC_THIN_DOWN_BOX, engine::tpl_input_box<&ThemePalette::input_frame>, 2, 3, 4, 6);
                Fl::set_boxtype(FL_PLASTIC_ROUND_DOWN_BOX, engine::tpl_static_frame<&ThemePalette::input_frame>, 2, 3, 4, 6);
                Fl::set_boxtype(FL_GTK_ROUND_DOWN_BOX, engine::radio_round_down_box, 3, 3, 6, 6);
                Fl::set_boxtype(FL_GTK_THIN_UP_BOX, engine::tpl_flat_box<&ThemePalette::thin_up_frame>, 1, 1, 2, 2);
                Fl::set_boxtype(FL_GTK_THIN_UP_FRAME, engine::tpl_static_frame<&ThemePalette::thin_up_frame>, 1, 1, 2, 2);
                Fl::set_boxtype(FL_GTK_THIN_DOWN_BOX, engine::tpl_thin_down_box<&ThemePalette::thin_down_frame>, 1, 1, 2, 2);
                Fl::set_boxtype(FL_GTK_THIN_DOWN_FRAME, engine::tpl_static_frame<&ThemePalette::thin_down_frame>, 1, 1, 2, 2);
                Fl::set_boxtype(FL_UP_BOX, engine::tpl_flat_box<&ThemePalette::btn_frame>, 1, 1, 2, 2);
                Fl::set_boxtype(FL_DOWN_BOX, engine::smart_down_box, 4, 2, 8, 4);
                Fl::set_boxtype(FL_ROUND_DOWN_BOX, engine::radio_round_down_box, 2, 2, 4, 4);
                Fl::set_boxtype(FL_EMBOSSED_BOX, engine::tpl_box<&ThemePalette::btn_grad, &ThemePalette::btn_frame>, 2, 1, 4, 2);
                Fl::set_boxtype(FL_ENGRAVED_BOX, engine::tpl_thin_down_box<&ThemePalette::thin_down_frame>, 2, 2, 4, 4);
                Fl::set_boxtype(FL_GLEAM_UP_BOX, engine::tpl_box<&ThemePalette::btn_grad, &ThemePalette::btn_frame>, 2, 2, 4, 4);
                Fl::set_boxtype(FL_GLEAM_DOWN_BOX, engine::smart_down_box, 2, 2, 4, 4);
                Fl::set_boxtype(FL_GLEAM_ROUND_UP_BOX, FL_FLAT_BOX);

                Fl::set_boxtype(Theme::schemes::ROUNDED_BUTTON_UP_BOX, engine::tpl_box<&ThemePalette::btn_grad, &ThemePalette::btn_frame>, 2, 2, 4, 4);
                Fl::set_boxtype(Theme::schemes::ROUNDED_BUTTON_UP_FRAME, engine::tpl_frame<&ThemePalette::btn_frame>, 2, 2, 4, 4);
                Fl::set_boxtype(Theme::schemes::ROUNDED_HOVERED_UP_BOX, engine::tpl_box<&ThemePalette::hover_grad, &ThemePalette::hover_frame>, 2, 2, 4, 4);
                Fl::set_boxtype(Theme::schemes::ROUNDED_HOVERED_UP_FRAME, engine::tpl_frame<&ThemePalette::hover_frame>, 2, 2, 4, 4);
                Fl::set_boxtype(Theme::schemes::ROUNDED_DEPRESSED_DOWN_BOX, engine::tpl_box<&ThemePalette::down_grad, &ThemePalette::down_frame>, 2, 2, 4, 4);
                Fl::set_boxtype(Theme::schemes::ROUNDED_DEPRESSED_DOWN_FRAME, engine::tpl_frame<&ThemePalette::down_frame>, 2, 2, 4, 4);
                Fl::set_boxtype(Theme::schemes::ROUNDED_INPUT_THIN_DOWN_BOX, engine::tpl_input_box<&ThemePalette::input_frame>, 2, 3, 4, 6);
                Fl::set_boxtype(Theme::schemes::ROUNDED_INPUT_THIN_DOWN_FRAME, engine::tpl_static_frame<&ThemePalette::input_frame>, 2, 3, 4, 6);
                Fl::set_boxtype(Theme::schemes::ROUNDED_PANEL_THIN_UP_BOX, engine::tpl_flat_box<&ThemePalette::thin_up_frame>, 1, 1, 2, 2);
                Fl::set_boxtype(Theme::schemes::ROUNDED_PANEL_THIN_UP_FRAME, engine::tpl_static_frame<&ThemePalette::thin_up_frame>, 1, 1, 2, 2);
                Fl::set_boxtype(Theme::schemes::BUTTON_DOWN_BOX, engine::tpl_box<&ThemePalette::down_grad, &ThemePalette::down_frame>, 2, 2, 4, 4);
                Fl::set_boxtype(Theme::schemes::MENU_POPUP_BOX, engine::tpl_flat_box<&ThemePalette::btn_frame>, 1, 1, 2, 2);
                Fl::set_boxtype(Theme::schemes::BG_BOX, FL_FLAT_BOX);
                Fl::set_boxtype(Theme::schemes::BG_DOWN_BOX, Theme::schemes::BG_BOX);
                Fl::set_boxtype(Theme::schemes::TOOLBAR_FRAME, engine::tpl_static_frame<&ThemePalette::thin_up_frame>, 1, 1, 2, 2);
                Fl::set_boxtype(Theme::schemes::TEXT_INSET_BOX, [](int, int, int, int, Fl_Color) {}, 8, 0, 16, 0);

                Fl::visible_focus(1);

#ifdef _WIN32
                Fl_Tooltip::size(12);
#else
                Fl_Tooltip::size(13);
#endif
                Fl_Tooltip::delay(0.5f);
            });
    }

    namespace Theme
    {
        inline void apply_global_settings()
        {
            fl_contrast_mode(FL_CONTRAST_CIELAB);
            fl_contrast_level(45);
        }

        inline void apply()
        {
#if defined(_WIN32)
            Fl::set_font(FL_HELVETICA, "Segoe UI");
            Fl::set_font(FL_HELVETICA_BOLD, "Segoe UI Bold");
            Fl::set_font(FL_HELVETICA_ITALIC, "Segoe UI Italic");
            Fl::set_font(FL_HELVETICA_BOLD_ITALIC, "Segoe UI Bold Italic");
#elif defined(__APPLE__)
            Fl::set_font(FL_HELVETICA, "Helvetica Neue");
            Fl::set_font(FL_HELVETICA_BOLD, "Helvetica Neue Bold");
            Fl::set_font(FL_HELVETICA_ITALIC, "Helvetica Neue Italic");
            Fl::set_font(FL_HELVETICA_BOLD_ITALIC, "Helvetica Neue Bold Italic");
#else
            Fl::set_font(FL_HELVETICA, "Noto Sans");
            Fl::set_font(FL_HELVETICA_BOLD, "Noto Sans Bold");
            Fl::set_font(FL_HELVETICA_ITALIC, "Noto Sans Italic");
            Fl::set_font(FL_HELVETICA_BOLD_ITALIC, "Noto Sans Bold Italic");
#endif
            ThemeManager::apply_theme(BLUE_LIGHT_PALETTE);
            apply_global_settings();
        }
    }
}
