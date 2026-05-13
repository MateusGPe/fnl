// Layout.hpp
#pragma once
#include <FL/Fl_Flex.H>
#include <FL/Fl_Grid.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Wizard.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <memory>
#include <utility>

namespace mui
{
    // --- Geometry Fallback ---
    // Grabs dimensions from the current active group to ensure layout
    // engines like Fl_Flex have non-zero geometry to calculate correctly.
    inline void get_default_bounds(int &x, int &y, int &w, int &h)
    {
        if (Fl_Group *p = Fl_Group::current())
        {
            x = p->x();
            y = p->y();
            w = p->w();
            h = p->h();
        }
        else
        {
            x = 10;
            y = 10;
            w = 100;
            h = 100;
        }
    }

    // --- Fluent Builder API ---
    // Uses CRTP and returns std::shared_ptr<Derived> to allow safe, chained initializations.
    template <typename T, typename Derived>
    class FluentBuilderBase : public std::enable_shared_from_this<Derived>
    {
    protected:
        T *m_widget;

    public:
        explicit FluentBuilderBase(T *widget) : m_widget(widget) {}

#define MUI_BUILDER_METHOD(MethodName)                                  \
        template <typename... Args>                                     \
        std::shared_ptr<Derived> MethodName(Args &&...args)             \
        {                                                               \
            m_widget->MethodName(std::forward<Args>(args)...);          \
            return this->shared_from_this();                            \
        }

        // Common Fl_Widget methods
        MUI_BUILDER_METHOD(activate)
        MUI_BUILDER_METHOD(active)
        MUI_BUILDER_METHOD(align)
        MUI_BUILDER_METHOD(box)
        MUI_BUILDER_METHOD(callback)
        MUI_BUILDER_METHOD(clear_active)
        MUI_BUILDER_METHOD(clear_output)
        MUI_BUILDER_METHOD(clear_visible)
        MUI_BUILDER_METHOD(clear_visible_focus)
        MUI_BUILDER_METHOD(color)
        MUI_BUILDER_METHOD(copy_label)
        MUI_BUILDER_METHOD(copy_tooltip)
        MUI_BUILDER_METHOD(deactivate)
        MUI_BUILDER_METHOD(deimage)
        MUI_BUILDER_METHOD(hide)
        MUI_BUILDER_METHOD(image)
        MUI_BUILDER_METHOD(label)
        MUI_BUILDER_METHOD(labelcolor)
        MUI_BUILDER_METHOD(labelfont)
        MUI_BUILDER_METHOD(labelsize)
        MUI_BUILDER_METHOD(labeltype)
        MUI_BUILDER_METHOD(output)
        MUI_BUILDER_METHOD(position)
        MUI_BUILDER_METHOD(resize)
        MUI_BUILDER_METHOD(selection_color)
        MUI_BUILDER_METHOD(set_active)
        MUI_BUILDER_METHOD(set_output)
        MUI_BUILDER_METHOD(set_visible)
        MUI_BUILDER_METHOD(set_visible_focus)
        MUI_BUILDER_METHOD(show)
        MUI_BUILDER_METHOD(size)
        MUI_BUILDER_METHOD(tooltip)
        MUI_BUILDER_METHOD(type)
        MUI_BUILDER_METHOD(user_data)
        MUI_BUILDER_METHOD(visible)
        MUI_BUILDER_METHOD(visible_focus)
        MUI_BUILDER_METHOD(when)

        // Common Valuator / Input methods
        MUI_BUILDER_METHOD(bounds)
        MUI_BUILDER_METHOD(maximum)
        MUI_BUILDER_METHOD(minimum)
        MUI_BUILDER_METHOD(step)
        MUI_BUILDER_METHOD(value)

        // Common Container / Browser methods
        MUI_BUILDER_METHOD(add)
        MUI_BUILDER_METHOD(clear)
        MUI_BUILDER_METHOD(insert)
        MUI_BUILDER_METHOD(remove)
        MUI_BUILDER_METHOD(resizable)

#undef MUI_BUILDER_METHOD

