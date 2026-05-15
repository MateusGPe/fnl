#pragma once
#include "Policies.hpp"

#include <FL/Fl_Adjuster.H>
#include <FL/Fl_Chart.H>
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
    using Adjuster = Focusable<CallbackRouter<Fl_Adjuster>>;
    using HelpView = Focusable<CallbackRouter<Fl_Help_View>>;
    using MenuButton = Focusable<CallbackRouter<Fl_Menu_Button>>;

    using MultilineOutput = Focusable<CallbackRouter<Fl_Multiline_Output>>;
    using Counter = Focusable<CallbackRouter<Fl_Counter>>;
    using TextEditor = Focusable<CallbackRouter<Fl_Text_Editor>>;

    using Output = RingHover<CallbackRouter<Fl_Output>>;
    using SecretInput = Blinkable<RingHover<CallbackRouter<Fl_Secret_Input>>>;
    using Input = Blinkable<RingHover<CallbackRouter<Fl_Input>>>;
    using ValueInput = RingHover<CallbackRouter<Fl_Value_Input>>;
    using Roller = RingHover<CallbackRouter<Fl_Roller>>;
    using ValueOutput = RingHover<CallbackRouter<Fl_Value_Output>>;
    using Chart = RingHover<CallbackRouter<Fl_Chart>>;
    using Clock = RingHover<CallbackRouter<Fl_Clock>>;
    using Dial = RingHover<CallbackRouter<Fl_Dial>>;


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