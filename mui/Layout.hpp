// Layout.hpp
#pragma once
#include "Widgets.hpp"
#include "Tabs.hpp"
#include <memory>
#include <utility>

namespace mui
{
    // --- Fluent Builder API ---
    // Uses CRTP and returns std::shared_ptr<Derived> to allow safe, chained initializations.
    template <typename T, typename Derived>
    class FluentBuilderBase : public std::enable_shared_from_this<Derived>
    {
    protected:
        T *m_widget;

    public:
        explicit FluentBuilderBase(T *widget) : m_widget(widget) {}
// MUI_BUILDER_METHOD
#define MBM(MethodName)                                    \
    template <typename... Args>                            \
    std::shared_ptr<Derived> MethodName(Args &&...args)    \
    {                                                      \
        m_widget->MethodName(std::forward<Args>(args)...); \
        return this->shared_from_this();                   \
    }

        // Common Fl_Widget methods
        MBM(activate)
        MBM(active)
        MBM(align)
        MBM(box)
        MBM(callback)
        MBM(clear_active)
        MBM(clear_output)
        MBM(clear_visible)
        MBM(clear_visible_focus)
        MBM(color)
        MBM(copy_label)
        MBM(copy_tooltip)
        MBM(deactivate)
        MBM(deimage)
        MBM(hide)
        MBM(image)
        MBM(label)
        MBM(labelcolor)
        MBM(labelfont)
        MBM(labelsize)
        MBM(labeltype)
        MBM(output)
        MBM(position)
        MBM(resize)
        MBM(selection_color)
        MBM(set_active)
        MBM(set_output)
        MBM(set_visible)
        MBM(set_visible_focus)
        MBM(show)
        MBM(size)
        MBM(tooltip)
        MBM(type)
        MBM(user_data)
        MBM(visible)
        MBM(visible_focus)
        MBM(when)
        // Common Valuator / Input methods
        MBM(bounds)
        MBM(maximum)
        MBM(minimum)
        MBM(step)
        MBM(value)
        MBM(rate)
        // Common Container / Browser methods
        MBM(add)
        MBM(clear)
        MBM(insert)
        MBM(remove)
        MBM(resizable)

#undef MBM

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
    std::shared_ptr<WidgetBuilder<T>> build(const char *label = nullptr, int x = 0, int y = 0, int w = 0, int h = 0)
    {
        return std::make_shared<WidgetBuilder<T>>(new T(x, y, w, h, label));
    }

    // --- Legacy Make Factory ---
    template <typename T, typename SetupFunc>
    T *make(SetupFunc &&setup, const char *label = nullptr, int x = 0, int y = 0, int w = 0, int h = 0)
    {
        T *widget = new T(x, y, w, h, label);
        setup(widget);
        return widget;
    }

    template <typename T>
    T *make(const char *label = nullptr, int x = 0, int y = 0, int w = 0, int h = 0)
    {
        return new T(x, y, w, h, label);
    }

    template <typename T>
    T *make(T *&widget, const char *label = nullptr, int x = 0, int y = 0, int w = 0, int h = 0)
    {
        widget = new T(x, y, w, h, label);
        return widget;
    }

    // --- Component Factories ---
    inline Box *make_label(const char *text, Fl_Align alignment = FL_ALIGN_LEFT | FL_ALIGN_INSIDE)
    {
        auto *box = make<Box>(text);
        box->align(alignment);
        return box;
    }

