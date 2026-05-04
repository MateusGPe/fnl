#pragma once
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl.H>
#include "Theme.hpp"

namespace mui
{
    class PopupMenu
    {
    private:
        Fl_Menu_Button m_menu_button;
        int m_last_add_idx = -1;

    public:
        Fl_Menu_Button *handle() { return &m_menu_button; }

        PopupMenu() : m_menu_button(0, 0, 0, 0, nullptr)
        {
            m_menu_button.type(Fl_Menu_Button::POPUP3);
            m_menu_button.down_box(Theme::schemes::MENU_POPUP_BOX);
        }

        template <typename T, void (T::*Method)()>
        PopupMenu &add(const char *path, T *instance, int shortcut = 0, int flags = 0)
        {
            auto thunk = [](Fl_Widget *, void *data)
            {
                (static_cast<T *>(data)->*Method)();
            };
            m_last_add_idx = m_menu_button.add(path, shortcut, thunk, instance, flags);
            return *this;
        }

        PopupMenu &add(const char *path, int shortcut = 0, int flags = 0)
        {
            m_last_add_idx = m_menu_button.add(path, shortcut, nullptr, nullptr, flags);
            return *this;
        }

        PopupMenu &add_packed(const char *str)
        {
            m_last_add_idx = m_menu_button.add(str);
            return *this;
        }

        PopupMenu &add_divider(const char *path, int flags = 0)
        {
            return add(path, 0, FL_MENU_DIVIDER | flags);
        }

        PopupMenu &add_divider()
        {
            if (m_last_add_idx >= 0)
            {
                m_menu_button.mode(m_last_add_idx, m_menu_button.mode(m_last_add_idx) | FL_MENU_DIVIDER);
            }
            return *this;
        }

        template <typename T, void (T::*Method)()>
        PopupMenu &add_toggle(const char *path, T *instance, int shortcut = 0, bool checked = false, int flags = 0)
        {
            int f = FL_MENU_TOGGLE | flags;
            if (checked)
                f |= FL_MENU_VALUE;
            return add<T, Method>(path, instance, shortcut, f);
        }

        PopupMenu &add_toggle(const char *path, int shortcut = 0, bool checked = false, int flags = 0)
        {
            int f = FL_MENU_TOGGLE | flags;
            if (checked)
                f |= FL_MENU_VALUE;
            return add(path, shortcut, f);
        }

        template <typename T, void (T::*Method)()>
        PopupMenu &add_radio(const char *path, T *instance, int shortcut = 0, bool checked = false, int flags = 0)
        {
            int f = FL_MENU_RADIO | flags;
            if (checked)
                f |= FL_MENU_VALUE;
            return add<T, Method>(path, instance, shortcut, f);
        }

        PopupMenu &add_radio(const char *path, int shortcut = 0, bool checked = false, int flags = 0)
        {
            int f = FL_MENU_RADIO | flags;
            if (checked)
                f |= FL_MENU_VALUE;
            return add(path, shortcut, f);
        }

        template <typename T, void (T::*Method)()>
        PopupMenu &add_inactive(const char *path, T *instance, int shortcut = 0, int flags = 0)
        {
            return add<T, Method>(path, instance, shortcut, FL_MENU_INACTIVE | flags);
        }

        PopupMenu &add_inactive(const char *path, int shortcut = 0, int flags = 0)
        {
            return add(path, shortcut, FL_MENU_INACTIVE | flags);
        }

        template <typename T, void (T::*Method)()>
        PopupMenu &add_invisible(const char *path, T *instance, int shortcut = 0, int flags = 0)
        {
            return add<T, Method>(path, instance, shortcut, FL_MENU_INVISIBLE | flags);
        }

        PopupMenu &add_invisible(const char *path, int shortcut = 0, int flags = 0)
        {
            return add(path, shortcut, FL_MENU_INVISIBLE | flags);
        }

        template <typename T, void (T::*Method)()>
        PopupMenu &insert(int index, const char *path, T *instance, int shortcut = 0, int flags = 0)
        {
            auto thunk = [](Fl_Widget *, void *data)
            {
                (static_cast<T *>(data)->*Method)();
            };
            m_last_add_idx = m_menu_button.insert(index, path, shortcut, thunk, instance, flags);
            return *this;
        }

        PopupMenu &insert(int index, const char *path, int shortcut = 0, int flags = 0)
        {
            m_last_add_idx = m_menu_button.insert(index, path, shortcut, nullptr, nullptr, flags);
            return *this;
        }

        PopupMenu &replace(int index, const char *text)
        {
            m_menu_button.replace(index, text);
            return *this;
        }

        PopupMenu &remove(int index)
        {
            m_menu_button.remove(index);
            return *this;
        }

        PopupMenu &clear()
        {
            m_menu_button.clear();
            return *this;
        }

        PopupMenu &clear_submenu(int index)
        {
            m_menu_button.clear_submenu(index);
            return *this;
        }

        PopupMenu &mode(int index, int fl)
        {
            m_menu_button.mode(index, fl);
            return *this;
        }

        int mode(int index) const
        {
            return m_menu_button.mode(index);
        }

        int size() const
        {
            return m_menu_button.size();
        }

        const char *text(int index) const
        {
            return m_menu_button.text(index);
        }

        const Fl_Menu_Item *show()
        {
            m_menu_button.box(Theme::schemes::MENU_POPUP_BOX);
            m_menu_button.color(ThemeManager::get_palette().bg_main);
            m_menu_button.selection_color(ThemeManager::get_palette().selection);
            m_menu_button.textcolor(fl_contrast(ThemeManager::get_palette().fg_main, ThemeManager::get_palette().bg_main));
            return m_menu_button.popup();
        }

        const Fl_Menu_Item *show(int x, int y, const char *title = nullptr)
        {
            m_menu_button.box(Theme::schemes::MENU_POPUP_BOX);
            m_menu_button.color(ThemeManager::get_palette().bg_main);
            m_menu_button.selection_color(ThemeManager::get_palette().selection);
            m_menu_button.textcolor(fl_contrast(ThemeManager::get_palette().fg_main, ThemeManager::get_palette().bg_main));
            const Fl_Menu_Item *m = m_menu_button.menu();
            if (!m)
                return nullptr;
            return m->popup(x, y, title, m_menu_button.mvalue(), &m_menu_button);
        }
    };
}