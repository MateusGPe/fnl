#pragma once

#include <FL/Enumerations.H>
#include <FL/Fl.H>
#include <FL/Fl_Tooltip.H>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdio>
#include <memory>

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
    private:
        static inline ThemePalette *current_palette_ = &DARK_PALETTE;
        static inline bool boxes_registered_ = false;

    public:
        ThemeManager() = delete;

        static ThemePalette &get_palette() { return *current_palette_; }

        static void register_boxtypes();

        static void apply_theme(ThemePalette &palette)
        {
            current_palette_ = &palette;

            register_boxtypes();

            Fl::scrollbar_size(15);

            unsigned char bg_r = (palette.bg_main >> 24) & 0xFF, bg_g = (palette.bg_main >> 16) & 0xFF, bg_b = (palette.bg_main >> 8) & 0xFF;
            unsigned char bg2_r = (palette.bg_sec >> 24) & 0xFF, bg2_g = (palette.bg_sec >> 16) & 0xFF, bg2_b = (palette.bg_sec >> 8) & 0xFF;
            unsigned char fg_r = (palette.fg_main >> 24) & 0xFF, fg_g = (palette.fg_main >> 16) & 0xFF, fg_b = (palette.fg_main >> 8) & 0xFF;

            Fl::background(bg_r, bg_g, bg_b);
            Fl::background2(bg2_r, bg2_g, bg2_b);
            Fl::foreground(fg_r, fg_g, fg_b);
            Fl::set_color(FL_INACTIVE_COLOR, palette.inactive);
            Fl::set_color(FL_SELECTION_COLOR, palette.selection);
            Fl::set_color(FL_FREE_COLOR, palette.free_bg);
            Fl_Tooltip::color(palette.tt_bg);
            Fl_Tooltip::textcolor(palette.tt_fg);
            Fl::redraw();
        }

        static void set_palette(const ThemePalette &palette)
        {
            // Copy the new palette's data into the currently active palette object.
            *current_palette_ = palette;
            // Now, call apply_theme with the (modified) active palette. This is safe
            // and ensures all global FLTK theme settings are updated.
            apply_theme(*current_palette_);
        }
    };

    namespace engine
    {
        template <auto GradPtr, auto FramePtr>
        inline void tpl_box(int x, int y, int w, int h, Fl_Color)
        {
            engine::draw_box(x, y, w, h, ThemeManager::get_palette().*GradPtr, ThemeManager::get_palette().*FramePtr, 2, Fl::draw_box_active(), ThemeManager::get_palette());
        }

        template <auto FramePtr>
        inline void tpl_frame(int x, int y, int w, int h, Fl_Color)
        {
            engine::draw_frame(x, y, w, h, ThemeManager::get_palette().*FramePtr, Fl::draw_box_active(), ThemeManager::get_palette());
        }

        template <auto FramePtr>
        inline void tpl_static_frame(int x, int y, int w, int h, Fl_Color)
        {
            engine::draw_static_frame(x, y, w, h, ThemeManager::get_palette().*FramePtr, Fl::draw_box_active(), ThemeManager::get_palette());
        }

        template <auto FramePtr>
        inline void tpl_flat_box(int x, int y, int w, int h, Fl_Color c)
        {
            Gradient4 fill = {c, c, c, c};
            engine::draw_box(x, y, w, h, fill, ThemeManager::get_palette().*FramePtr, 1, Fl::draw_box_active(), ThemeManager::get_palette());
        }

        template <auto FramePtr>
        inline void tpl_input_box(int x, int y, int w, int h, Fl_Color c)
        {
            bool active = Fl::draw_box_active();
            int radius = ThemeManager::get_palette().metrics.radius;
            fl_color(core::activated_color(c, active));

            if (radius > 0)
                fl_rounded_rectf(x, y, w, h, radius);
            else
                fl_rectf(x, y, w, h);

            engine::draw_static_frame(x, y, w, h, ThemeManager::get_palette().*FramePtr, active, ThemeManager::get_palette());
        }

        template <auto FramePtr>
        inline void tpl_thin_down_box(int x, int y, int w, int h, Fl_Color c)
        {
            bool active = Fl::draw_box_active();
            int radius = ThemeManager::get_palette().metrics.radius;
            fl_color(core::activated_color(c, active));

            if (radius > 0)
                fl_rounded_rectf(x, y, w, h, radius);
            else
                fl_rectf(x, y, w, h);

            engine::draw_static_frame(x, y, w, h, ThemeManager::get_palette().*FramePtr, active, ThemeManager::get_palette());
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
            fl_color(core::activated_color(c, Fl::draw_box_active()));
            fl_pie(x, y, w, h, 0.0, 360.0);
            fl_color(core::activated_color(ThemeManager::get_palette().input_frame.out_top, Fl::draw_box_active()));
            fl_arc(x, y, w, h, 0.0, 360.0);
        }

        inline void dispatch_button(int x, int y, int w, int h, const WidgetState &state) { draw_button(x, y, w, h, state, ThemeManager::get_palette()); }
        inline void dispatch_choice(int x, int y, int w, int h, const WidgetState &state, bool is_pressed) { draw_choice(x, y, w, h, state, is_pressed, ThemeManager::get_palette()); }
    }

    inline void ThemeManager::register_boxtypes()
    {
        if (boxes_registered_)
            return;

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

        Fl::visible_focus(0);
#ifdef _WIN32
        Fl_Tooltip::size(12);
#else
        Fl_Tooltip::size(13);
#endif
        Fl_Tooltip::delay(0.5f);
        boxes_registered_ = true;
    }

    namespace Theme
    {
        static void apply_global_settings()
        {
            fl_contrast_mode(FL_CONTRAST_CIELAB);
            fl_contrast_level(45);
        }

        static void apply()
        {
            const char *font_regular, *font_bold, *font_italic, *font_bold_italic;

#if defined(_WIN32)
            font_regular = "Segoe UI";
            font_bold = "Segoe UI Bold";
            font_italic = "Segoe UI Italic";
            font_bold_italic = "Segoe UI Bold Italic";
#elif defined(__APPLE__)
            font_regular = "Helvetica Neue";
            font_bold = "Helvetica Neue Bold";
            font_italic = "Helvetica Neue Italic";
            font_bold_italic = "Helvetica Neue Bold Italic";
#else
            font_regular = "Noto Sans";
            font_bold = "Noto Sans Bold";
            font_italic = "Noto Sans Italic";
            font_bold_italic = "Noto Sans Bold Italic";
#endif

            Fl::set_font(FL_HELVETICA, font_regular);
            Fl::set_font(FL_HELVETICA_BOLD, font_bold);
            Fl::set_font(FL_HELVETICA_ITALIC, font_italic);
            Fl::set_font(FL_HELVETICA_BOLD_ITALIC, font_bold_italic);

            ThemeManager::apply_theme(DARK_PALETTE);
            apply_global_settings();
        }
    }
}