#include <iostream>
#include <memory>
#include <cstdio>
#include <fstream>
#include <mui.hpp>
#include <mui/Theme/data.hpp>
#include "mui_img/ImageViewer.hpp"
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Output.H>

class ImageViewerApp;
class LayerBrowser : public Fl_Browser
{
public:
    ImageViewerApp *app = nullptr;
    int drag_source = -1;

    LayerBrowser(int x, int y, int w, int h) : Fl_Browser(x, y, w, h)
    {
        type(FL_SELECT_BROWSER);
    }
    int handle(int event) override;
};

class ImageViewerApp
{
private:
    Fl_Double_Window *m_win = nullptr;
    mui::ImageViewer *m_viewer = nullptr;
    LayerBrowser *m_layer_list = nullptr;
    Fl_Output *m_status_zoom_handle = nullptr;
    Fl_Output *m_status_coords_handle = nullptr;
    Fl_Output *m_status_color_handle = nullptr;
    mui::Choice *m_tool_choice_handle = nullptr;
    mui::Slider *m_opacity_slider_handle = nullptr;
    mui::Choice *m_blend_mode_choice_handle = nullptr;
    mui::LightButton *m_minimap_btn_handle = nullptr;
    mui::ValueSlider *m_rot_slider_handle = nullptr;
    mui::ValueSlider *m_grid_size_slider_handle = nullptr;
    mui::Choice *m_canvas_mode_choice_handle = nullptr;
    mui::Spinner *m_canvas_w_spinner_handle = nullptr;
    mui::Spinner *m_canvas_h_spinner_handle = nullptr;

    bool m_use_solid_bg = false;
    Fl_Color m_solid_bg_color = FL_WHITE;
    int m_grid_size = 25;
    int m_next_layer_id = 1;

public:
    ImageViewerApp()
    {
        build_ui();
        m_layer_list->app = this;
    }

    void load_image(const char *filepath, int x, int y)
    {
        m_viewer->load_layer_async<ImageViewerApp, &ImageViewerApp::sync_layer_list>(filepath, fl_filename_name(filepath), x, y, this);
    }

    void on_open_image()
    {
        mui::NativeFileChooser chooser(mui::NativeFileChooser::BrowseMultiFile);
        chooser.options(mui::NativeFileChooser::Preview);
        chooser.title("Open Image")
            .filter("Image Files\t*.{png,jpg,jpeg,gif,bmp}\nAll Files\t * ");
        if (chooser.show())
        {
            for (int n = 0; n < chooser.count(); n++)
            {
                load_image(chooser.filename(n), 0, 0);
            }
        }
    }

    void
    on_quit()
    {
        m_win->hide();
    }
    void on_reset_view()
    {
        m_viewer->reset_view();
    }
    void on_fit_all()
    {
        m_viewer->fit_all();
    }
    void on_center_all()
    {
        m_viewer->center_all();
    }
    void on_fit_to_canvas()
    {
        m_viewer->fit_to_canvas();
    }

    void on_export_json()
    {
        mui::NativeFileChooser chooser(mui::NativeFileChooser::SaveFile);
        chooser.title("Export JSON")
            .filter("JSON Files\t*.json")
            .options(mui::NativeFileChooser::SaveAsConfirm);
        if (chooser.show())
        {
            std::ofstream out(chooser.filename());
            out << m_viewer->export_json();
        }
    }

    void on_export_image()
    {
        m_viewer->export_image();
    }

    void on_undo()
    {
        m_viewer->undo();
        sync_layer_list();
    }

    void on_redo()
    {
        m_viewer->redo();
        sync_layer_list();
    }

    void on_clear_layers()
    {
        m_viewer->clear_layers();
        sync_layer_list();
    }

    void on_toggle_minimap()
    {
        m_viewer->show_minimap(!m_viewer->show_minimap());
        if (m_minimap_btn_handle)
            m_minimap_btn_handle->value(m_viewer->show_minimap());
    }

