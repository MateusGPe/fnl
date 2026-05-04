// Theme/Theme.hpp
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
        static inline const ThemePalette *current_palette_ = &AERO_PALETTE;
        static inline std::unique_ptr<engine::IThemeRenderer> renderer_ = std::make_unique<engine::BeveledRenderer>();
        static inline bool boxes_registered_ = false;

        static std::unique_ptr<engine::IThemeRenderer> create_renderer(BoxStyle style)
        {
            switch (style)
            {
            case BoxStyle::Classic:
                return std::make_unique<engine::ClassicRenderer>();
            case BoxStyle::WinXP:
                return std::make_unique<engine::WinXPRenderer>();
            case BoxStyle::Solid:
                return std::make_unique<engine::SolidRenderer>();
            case BoxStyle::RoundedArcs:
                return std::make_unique<engine::RoundedArcsRenderer>();
            case BoxStyle::Beveled:
            default:
                return std::make_unique<engine::BeveledRenderer>();
            }
        }

    public:
        ThemeManager() = delete;

        static const ThemePalette &get_palette() { return *current_palette_; }
        static const engine::IThemeRenderer &get_renderer() { return *renderer_; }

        static void register_boxtypes();

        static void apply_theme(const ThemePalette &palette)
        {
            current_palette_ = &palette;
            renderer_ = create_renderer(palette.style);

            register_boxtypes();

            // Set global FLTK settings based on palette
            if (palette.type == ThemeType::WinXP || palette.type == ThemeType::Classic)
            {
                Fl::scrollbar_size(17);
            }
            else
            {
                Fl::scrollbar_size(15);
            }

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
    };

    // ---------------------------------------------------------
    // FLTK Box Bridge Wrappers (Route FLTK C-calls to the OOP Renderer)
    // ---------------------------------------------------------
    namespace engine
    {
        // Frames & Boxes
        inline void btn_up_frame(int x, int y, int w, int h, Fl_Color) { ThemeManager::get_renderer().draw_frame(x, y, w, h, ThemeManager::get_palette().btn_frame, Fl::draw_box_active(), ThemeManager::get_palette()); }
        inline void btn_up_box(int x, int y, int w, int h, Fl_Color) { ThemeManager::get_renderer().draw_box(x, y, w, h, ThemeManager::get_palette().btn_grad, ThemeManager::get_palette().btn_frame, 2, Fl::draw_box_active(), ThemeManager::get_palette()); }
        inline void hover_up_frame(int x, int y, int w, int h, Fl_Color) { ThemeManager::get_renderer().draw_frame(x, y, w, h, ThemeManager::get_palette().hover_frame, Fl::draw_box_active(), ThemeManager::get_palette()); }
        inline void hover_up_box(int x, int y, int w, int h, Fl_Color) { ThemeManager::get_renderer().draw_box(x, y, w, h, ThemeManager::get_palette().hover_grad, ThemeManager::get_palette().hover_frame, 2, Fl::draw_box_active(), ThemeManager::get_palette()); }
        inline void down_frame(int x, int y, int w, int h, Fl_Color) { ThemeManager::get_renderer().draw_frame(x, y, w, h, ThemeManager::get_palette().down_frame, Fl::draw_box_active(), ThemeManager::get_palette()); }
        inline void down_box(int x, int y, int w, int h, Fl_Color) { ThemeManager::get_renderer().draw_box(x, y, w, h, ThemeManager::get_palette().down_grad, ThemeManager::get_palette().down_frame, 2, Fl::draw_box_active(), ThemeManager::get_palette()); }
        inline void def_btn_frame(int x, int y, int w, int h, Fl_Color) { ThemeManager::get_renderer().draw_frame(x, y, w, h, ThemeManager::get_palette().def_btn_frame, Fl::draw_box_active(), ThemeManager::get_palette()); }
        inline void def_btn_box(int x, int y, int w, int h, Fl_Color) { ThemeManager::get_renderer().draw_box(x, y, w, h, ThemeManager::get_palette().def_btn_grad, ThemeManager::get_palette().def_btn_frame, 2, Fl::draw_box_active(), ThemeManager::get_palette()); }

        inline void thin_up_frame(int x, int y, int w, int h, Fl_Color) { ThemeManager::get_renderer().draw_static_frame(x, y, w, h, ThemeManager::get_palette().thin_up_frame, Fl::draw_box_active(), ThemeManager::get_palette()); }
        inline void panel_box(int x, int y, int w, int h, Fl_Color c)
        {
            Gradient4 fill = {c, c, c, c};
            ThemeManager::get_renderer().draw_box(x, y, w, h, fill, ThemeManager::get_palette().btn_frame, 1, Fl::draw_box_active(), ThemeManager::get_palette());
        }
        inline void menu_popup_box(int x, int y, int w, int h, Fl_Color c)
        {
            Gradient4 fill = {c, c, c, c};
            ThemeManager::get_renderer().draw_box(x, y, w, h, fill, ThemeManager::get_palette().btn_frame, 1, Fl::draw_box_active(), ThemeManager::get_palette());
        }
        inline void thin_up_box(int x, int y, int w, int h, Fl_Color c)
        {
            Gradient4 fill = {c, c, c, c};
            ThemeManager::get_renderer().draw_box(x, y, w, h, fill, ThemeManager::get_palette().thin_up_frame, 1, Fl::draw_box_active(), ThemeManager::get_palette());
        }
        inline void thin_down_frame(int x, int y, int w, int h, Fl_Color) { ThemeManager::get_renderer().draw_static_frame(x, y, w, h, ThemeManager::get_palette().thin_down_frame, Fl::draw_box_active(), ThemeManager::get_palette()); }
        inline void thin_down_box(int x, int y, int w, int h, Fl_Color c)
        {
            fl_color(core::activated_color(c, Fl::draw_box_active()));
            fl_rectf(x + 1, y + 1, w - 2, h - 2);
            ThemeManager::get_renderer().draw_static_frame(x, y, w, h, ThemeManager::get_palette().thin_down_frame, Fl::draw_box_active(), ThemeManager::get_palette());
        }

        inline void input_frame(int x, int y, int w, int h, Fl_Color) { ThemeManager::get_renderer().draw_static_frame(x, y, w, h, ThemeManager::get_palette().input_frame, Fl::draw_box_active(), ThemeManager::get_palette()); }
        inline void input_box(int x, int y, int w, int h, Fl_Color c)
        {
            fl_color(core::activated_color(c, Fl::draw_box_active()));
            fl_rectf(x + 2, y + 2, w - 4, h - 4);
            ThemeManager::get_renderer().draw_static_frame(x, y, w, h, ThemeManager::get_palette().input_frame, Fl::draw_box_active(), ThemeManager::get_palette());
        }

        inline void smart_down_frame(int x, int y, int w, int h, Fl_Color c)
        {
            if (c == FL_BACKGROUND2_COLOR)
                ThemeManager::get_renderer().draw_static_frame(x, y, w, h, ThemeManager::get_palette().input_frame, Fl::draw_box_active(), ThemeManager::get_palette());
            else
                ThemeManager::get_renderer().draw_frame(x, y, w, h, ThemeManager::get_palette().down_frame, Fl::draw_box_active(), ThemeManager::get_palette());
        }
        inline void smart_down_box(int x, int y, int w, int h, Fl_Color c)
        {
            if (c == FL_BACKGROUND2_COLOR)
                input_box(x, y, w, h, c);
            else
                down_box(x, y, w, h, c);
        }

        inline void radio_round_down_box(int x, int y, int w, int h, Fl_Color c)
        {
            fl_color(core::activated_color(c, Fl::draw_box_active()));
            fl_pie(x + 1, y + 1, w - 2, h - 2, 0.0, 360.0);
            fl_color(core::activated_color(ThemeManager::get_palette().input_frame.out_top, Fl::draw_box_active()));
            fl_arc(x, y, w, h, 0.0, 360.0);
        }

        // Rounded wrappers
        inline void rounded_btn_up_frame(int x, int y, int w, int h, Fl_Color) { core::draw_rounded_frame_h(x, y, w, h, ThemeManager::get_palette().btn_frame, ThemeManager::get_palette().metrics.radius, Fl::draw_box_active()); }
        inline void rounded_btn_up_box(int x, int y, int w, int h, Fl_Color)
        {
            Gradient4 g = ThemeManager::get_palette().btn_grad;
            core::draw_rounded_gradient_box(x + 2, y + 2, w - 4, h - 4, g.t_start, g.t_end, g.b_start, g.b_end, ThemeManager::get_palette().metrics.radius, Fl::draw_box_active());
            rounded_btn_up_frame(x, y, w, h, 0);
        }
        inline void rounded_hover_up_frame(int x, int y, int w, int h, Fl_Color) { core::draw_rounded_frame_h(x, y, w, h, ThemeManager::get_palette().hover_frame, ThemeManager::get_palette().metrics.radius, Fl::draw_box_active()); }
        inline void rounded_hover_up_box(int x, int y, int w, int h, Fl_Color)
        {
            Gradient4 g = ThemeManager::get_palette().hover_grad;
            core::draw_rounded_gradient_box(x + 2, y + 2, w - 4, h - 4, g.t_start, g.t_end, g.b_start, g.b_end, ThemeManager::get_palette().metrics.radius, Fl::draw_box_active());
            rounded_hover_up_frame(x, y, w, h, 0);
        }
        inline void rounded_down_frame(int x, int y, int w, int h, Fl_Color) { core::draw_rounded_frame_h(x, y, w, h, ThemeManager::get_palette().down_frame, ThemeManager::get_palette().metrics.radius, Fl::draw_box_active()); }
        inline void rounded_down_box(int x, int y, int w, int h, Fl_Color)
        {
            Gradient4 g = ThemeManager::get_palette().down_grad;
            core::draw_rounded_gradient_box(x + 2, y + 2, w - 4, h - 4, g.t_start, g.t_end, g.b_start, g.b_end, ThemeManager::get_palette().metrics.radius, Fl::draw_box_active());
            rounded_down_frame(x, y, w, h, 0);
        }
        inline void rounded_thin_up_frame(int x, int y, int w, int h, Fl_Color) { core::draw_rounded_frame_h(x, y, w, h, ThemeManager::get_palette().thin_up_frame, ThemeManager::get_palette().metrics.radius, Fl::draw_box_active()); }
        inline void rounded_thin_up_box(int x, int y, int w, int h, Fl_Color c)
        {
            fl_color(core::activated_color(c, Fl::draw_box_active()));
            fl_rounded_rectf(x + 1, y + 1, w - 2, h - 2, ThemeManager::get_palette().metrics.radius);
            rounded_thin_up_frame(x, y, w, h, c);
        }
        inline void rounded_input_frame(int x, int y, int w, int h, Fl_Color) { core::draw_rounded_frame_h(x, y, w, h, ThemeManager::get_palette().input_frame, ThemeManager::get_palette().metrics.radius, Fl::draw_box_active()); }
        inline void rounded_input_box(int x, int y, int w, int h, Fl_Color c)
        {
            fl_color(core::activated_color(c, Fl::draw_box_active()));
            fl_rounded_rectf(x + 2, y + 2, w - 4, h - 4, ThemeManager::get_palette().metrics.radius);
            rounded_input_frame(x, y, w, h, c);
        }

        // Component Dispatchers (To not break existing FLTK widget codes relying on engine::dispatch...)
        inline void dispatch_button(int x, int y, int w, int h, const WidgetState &state) { ThemeManager::get_renderer().draw_button(x, y, w, h, state, ThemeManager::get_palette()); }
        inline void dispatch_checkbox(int x, int y, int w, int h, const WidgetState &state) { ThemeManager::get_renderer().draw_checkbox(x, y, w < h ? w : h, state, ThemeManager::get_palette()); }
        inline void dispatch_radio_button(int x, int y, int w, int h, const WidgetState &state) { ThemeManager::get_renderer().draw_radio_button(x, y, w < h ? w : h, state, ThemeManager::get_palette()); }
        inline void dispatch_slider(int x, int y, int w, int h, const WidgetState &state) { ThemeManager::get_renderer().draw_slider(x, y, w, h, state, ThemeManager::get_palette()); }
        inline void dispatch_spinner(int x, int y, int w, int h, const WidgetState &state) { ThemeManager::get_renderer().draw_spinner(x, y, w, h, state, ThemeManager::get_palette()); }
        inline void dispatch_choice(int x, int y, int w, int h, const WidgetState &state, bool is_pressed) { ThemeManager::get_renderer().draw_choice(x, y, w, h, state, is_pressed, ThemeManager::get_palette()); }
        inline void dispatch_tabs(int x, int y, int w, int h, const WidgetState &state) { ThemeManager::get_renderer().draw_tabs(x, y, w, h, state, ThemeManager::get_palette()); }
        inline void dispatch_tab_item(int x, int y, int w, int h, const WidgetState &state) { ThemeManager::get_renderer().draw_tab_item(x, y, w, h, state, ThemeManager::get_palette()); }
        inline void dispatch_progress(int x, int y, int w, int h, const WidgetState &state) { ThemeManager::get_renderer().draw_progress(x, y, w, h, state, ThemeManager::get_palette()); }
        inline void dispatch_toggle(int x, int y, int w, int h, const WidgetState &state) { ThemeManager::get_renderer().draw_toggle(x, y, w, h, state, ThemeManager::get_palette()); }
        inline void dispatch_input(int x, int y, int w, int h, const WidgetState &state) { ThemeManager::get_renderer().draw_input(x, y, w, h, state, ThemeManager::get_palette()); }
    }

    // Must be declared after bridges are defined so FLTK functions exist to hook to
    inline void ThemeManager::register_boxtypes()
    {
        if (boxes_registered_)
            return;

        Fl::scheme("gtk+");
        Fl::set_boxtype(Theme::schemes::ROUNDED_BUTTON_UP_BOX, engine::rounded_btn_up_box, 2, 2, 4, 4);
        Fl::set_boxtype(Theme::schemes::ROUNDED_BUTTON_UP_FRAME, engine::rounded_btn_up_frame, 2, 2, 4, 4);
        Fl::set_boxtype(Theme::schemes::ROUNDED_HOVERED_UP_BOX, engine::rounded_hover_up_box, 2, 2, 4, 4);
        Fl::set_boxtype(Theme::schemes::ROUNDED_HOVERED_UP_FRAME, engine::rounded_hover_up_frame, 2, 2, 4, 4);
        Fl::set_boxtype(Theme::schemes::ROUNDED_DEPRESSED_DOWN_BOX, engine::rounded_down_box, 2, 2, 4, 4);
        Fl::set_boxtype(Theme::schemes::ROUNDED_DEPRESSED_DOWN_FRAME, engine::rounded_down_frame, 2, 2, 4, 4);
        Fl::set_boxtype(Theme::schemes::ROUNDED_INPUT_THIN_DOWN_BOX, engine::rounded_input_box, 2, 3, 4, 6);
        Fl::set_boxtype(Theme::schemes::ROUNDED_INPUT_THIN_DOWN_FRAME, engine::rounded_input_frame, 2, 3, 4, 6);
        Fl::set_boxtype(Theme::schemes::ROUNDED_PANEL_THIN_UP_BOX, engine::rounded_thin_up_box, 1, 1, 2, 2);
        Fl::set_boxtype(Theme::schemes::ROUNDED_PANEL_THIN_UP_FRAME, engine::rounded_thin_up_frame, 1, 1, 2, 2);
        Fl::set_boxtype(FL_GTK_UP_BOX, engine::btn_up_box, 2, 2, 4, 4);
        Fl::set_boxtype(FL_GTK_UP_FRAME, engine::btn_up_frame, 2, 2, 4, 4);
        Fl::set_boxtype(FL_PLASTIC_UP_BOX, engine::hover_up_box, 2, 2, 4, 4);
        Fl::set_boxtype(FL_PLASTIC_UP_FRAME, engine::hover_up_frame, 2, 2, 4, 4);
        Fl::set_boxtype(FL_PLASTIC_DOWN_BOX, engine::down_box, 2, 2, 4, 4);
        Fl::set_boxtype(FL_PLASTIC_DOWN_FRAME, engine::down_frame, 2, 2, 4, 4);
        Fl::set_boxtype(FL_DIAMOND_UP_BOX, engine::def_btn_box, 2, 2, 4, 4);
        Fl::set_boxtype(FL_PLASTIC_THIN_UP_BOX, engine::hover_up_box, 2, 2, 4, 4);
        Fl::set_boxtype(FL_DIAMOND_DOWN_BOX, engine::down_box, 2, 2, 4, 4);
        Fl::set_boxtype(FL_GTK_DOWN_BOX, engine::smart_down_box, 2, 2, 4, 4);
        Fl::set_boxtype(FL_GTK_DOWN_FRAME, engine::smart_down_frame, 2, 2, 4, 4);
        Fl::set_boxtype(FL_PLASTIC_THIN_DOWN_BOX, engine::input_box, 2, 3, 4, 6);
        Fl::set_boxtype(FL_PLASTIC_ROUND_DOWN_BOX, engine::input_frame, 2, 3, 4, 6);
        Fl::set_boxtype(FL_GTK_ROUND_DOWN_BOX, engine::radio_round_down_box, 3, 3, 6, 6);
        Fl::set_boxtype(FL_GTK_THIN_UP_BOX, engine::thin_up_box, 1, 1, 2, 2);
        Fl::set_boxtype(FL_GTK_THIN_UP_FRAME, engine::thin_up_frame, 1, 1, 2, 2);
        Fl::set_boxtype(FL_GTK_THIN_DOWN_BOX, engine::thin_down_box, 1, 1, 2, 2);
        Fl::set_boxtype(FL_GTK_THIN_DOWN_FRAME, engine::thin_down_frame, 1, 1, 2, 2);
        Fl::set_boxtype(Theme::schemes::BUTTON_DOWN_BOX, engine::down_box, 2, 2, 4, 4);
        Fl::set_boxtype(Theme::schemes::MENU_POPUP_BOX, engine::menu_popup_box, 1, 1, 2, 2);
        Fl::set_boxtype(FL_UP_BOX, engine::panel_box, 1, 1, 2, 2);
        Fl::set_boxtype(FL_DOWN_BOX, engine::smart_down_box, 4, 2, 8, 4);
        Fl::set_boxtype(FL_ROUND_DOWN_BOX, engine::radio_round_down_box, 2, 2, 4, 4);
        Fl::set_boxtype(FL_EMBOSSED_BOX, engine::btn_up_box, 2, 1, 4, 2);
        Fl::set_boxtype(FL_ENGRAVED_BOX, engine::thin_down_box, 2, 2, 4, 4);
        Fl::set_boxtype(FL_GLEAM_UP_BOX, engine::btn_up_box, 2, 2, 4, 4);
        Fl::set_boxtype(FL_GLEAM_DOWN_BOX, engine::smart_down_box, 2, 2, 4, 4);
        Fl::set_boxtype(FL_GLEAM_ROUND_UP_BOX, FL_FLAT_BOX);
        Fl::set_boxtype(Theme::schemes::BG_BOX, FL_FLAT_BOX);
        Fl::set_boxtype(Theme::schemes::BG_DOWN_BOX, Theme::schemes::BG_BOX);
        Fl::set_boxtype(Theme::schemes::TOOLBAR_FRAME, engine::thin_up_frame, 1, 1, 2, 2);
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

    // ---------------------------------------------------------
    // Legacy `Theme::` Namespace for backwards compatibility
    // ---------------------------------------------------------
    namespace Theme
    {
        static void apply_global_settings()
        {
            fl_contrast_mode(FL_CONTRAST_CIELAB);
            fl_contrast_level(45);
        }

        static void apply(ThemeType typ)
        {
            const char *font_regular, *font_bold, *font_italic, *font_bold_italic;
            if (typ == ThemeType::WinXP || typ == ThemeType::Classic)
            {
                font_regular = "Tahoma";
                font_bold = "Tahoma Bold";
                font_italic = "Tahoma Italic";
                font_bold_italic = "Tahoma Bold Italic";
            }
            else
            {
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
            }
            Fl::set_font(FL_HELVETICA, font_regular);
            Fl::set_font(FL_HELVETICA_BOLD, font_bold);
            Fl::set_font(FL_HELVETICA_ITALIC, font_italic);
            Fl::set_font(FL_HELVETICA_BOLD_ITALIC, font_bold_italic);

            switch (typ)
            {
            case ThemeType::Aero:
                ThemeManager::apply_theme(AERO_PALETTE);
                break;
            case ThemeType::Blue:
                ThemeManager::apply_theme(BLUE_PALETTE);
                break;
            case ThemeType::Dark:
                ThemeManager::apply_theme(DARK_PALETTE);
                break;
            case ThemeType::Metro:
                ThemeManager::apply_theme(METRO_PALETTE);
                break;
            case ThemeType::Greybird:
                ThemeManager::apply_theme(GREYBIRD_PALETTE);
                break;
            case ThemeType::AquaClassic:
                ThemeManager::apply_theme(AQUA_CLASSIC_PALETTE);
                break;
            case ThemeType::Classic:
                ThemeManager::apply_theme(CLASSIC_PALETTE);
                break;
            case ThemeType::Win10:
                ThemeManager::apply_theme(WIN10_PALETTE);
                break;
            case ThemeType::WinXP:
                ThemeManager::apply_theme(WINXP_PALETTE);
                break;
            case ThemeType::Flat:
                ThemeManager::apply_theme(FLAT_PALETTE);
                break;
            case ThemeType::Material:
                ThemeManager::apply_theme(MATERIAL_PALETTE);
                break;
            }
            apply_global_settings();
        }

        namespace registry
        {
            inline std::unordered_map<std::string, ThemeType> available_themes = {
                {"Classic", ThemeType::Classic}, {"Aero", ThemeType::Aero}, {"Metro", ThemeType::Metro}, {"AquaClassic", ThemeType::AquaClassic}, {"Greybird", ThemeType::Greybird}, {"Blue", ThemeType::Blue}, {"Dark", ThemeType::Dark}, {"Win10", ThemeType::Win10}, {"WinXP", ThemeType::WinXP}, {"Flat", ThemeType::Flat}, {"Material", ThemeType::Material}};

            inline std::vector<std::string> list_themes()
            {
                std::vector<std::string> names;
                names.reserve(available_themes.size());
                for (const auto &[name, _] : available_themes)
                    names.push_back(name);
                return names;
            }

            inline bool load_by_name(const std::string &name)
            {
                auto it = available_themes.find(name);
                if (it == available_themes.end())
                {
                    apply(ThemeType::Aero);
                    return false;
                }
                apply(it->second);
                return true;
            }
        }
    }
}
