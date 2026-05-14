#pragma once
#include "Policies.hpp"

#include <FL/Fl_Adjuster.H>
#include <FL/Fl_Chart.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Clock.H>
#include <FL/Fl_Counter.H>
#include <FL/Fl_Dial.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Help_View.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Repeat_Button.H>
#include <FL/Fl_Roller.H>
#include <FL/Fl_Secret_Input.H>
#include <FL/Fl_Table.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Value_Output.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Wizard.H>

#include <FL/Fl_Flex.H>
#include <FL/Fl_Grid.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Wizard.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>

namespace mui
{
    using namespace policy;
    using Adjuster = Focusable<CallbackRouter<Fl_Adjuster>>;
    using Chart = Focusable<CallbackRouter<Fl_Chart>>;
    using CheckButton = Focusable<CallbackRouter<Fl_Check_Button>>;
    using Clock = Focusable<CallbackRouter<Fl_Clock>>;
    using Counter = Focusable<CallbackRouter<Fl_Counter>>;
    using Dial = Focusable<CallbackRouter<Fl_Dial>>;
    using DoubleWindow = Focusable<CallbackRouter<Fl_Double_Window>>;
    using HelpView = Focusable<CallbackRouter<Fl_Help_View>>;
    using Input = Focusable<CallbackRouter<Fl_Input>>;
    using MenuButton = Focusable<CallbackRouter<Fl_Menu_Button>>;
    using MultilineOutput = Focusable<CallbackRouter<Fl_Multiline_Output>>;
    using Output = Focusable<CallbackRouter<Fl_Output>>;
    using RepeatButton = Focusable<CallbackRouter<Fl_Repeat_Button>>;
    using Roller = Focusable<CallbackRouter<Fl_Roller>>;
    using SecretInput = Focusable<CallbackRouter<Fl_Secret_Input>>;
    using TextEditor = Focusable<CallbackRouter<Fl_Text_Editor>>;
    using ValueInput = Focusable<CallbackRouter<Fl_Value_Input>>;
    using ValueOutput = Focusable<CallbackRouter<Fl_Value_Output>>;
    using Window = Focusable<CallbackRouter<Fl_Window>>;

    class InputChoice : public CallbackRouter<Fl_Input_Choice>
    {
    protected:
        using FlBase = CallbackRouter<Fl_Input_Choice>;
        void draw() override
        {
            // This is copied from Fl_Choice::draw() and customized
            Fl_Boxtype btype = FL_UP_BOX; // default scheme uses down box
            int dx = Fl::box_dx(btype);
            int dy = Fl::box_dy(btype);

            // From "original" code: modify the box color *only* for the default scheme.
            // This is weird (why?). I believe we should either make sure that the text
            // color contrasts well when the text is rendered *or* we should do this for
            // *all* schemes. Anyway, adapting the old code... (Albrecht)
            //
            Fl_Color box_color = color();

            // Draw the widget box
            draw_box(btype, box_color);

            // Draw menu button
            //draw_child(*menubutton());

            // Draw vertical divider lines for: gtk+, gleam, oxy
            //
            // Scheme:            Box or divider line
            // ----------------------------------------
            // Default (None):    Arrow box (FL_UP_BOX)
            // gtk+, gleam, oxy:  Divider line
            // else:              Nothing - Fl_Group::box() shows through
            //
            int woff = 2;
            int x1 = menu_x() - dx;
            int y1 = y() + dy;
            int y2 = y() + h() - dy;

            //fl_color(fl_darker(color()));
            //fl_yxline(x1 + 0, y1, y2);
//
            //fl_color(fl_lighter(color()));
            //fl_yxline(x1 + 1, y1, y2);

            // Draw the input field
            fl_push_clip(inp_x(), inp_y(), inp_w() - woff, inp_h());
            draw_child(*input());
            fl_pop_clip();

            // Widget's label
            draw_label();
            const bool is_focused = ((Fl::focus() == input() || Fl::focus() == menubutton()) && FlBase::visible_focus() && Fl::visible_focus());
            if (is_focused)
                Focusable<FlBase>::draw_box_focus(FL_NO_BOX, FlBase::x(), FlBase::y(), FlBase::w(), FlBase::h());
        }

    public:
        template <typename... Args>
        InputChoice(Args &&...args) : FlBase(std::forward<Args>(args)...)
        {
            Fl_Group::box(Theme::schemes::BUTTON_DOWN_BOX);
        }
    };
    using Table = Fl_Table;
    using Flex = Fl_Flex;
    using Grid = Fl_Grid;
    using Pack = Fl_Pack;
    using Scroll = Fl_Scroll;
    using Tile = Fl_Tile;
    using Wizard = Fl_Wizard;
    using Group = Fl_Group;
    using Box = Fl_Box;
}