        T *end() { return m_widget; }
    };

    template <typename T>
    class WidgetBuilder : public FluentBuilderBase<T, WidgetBuilder<T>>
    {
    public:
        explicit WidgetBuilder(T *widget) : FluentBuilderBase<T, WidgetBuilder<T>>(widget) {}
    };

    // Main build factory
    template <typename T>
    std::shared_ptr<WidgetBuilder<T>> build(const char *label = nullptr, int x = -1, int y = -1, int w = -1, int h = -1)
    {
        if (x == -1 && y == -1 && w == -1 && h == -1)
        {
            get_default_bounds(x, y, w, h);
        }
        return std::make_shared<WidgetBuilder<T>>(new T(x, y, w, h, label));
    }

    // --- Legacy Make Factory ---
    template <typename T, typename SetupFunc>
    T *make(SetupFunc &&setup, const char *label = nullptr, int x = -1, int y = -1, int w = -1, int h = -1)
    {
        if (x == -1 && y == -1 && w == -1 && h == -1)
            get_default_bounds(x, y, w, h);
        T *widget = new T(x, y, w, h, label);
        setup(widget);
        return widget;
    }

    template <typename T>
    T *make(const char *label = nullptr, int x = -1, int y = -1, int w = -1, int h = -1)
    {
        if (x == -1 && y == -1 && w == -1 && h == -1)
            get_default_bounds(x, y, w, h);
        return new T(x, y, w, h, label);
    }

    template <typename T>
    T *make(T *&widget, const char *label = nullptr, int x = -1, int y = -1, int w = -1, int h = -1)
    {
        if (x == -1 && y == -1 && w == -1 && h == -1)
            get_default_bounds(x, y, w, h);
        widget = new T(x, y, w, h, label);
        return widget;
    }

    // --- Component Factories ---
    inline Fl_Box *make_label(const char *text, Fl_Align alignment = FL_ALIGN_LEFT | FL_ALIGN_INSIDE)
    {
        auto *box = make<Fl_Box>(text);
        box->align(alignment);
        return box;
    }

    inline Fl_Box *make_header(const char *text)
    {
        auto *box = make<Fl_Box>(text);
        box->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        box->labelfont(FL_HELVETICA_BOLD);
        return box;
    }

    // --- Flex & Layout Constructs ---
    struct FlexItem
    {
        Fl_Widget *widget;
        int fixed_size;
    };

    inline FlexItem Fix(Fl_Widget *w, int size) { return {w, size}; }
    inline FlexItem Stretch(Fl_Widget *w) { return {w, 0}; }
    inline FlexItem Spacer() { return {new Fl_Box(0, 0, 0, 0), 0}; }
    inline FlexItem FixedSpacer(int size) { return {new Fl_Box(0, 0, 0, 0), size}; }

    template <typename... Items>
    Fl_Flex *make_hbox(int x, int y, int w, int h, int gap, Items... items)
    {
        Fl_Flex *flex = new Fl_Flex(x, y, w, h, Fl_Flex::HORIZONTAL);
        (..., flex->add(items.widget));
        flex->gap(gap);
        (..., (items.fixed_size > 0 ? flex->fixed(items.widget, items.fixed_size) : (void)0));
        flex->end();
        return flex;
    }

    template <typename... Items>
    Fl_Flex *make_hbox(int w, int h, int gap, Items... items)
    {
        int x = 0, y = 0, dummy_w, dummy_h;
        if (Fl_Group *p = Fl_Group::current())
        {
            x = p->x();
            y = p->y();
        }
        return make_hbox(x, y, w, h, gap, std::forward<Items>(items)...);
    }

    template <typename... Items>
    Fl_Flex *make_hbox(int gap, Items... items)
    {
        int x, y, w, h;
        get_default_bounds(x, y, w, h);
        return make_hbox(x, y, w, h, gap, std::forward<Items>(items)...);
    }

