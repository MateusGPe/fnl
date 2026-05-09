#pragma once
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl.H>
#include "Theme.hpp"

namespace mui
{
    class PopupMenu
    {
    public:
        [[nodiscard]] Fl_Menu_Button *handle() { return &m_btn; }

        PopupMenu() : m_btn(-1, -1, 1, 1, nullptr)
        {
            m_btn.type(Fl_Menu_Button::POPUP3);
            m_btn.down_box(Theme::schemes::MENU_POPUP_BOX);
        }

        template <typename T, void (T::*Method)()>
        PopupMenu &add(const char *path, T *instance,
                       int shortcut = 0, int flags = 0)
        {
            m_last_idx = m_btn.add(path, shortcut,
                                   make_thunk<T, Method>(), instance, flags);
            return *this;
        }

        PopupMenu &add(const char *path, int shortcut = 0, int flags = 0)
        {
            m_last_idx = m_btn.add(path, shortcut, nullptr, nullptr, flags);
            return *this;
        }

        PopupMenu &add_packed(const char *str)
        {
            m_last_idx = m_btn.add(str);
            return *this;
        }

        PopupMenu &add_divider(const char *path, int flags = 0)
        {
            return add(path, 0, FL_MENU_DIVIDER | flags);
        }

        PopupMenu &add_divider()
        {
            if (m_last_idx >= 0)
                m_btn.mode(m_last_idx, m_btn.mode(m_last_idx) | FL_MENU_DIVIDER);
            return *this;
        }

        template <typename T, void (T::*Method)()>
        PopupMenu &add_toggle(const char *path, T *instance,
                              int shortcut = 0, bool checked = false, int flags = 0)
        {
            return add<T, Method>(path, instance, shortcut,
                                  FL_MENU_TOGGLE | (checked ? FL_MENU_VALUE : 0) | flags);
        }

        PopupMenu &add_toggle(const char *path,
                              int shortcut = 0, bool checked = false, int flags = 0)
        {
            return add(path, shortcut,
                       FL_MENU_TOGGLE | (checked ? FL_MENU_VALUE : 0) | flags);
        }

        template <typename T, void (T::*Method)()>
        PopupMenu &add_radio(const char *path, T *instance,
                             int shortcut = 0, bool checked = false, int flags = 0)
        {
            return add<T, Method>(path, instance, shortcut,
                                  FL_MENU_RADIO | (checked ? FL_MENU_VALUE : 0) | flags);
        }

        PopupMenu &add_radio(const char *path,
                             int shortcut = 0, bool checked = false, int flags = 0)
        {
            return add(path, shortcut,
                       FL_MENU_RADIO | (checked ? FL_MENU_VALUE : 0) | flags);
        }

        template <typename T, void (T::*Method)()>
        PopupMenu &add_inactive(const char *path, T *instance,
                                int shortcut = 0, int flags = 0)
        {
            return add<T, Method>(path, instance, shortcut, FL_MENU_INACTIVE | flags);
        }

        PopupMenu &add_inactive(const char *path, int shortcut = 0, int flags = 0)
        {
            return add(path, shortcut, FL_MENU_INACTIVE | flags);
        }

        template <typename T, void (T::*Method)()>
        PopupMenu &add_invisible(const char *path, T *instance,
                                 int shortcut = 0, int flags = 0)
        {
            return add<T, Method>(path, instance, shortcut, FL_MENU_INVISIBLE | flags);
        }

        PopupMenu &add_invisible(const char *path, int shortcut = 0, int flags = 0)
        {
            return add(path, shortcut, FL_MENU_INVISIBLE | flags);
        }

        template <typename T, void (T::*Method)()>
        PopupMenu &insert(int index, const char *path, T *instance,
                          int shortcut = 0, int flags = 0)
        {
            m_last_idx = m_btn.insert(index, path, shortcut,
                                      make_thunk<T, Method>(), instance, flags);
            return *this;
        }

        PopupMenu &insert(int index, const char *path,
                          int shortcut = 0, int flags = 0)
        {
            m_last_idx = m_btn.insert(index, path, shortcut, nullptr, nullptr, flags);
            return *this;
        }

        PopupMenu &replace(int index, const char *text)
        {
            m_btn.replace(index, text);
            return *this;
        }
        PopupMenu &remove(int index)
        {
            m_btn.remove(index);
            return *this;
        }
        PopupMenu &clear()
        {
            m_btn.clear();
            return *this;
        }
        PopupMenu &clear_submenu(int index)
        {
            m_btn.clear_submenu(index);
            return *this;
        }

        PopupMenu &mode(int index, int fl)
        {
            m_btn.mode(index, fl);
            return *this;
        }
        [[nodiscard]] int mode(int index) const { return m_btn.mode(index); }

        [[nodiscard]] int size() const { return m_btn.size(); }
        [[nodiscard]] const char *text(int i) const { return m_btn.text(i); }

        const Fl_Menu_Item *show()
        {
            prepare_theme();
            return m_btn.popup();
        }

        const Fl_Menu_Item *show(int x, int y, const char *title = nullptr)
        {
            prepare_theme();
            const Fl_Menu_Item *m = m_btn.menu();
            return m ? m->popup(x, y, title, m_btn.mvalue(), &m_btn) : nullptr;
        }

    private:
        Fl_Menu_Button m_btn;
        int m_last_idx = -1;

        void prepare_theme()
        {
            const auto &p = ThemeManager::get_palette();
            m_btn.box(Theme::schemes::MENU_POPUP_BOX);
            m_btn.color(p.bg_main);
            m_btn.selection_color(p.selection);
            m_btn.textcolor(fl_contrast(p.fg_main, p.bg_main));
        }

        template <typename T, void (T::*Method)()>
        [[nodiscard]] static Fl_Callback *make_thunk()
        {
            return [](Fl_Widget *, void *data)
            {
                (static_cast<T *>(data)->*Method)();
            };
        }
    };
}
