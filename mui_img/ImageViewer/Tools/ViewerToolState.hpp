#pragma once

namespace mui
{
    class InternalImageViewer;
    
    struct ViewerMouseEvent
    {
        InternalImageViewer *viewer;
        int mx, my;
        double world_x, world_y;
    };
    
    class ViewerToolState
    {
    public:
        virtual ~ViewerToolState() = default;
        virtual int on_mouse_down(const ViewerMouseEvent &e) { return 0; }
        virtual int on_mouse_drag(const ViewerMouseEvent &e, double dx, double dy) { return 0; }
        virtual int on_mouse_up(const ViewerMouseEvent &e) { return 0; }
        virtual int on_key_press(InternalImageViewer *viewer, int key) { return 0; }
        virtual void on_render_overlay(InternalImageViewer *viewer, int cx, int cy, int cw, int ch) {}
    };
}