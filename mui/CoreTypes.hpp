#pragma once
#include <cstdint>
#include <FL/Enumerations.H>

namespace mui
{
    enum class Align
    {
        Center,
        Left,
        Right,
        Top,
        Bottom,
        Inside
    };
    // Values intentionally match Fl_Font enumerations for zero-overhead casting
    enum class Font
    {
        Regular = FL_HELVETICA,
        Bold = FL_HELVETICA_BOLD,
        Italic = FL_HELVETICA_ITALIC,
        BoldItalic = FL_HELVETICA_BOLD_ITALIC
    };

    using Color = uint32_t;

    inline Fl_Font to_fl_font(mui::Font f) { return static_cast<Fl_Font>(f); }
    inline Fl_Color to_fl_color(mui::Color c) { return static_cast<Fl_Color>(c); }
}