#include "mui.hpp"
#include <string> // For std::to_string
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Output.H>

int main()
{
    // It's good practice to initialize threads support, especially for cross-platform apps.
    // This calls Fl::lock() to make FLTK thread-safe.
    mui::System::init_threads();

    // Apply a theme. MUI comes with several built-in themes.
    // You can try others like: Dark, Metro, Greybird, Blue, etc.
    mui::Theme::apply(mui::ThemeType::Win10);

    // Create the main window.
    // Fl_Window is a styled wrapper around Fl_Window.
    Fl_Window *window = new Fl_Window(400, 200, "MUI Example");

    // Widgets added between begin() and end() automatically become children of the window.
    window->begin();

    // Create an output box to display text.
    Fl_Output *output_box = new Fl_Output(20, 40, 360, 50, "Hello, MUI!");
    output_box->align(FL_ALIGN_CENTER);
    output_box->textsize(20);

    // Create a button.
    Fl_Button *button = new Fl_Button(150, 120, 100, 40, "Click Me!");

    // Finish adding widgets to the window.
    window->end();

    // Set up a callback for the button using a C++ lambda.
    button->callback([](Fl_Widget*, void* v) {
        auto output_box = static_cast<Fl_Output*>(v);
        static int click_count = 0;
        click_count++;
        std::string message = "Clicked " + std::to_string(click_count) + " times!";
        output_box->value(message.c_str()); 
    }, output_box);

    // Show the window.
    window->show();

    // Run the application's event loop. This function will return when all windows are closed.
    return mui::System::run();
}
