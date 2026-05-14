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

#include <iostream>

namespace mui
{
    using namespace policy;
    using Window = Focusable<CallbackRouter<Fl_Window>>;
    using Adjuster = Focusable<CallbackRouter<Fl_Adjuster>>;
    using DoubleWindow = Focusable<CallbackRouter<Fl_Double_Window>>;
    using HelpView = Focusable<CallbackRouter<Fl_Help_View>>;
    using MenuButton = Focusable<CallbackRouter<Fl_Menu_Button>>;

    using MultilineOutput = HoverTracker<Focusable<CallbackRouter<Fl_Multiline_Output>>, true>;
    using Output = HoverTracker<Focusable<CallbackRouter<Fl_Output>>, true>;
    using Input = HoverTracker<Focusable<CallbackRouter<Fl_Input>>, true>;
    using SecretInput = HoverTracker<Focusable<CallbackRouter<Fl_Secret_Input>>, true>;

    using RepeatButton = HoverTracker<Focusable<CallbackRouter<Fl_Repeat_Button>>, true>;
    using Roller = HoverTracker<Focusable<CallbackRouter<Fl_Roller>>, true>;
    using TextEditor = HoverTracker<Focusable<CallbackRouter<Fl_Text_Editor>>, true>;
    using ValueInput = HoverTracker<Focusable<CallbackRouter<Fl_Value_Input>>, true>;
    using ValueOutput = HoverTracker<Focusable<CallbackRouter<Fl_Value_Output>>, true>;
    using Chart = HoverTracker<Focusable<CallbackRouter<Fl_Chart>>, true>;
    using CheckButton = HoverTracker<Focusable<CallbackRouter<Fl_Check_Button>>, true>;
    using Clock = HoverTracker<Focusable<CallbackRouter<Fl_Clock>>, true>;
    using Counter = HoverTracker<Focusable<CallbackRouter<Fl_Counter>>, true>;
    using Dial = HoverTracker<Focusable<CallbackRouter<Fl_Dial>>, true>;

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