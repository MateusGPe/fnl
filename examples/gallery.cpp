#include <iostream>
#include <string>
#include <cstdio>
#include <mui.hpp>
#include <FL/Fl_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Radio_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Secret_Input.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Counter.H>
#include <FL/Fl_Adjuster.H>
#include <FL/Fl_Dial.H>
#include <FL/Fl_Roller.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Value_Output.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Help_View.H>
#include <FL/Fl_Chart.H>
#include <FL/Fl_Table.H>
#include <FL/Fl_Clock.H>
#include <FL/Fl_Wizard.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/fl_draw.H>

class DemoTable : public Fl_Table
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
    DemoTable(int X, int Y, int W, int H, const char *l = 0) : Fl_Table(X, Y, W, H, l) {}
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
    Fl_Window *m_win;
    Fl_Text_Buffer *m_editor_buffer;
    Fl_Wizard *m_wizard_handle = nullptr;
    Fl_Menu_Bar *m_menu_bar_handle = nullptr;

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
        // Themes are no longer dynamic.
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
        m_win = new Fl_Window(800, 600, "MUI Widget Gallery");
        m_win->begin();

        m_menu_bar_handle = new Fl_Menu_Bar(0, 0, 800, 30);
        m_menu_bar_handle->add("File/Quit", 0, on_quit, this);
        m_menu_bar_handle->add("Help/About");

        m_menu_bar_handle->add("Theme/Aero", 0, on_theme_menu_select, this, FL_MENU_RADIO);
        m_menu_bar_handle->add("Theme/AquaClassic", 0, on_theme_menu_select, this, FL_MENU_RADIO);
        m_menu_bar_handle->add("Theme/Blue", 0, on_theme_menu_select, this, FL_MENU_RADIO);
        m_menu_bar_handle->add("Theme/Classic", 0, on_theme_menu_select, this, FL_MENU_RADIO);
        m_menu_bar_handle->add("Theme/Dark", 0, on_theme_menu_select, this, FL_MENU_RADIO);
        m_menu_bar_handle->add("Theme/Flat", 0, on_theme_menu_select, this, FL_MENU_RADIO);
        m_menu_bar_handle->add("Theme/Greybird", 0, on_theme_menu_select, this, FL_MENU_RADIO);
        m_menu_bar_handle->add("Theme/Material", 0, on_theme_menu_select, this, FL_MENU_RADIO);
        m_menu_bar_handle->add("Theme/Metro", 0, on_theme_menu_select, this, FL_MENU_RADIO);
        m_menu_bar_handle->add("Theme/Win10", 0, on_theme_menu_select, this, FL_MENU_RADIO | FL_MENU_VALUE);
        m_menu_bar_handle->add("Theme/WinXP", 0, on_theme_menu_select, this, FL_MENU_RADIO);

        mui::Tabs *tabs = new mui::Tabs(10, 40, 780, 550);
        {
            Fl_Group *tab1 = new Fl_Group(10, 70, 780, 520, "Basic Controls");
            {
                int sy = 90;
                Fl_Box *b1 = new Fl_Box(20, sy, 200, 20, "Buttons & Toggles");
                b1->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
                sy += 30;

                mui::Button *btn = new mui::Button(20, sy, 80, 30, "Button");
                btn->callback(on_generic_click, this);

                Fl_Return_Button *rbtn = new Fl_Return_Button(110, sy, 80, 30, "Return");
                rbtn->callback(on_generic_click, this);

                mui::LightButton *lbtn = new mui::LightButton(200, sy, 80, 30, "Light");
                lbtn->value(1);
                lbtn->callback(on_generic_toggle, this);
                sy += 40;

                Fl_Check_Button *cbtn = new Fl_Check_Button(20, sy, 80, 30, "Check");
                cbtn->value(1);
                cbtn->callback(on_generic_toggle, this);

                Fl_Radio_Button *rad1 = new Fl_Radio_Button(110, sy, 80, 30, "Radio 1");
                rad1->value(1);
                rad1->callback(on_generic_toggle, this);

                Fl_Radio_Button *rad2 = new Fl_Radio_Button(200, sy, 80, 30, "Radio 2");
                rad2->callback(on_generic_toggle, this);
                sy += 50;

                Fl_Box *b2 = new Fl_Box(20, sy, 200, 20, "Inputs & Choices");
                b2->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
                sy += 30;

                Fl_Input *inp = new Fl_Input(20, sy, 100, 30);
                inp->value("Text");

                Fl_Secret_Input *sinp = new Fl_Secret_Input(130, sy, 100, 30);
                sinp->value("secret");

                mui::Choice *ch = new mui::Choice(240, sy, 120, 30);
                ch->add("Option A|Option B|Option C");
                ch->value(1);
                ch->callback(on_generic_select, this);
                sy += 50;

                Fl_Box *b3 = new Fl_Box(20, sy, 200, 20, "Sliders & Progress");
                b3->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
                sy += 30;

                mui::Slider *sl = new mui::Slider(20, sy, 200, 30, "Slider");
                sl->type(FL_HOR_SLIDER);
                sl->bounds(0, 100);
                sl->value(50);
                sl->callback(on_generic_change, this);
                sy += 40;

                mui::Progress *prg = new mui::Progress(20, sy, 200, 25);
                prg->minimum(0);
                prg->maximum(100);
                prg->value(65);
            }
            tab1->end();

            Fl_Group *tab2 = new Fl_Group(10, 70, 780, 520, "Valuators");
            tab2->hide();
            {
                int sy = 90;
                auto make_row = [&](Fl_Widget *w, const char *label, int x, int y)
                {
                    Fl_Box *l = new Fl_Box(x, y, 100, 30, label);
                    l->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
                    w->resize(x + 110, y, 150, 30);
                    w->callback(on_generic_change, this);
                };

                Fl_Value_Input *vi = new Fl_Value_Input(0, 0, 0, 0);
                vi->bounds(0, 1000);
                vi->step(1);
                vi->value(123);
                make_row(vi, "ValueInput:", 20, sy);
                sy += 40;

                mui::Spinner *sp = new mui::Spinner(0, 0, 0, 0);
                sp->minimum(0);
                sp->maximum(50);
                sp->step(1);
                sp->value(10);
                make_row(sp, "Spinner:", 20, sy);
                sy += 40;

                Fl_Counter *ct = new Fl_Counter(0, 0, 0, 0);
                ct->bounds(0, 20);
                ct->step(1, 5);
                ct->value(5);
                make_row(ct, "Counter:", 20, sy);
                sy += 40;

                Fl_Adjuster *adj = new Fl_Adjuster(0, 0, 0, 0);
                adj->bounds(-1, 1);
                adj->value(0);
                make_row(adj, "Adjuster:", 20, sy);
                sy += 40;

                mui::ValueSlider *vs = new mui::ValueSlider(0, 0, 0, 0);
                vs->type(FL_HOR_SLIDER);
                vs->bounds(0, 100);
                vs->value(25);
                make_row(vs, "ValueSlider:", 20, sy);
                sy += 60;

                Fl_Dial *dial = new Fl_Dial(20, sy, 60, 60, "Dial");
                dial->bounds(0, 360);
                dial->value(90);
                dial->callback(on_generic_change, this);

                Fl_Roller *roller = new Fl_Roller(100, sy, 30, 60, "Roller");
                roller->bounds(0, 100);
                roller->value(30);
                roller->step(0.1);
                roller->callback(on_generic_change, this);
            }
            tab2->end();

            Fl_Group *tab3 = new Fl_Group(10, 70, 780, 520, "Data Display");
            tab3->hide();
            {
                int sy = 90;
                Fl_Output *out = new Fl_Output(20, sy, 100, 30);
                out->value("Read-only");

                Fl_Value_Output *vout = new Fl_Value_Output(130, sy, 80, 30);
                vout->value(3.14159);
                sy += 50;

                Fl_Multiline_Output *mout = new Fl_Multiline_Output(20, sy, 200, 60);
                mout->value("Line 1\nLine 2\nLine 3");
                sy += 80;

                Fl_Browser *br = new Fl_Browser(20, sy, 150, 150);
                br->type(FL_MULTI_BROWSER);
                br->add("Item 1");
                br->add("Item 2");
                br->add("Item 3");
                br->callback(on_generic_select, this);

                mui::Tree *tr = new mui::Tree(180, sy, 200, 150);
                tr->add("Root/Branch1/Leaf");
                tr->add("Root/Branch2");
                tr->callback(on_generic_select, this);
            }
            tab3->end();

            Fl_Group *tab4 = new Fl_Group(10, 70, 780, 520, "Text HTML");
            tab4->hide();
            {
                Fl_Text_Editor *ed = new Fl_Text_Editor(20, 90, 740, 200);
                ed->buffer(m_editor_buffer);

                Fl_Help_View *hv = new Fl_Help_View(20, 310, 740, 200);
                hv->value("<h2>HelpView</h2><p>Renders <b>simple</b> HTML, like this.</p><hr><p>Useful for about boxes or simple documentation.</p>");
            }
            tab4->end();

            Fl_Group *tab5 = new Fl_Group(10, 70, 780, 520, "Graphics Advanced");
            tab5->hide();
            {
                Fl_Chart *ch = new Fl_Chart(20, 90, 300, 200);
                ch->type(FL_PIE_CHART);
                ch->add(30, "A", FL_RED);
                ch->add(50, "B", FL_GREEN);
                ch->add(20, "C", FL_BLUE);

                DemoTable *tb = new DemoTable(340, 90, 400, 200);
                tb->rows(10);
                tb->cols(5);
                tb->row_header(1);
                tb->col_header(1);
                tb->end();

                CustomCanvas *cc = new CustomCanvas(20, 310, 300, 200);
            }
            tab5->end();

            Fl_Group *tab6 = new Fl_Group(10, 70, 780, 520, "Utilities");
            tab6->hide();
            {
                Fl_Clock *cl1 = new Fl_Clock(20, 90, 100, 100);
                cl1->type(FL_SQUARE_CLOCK);
                Fl_Clock *cl2 = new Fl_Clock(130, 90, 100, 100);
                cl2->type(FL_ROUND_CLOCK);

                mui::Button *fb = new mui::Button(20, 210, 120, 30, "Open File...");
                fb->callback(on_file_chooser, this);

                m_wizard_handle = new Fl_Wizard(20, 260, 400, 200);
                {
                    Fl_Group *w1 = new Fl_Group(20, 260, 400, 200);
                    Fl_Box *b1 = new Fl_Box(20, 260, 400, 200, "Content for step 1\nStep 1");
                    b1->align(FL_ALIGN_CENTER);
                    w1->end();

                    Fl_Group *w2 = new Fl_Group(20, 260, 400, 200);
                    Fl_Box *b2 = new Fl_Box(20, 260, 400, 200, "Content for step 2\nStep 2");
                    b2->align(FL_ALIGN_CENTER);
                    w2->end();

                    Fl_Group *w3 = new Fl_Group(20, 260, 400, 200);
                    Fl_Box *b3 = new Fl_Box(20, 260, 400, 200, "Content for step 3\nStep 3");
                    b3->align(FL_ALIGN_CENTER);
                    w3->end();
                }
                m_wizard_handle->end();

                mui::Button *wp = new mui::Button(20, 470, 80, 30, "Prev");
                wp->callback(on_wizard_prev, this);
                mui::Button *wn = new mui::Button(110, 470, 80, 30, "Next");
                wn->callback(on_wizard_next, this);
            }
            tab6->end();
        }
        tabs->end();

        m_win->end();
        m_win->resizable(tabs);
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