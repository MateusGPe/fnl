#pragma once
#include "primitives.hpp"

#include <FL/Enumerations.H>
#include <string>
#include <utility>
#include <vector>

namespace mui
{
    constexpr Fl_Color rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0) noexcept
    {
        return static_cast<Fl_Color>(
            (static_cast<uint32_t>(r) << 24) |
            (static_cast<uint32_t>(g) << 16) |
            (static_cast<uint32_t>(b) << 8) |
            static_cast<uint32_t>(a));
    }

    constexpr Fl_Color frgb(uint8_t r, uint8_t g, uint8_t b) noexcept
    {
        return rgba(r, g, b, 0);
    }

    struct Gradient4
    {
        Fl_Color t_start, t_end;
        Fl_Color b_start, b_end;
    };

    struct ThemeMetrics
    {
        int radius = 5;
        float focus_ring_opacity = 0.2f;
        int focus_ring_width = 1;
        int choice_arrow_padding = 14;
        int choice_arrow_thickness = 2;
        int choice_arrow_height = 4;
        int choice_arrow_size = 8;
        int slider_track_height = 4;
        int slider_thumb_size = 16;
        int slider_thumb_focus_halo_size = 20;
        float slider_thumb_hover_halo_opacity = 0.9f;
        int valueslider_gap = 4;
        int valueslider_input_width = 40;
        int valueslider_input_height = 30;
        int valueslider_min_slider_size = 20;
        int spinner_arrow_size = 8;
        int progress_height = 6;
        int imageviewer_grid_size = 20;
        int imageviewer_handle_size = 8;
        int imageviewer_handle_hit_padding = 2;
        int imageviewer_selection_dash_width = 1;
        float widget_disabled_label_factor = 0.5f;

        ThemeMetrics() = default;
    };

    struct ThemePalette
    {
        Fl_Color bg_main, bg_sec, fg_main;
        Fl_Color inactive, selection;
        Fl_Color error_main, success_main;
        Fl_Color free_bg, tt_bg, tt_fg;
        Gradient4 btn_grad;
        core::FrameColors btn_frame;
        Gradient4 hover_grad;
        core::FrameColors hover_frame;
        Gradient4 down_grad;
        core::FrameColors down_frame;
        Gradient4 def_btn_grad;
        core::FrameColors def_btn_frame;
        core::FrameColors thin_up_frame;
        core::FrameColors thin_down_frame;
        Fl_Color input_bg;
        core::FrameColors input_frame;
        Fl_Color focus_ring;
        Fl_Color toggle_tick;
        Fl_Color toggle_border;
        Fl_Color slider_track;
        Fl_Color spinner_arrow;
        inline static const ThemeMetrics metrics{};
    };

    //

    inline ThemePalette DARK_PALETTE = {
        frgb(240, 240, 240),
        frgb(255, 255, 255),
        frgb(30, 30, 30),
        frgb(150, 150, 150),
        frgb(0, 120, 215),
        frgb(220, 50, 47),
        frgb(42, 157, 143),
        frgb(255, 255, 255),
        frgb(255, 255, 220),
        frgb(0, 0, 0),
        {frgb(252, 252, 252), frgb(245, 245, 245), frgb(235, 235, 235), frgb(225, 225, 225)},
        {frgb(170, 170, 170), frgb(170, 170, 170), frgb(170, 170, 170),
         frgb(255, 255, 255), frgb(255, 255, 255), frgb(255, 255, 255), 0, 0, nullptr},
        {frgb(240, 248, 255), frgb(220, 238, 255), frgb(200, 228, 255), frgb(180, 218, 255)},
        {frgb(0, 120, 215), frgb(0, 120, 215), frgb(0, 120, 215),
         frgb(255, 255, 255), frgb(255, 255, 255), frgb(255, 255, 255), 0, 0, nullptr},
        {frgb(210, 210, 210), frgb(200, 200, 200), frgb(190, 190, 190), frgb(180, 180, 180)},
        {frgb(150, 150, 150), frgb(150, 150, 150), frgb(150, 150, 150), 0, 0, 0, 0, 0, nullptr},
        {frgb(0, 140, 235), frgb(0, 120, 215), frgb(0, 100, 195), frgb(0, 80, 175)},
        {frgb(0, 60, 155), frgb(0, 60, 155), frgb(0, 60, 155),
         frgb(100, 180, 255), frgb(100, 180, 255), frgb(100, 180, 255), 0, 0, nullptr},
        {frgb(180, 180, 180), frgb(180, 180, 180), frgb(180, 180, 180), 0, 0, 0, 0, 0, nullptr},
        {frgb(150, 150, 150), frgb(150, 150, 150), frgb(150, 150, 150), 0, 0, 0, 0, 0, nullptr},
        frgb(255, 255, 255),
        {frgb(180, 180, 180), frgb(180, 180, 180), frgb(180, 180, 180), 0, 0, 0, 0, 0, nullptr},
        frgb(0, 120, 215),
        frgb(30, 30, 30),
        frgb(150, 150, 150),
        frgb(0, 120, 215),
        frgb(30, 30, 30),
    };

    inline ThemePalette FUTURISTIC_DARK_PALETTE = {
        frgb(20, 20, 35),
        frgb(30, 30, 45),
        frgb(230, 230, 240),
        frgb(80, 80, 100),
        frgb(0, 255, 255),
        frgb(255, 0, 128),
        frgb(0, 255, 128),
        frgb(30, 30, 45),
        frgb(40, 40, 55),
        frgb(230, 230, 240),
        {frgb(50, 50, 70), frgb(40, 40, 60), frgb(35, 35, 55), frgb(30, 30, 50)},
        {frgb(80, 80, 100), frgb(80, 80, 100), frgb(80, 80, 100), 0, 0, 0, 0, 0, nullptr},
        {frgb(90, 90, 130), frgb(70, 70, 110), frgb(65, 65, 105), frgb(55, 55, 95)},
        {frgb(0, 255, 255), frgb(0, 255, 255), frgb(0, 255, 255), 0, 0, 0, 0, 0, nullptr},
        {frgb(15, 15, 30), frgb(25, 25, 40), frgb(25, 25, 40), frgb(35, 35, 55)},
        {frgb(0, 255, 255), frgb(0, 255, 255), frgb(0, 255, 255), 0, 0, 0, 0, 0, nullptr},
        {frgb(255, 0, 128), frgb(128, 128, 255), frgb(128, 128, 255), frgb(0, 255, 255)},
        {frgb(0, 200, 200), frgb(0, 200, 200), frgb(0, 200, 200), 0, 0, 0, 0, 0, nullptr},
        {frgb(80, 80, 100), frgb(80, 80, 100), frgb(80, 80, 100), 0, 0, 0, 0, 0, nullptr},
        {frgb(80, 80, 100), frgb(80, 80, 100), frgb(80, 80, 100), 0, 0, 0, 0, 0, nullptr},
        frgb(20, 20, 35),
        {frgb(80, 80, 100), frgb(80, 80, 100), frgb(80, 80, 100), 0, 0, 0, 0, 0, nullptr},
        frgb(255, 0, 255),
        frgb(230, 230, 240),
        frgb(80, 80, 100),
        frgb(80, 80, 100),
        frgb(230, 230, 240),
    };

    inline ThemePalette FUTURISTIC_DARK_ORANGE_PALETTE = {
        frgb(20, 20, 35),
        frgb(30, 30, 45),
        frgb(230, 230, 240),
        frgb(80, 80, 100),
        frgb(255, 150, 50),
        frgb(255, 0, 128),
        frgb(0, 255, 128),
        frgb(30, 30, 45),
        frgb(40, 40, 55),
        frgb(230, 230, 240),
        {frgb(50, 50, 70), frgb(40, 40, 60), frgb(35, 35, 55), frgb(30, 30, 50)},
        {frgb(80, 80, 100), frgb(80, 80, 100), frgb(80, 80, 100), 0, 0, 0, 0, 0, nullptr},
        {frgb(90, 90, 130), frgb(70, 70, 110), frgb(65, 65, 105), frgb(55, 55, 95)},
        {frgb(255, 150, 50), frgb(255, 150, 50), frgb(255, 150, 50), 0, 0, 0, 0, 0, nullptr},
        {frgb(15, 15, 30), frgb(25, 25, 40), frgb(25, 25, 40), frgb(35, 35, 55)},
        {frgb(255, 150, 50), frgb(255, 150, 50), frgb(255, 150, 50), 0, 0, 0, 0, 0, nullptr},
        {frgb(255, 0, 128), frgb(255, 75, 90), frgb(255, 75, 90), frgb(255, 150, 50)},
        {frgb(220, 120, 40), frgb(220, 120, 40), frgb(220, 120, 40), 0, 0, 0, 0, 0, nullptr},
        {frgb(80, 80, 100), frgb(80, 80, 100), frgb(80, 80, 100), 0, 0, 0, 0, 0, nullptr},
        {frgb(80, 80, 100), frgb(80, 80, 100), frgb(80, 80, 100), 0, 0, 0, 0, 0, nullptr},
        frgb(20, 20, 35),
        {frgb(80, 80, 100), frgb(80, 80, 100), frgb(80, 80, 100), 0, 0, 0, 0, 0, nullptr},
        frgb(50, 50, 255),
        frgb(230, 230, 240),
        frgb(80, 80, 100),
        frgb(80, 80, 100),
        frgb(230, 230, 240),
    };

    inline ThemePalette SLATE_TEAL_PALETTE = {
        frgb(49, 63, 64),
        frgb(44, 54, 56),
        frgb(160, 174, 173),
        frgb(80, 95, 96),
        frgb(107, 178, 160),
        frgb(224, 108, 117),
        frgb(152, 195, 121),
        frgb(44, 54, 56),
        frgb(35, 45, 46),
        frgb(230, 240, 240),
        {frgb(60, 75, 76), frgb(55, 70, 71), frgb(49, 63, 64), frgb(45, 59, 60)},
        {frgb(80, 95, 96), frgb(80, 95, 96), frgb(80, 95, 96), 0, 0, 0, 0, 0, nullptr},
        {frgb(70, 85, 86), frgb(65, 80, 81), frgb(59, 73, 74), frgb(55, 69, 70)},
        {frgb(107, 178, 160), frgb(107, 178, 160), frgb(107, 178, 160), 0, 0, 0, 0, 0, nullptr},
        {frgb(40, 50, 51), frgb(45, 55, 56), frgb(45, 55, 56), frgb(49, 63, 64)},
        {frgb(107, 178, 160), frgb(107, 178, 160), frgb(107, 178, 160), 0, 0, 0, 0, 0, nullptr},
        {frgb(127, 198, 180), frgb(107, 178, 160), frgb(97, 168, 150), frgb(87, 158, 140)},
        {frgb(77, 148, 130), frgb(77, 148, 130), frgb(77, 148, 130), 0, 0, 0, 0, 0, nullptr},
        {frgb(80, 95, 96), frgb(80, 95, 96), frgb(80, 95, 96), 0, 0, 0, 0, 0, nullptr},
        {frgb(80, 95, 96), frgb(80, 95, 96), frgb(80, 95, 96), 0, 0, 0, 0, 0, nullptr},
        frgb(38, 49, 50),
        {frgb(80, 95, 96), frgb(80, 95, 96), frgb(80, 95, 96), 0, 0, 0, 0, 0, nullptr},
        frgb(255, 123, 0),
        frgb(160, 174, 173),
        frgb(80, 95, 96),
        frgb(107, 178, 160),
        frgb(160, 174, 173),
    };

    struct NamedPalette
    {
        std::string name;
        ThemePalette palette;
    };

    inline std::vector<NamedPalette> &get_theme_palettes()
    {
        static std::vector<NamedPalette> palettes = {
            {"Dark", DARK_PALETTE},
            {"Futuristic Dark", FUTURISTIC_DARK_PALETTE},
            {"Futuristic Dark Orange", FUTURISTIC_DARK_ORANGE_PALETTE},
            {"Slate Teal", SLATE_TEAL_PALETTE},
        };
        return palettes;
    }

    inline void add_theme_palette(std::string name, const ThemePalette &palette)
    {
        get_theme_palettes().push_back({std::move(name), palette});
    }
}