    template <typename... Items>
    Fl_Flex *make_vbox(int x, int y, int w, int h, int gap, Items... items)
    {
        Fl_Flex *flex = new Fl_Flex(x, y, w, h, Fl_Flex::VERTICAL);
        (..., flex->add(items.widget));

        flex->gap(gap);
        (..., (items.fixed_size > 0 ? flex->fixed(items.widget, items.fixed_size) : (void)0));

        flex->end();
        return flex;
    }

    template <typename... Items>
    Fl_Flex *make_vbox(int w, int h, int gap, Items... items)
    {
        int x = 0, y = 0, dummy_w, dummy_h;
        if (Fl_Group *p = Fl_Group::current())
        {
            x = p->x();
            y = p->y();
        }
        return make_vbox(x, y, w, h, gap, std::forward<Items>(items)...);
    }

    template <typename... Items>
    Fl_Flex *make_vbox(int gap, Items... items)
    {
        int x, y, w, h;
        get_default_bounds(x, y, w, h);
        return make_vbox(x, y, w, h, gap, std::forward<Items>(items)...);
    }

#define MUI_MAKE_CONTAINER_HELPER(TypeName, FuncName)                                     \
    template <typename... Widgets>                                                        \
    TypeName *FuncName(int x, int y, int w, int h, Widgets *...widgets)                   \
    {                                                                                     \
        static_assert((... && std::is_base_of_v<Fl_Widget, Widgets>),                     \
                      "All arguments must be pointers to Fl_Widget or derived classes."); \
        TypeName *g = new TypeName(x, y, w, h);                                           \
        (..., g->add(widgets));                                                           \
        g->end();                                                                         \
        return g;                                                                         \
    }                                                                                     \
    template <typename... Widgets>                                                        \
    TypeName *FuncName(int w, int h, Widgets *...widgets)                                 \
    {                                                                                     \
        int x = 0, y = 0, dummy_w, dummy_h;                                               \
        if (Fl_Group *p = Fl_Group::current())                                            \
        {                                                                                 \
            x = p->x();                                                                   \
            y = p->y();                                                                   \
        }                                                                                 \
        return FuncName(x, y, w, h, widgets...);                                          \
    }                                                                                     \
    template <typename... Widgets>                                                        \
    TypeName *FuncName(Widgets *...widgets)                                               \
    {                                                                                     \
        int x, y, w, h;                                                                   \
        get_default_bounds(x, y, w, h);                                                   \
        return FuncName(x, y, w, h, widgets...);                                          \
    }

    MUI_MAKE_CONTAINER_HELPER(Fl_Group, make_group)
    MUI_MAKE_CONTAINER_HELPER(Fl_Pack, make_pack)
    MUI_MAKE_CONTAINER_HELPER(Fl_Scroll, make_scroll)
    MUI_MAKE_CONTAINER_HELPER(Fl_Tabs, make_tabs)
    MUI_MAKE_CONTAINER_HELPER(Fl_Tile, make_tile)
    MUI_MAKE_CONTAINER_HELPER(Fl_Wizard, make_wizard)

#undef MUI_MAKE_CONTAINER_HELPER

    // --- Grid System ---
    namespace detail
    {
        struct GridOption
        {
        };
    }

    struct GridDimensions : detail::GridOption
    {
        int rows, cols;
        void apply(Fl_Grid *grid) const { grid->layout(rows, cols); }
        GridDimensions(int r, int c) : rows(r), cols(c) {}
    };
    inline GridDimensions Layout(int rows, int cols) { return GridDimensions{rows, cols}; }

    struct GridMargin : detail::GridOption
    {
        int left, top, right, bottom;
        void apply(Fl_Grid *grid) const { grid->margin(left, top, right, bottom); }
        GridMargin(int l, int t, int r, int b) : left(l), top(t), right(r), bottom(b) {}
    };
    inline GridMargin Margin(int m) { return GridMargin{m, m, m, m}; }
    inline GridMargin Margin(int l, int t, int r, int b) { return GridMargin{l, t, r, b}; }

