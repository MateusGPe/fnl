#include "mui.hpp"
#include <string>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Output.H>

int main()
{
    mui::System::init_threads();
    mui::Theme::apply();

    Fl_Window *window = new Fl_Window(400, 200, "MUI Example");
    window->begin();

    Fl_Output *output_box = new Fl_Output(20, 40, 360, 50, "Hello, MUI!");
    output_box->align(FL_ALIGN_CENTER);
    output_box->textsize(20);

    Fl_Button *button = new Fl_Button(150, 120, 100, 40, "Click Me!");
    window->end();

    button->callback(
        [](Fl_Widget *, void *v)
        {
            auto output_box = static_cast<Fl_Output *>(v);
            static int click_count = 0;
            click_count++;
            std::string message = "Clicked " + std::to_string(click_count) + " times!";
            output_box->value(message.c_str());
        },
        output_box);

    window->show();

    return mui::System::run();
}
