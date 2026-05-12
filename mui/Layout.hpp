// Layout.hpp
#pragma once
#include <FL/Fl_Flex.H>
#include <FL/Fl_Grid.H>
#include <FL/Fl_Group.H> // For Fl_Group::add
#include <FL/Fl_Box.H>
#include <utility>

namespace mui
{
    // ---------------------------------------------------------
    // 1. The 'make' Factory (Lambda Setup Pattern)
    // ---------------------------------------------------------
    // Allocates a widget with modern (0,0,0,0) layout sizing,
    // applies your configuration lambda, and returns the pointer.
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
        T *widget = new T(x, y, w, h, label);
        return widget;
    }

    // ---------------------------------------------------------
    // 2. Variadic Flex Helpers (C++17 Fold Expressions)
    // ---------------------------------------------------------
    struct FlexItem
    {
        Fl_Widget *widget;
        int fixed_size;
    };

    // Helper syntax for defining flex items
    inline FlexItem Fix(Fl_Widget *w, int size) { return {w, size}; }
    inline FlexItem Stretch(Fl_Widget *w) { return {w, 0}; }

    // Creates an invisible spring that pushes widgets apart
    inline FlexItem Spacer() { return {new Fl_Box(0, 0, 0, 0), 0}; }
    inline FlexItem FixedSpacer(int size) { return {new Fl_Box(0, 0, 0, 0), size}; }

    // Generates a Horizontal Flex container in a single statement
    template <typename... Items>
    Fl_Flex *make_hbox(int gap, Items... items)
    {
        Fl_Flex *flex = new Fl_Flex(0, 0, 0, 0, Fl_Flex::HORIZONTAL);
        // All widgets passed must be added to the group BEFORE end() is called.
        (..., flex->add(items.widget));

        flex->gap(gap);

        // C++17 Fold Expression: applies fixed() to every item efficiently at compile-time
        (..., (items.fixed_size > 0 ? flex->fixed(items.widget, items.fixed_size) : (void)0));

        flex->end();
        return flex;
    }

    // Generates a Vertical Flex container in a single statement
    template <typename... Items>
    Fl_Flex *make_vbox(int gap, Items... items)
    {
        Fl_Flex *flex = new Fl_Flex(0, 0, 0, 0, Fl_Flex::VERTICAL);
        (..., flex->add(items.widget));

        flex->gap(gap);
        (..., (items.fixed_size > 0 ? flex->fixed(items.widget, items.fixed_size) : (void)0));

        flex->end();
        return flex;
    }

    // ---------------------------------------------------------
    // 3. Grid Helpers
    // ---------------------------------------------------------

    // --- Grid Configuration Options ---
    namespace detail
    {
        struct GridOption
        {
        }; // Base class for type-checking
    }

    // Option for grid dimensions (rows, cols)
    struct GridDimensions : detail::GridOption
    {
        int rows, cols;
        void apply(Fl_Grid *grid) const { grid->layout(rows, cols); }
        GridDimensions(int r, int c) : rows(r), cols(c) {}
    };
    inline GridDimensions Layout(int rows, int cols) { return GridDimensions{rows, cols}; }

    // Option for grid margin
    struct GridMargin : detail::GridOption
    {
        int left, top, right, bottom;
        void apply(Fl_Grid *grid) const { grid->margin(left, top, right, bottom); }
        GridMargin(int l, int t, int r, int b) : left(l), top(t), right(r), bottom(b) {}
    };
    inline GridMargin Margin(int m) { return GridMargin{m, m, m, m}; }
    inline GridMargin Margin(int l, int t, int r, int b) { return GridMargin{l, t, r, b}; }

    // Option for grid gap
    struct GridGap : detail::GridOption
    {
        int row_gap, col_gap;
        void apply(Fl_Grid *grid) const { grid->gap(row_gap, col_gap); }
        GridGap(int rg, int cg) : row_gap(rg), col_gap(cg) {}
    };
    inline GridGap Gap(int g) { return GridGap{g, g}; }
    inline GridGap Gap(int rg, int cg) { return GridGap{rg, cg}; }

    // Option for column weights
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
    inline GridColWeights<Weights...> ColWeights(Weights... w) { return GridColWeights<Weights...>{ {}, {w...}}; }

    // Option for row weights
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
    inline GridRowWeights<Weights...> RowWeights(Weights... w) { return GridRowWeights<Weights...>{ {}, {w...}}; }

    // Option for column widths
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
    inline GridColWidths<Widths...> ColWidths(Widths... w) { return GridColWidths<Widths...>{ {}, {w...}}; }

    // Option for row heights
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
    inline GridRowHeights<Heights...> RowHeights(Heights... h) { return GridRowHeights<Heights...>{ {}, {h...}}; }

    // --- Grid Items ---
    struct GridItem
    {
        Fl_Widget *widget;
        int row, col, rowspan, colspan;
    };

    // Helper syntax for defining grid items
    inline GridItem Cell(Fl_Widget *w, int r, int c, int rs = 1, int cs = 1)
    {
        return {w, r, c, rs, cs};
    }

    // Factory for Fl_Grid using a variadic list of options and items.
    // Automatically calls end() on the grid.
    template <typename... Args>
    Fl_Grid *make_grid(int x, int y, int w, int h, Args... args)
    {
        Fl_Grid *grid = new Fl_Grid(x, y, w, h);
        // Pass 1: Add all widgets to the group first.
        (..., [&]
         {
        if constexpr (std::is_same_v<std::decay_t<Args>, GridItem>) { grid->add(args.widget); } }());
        // Pass 2: Apply all configuration options.
        (..., [&]
         {
        if constexpr (std::is_base_of_v<detail::GridOption, std::decay_t<Args>>) { args.apply(grid); } }());
        // Pass 3: Assign widgets to their cells in the grid.
        (..., [&]
         {
        if constexpr (std::is_same_v<std::decay_t<Args>, GridItem>) { grid->widget(args.widget, args.row, args.col, args.rowspan, args.colspan); } }());
        grid->end();
        return grid;
    }

    template <typename... Args>
    Fl_Grid *make_grid(int w, int h, Args... args)
    {
        return make_grid(0, 0, w, h, std::forward<Args>(args)...);
    }

    template <typename... Args>
    Fl_Grid *make_grid(Args... args)
    {
        return make_grid(0, 0, 0, 0, std::forward<Args>(args)...);
    }
}