    struct GridGap : detail::GridOption
    {
        int row_gap, col_gap;
        void apply(Fl_Grid *grid) const { grid->gap(row_gap, col_gap); }
        GridGap(int rg, int cg) : row_gap(rg), col_gap(cg) {}
    };
    inline GridGap Gap(int g) { return GridGap{g, g}; }
    inline GridGap Gap(int rg, int cg) { return GridGap{rg, cg}; }

    template <typename... Weights>
    struct GridColWeights : detail::GridOption
    {
        std::tuple<Weights...> weights;
        void apply(Fl_Grid *grid) const
        {
            int i = 0;
            std::apply([&](auto... w_args)
                       { (..., grid->col_weight(i++, w_args)); },
                       weights);
        }
    };
    template <typename... Weights>
    inline GridColWeights<Weights...> ColWeights(Weights... w) { return GridColWeights<Weights...>{{}, {w...}}; }

    template <typename... Weights>
    struct GridRowWeights : detail::GridOption
    {
        std::tuple<Weights...> weights;
        void apply(Fl_Grid *grid) const
        {
            int i = 0;
            std::apply([&](auto... w_args)
                       { (..., grid->row_weight(i++, w_args)); },
                       weights);
        }
    };
    template <typename... Weights>
    inline GridRowWeights<Weights...> RowWeights(Weights... w) { return GridRowWeights<Weights...>{{}, {w...}}; }

    template <typename... Widths>
    struct GridColWidths : detail::GridOption
    {
        std::tuple<Widths...> widths;
        void apply(Fl_Grid *grid) const
        {
            int i = 0;
            std::apply([&](auto... w_args)
                       { (..., grid->col_width(i++, w_args)); },
                       widths);
        }
    };
    template <typename... Widths>
    inline GridColWidths<Widths...> ColWidths(Widths... w) { return GridColWidths<Widths...>{{}, {w...}}; }

    template <typename... Heights>
    struct GridRowHeights : detail::GridOption
    {
        std::tuple<Heights...> heights;
        void apply(Fl_Grid *grid) const
        {
            int i = 0;
            std::apply([&](auto... h_args)
                       { (..., grid->row_height(i++, h_args)); },
                       heights);
        }
    };
    template <typename... Heights>
    inline GridRowHeights<Heights...> RowHeights(Heights... h) { return GridRowHeights<Heights...>{{}, {h...}}; }

    struct GridItem
    {
        Fl_Widget *widget;
        int row, col, rowspan, colspan;
    };

    inline GridItem Cell(Fl_Widget *w, int r, int c, int rs = 1, int cs = 1)
    {
        return {w, r, c, rs, cs};
    }

    template <typename... Args>
    Fl_Grid *make_grid(int x, int y, int w, int h, Args... args)
    {
        Fl_Grid *grid = new Fl_Grid(x, y, w, h);

        (..., [&]
         {
        if constexpr (std::is_same_v<std::decay_t<Args>, GridItem>) { grid->add(args.widget); } }());

        (..., [&]
         {
        if constexpr (std::is_base_of_v<detail::GridOption, std::decay_t<Args>>) { args.apply(grid); } }());

        (..., [&]
         {
        if constexpr (std::is_same_v<std::decay_t<Args>, GridItem>) { grid->widget(args.widget, args.row, args.col, args.rowspan, args.colspan); } }());
        grid->end();
        return grid;
    }

    template <typename... Args>
    Fl_Grid *make_grid(int w, int h, Args... args)
    {
        int x = 0, y = 0, dummy_w, dummy_h;
        if (Fl_Group *p = Fl_Group::current())
        {
            x = p->x();
            y = p->y();
        }
        return make_grid(x, y, w, h, std::forward<Args>(args)...);
    }

    template <typename... Args>
    Fl_Grid *make_grid(Args... args)
    {
        int x, y, w, h;
        get_default_bounds(x, y, w, h);
        return make_grid(x, y, w, h, std::forward<Args>(args)...);
    }
}