    void on_change_theme(const char *theme_name)
    {
        for (const auto &named_palette : mui::get_theme_palettes())
        {
            if (named_palette.name == theme_name)
            {
                mui::ThemeManager::set_palette(named_palette.palette);
                break;
            }
        }
    }

    void on_tool_changed()
    {
        if (m_tool_choice_handle)
            m_viewer->active_tool(static_cast<mui::ViewerTool>(m_tool_choice_handle->value()));
    }

    void set_tool(int idx)
    {
        if (m_tool_choice_handle)
        {
            m_tool_choice_handle->value(idx);
            on_tool_changed();
        }
    }
    void on_tool_select()
    {
        set_tool(0);
    }
    void on_tool_pan()
    {
        set_tool(1);
    }
    void on_tool_move()
    {
        set_tool(2);
    }
    void on_tool_crop()
    {
        set_tool(3);
    }
    void on_tool_eye()
    {
        set_tool(4);
    }

    void on_opacity_changed()
    {
        if (m_opacity_slider_handle && m_viewer->selected_layer() >= 0)
            m_viewer->layer_opacity(m_viewer->selected_layer(), m_opacity_slider_handle->value() / 100.0);
    }

    void on_blend_mode_changed()
    {
        if (m_blend_mode_choice_handle && m_viewer->selected_layer() >= 0)
            m_viewer->layer_blend_mode(m_viewer->selected_layer(), static_cast<mui::BlendMode>(m_blend_mode_choice_handle->value()));
    }

    void on_rot_changed()
    {
        if (m_rot_slider_handle && m_viewer->selected_layer() >= 0)
            m_viewer->rotate_layer(m_viewer->selected_layer(), m_rot_slider_handle->value());
    }

    void on_flip_h()
    {
        if (m_viewer->selected_layer() >= 0)
            m_viewer->flip_layer_horizontal(m_viewer->selected_layer());
    }

    void on_flip_v()
    {
        if (m_viewer->selected_layer() >= 0)
            m_viewer->flip_layer_vertical(m_viewer->selected_layer());
    }

    void on_rot_90()
    {
        if (m_viewer->selected_layer() >= 0)
        {
            auto l = std::static_pointer_cast<mui::ImageLayer>(m_viewer->document()->layers()[m_viewer->selected_layer()]);
            m_viewer->rotate_layer(m_viewer->selected_layer(), l->rotation_angle + 90.0);
        }
    }

    void on_group_layer()
    {
        int sel = m_viewer->selected_layer();
        if (sel > 0 && sel < (int)m_viewer->document()->layer_count())
        {
            m_viewer->layer_parent(sel, m_viewer->document()->layers()[sel - 1]->id);
            sync_layer_list();
        }
    }

    void on_ungroup_layer()
    {
        if (m_viewer->selected_layer() >= 0)
            m_viewer->layer_parent(m_viewer->selected_layer(), -1);
    }

