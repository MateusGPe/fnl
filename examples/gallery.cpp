#include <iostream>
#include <string>
#include <cstdio>
#include <vector>
#include <cstdlib>
#include <mui.hpp>
#include <FL/Fl_Window.H>
#include <mui/Theme/data.hpp>
#include <FL/Fl_Text_Buffer.H>
#include <FL/fl_draw.H>

class DemoTable : public mui::Table
{
protected:
    void draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H) override
    {
        static char s[40];
        switch (context)
        {
        case CONTEXT_COL_HEADER:
        case CONTEXT_ROW_HEADER:
            fl_push_clip(X, Y, W, H);
            fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, mui::ThemeManager::get_palette().bg_sec);
            fl_color(mui::ThemeManager::get_palette().inactive);
            snprintf(s, 40, context == CONTEXT_COL_HEADER ? "Col %d" : "Row %d", context == CONTEXT_COL_HEADER ? C : R);
            fl_draw(s, X, Y, W, H, FL_ALIGN_CENTER);
            fl_pop_clip();
            return;
        case CONTEXT_CELL:
            fl_push_clip(X, Y, W, H);
            fl_draw_box(FL_THIN_DOWN_BOX, X, Y, W, H, mui::ThemeManager::get_palette().bg_main);
            fl_color(mui::ThemeManager::get_palette().fg_main);
            snprintf(s, 40, "R%d/C%d", R, C);
            fl_draw(s, X, Y, W, H, FL_ALIGN_CENTER);
            fl_pop_clip();
            return;
        default:
            return;
        }
    }

public:
    DemoTable(int X, int Y, int W, int H, const char *l = 0) : mui::Table(X, Y, W, H, l)
    {
        end();
    }
};

class CustomCanvas : public Fl_Widget
{
protected:
    void draw() override
    {
        fl_color(mui::ThemeManager::get_palette().bg_sec);
        fl_rectf(x(), y(), w(), h());
        fl_color(mui::ThemeManager::get_palette().selection);
        fl_pie(x() + 10, y() + 10, w() - 20, h() - 20, 0, 240);
        fl_color(mui::ThemeManager::get_palette().fg_main);
        fl_draw("Custom Draw", x(), y(), w(), h(), FL_ALIGN_CENTER);
    }

public:
    CustomCanvas(int x, int y, int w, int h, const char *l = 0) : Fl_Widget(x, y, w, h, l) {}
};

class WidgetGallery
{
private:
    Fl_Double_Window *m_win;
    Fl_Text_Buffer *m_editor_buffer;
    mui::Wizard *m_wizard_handle = nullptr;
    mui::MenuBar *m_menu_bar_handle = nullptr;
    std::vector<std::string> m_theme_strings;
    mui::Progress *m_progress_handle = nullptr;

public:
    WidgetGallery()
    {
        m_editor_buffer = new Fl_Text_Buffer();
        m_editor_buffer->text("This is a Fl_Text_Editor.\nIt supports rich text operations.\n");
    }

    ~WidgetGallery()
    {
        delete m_editor_buffer;
    }

    static void on_generic_click(Fl_Widget *, void *) { std::cout << "Event: Button clicked." << std::endl; }
    static void on_generic_change(Fl_Widget *, void *) { std::cout << "Event: Value changed." << std::endl; }
    static void on_generic_toggle(Fl_Widget *, void *) { std::cout << "Event: State toggled." << std::endl; }
    static void on_generic_select(Fl_Widget *, void *) { std::cout << "Event: Item selected." << std::endl; }
    static void on_quit(Fl_Widget *, void *v) { static_cast<WidgetGallery *>(v)->m_win->hide(); }

    static void on_theme_menu_select(Fl_Widget *w, void *v)
    {
        auto *bar = static_cast<mui::MenuBar *>(w);
        const Fl_Menu_Item *item = bar->mvalue();
        if (!item || !item->label())
            return;

        std::string path = item->label();
        size_t last_slash = path.rfind('/');
        std::string name = (last_slash != std::string::npos) ? path.substr(last_slash + 1) : path;

        for (const auto &named_palette : mui::get_theme_palettes())
        {
            if (named_palette.name == name)
            {
                mui::ThemeManager::set_palette(named_palette.palette);
            }
        }
    }

    static void on_file_chooser(Fl_Widget *, void *)
    {
        mui::NativeFileChooser chooser(mui::NativeFileChooser::BrowseFile);
        chooser.title("Open File");
        if (chooser.show())
        {
            std::cout << "File Chooser: Selected " << chooser.filename() << std::endl;
        }
        else
        {
            std::cout << "File Chooser: Canceled." << std::endl;
        }
    }

