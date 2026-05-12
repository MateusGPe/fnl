#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Grid.H>
#include <FL/Fl_Flex.H>
#include "mui.hpp"

class AppWindow : public Fl_Double_Window
{
public:
    AppWindow(int w, int h, const char *title) : Fl_Double_Window(w, h, title)
    {
        auto *btn_new = mui::make<mui::Button>("New");
        auto *btn_open = mui::make<mui::Button>("Open");
        auto *btn_save = mui::make<mui::Button>("Save");
        auto *tool_mode = mui::make<mui::Choice>("Select Tool");

        auto toolbar = mui::make_hbox(
            8,
            mui::Fix(btn_new, 70),
            mui::Fix(btn_open, 70),
            mui::Fix(btn_save, 70),
            mui::Spacer(),
            mui::Fix(tool_mode, 120));

        mui::Tree *layer_tree = new mui::Tree(0, 0, 0, 0);

        mui::ImageViewer *viewer = new mui::ImageViewer(0, 0, 0, 0);
        viewer->grid(true);

        auto *sl_opacity = mui::make<mui::ValueSlider>();
        auto *sp_radius = mui::make<mui::Spinner>();
        auto *btn_apply = mui::make<mui::Button>("Apply Filter");

        sl_opacity->type(FL_HOR_SLIDER);
        sl_opacity->align(FL_ALIGN_TOP_LEFT);
        sp_radius->align(FL_ALIGN_TOP_LEFT);

        auto *properties = mui::make_vbox(
            12,
            mui::Fix(sl_opacity, 30),
            mui::Fix(sp_radius, 30),
            mui::Fix(btn_apply, 35),
            mui::Spacer());

        Fl_Grid *grid = mui::make_grid(
            w, h,
            mui::Layout(2, 3),
            mui::Margin(10),
            mui::Gap(10),
            mui::ColWeights(0, 1, 0),
            mui::RowWeights(0, 1),
            mui::ColWidths(200, 0, 220),
            mui::RowHeights(35),

            mui::Cell(toolbar, 0, 0, 1, 3),
            mui::Cell(layer_tree, 1, 0),
            mui::Cell(viewer, 1, 1),
            mui::Cell(properties, 1, 2));

        this->resizable(grid);
        this->size_range(600, 400);
    }
};

int main(int argc, char **argv)
{
    mui::System::init_threads();
    mui::Theme::apply();

    AppWindow *win = new AppWindow(1024, 768, "MUI Responsive App Shell");

    win->show(argc, argv);

    return mui::System::run();
}