    void on_color_picked(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "RGBA: %d, %d, %d, %d", r, g, b, a);
        if (m_status_color_handle)
            m_status_color_handle->value(buf);
    }

    void on_context_menu()
    {
        Fl_Menu_Item menu[] = {
            {"Bring Forward", 0,
             [](Fl_Widget *, void *v)
             {
                 static_cast<ImageViewerApp *>(v)->on_layer_up();
             },
             this},
            {"Send Backward", 0,
             [](Fl_Widget *, void *v)
             {
                 static_cast<ImageViewerApp *>(v)->on_layer_down();
             },
             this, FL_MENU_DIVIDER},
            {"Group to Below Layer", 0,
             [](Fl_Widget *, void *v)
             {
                 static_cast<ImageViewerApp *>(v)->on_group_layer();
             },
             this},
            {"Ungroup Layer", 0,
             [](Fl_Widget *, void *v)
             {
                 static_cast<ImageViewerApp *>(v)->on_ungroup_layer();
             },
             this, FL_MENU_DIVIDER},
            {"Toggle Visibility", 0,
             [](Fl_Widget *, void *v)
             {
                 static_cast<ImageViewerApp *>(v)->on_layer_toggle_vis();
             },
             this},
            {"Toggle Lock", 0,
             [](Fl_Widget *, void *v)
             {
                 static_cast<ImageViewerApp *>(v)->on_layer_toggle_lock();
             },
             this, FL_MENU_DIVIDER},
            {"Delete Layer", 0,
             [](Fl_Widget *, void *v)
             {
                 static_cast<ImageViewerApp *>(v)->on_layer_delete();
             },
             this, FL_MENU_DIVIDER},
            {"Flip Horizontal", 0,
             [](Fl_Widget *, void *v)
             {
                 static_cast<ImageViewerApp *>(v)->on_flip_h();
             },
             this},
            {"Flip Vertical", 0,
             [](Fl_Widget *, void *v)
             {
                 static_cast<ImageViewerApp *>(v)->on_flip_v();
             },
             this},
            {"Rotate 90 Degrees", 0,
             [](Fl_Widget *, void *v)
             {
                 static_cast<ImageViewerApp *>(v)->on_rot_90();
             },
             this, FL_MENU_DIVIDER},
            {"Reset View (100%)", 0,
             [](Fl_Widget *, void *v)
             {
                 static_cast<ImageViewerApp *>(v)->on_reset_view();
             },
             this},
            {"Fit All", 0,
             [](Fl_Widget *, void *v)
             {
                 static_cast<ImageViewerApp *>(v)->on_fit_all();
             },
             this},
            {"Center All", 0,
             [](Fl_Widget *, void *v)
             {
                 static_cast<ImageViewerApp *>(v)->on_center_all();
             },
             this},
            {"Fit to Canvas", 0,
             [](Fl_Widget *, void *v)
             {
                 static_cast<ImageViewerApp *>(v)->on_fit_to_canvas();
             },
             this},
            {"Export Image...", 0,
             [](Fl_Widget *, void *v)
             {
                 static_cast<ImageViewerApp *>(v)->on_export_image();
             },
             this},
            {"Export JSON to Console", 0,
             [](Fl_Widget *, void *v)
             {
                 static_cast<ImageViewerApp *>(v)->on_export_json();
             },
             this},
            {0}};

        const Fl_Menu_Item *m = menu + ((m_viewer->selected_layer() != -1) ? 0 : 10);
        const Fl_Menu_Item *picked = m->popup(Fl::event_x(), Fl::event_y(), nullptr, nullptr, nullptr);
        if (picked && picked->callback())
        {
            picked->do_callback(nullptr);
        }
    }
    void sync_layer_list()
    {
        m_layer_list->clear();
        const auto &layers = m_viewer->document()->layers();

        for (auto &l : layers)
            if (l->id == -1)
                l->id = m_next_layer_id++;
        for (int i = (int)layers.size() - 1; i >= 0; --i)
        {
            std::string label = layers[i]->name;
            if (layers[i]->parent_id != -1)
                label = "   |_ " + label;
            if (!layers[i]->visible)
                label += " (Hidden)";
            if (layers[i]->locked)
                label += " [Locked]";
            m_layer_list->add(label.c_str(), reinterpret_cast<void *>(static_cast<intptr_t>(i)));
        }
        int sel = m_viewer->selected_layer();
        if (sel >= 0)
        {
            for (int line = 1; line <= m_layer_list->size(); ++line)
            {
                if (reinterpret_cast<intptr_t>(m_layer_list->data(line)) == sel)
                {
                    m_layer_list->select(line);
                    break;
                }
            }
        }
    }
    void reorder_layers(int ui_source, int ui_target)
    {
        int source_idx = static_cast<int>(reinterpret_cast<intptr_t>(m_layer_list->data(ui_source)));
        int target_idx = static_cast<int>(reinterpret_cast<intptr_t>(m_layer_list->data(ui_target)));
        if (source_idx < target_idx)
        {
            while (source_idx < target_idx)
            {
                m_viewer->move_layer_up(source_idx);
                source_idx++;
            }
        }
        else
        {
            while (source_idx > target_idx)
            {
                m_viewer->move_layer_down(source_idx);
                source_idx--;
            }
        }
        sync_layer_list();
    }

    void on_browser_select()
    {
        int line = m_layer_list->value();
        if (line > 0)
        {
            int actual_index = static_cast<int>(reinterpret_cast<intptr_t>(m_layer_list->data(line)));
            m_viewer->select_layer(actual_index);
        }
    }

    void on_canvas_select()
    {
        sync_layer_list();
        int sel = m_viewer->selected_layer();
        if (sel >= 0 && sel < (int)m_viewer->document()->layers().size())
        {
            auto l = m_viewer->document()->layers()[sel];
            m_opacity_slider_handle->value(l->alpha * 100.0);
            m_blend_mode_choice_handle->value(static_cast<int>(l->blend_mode));
            if (m_rot_slider_handle && l->get_type() == mui::LayerType::Image)
                m_rot_slider_handle->value(std::static_pointer_cast<mui::ImageLayer>(l)->rotation_angle);
        }
    }

    void on_layer_up()
    {
        m_viewer->move_layer_up(m_viewer->selected_layer());
        sync_layer_list();
    }

    void on_layer_down()
    {
        m_viewer->move_layer_down(m_viewer->selected_layer());
        sync_layer_list();
    }

    void on_layer_toggle_vis()
    {
        m_viewer->toggle_layer_visibility(m_viewer->selected_layer());
        sync_layer_list();
    }

    void on_layer_toggle_lock()
    {
        int sel = m_viewer->selected_layer();
        if (sel >= 0)
        {
            m_viewer->toggle_layer_lock(sel);
            sync_layer_list();
        }
    }

    void on_layer_delete()
    {
        m_viewer->remove_layer(m_viewer->selected_layer());
        sync_layer_list();
    }

    void on_toggle_grid()
    {
        static int s = 0;
        s = (s + 1) % 3;
        if (s == 0)
        {
            m_use_solid_bg = false;
            m_solid_bg_color = FL_WHITE;
        }
        else if (s == 1)
        {
            m_use_solid_bg = true;
            m_solid_bg_color = FL_DARK3;
        }
        else
        {
            m_use_solid_bg = true;
            m_solid_bg_color = FL_WHITE;
        }
        m_viewer->grid(m_use_solid_bg, m_solid_bg_color, m_grid_size);
    }

    void on_grid_size_changed()
    {
        if (m_grid_size_slider_handle)
        {
            m_grid_size = static_cast<int>(m_grid_size_slider_handle->value());
            m_viewer->grid(m_use_solid_bg, m_solid_bg_color, m_grid_size);
        }
    }

    void on_canvas_mode_changed()
    {
        if (m_canvas_mode_choice_handle)
            m_viewer->document_mode(static_cast<mui::DocumentMode>(m_canvas_mode_choice_handle->value()));
    }

    void on_canvas_size_changed()
    {
        if (m_canvas_w_spinner_handle && m_canvas_h_spinner_handle)
            m_viewer->canvas_size(static_cast<int>(m_canvas_w_spinner_handle->value()), static_cast<int>(m_canvas_h_spinner_handle->value()));
    }

    void initial_fit()
    {
        m_viewer->fit_all();
    }

    void update_status_bar()
    {
        char zoom_buf[64], coords_buf[128];
        snprintf(zoom_buf, sizeof(zoom_buf), "Zoom: %.1f%%", m_viewer->scale() * 100.0);
        if (m_status_zoom_handle)
            m_status_zoom_handle->value(zoom_buf);

        if (Fl::event_inside(m_viewer))
        {
            double wx = m_viewer->view_x() + (Fl::event_x() - m_viewer->x()) / m_viewer->scale();
            double wy = m_viewer->view_y() + (Fl::event_y() - m_viewer->y()) / m_viewer->scale();
            snprintf(coords_buf, sizeof(coords_buf), "X: %.1f, Y: %.1f", wx, wy);
        }
        else
        {
            snprintf(coords_buf, sizeof(coords_buf), "X: -, Y: -");
        }
        if (m_status_coords_handle)
            m_status_coords_handle->value(coords_buf);
    }

    void build_ui()
    {
        m_win = new Fl_Double_Window(1200, 800, "MUI Image Compositor");
        m_win->begin();

        m_viewer = new mui::ImageViewer(300, 30, 900, 745);
        m_viewer->on_view_change<ImageViewerApp, &ImageViewerApp::update_status_bar>(this);
        m_viewer->on_layer_select<ImageViewerApp, &ImageViewerApp::on_canvas_select>(this);
        m_viewer->on_right_click<ImageViewerApp, &ImageViewerApp::on_context_menu>(this);
        m_viewer->on_color_picked<ImageViewerApp, &ImageViewerApp::on_color_picked>(this);

        Fl_Menu_Bar *menu_bar = new Fl_Menu_Bar(0, 0, 1200, 30);
        menu_bar->add("File/Open...", FL_CTRL + 'o', [](Fl_Widget *, void *v)
                      { static_cast<ImageViewerApp *>(v)->on_open_image(); }, this);
        menu_bar->add("File/Export Flat Image...", FL_CTRL + 'e', [](Fl_Widget *, void *v)
                      { static_cast<ImageViewerApp *>(v)->on_export_image(); }, this);
        menu_bar->add("File/Quit", FL_CTRL + 'q', [](Fl_Widget *, void *v)
                      { static_cast<ImageViewerApp *>(v)->on_quit(); }, this);
        menu_bar->add("Edit/Undo", FL_CTRL + 'z', [](Fl_Widget *, void *v)
                      { static_cast<ImageViewerApp *>(v)->on_undo(); }, this);
        menu_bar->add("Edit/Redo", FL_CTRL + 'y', [](Fl_Widget *, void *v)
                      { static_cast<ImageViewerApp *>(v)->on_redo(); }, this);
        menu_bar->add("Edit/Clear All", 0, [](Fl_Widget *, void *v)
                      { static_cast<ImageViewerApp *>(v)->on_clear_layers(); }, this);
        menu_bar->add("View/Reset View", FL_CTRL + '0', [](Fl_Widget *, void *v)
                      { static_cast<ImageViewerApp *>(v)->on_reset_view(); }, this);
        menu_bar->add("View/Toggle Grid", FL_CTRL + 'g', [](Fl_Widget *, void *v)
                      { static_cast<ImageViewerApp *>(v)->on_toggle_grid(); }, this);
        menu_bar->add("View/Fit All", FL_CTRL + 'f', [](Fl_Widget *, void *v)
                      { static_cast<ImageViewerApp *>(v)->on_fit_all(); }, this);
        menu_bar->add("View/Toggle Minimap", FL_CTRL + 'm', [](Fl_Widget *, void *v)
                      { static_cast<ImageViewerApp *>(v)->on_toggle_minimap(); }, this);

        auto theme_cb =
            [](Fl_Widget *w, void *v)
        {
            auto *bar = static_cast<Fl_Menu_Bar *>(w);
            const Fl_Menu_Item *item = bar->mvalue();
            if (!item || !item->label())
                return;

            std::string path = item->label();
            size_t last_slash = path.rfind('/');
            std::string name = (last_slash != std::string::npos) ? path.substr(last_slash + 1) : path;
            static_cast<ImageViewerApp *>(v)->on_change_theme(name.c_str());
        };

        menu_bar->add("View/Theme", 0, 0, 0, FL_SUBMENU);
        bool first = true;
        for (const auto &named_palette : mui::get_theme_palettes())
        {
            std::string path = "View/Theme/" + named_palette.name;
            int flags = FL_MENU_RADIO;
            if (first)
            {
                flags |= FL_MENU_VALUE;
                first = false;
            }
            menu_bar->add(path.c_str(), 0, theme_cb, this, flags);
        }

        menu_bar->add("Tools/Select", 'v', [](Fl_Widget *, void *v)
                      { static_cast<ImageViewerApp *>(v)->on_tool_select(); }, this);
        menu_bar->add("Tools/Pan", 'h', [](Fl_Widget *, void *v)
                      { static_cast<ImageViewerApp *>(v)->on_tool_pan(); }, this);
        menu_bar->add("Tools/Move", 'm', [](Fl_Widget *, void *v)
                      { static_cast<ImageViewerApp *>(v)->on_tool_move(); }, this);
        menu_bar->add("Tools/Crop", 'c', [](Fl_Widget *, void *v)
                      { static_cast<ImageViewerApp *>(v)->on_tool_crop(); }, this);
        menu_bar->add("Tools/Eyedropper", 'i', [](Fl_Widget *, void *v)
                      { static_cast<ImageViewerApp *>(v)->on_tool_eye(); }, this);

        int sy = 40;
        int sx = 10;
        int sw = 280;

        Fl_Box *tool_lbl = new Fl_Box(sx, sy, 40, 25, "Tool:");
        m_tool_choice_handle = new mui::Choice(sx + 45, sy, sw - 45, 25);
        m_tool_choice_handle->add("Select|Pan|Move|Crop|Eyedropper");
        m_tool_choice_handle->value(0);
        m_tool_choice_handle->callback(
            [](Fl_Widget *, void *v)
            {
                static_cast<ImageViewerApp *>(v)->on_tool_changed();
            },
            this);
        sy += 35;

        mui::Button *fit_btn = new mui::Button(sx, sy, 70, 25, "Fit All");
        fit_btn->callback(
            [](Fl_Widget *, void *v)
            {
                static_cast<ImageViewerApp *>(v)->on_fit_all();
            },
            this);

        mui::Button *reset_btn = new mui::Button(sx + 75, sy, 70, 25, "100%");
        reset_btn->callback(
            [](Fl_Widget *, void *v)
            {
                static_cast<ImageViewerApp *>(v)->on_reset_view();
            },
            this);

        m_minimap_btn_handle = new mui::LightButton(sx + 150, sy, sw - 150, 25, "Minimap");
        m_minimap_btn_handle->value(m_viewer->show_minimap());
        m_minimap_btn_handle->callback(
            [](Fl_Widget *, void *v)
            {
                static_cast<ImageViewerApp *>(v)->on_toggle_minimap();
            },
            this);
        sy += 35;

        mui::Button *grid_btn = new mui::Button(sx, sy, 100, 25, "Toggle Grid");
        grid_btn->callback(
            [](Fl_Widget *, void *v)
            {
                static_cast<ImageViewerApp *>(v)->on_toggle_grid();
            },
            this);

        Fl_Box *grid_size_lbl = new Fl_Box(sx + 105, sy, 40, 25, "Size:");
        grid_size_lbl->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
        m_grid_size_slider_handle = new mui::ValueSlider(sx + 150, sy, sw - 150, 25);
        m_grid_size_slider_handle->type(FL_HOR_SLIDER);
        m_grid_size_slider_handle->bounds(10, 100);
        m_grid_size_slider_handle->step(5);
        m_grid_size_slider_handle->value(25);
        m_grid_size_slider_handle->callback(
            [](Fl_Widget *, void *v)
            {
                static_cast<ImageViewerApp *>(v)->on_grid_size_changed();
            },
            this);
        sy += 35;

        Fl_Box *canvas_mode_lbl = new Fl_Box(sx, sy, 60, 25, "Canvas:");
        canvas_mode_lbl->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
        m_canvas_mode_choice_handle = new mui::Choice(sx + 65, sy, sw - 65, 25);
        m_canvas_mode_choice_handle->add("Infinite|Fixed");
        m_canvas_mode_choice_handle->value(0);
        m_canvas_mode_choice_handle->callback(
            [](Fl_Widget *, void *v)
            {
                static_cast<ImageViewerApp *>(v)->on_canvas_mode_changed();
            },
            this);
        sy += 35;

        Fl_Box *canvas_size_lbl = new Fl_Box(sx, sy, 60, 25, "Size:");
        canvas_size_lbl->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
        m_canvas_w_spinner_handle = new mui::Spinner(sx + 65, sy, 70, 25);
        m_canvas_w_spinner_handle->type(FL_INT_INPUT);
        m_canvas_w_spinner_handle->minimum(10);
        m_canvas_w_spinner_handle->maximum(4000);
        m_canvas_w_spinner_handle->step(10);
        m_canvas_w_spinner_handle->value(800);
        m_canvas_w_spinner_handle->callback(
            [](Fl_Widget *, void *v)
            {
                static_cast<ImageViewerApp *>(v)->on_canvas_size_changed();
            },
            this);

        m_canvas_h_spinner_handle = new mui::Spinner(sx + 140, sy, 70, 25);
        m_canvas_h_spinner_handle->type(FL_INT_INPUT);
        m_canvas_h_spinner_handle->minimum(10);
        m_canvas_h_spinner_handle->maximum(4000);
        m_canvas_h_spinner_handle->step(10);
        m_canvas_h_spinner_handle->value(600);
        m_canvas_h_spinner_handle->callback(
            [](Fl_Widget *, void *v)
            {
                static_cast<ImageViewerApp *>(v)->on_canvas_size_changed();
            },
            this);
        sy += 35;

        mui::Button *undo_btn = new mui::Button(sx, sy, sw / 2 - 2, 25, "Undo");
        undo_btn->callback(
            [](Fl_Widget *, void *v)
            {
                static_cast<ImageViewerApp *>(v)->on_undo();
            },
            this);
        mui::Button *redo_btn = new mui::Button(sx + sw / 2 + 2, sy, sw / 2 - 2, 25, "Redo");
        redo_btn->callback(
            [](Fl_Widget *, void *v)
            {
                static_cast<ImageViewerApp *>(v)->on_redo();
            },
            this);
        sy += 35;

        Fl_Box *layers_lbl = new Fl_Box(sx, sy, sw, 25, "Layers");
        sy += 30;

        int btn_w = sw / 4 - 2;
        mui::Button *up_btn = new mui::Button(sx, sy, btn_w, 25, "Up");
        up_btn->callback(
            [](Fl_Widget *, void *v)
            {
                static_cast<ImageViewerApp *>(v)->on_layer_up();
            },
            this);
        mui::Button *down_btn = new mui::Button(sx + btn_w + 2, sy, btn_w, 25, "Down");
        down_btn->callback(
            [](Fl_Widget *, void *v)
            {
                static_cast<ImageViewerApp *>(v)->on_layer_down();
            },
            this);
        mui::Button *group_btn = new mui::Button(sx + 2 * (btn_w + 2), sy, btn_w, 25, "Group");
        group_btn->callback(
            [](Fl_Widget *, void *v)
            {
                static_cast<ImageViewerApp *>(v)->on_group_layer();
            },
            this);
        mui::Button *ungroup_btn = new mui::Button(sx + 3 * (btn_w + 2), sy, btn_w, 25, "Ungroup");
        ungroup_btn->callback(
            [](Fl_Widget *, void *v)
            {
                static_cast<ImageViewerApp *>(v)->on_ungroup_layer();
            },
            this);
        sy += 30;

        m_layer_list = new LayerBrowser(sx, sy, sw, 200);
        m_layer_list->callback(
            [](Fl_Widget *, void *v)
            {
                static_cast<ImageViewerApp *>(v)->on_browser_select();
            },
            this);
        sy += 205;

        m_blend_mode_choice_handle = new mui::Choice(sx, sy, 120, 25);
        m_blend_mode_choice_handle->add("Normal|Multiply|Screen|Overlay");
        m_blend_mode_choice_handle->value(0);
        m_blend_mode_choice_handle->callback(
            [](Fl_Widget *, void *v)
            {
                static_cast<ImageViewerApp *>(v)->on_blend_mode_changed();
            },
            this);

        m_opacity_slider_handle = new mui::Slider(sx + 125, sy, sw - 125, 25);
        m_opacity_slider_handle->type(FL_HOR_SLIDER);
        m_opacity_slider_handle->bounds(0, 100);
        m_opacity_slider_handle->step(1);
        m_opacity_slider_handle->value(100);
        m_opacity_slider_handle->callback(
            [](Fl_Widget *, void *v)
            {
                static_cast<ImageViewerApp *>(v)->on_opacity_changed();
            },
            this);
        sy += 35;

        mui::Button *flip_h_btn = new mui::Button(sx, sy, btn_w, 25, "Flip H");
        flip_h_btn->callback(
            [](Fl_Widget *, void *v)
            {
                static_cast<ImageViewerApp *>(v)->on_flip_h();
            },
            this);
        mui::Button *flip_v_btn = new mui::Button(sx + btn_w + 2, sy, btn_w, 25, "Flip V");
        flip_v_btn->callback(
            [](Fl_Widget *, void *v)
            {
                static_cast<ImageViewerApp *>(v)->on_flip_v();
            },
            this);
        mui::Button *rot_90_btn = new mui::Button(sx + 2 * (btn_w + 2), sy, btn_w, 25, "Rot 90");
        rot_90_btn->callback(
            [](Fl_Widget *, void *v)
            {
                static_cast<ImageViewerApp *>(v)->on_rot_90();
            },
            this);
        sy += 35;

        Fl_Box *rot_lbl = new Fl_Box(sx, sy, 60, 25, "Rotate:");
        rot_lbl->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
        m_rot_slider_handle = new mui::ValueSlider(sx + 65, sy, sw - 65, 25);
        m_rot_slider_handle->type(FL_HOR_SLIDER);
        m_rot_slider_handle->bounds(-180, 180);
        m_rot_slider_handle->step(1);
        m_rot_slider_handle->value(0);
        m_rot_slider_handle->callback(
            [](Fl_Widget *, void *v)
            {
                static_cast<ImageViewerApp *>(v)->on_rot_changed();
            },
            this);
        sy += 35;

        mui::Button *hide_btn = new mui::Button(sx, sy, btn_w, 25, "Hide");
        hide_btn->callback(
            [](Fl_Widget *, void *v)
            {
                static_cast<ImageViewerApp *>(v)->on_layer_toggle_vis();
            },
            this);
        mui::Button *lock_btn = new mui::Button(sx + btn_w + 2, sy, btn_w, 25, "Lock");
        lock_btn->callback(
            [](Fl_Widget *, void *v)
            {
                static_cast<ImageViewerApp *>(v)->on_layer_toggle_lock();
            },
            this);
        mui::Button *del_btn = new mui::Button(sx + 2 * (btn_w + 2), sy, btn_w, 25, "Del");
        del_btn->callback(
            [](Fl_Widget *, void *v)
            {
                static_cast<ImageViewerApp *>(v)->on_layer_delete();
            },
            this);
        mui::Button *clear_btn = new mui::Button(sx + 3 * (btn_w + 2), sy, btn_w, 25, "Clear");
        clear_btn->callback(
            [](Fl_Widget *, void *v)
            {
                static_cast<ImageViewerApp *>(v)->on_clear_layers();
            },
            this);

        int sty = 775;
        m_status_zoom_handle = new Fl_Output(5, sty, 150, 25);
        m_status_coords_handle = new Fl_Output(160, sty, 200, 25);
        m_status_color_handle = new Fl_Output(365, sty, 150, 25);
        m_status_color_handle->value("RGBA: -, -, -, -");

        m_win->end();
        m_win->resizable(m_viewer);
    }

    int run()
    {
        load_image("assets/fltk.png", 0, 0);
        load_image("assets/gears.png", 200, 150);

        sync_layer_list();
        m_win->show();
        mui::Dispatcher::timeout<ImageViewerApp, &ImageViewerApp::initial_fit>(0.1, this);
        return mui::System::run();
    }
};

int LayerBrowser::handle(int event)
{
    int ret = Fl_Browser::handle(event);
    if (event == FL_PUSH)
        drag_source = value();
    if (event == FL_RELEASE && drag_source > 0)
    {
        int target = value();
        if (target > 0 && target != drag_source)
        {
            app->reorder_layers(drag_source, target);
        }
        drag_source = -1;
    }
    return ret;
}

int main()
{
    mui::System::init_threads();
    mui::Theme::apply();

    ImageViewerApp app;
    return app.run();
}