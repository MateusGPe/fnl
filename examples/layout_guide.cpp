#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Grid.H>
#include <FL/Fl_Flex.H>
#include "mui_img/ImageViewer.hpp"
#include "mui.hpp"

class AppWindow : public Fl_Double_Window
{
public:
    AppWindow(int w, int h, const char *title) : Fl_Double_Window(w, h, title)
    {
        end();

        auto *layer_tree = mui::make<mui::Tree>();
        auto *viewer = mui::make<mui::ImageViewer>();
        auto *btn_new = mui::make<mui::Button>("New");
        auto *btn_open = mui::make<mui::Button>("Open");
        auto *btn_save = mui::make<mui::Button>("Save");
        auto *tool_mode = mui::make<mui::Choice>("Select Tool");
        auto *sl_opacity = mui::make<mui::ValueSlider>();
        auto *sp_radius = mui::make<mui::Spinner>();
        auto *btn_apply = mui::make<mui::Button>("Apply Filter");

        viewer->grid(true);
        sl_opacity->type(FL_HOR_SLIDER);
        sl_opacity->align(FL_ALIGN_TOP_LEFT);
        sp_radius->align(FL_ALIGN_TOP_LEFT);

        Fl_Grid *grid = mui::make_grid(
            w, h,
            mui::Layout(2, 3),
            mui::Margin(6),
            mui::Gap(5),
            mui::ColWeights(0, 1, 0),
            mui::ColWidths(200, 0, 220),
            mui::RowHeights(35),
            mui::RowWeights(0, 1),
            mui::Cell(
                mui::make_hbox(
                    8,
                    mui::Fix(btn_new, 70),
                    mui::Fix(btn_open, 70),
                    mui::Fix(btn_save, 70),
                    mui::Spacer(),
                    mui::Fix(tool_mode, 120)),
                0, 0, 1, 3),
            mui::Cell(layer_tree, 1, 0),
            mui::Cell(viewer, 1, 1),
            mui::Cell(
                mui::make_vbox(
                    12,
                    mui::Fix(sl_opacity, 30),
                    mui::Fix(sp_radius, 30),
                    mui::Fix(btn_apply, 35),
                    mui::Spacer()),
                1, 2));

        add(grid);
        resizable(grid);
        size_range(w, h);
    }
};

int main(int argc, char **argv)
{
    mui::System::init_threads();
    mui::Theme::apply();

    AppWindow *win = new AppWindow(600, 400, "MUI Responsive App Shell");

    win->show(argc, argv);

    return mui::System::run();
}