    inline Box *make_header(const char *text)
    {
        auto *box = make<Box>(text);
        box->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        box->labelfont(FL_HELVETICA_BOLD);
        box->box(FL_FLAT_BOX);
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
    inline FlexItem Spacer() { return {new Box(0, 0, 0, 0), 0}; }
    inline FlexItem FixedSpacer(int size) { return {new Box(0, 0, 0, 0), size}; }

    template <typename... Items>
    Flex *make_hbox(int x, int y, int w, int h, int gap, Items... items)
    {
        Flex *flex = new Flex(x, y, w, h, Flex::HORIZONTAL);
        (..., flex->add(items.widget));
        flex->gap(gap);
        (..., (items.fixed_size > 0 ? flex->fixed(items.widget, items.fixed_size) : (void)0));
        flex->end();
        return flex;
    }

    template <typename... Items>
    Flex *make_hbox(int w, int h, int gap, Items... items)
    {
        return make_hbox(0, 0, w, h, gap, std::forward<Items>(items)...);
    }

    template <typename... Items>
    Flex *make_hbox(int gap, Items... items)
    {
        return make_hbox(0, 0, 0, 0, gap, std::forward<Items>(items)...);
    }

    template <typename... Items>
    Flex *make_vbox(int x, int y, int w, int h, int gap, Items... items)
    {
        Flex *flex = new Flex(x, y, w, h, Flex::VERTICAL);
        (..., flex->add(items.widget));

        flex->gap(gap);
        (..., (items.fixed_size > 0 ? flex->fixed(items.widget, items.fixed_size) : (void)0));

        flex->end();
        return flex;
    }

    template <typename... Items>
    Flex *make_vbox(int w, int h, int gap, Items... items)
    {
        return make_vbox(0, 0, w, h, gap, std::forward<Items>(items)...);
    }

    template <typename... Items>
    Flex *make_vbox(int gap, Items... items)
    {
        return make_vbox(0, 0, 0, 0, gap, std::forward<Items>(items)...);
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
        return FuncName(0, 0, w, h, widgets...);                                          \
    }                                                                                     \
    template <typename... Widgets>                                                        \
    TypeName *FuncName(Widgets *...widgets)                                               \
    {                                                                                     \
        return FuncName(0, 0, 0, 0, widgets...);                                          \
    }

    MUI_MAKE_CONTAINER_HELPER(Group, make_group)
    MUI_MAKE_CONTAINER_HELPER(Pack, make_pack)
    MUI_MAKE_CONTAINER_HELPER(Scroll, make_scroll)
    MUI_MAKE_CONTAINER_HELPER(Tabs, make_tabs)
    MUI_MAKE_CONTAINER_HELPER(Tile, make_tile)
    MUI_MAKE_CONTAINER_HELPER(Wizard, make_wizard)

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
        void apply(Grid *grid) const { grid->layout(rows, cols); }
        GridDimensions(int r, int c) : rows(r), cols(c) {}
    };
    inline GridDimensions Layout(int rows, int cols) { return GridDimensions{rows, cols}; }

    struct GridMargin : detail::GridOption
    {
        int left, top, right, bottom;
        void apply(Grid *grid) const { grid->margin(left, top, right, bottom); }
        GridMargin(int l, int t, int r, int b) : left(l), top(t), right(r), bottom(b) {}
    };
    inline GridMargin Margin(int m) { return GridMargin{m, m, m, m}; }
    inline GridMargin Margin(int l, int t, int r, int b) { return GridMargin{l, t, r, b}; }

    struct GridGap : detail::GridOption
    {
        int row_gap, col_gap;
        void apply(Grid *grid) const { grid->gap(row_gap, col_gap); }
        GridGap(int rg, int cg) : row_gap(rg), col_gap(cg) {}
    };
    inline GridGap Gap(int g) { return GridGap{g, g}; }
    inline GridGap Gap(int rg, int cg) { return GridGap{rg, cg}; }

    template <typename... Weights>
    struct GridColWeights : detail::GridOption
    {
        std::tuple<Weights...> weights;
        void apply(Grid *grid) const
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
        void apply(Grid *grid) const
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
        void apply(Grid *grid) const
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
        void apply(Grid *grid) const
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
    Grid *make_grid(int x, int y, int w, int h, Args... args)
    {
        Grid *grid = new Grid(x, y, w, h);

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
    Grid *make_grid(int w, int h, Args... args)
    {
        return make_grid(0, 0, w, h, std::forward<Args>(args)...);
    }

    template <typename... Args>
    Grid *make_grid(Args... args)
    {
        return make_grid(0, 0, 0, 0, std::forward<Args>(args)...);
    }
}