    static void on_wizard_next(Fl_Widget *, void *v)
    {
        auto app = static_cast<WidgetGallery *>(v);
        if (app->m_wizard_handle)
            app->m_wizard_handle->next();
    }

    static void on_wizard_prev(Fl_Widget *, void *v)
    {
        auto app = static_cast<WidgetGallery *>(v);
        if (app->m_wizard_handle)
            app->m_wizard_handle->prev();
    }

    void build_ui()
    {
        m_win = new Fl_Double_Window(800, 600, "MUI Widget Gallery");
        m_win->begin();

        m_menu_bar_handle = new mui::MenuBar(0, 0, 800, 30);
        {
            m_menu_bar_handle->add("File/Quit", 0, on_quit, this);
            m_menu_bar_handle->add("Help/About");

            m_menu_bar_handle->add("Theme", 0, 0, 0, FL_SUBMENU);
            bool first = true;
            for (const auto &named_palette : mui::get_theme_palettes())
            {
                m_theme_strings.push_back("Theme/" + named_palette.name);
                int flags = FL_MENU_RADIO;
                if (first)
                {
                    flags |= FL_MENU_VALUE;
                    first = false;
                }
                m_menu_bar_handle->add(m_theme_strings.back().c_str(), 0, on_theme_menu_select, this, flags);
            }
        }

        mui::Tabs *tabs = new mui::Tabs(10, 40, 780, 550);
        tabs->end();

        const int tx = 10, ty = 75, tw = 780, th = 515;

        auto tab1 = [&] { // --- Tab 1: Basic Controls ---
            auto *btn = mui::build<mui::Button>("Button")
                            ->callback(on_generic_click, this)
                            ->end();
            auto *rbtn = mui::build<mui::ReturnButton>("Return")
                             ->callback(on_generic_click, this)
                             ->end();
            auto *lbtn = mui::build<mui::LightButton>("Light")
                             ->callback(on_generic_toggle, this)
                             ->value(1)
                             ->end();
            auto *rrbtn = mui::build<mui::RepeatButton>("Repeat")
                              ->callback(on_generic_toggle, this)
                              ->end();

            auto *cbtn = mui::build<mui::CheckButton>("Check")
                             ->callback(on_generic_toggle, this)
                             ->value(1)
                             ->end();
            auto *rad1 = mui::build<mui::RadioButton>("Radio 1")
                             ->callback(on_generic_toggle, this)
                             ->value(1)
                             ->end();
            auto *rad2 = mui::build<mui::RadioButton>("Radio 2")
                             ->callback(on_generic_toggle, this)
                             ->end();

            auto *inp = mui::build<mui::Input>()
                            ->value("Text")
                            ->end();
            auto *sinp = mui::build<mui::SecretInput>()
                             ->value("secret")
                             ->end();
            auto *ch = mui::build<mui::Choice>()
                           ->callback(on_generic_select, this)
                           ->add("Option A|Option B|Option C")
                           ->value(1)
                           ->end();

            auto *sl = mui::build<mui::Slider>()
                           ->callback(on_generic_change, this)
                           ->type(FL_HOR_SLIDER)
                           ->bounds(0, 100)
                           ->value(50)
                           ->end();

            m_progress_handle = mui::build<mui::Progress>()
                                    ->minimum(0)
                                    ->maximum(100)
                                    ->value(65)
                                    ->rate(0.06)
                                    ->step(0.05)
                                    ->end();
            // m_progress_handle->indeterminate(true);

            auto *animate_prg_btn = mui::build<mui::RepeatButton>("Animate")
                                        ->callback(
                                            [](Fl_Widget *, void *data)
                                            {
                                                auto p = static_cast<mui::Progress *>(data);
                                                if (p->value() < 100)
                                                    p->value(int(p->value() + 1) % 101);
                                                else
                                                    p->value(0);
                                            },
                                            m_progress_handle)
                                        ->end();

            auto *ic = mui::build<mui::InputChoice>()
                           ->add("Choice 1")
                           ->add("Choice 2")
                           ->add("Choice 3")
                           ->end();

            auto *tab1 = mui::make_vbox(
                tx, ty, tw, th, 10,
                mui::Fix(mui::make_header("Buttons & Toggles"), 20),
                mui::Fix(mui::make_hbox(10, mui::Fix(btn, 80), mui::Fix(rbtn, 80), mui::Fix(lbtn, 80), mui::Fix(rrbtn, 80), mui::Spacer()), 30),
                mui::Fix(mui::make_hbox(10, mui::Fix(cbtn, 80), mui::Fix(rad1, 80), mui::Fix(rad2, 80), mui::Spacer()), 30),
                mui::Fix(mui::make_header("Inputs & Choices"), 20),
                mui::Fix(mui::make_hbox(10, mui::Stretch(inp), mui::Stretch(sinp), mui::Stretch(ch), mui::Stretch(ic)), 30),
                mui::Fix(mui::make_header("Sliders & Progress"), 20),
                mui::Fix(mui::make_hbox(10, mui::Stretch(sl)), 30),
                mui::Fix(mui::make_hbox(10, mui::Stretch(m_progress_handle), mui::Fix(animate_prg_btn, 80)), 30),
                mui::Spacer());
            tab1->margin(20);
            tab1->label("Basic Controls");
            return tab1;
        };

        auto tab2 = [&] { // --- Tab 2: Valuators ---
            auto *vi = mui::build<mui::ValueInput>()
                           ->callback(on_generic_change, this)
                           ->bounds(0, 100)
                           ->step(0.1)
                           ->value(123)
                           ->end();
            auto *sp = mui::build<mui::Spinner>()
                           ->callback(on_generic_change, this)
                           ->minimum(0)
                           ->maximum(50)
                           ->step(1)
                           ->value(10)
                           ->end();
            auto *ct = mui::build<mui::Counter>()
                           ->callback(on_generic_change, this)
                           ->minimum(0)
                           ->maximum(20)
                           ->step(1, 5)
                           ->value(5)
                           ->end();
            auto *adj = mui::build<mui::Adjuster>()
                            ->callback(on_generic_change, this)
                            ->minimum(-1)
                            ->maximum(1)
                            ->value(0)
                            ->end();
            auto *vs = mui::build<mui::ValueSlider>()
                           ->callback(on_generic_change, this)
                           ->type(FL_HOR_SLIDER)
                           ->bounds(0, 100)
                           ->value(25)
                           ->end();
            auto *dial = mui::build<mui::Dial>("Dial")
                             ->callback(on_generic_change, this)
                             ->bounds(0, 360)
                             ->value(90)
                             ->end();
            auto *roller = mui::build<mui::Roller>("Roller")
                               ->callback(on_generic_change, this)
                               ->bounds(0, 100)
                               ->value(30)
                               ->step(0.1)
                               ->end();

            auto *tab2 = mui::make_vbox(
                tx, ty, tw, th, 10,
                mui::Fix(mui::make_hbox(10, mui::Fix(mui::make_label("ValueInput:"), 100), mui::Stretch(vi)), 24),
                mui::Fix(mui::make_hbox(10, mui::Fix(mui::make_label("Spinner:"), 100), mui::Stretch(sp)), 30),
                mui::Fix(mui::make_hbox(10, mui::Fix(mui::make_label("Counter:"), 100), mui::Stretch(ct)), 30),
                mui::Fix(mui::make_hbox(10, mui::Fix(mui::make_label("Adjuster:"), 100), mui::Stretch(adj)), 30),
                mui::Fix(mui::make_hbox(10, mui::Fix(mui::make_label("ValueSlider:"), 100), mui::Stretch(vs)), 30),
                mui::FixedSpacer(20),
                mui::Fix(mui::make_hbox(20, mui::Fix(dial, 60), mui::Fix(roller, 30), mui::Spacer()), 60),
                mui::Spacer());
            tab2->margin(20);
            tab2->label("Valuators");
            return tab2;
        };

        auto tab3 = [&] { // --- Tab 3: Data Display ---
            auto *out = mui::build<mui::Output>()
                            ->value("Read-only")
                            ->end();
            auto *vout = mui::build<mui::ValueOutput>()
                             ->value(3.14159)
                             ->end();
            auto *mout = mui::build<mui::MultilineOutput>()
                             ->value("Line 1\nLine 2\nLine 3")
                             ->end();
            auto *br = mui::build<mui::Browser>()
                           ->callback(on_generic_select, this)
                           ->type(FL_MULTI_BROWSER)
                           ->add("Item 1")
                           ->add("Item 2")
                           ->add("Item 3")
                           ->end();
            auto *tr = mui::build<mui::Tree>()
                           ->callback(on_generic_select, this)
                           ->add("Root/Branch1/Leaf")
                           ->add("Root/Branch2")
                           ->end();

            auto *tab3 = mui::make_vbox(
                tx, ty, tw, th, 20,
                mui::Fix(mui::make_hbox(10, mui::Stretch(out), mui::Stretch(vout)), 30),
                mui::Stretch(mout),
                mui::Stretch(mui::make_hbox(10, mui::Stretch(br), mui::Stretch(tr))));
            tab3->margin(20);
            tab3->label("Data Display");
            return tab3;
        };

        auto tab4 = [&] { // --- Tab 4: Text & HTML ---
            auto *ed = mui::make<mui::TextEditor>();
            ed->buffer(m_editor_buffer);
            auto *hv = mui::build<mui::HelpView>()
                           ->value("<h2>HelpView</h2><p>Renders <b>simple</b> HTML, like this.</p>"
                                   "<hr><p>Useful for about boxes or simple documentation.</p>")
                           ->end();

            auto *tab4 = mui::make_vbox(
                tx, ty, tw, th, 20, mui::Stretch(ed), mui::Stretch(hv));
            tab4->margin(20);
            tab4->label("Text & HTML");
            return tab4;
        };

        auto tab5 = [&] { // --- Tab 5: Graphics & Advanced ---
            auto *ch = mui::build<mui::Chart>()
                           ->type(FL_PIE_CHART)
                           ->add(30, "A", FL_RED)
                           ->add(50, "B", FL_GREEN)
                           ->add(20, "C", FL_BLUE)
                           ->end();

            auto *tb = mui::make<DemoTable>();
            tb->rows(10);
            tb->cols(5);
            tb->row_header(1);
            tb->col_header(1);
            tb->redraw();

            auto *cc = mui::make<CustomCanvas>();

            auto *tab5 = mui::make_vbox(
                tx, ty, tw, th, 20,
                mui::Stretch(mui::make_hbox(20, mui::Stretch(ch), mui::Stretch(tb))),
                mui::Stretch(cc));
            tab5->margin(20);
            tab5->label("Graphics & Advanced");
            return tab5;
        };

        auto tab6 = [&] { // --- Tab 6: Utilities ---
            auto *cl1 = mui::build<mui::Clock>()->type(FL_SQUARE_CLOCK)->end();
            auto *cl2 = mui::build<mui::Clock>()->type(FL_ROUND_CLOCK)->end();

            auto *fb = mui::build<mui::Button>("Open File...")
                           ->callback(on_file_chooser, this)
                           ->end();

            auto make_wizard_page = [](const char *text)
            {
                auto *box = mui::make_header(text);
                auto *group = mui::make_group(box);
                group->resizable(box);
                return group;
            };

            m_wizard_handle = mui::make_wizard(
                make_wizard_page("Content for step 1\nStep 1"),
                make_wizard_page("Content for step 2\nStep 2"),
                make_wizard_page("Content for step 3\nStep 3"));

            m_wizard_handle->box(mui::Theme::schemes::ROUNDED_INPUT_THIN_DOWN_BOX);

            if (m_wizard_handle->children() > 0)
                m_wizard_handle->value(m_wizard_handle->child(0));

            auto *wp = mui::build<mui::Button>("Prev")
                           ->callback(on_wizard_prev, this)
                           ->end();
            auto *wn = mui::build<mui::Button>("Next")
                           ->callback(on_wizard_next, this)
                           ->end();

            auto *tab6 = mui::make_vbox(
                tx, ty, tw, th, 10,
                mui::Fix(mui::make_hbox(10, mui::Fix(cl1, 100), mui::Fix(cl2, 100), mui::Spacer()), 100),
                mui::FixedSpacer(10),
                mui::Fix(mui::make_hbox(0, mui::Fix(fb, 120), mui::Spacer()), 30),
                mui::FixedSpacer(10),
                mui::Stretch(m_wizard_handle),
                mui::Fix(mui::make_hbox(10, mui::Spacer(), mui::Fix(wp, 80), mui::Fix(wn, 80)), 30));
            tab6->margin(20);
            tab6->label("Utilities");
            return tab6;
        };
        tabs->add(tab1());
        tabs->add(tab2());
        tabs->add(tab3());
        tabs->add(tab4());
        tabs->add(tab5());
        tabs->add(tab6());

        auto *tabs_container = mui::make_vbox(0, mui::Stretch(tabs));
        tabs_container->margin(10);

        auto *main_layout = mui::make_vbox(0, 0, 800, 600, 0,
                                           mui::Fix(m_menu_bar_handle, 30),
                                           mui::Stretch(tabs_container));

        m_win->end();
        m_win->resizable(main_layout);
        m_win->size_range(600, 400);
    }

    int run()
    {
        build_ui();
        m_win->show();
        return mui::System::run();
    }
};

int main()
{
    mui::System::init_threads();
    mui::Theme::apply();

    WidgetGallery gallery;
    return gallery.run();
}