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
#include <FL/Fl_Wizard.H>

namespace mui
{
    using Adjuster = policy::CallbackRouter<Fl_Adjuster>;
    using Chart = policy::CallbackRouter<Fl_Chart>;
    using CheckButton = policy::CallbackRouter<Fl_Check_Button>;
    using Clock = policy::CallbackRouter<Fl_Clock>;
    using Counter = policy::CallbackRouter<Fl_Counter>;
    using Dial = policy::CallbackRouter<Fl_Dial>;
    using DoubleWindow = policy::CallbackRouter<Fl_Double_Window>;
    using HelpView = policy::CallbackRouter<Fl_Help_View>;
    using Input = policy::CallbackRouter<Fl_Input>;
    using MenuButton = policy::CallbackRouter<Fl_Menu_Button>;
    using MultilineOutput = policy::CallbackRouter<Fl_Multiline_Output>;
    using Output = policy::CallbackRouter<Fl_Output>;
    using RepeatButton = policy::CallbackRouter<Fl_Repeat_Button>;
    using Roller = policy::CallbackRouter<Fl_Roller>;
    using SecretInput = policy::CallbackRouter<Fl_Secret_Input>;
    using Table = policy::CallbackRouter<Fl_Table>;
    using TextEditor = policy::CallbackRouter<Fl_Text_Editor>;
    using ValueInput = policy::CallbackRouter<Fl_Value_Input>;
    using ValueOutput = policy::CallbackRouter<Fl_Value_Output>;
    using Window = policy::CallbackRouter<Fl_Window>;
}