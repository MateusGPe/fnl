#pragma once

namespace mui
{
    class InternalImageViewer;

    class ViewerToolState
    {
    public:
        virtual ~ViewerToolState() = default;
        virtual int on_mouse_down(InternalImageViewer *viewer, int mx, int my, double world_x, double world_y) { return 0; }
        virtual int on_mouse_drag(InternalImageViewer *viewer, int mx, int my, double world_x, double world_y, double dx, double dy) { return 0; }
        virtual int on_mouse_up(InternalImageViewer *viewer, int mx, int my, double world_x, double world_y) { return 0; }
        virtual int on_key_press(InternalImageViewer *viewer, int key) { return 0; }
        virtual void on_render_overlay(InternalImageViewer *viewer, int cx, int cy, int cw, int ch) {}
    };
}