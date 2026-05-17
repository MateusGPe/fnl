// BuiltinTools.hpp
#pragma once

#include <vector>
#include <utility>
#include <algorithm>
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include "Theme.hpp"
#include "Tools/ViewerToolState.hpp"
#include "Internal.hpp"
#include "Commands.hpp"

namespace mui
{
    enum class DragMode
    {
        None,
        Pan,
        MoveLayer,
        ScaleBR,
        ScaleBL,
        ScaleTR,
        ScaleTL,
        Crop,
        Eyedropper
    };

    inline bool is_scale_mode(DragMode m) noexcept
    {
        return m == DragMode::ScaleBR || m == DragMode::ScaleBL || m == DragMode::ScaleTR || m == DragMode::ScaleTL;
    }

    class PanTool : public ViewerToolState
    {
        double last_x = 0, last_y = 0;

    public:
        int on_mouse_down(const ViewerMouseEvent &e) override
        {
            last_x = e.mx;
            last_y = e.my;
            return 1;
        }
        int on_mouse_drag(const ViewerMouseEvent &e, double dx, double dy) override
        {
            e.viewer->pan_by(e.mx - last_x, e.my - last_y);
            last_x = e.mx;
            last_y = e.my;
            return 1;
        }
    };

    class SelectTool : public ViewerToolState
    {
        DragMode drag_mode = DragMode::None;
        double drag_start_x = 0, drag_start_y = 0;
        double orig_layer_x = 0, orig_layer_y = 0;
        double orig_layer_sx = 1, orig_layer_sy = 1;
        std::vector<std::pair<int, std::pair<double, double>>> multi_drag_origins;

        DragMode hit_test_gizmo(InternalImageViewer *v, int mx, int my)
        {
            auto lp = v->get_selected_image_layer();
            if (!lp || lp->locked)
                return DragMode::None;

            double wx, wy, lx, ly;
            v->mouse_to_world(mx, my, wx, wy);
            v->world_to_layer_local(*lp, wx, wy, lx, ly);

            Rect2D b = layer_effective_rect(*lp);
            double r = (ThemeManager::get_palette().metrics.imageviewer_handle_size * 0.5 +
                        ThemeManager::get_palette().metrics.imageviewer_handle_hit_padding) /
                       v->scale();

            auto near = [&](double px, double py)
            { return lx >= px - r && lx <= px + r && ly >= py - r && ly <= py + r; };
            if (near(b.x, b.y))
                return DragMode::ScaleTL;
            if (near(b.x + b.w, b.y))
                return DragMode::ScaleTR;
            if (near(b.x, b.y + b.h))
                return DragMode::ScaleBL;
            if (near(b.x + b.w, b.y + b.h))
                return DragMode::ScaleBR;
            return DragMode::None;
        }

    public:
        int on_mouse_down(const ViewerMouseEvent &e) override
        {
            drag_start_x = e.world_x;
            drag_start_y = e.world_y;

            DragMode gizmo = hit_test_gizmo(e.viewer, e.mx, e.my);
            if (is_scale_mode(gizmo))
            {
                auto lp = e.viewer->get_selected_image_layer();
                drag_mode = gizmo;
                orig_layer_sx = lp->scale_x;
                orig_layer_sy = lp->scale_y;
                orig_layer_x = lp->x;
                orig_layer_y = lp->y;
                return 1;
            }

            int hit_idx = e.viewer->hit_test(e.world_x, e.world_y);
            bool ctrl = Fl::event_state(FL_CTRL) || Fl::event_state(FL_META);

            if (hit_idx >= 0)
            {
                int hit_id = e.viewer->get_document()->get_layer(hit_idx)->id;
                if (ctrl)
                    e.viewer->toggle_selection(hit_id);
                else if (!e.viewer->is_in_selection(hit_id))
                {
                    e.viewer->clear_selection();
                    e.viewer->set_primary_selection(hit_id);
                }
                e.viewer->notify_layer_selected();
            }
            else if (!ctrl)
            {
                e.viewer->clear_selection();
                e.viewer->notify_layer_selected();
            }

            if (auto lp = e.viewer->get_selected_image_layer(); lp && !lp->locked)
            {
                drag_mode = DragMode::MoveLayer;
                orig_layer_x = lp->x;
                orig_layer_y = lp->y;

                multi_drag_origins.clear();
                for (int id : e.viewer->selection_ids())
                {
                    int idx = e.viewer->get_document()->get_layer_index(id);
                    if (auto ol = e.viewer->get_image_layer(idx))
                        multi_drag_origins.push_back({id, {ol->x, ol->y}});
                }
            }
            else
            {
                drag_mode = DragMode::None;
            }
            return 1;
        }

        int on_mouse_drag(const ViewerMouseEvent &e, double dx, double dy) override
        {
            if (drag_mode == DragMode::MoveLayer)
            {
                double ddx = e.world_x - drag_start_x;
                double ddy = e.world_y - drag_start_y;

                if (multi_drag_origins.size() > 1)
                {
                    for (auto &[id, origin] : multi_drag_origins)
                    {
                        int idx = e.viewer->get_document()->get_layer_index(id);
                        if (auto lp = e.viewer->get_image_layer(idx))
                        {
                            lp->x = origin.first + ddx;
                            lp->y = origin.second + ddy;
                        }
                    }
                }
                else if (auto lp = e.viewer->get_selected_image_layer())
                {
                    lp->x = orig_layer_x + ddx;
                    lp->y = orig_layer_y + ddy;
                }
                e.viewer->invalidate();
            }
            else if (is_scale_mode(drag_mode))
            {
                if (auto l = e.viewer->get_selected_image_layer())
                {
                    Rect2D b = l->get_effective_bounds();
                    double hw = (b.w * orig_layer_sx / l->scale_x) * 0.5;
                    double hh = (b.h * orig_layer_sy / l->scale_y) * 0.5;

                    double fx = (drag_mode == DragMode::ScaleTL || drag_mode == DragMode::ScaleBL) ? hw : -hw;
                    double fy = (drag_mode == DragMode::ScaleTL || drag_mode == DragMode::ScaleTR) ? hh : -hh;
                    double d_x = (drag_mode == DragMode::ScaleTL || drag_mode == DragMode::ScaleBL) ? -1.0 : 1.0;
                    double d_y = (drag_mode == DragMode::ScaleTL || drag_mode == DragMode::ScaleTR) ? -1.0 : 1.0;

                    Point2D orig_c = {(orig_layer_x + (b.x - l->x)) + hw, (orig_layer_y + (b.y - l->y)) + hh};
                    Point2D w_fixed = Transform::local_to_world(orig_c.x + fx, orig_c.y + fy, orig_c.x, orig_c.y, l->rotation_angle, l->flip_h, l->flip_v);
                    Point2D drag_loc = Transform::world_to_local(e.world_x, e.world_y, w_fixed.x, w_fixed.y, l->rotation_angle, l->flip_h, l->flip_v);

                    double new_w = std::max(5.0 / e.viewer->scale(), (drag_loc.x - w_fixed.x) * d_x);
                    double new_h = std::max(5.0 / e.viewer->scale(), (drag_loc.y - w_fixed.y) * d_y);

                    l->scale_x = new_w / (b.w / l->scale_x);
                    l->scale_y = new_h / (b.h / l->scale_y);

                    double nfx = (drag_mode == DragMode::ScaleTL || drag_mode == DragMode::ScaleBL) ? new_w / 2 : -new_w / 2;
                    double nfy = (drag_mode == DragMode::ScaleTL || drag_mode == DragMode::ScaleTR) ? new_h / 2 : -new_h / 2;
                    Point2D r_n = Transform::rotate_point(l->flip_h ? -nfx : nfx, l->flip_v ? -nfy : nfy, 0, 0, l->rotation_angle);
                    l->x = (w_fixed.x - r_n.x) - new_w * 0.5 - (l->crop_w >= 0 ? l->crop_x * l->scale_x : 0);
                    l->y = (w_fixed.y - r_n.y) - new_h * 0.5 - (l->crop_h >= 0 ? l->crop_y * l->scale_y : 0);
                    e.viewer->invalidate();
                }
            }
            return 1;
        }

        int on_mouse_up(const ViewerMouseEvent &e) override
        {
            if (drag_mode == DragMode::MoveLayer)
            {
                if (multi_drag_origins.size() > 1)
                {
                    for (auto &[id, origin] : multi_drag_origins)
                    {
                        int idx = e.viewer->get_document()->get_layer_index(id);
                        if (auto lp = e.viewer->get_image_layer(idx))
                        {
                            if (lp->x != origin.first || lp->y != origin.second)
                                e.viewer->push_command(std::make_shared<CommandMove>(id, origin.first, origin.second, lp->x, lp->y));
                        }
                    }
                }
                else if (auto lp = e.viewer->get_selected_image_layer())
                {
                    if (lp->x != orig_layer_x || lp->y != orig_layer_y)
                        e.viewer->push_command(std::make_shared<CommandMove>(lp->id, orig_layer_x, orig_layer_y, lp->x, lp->y));
                }
            }
            else if (is_scale_mode(drag_mode))
            {
                if (auto lp = e.viewer->get_selected_image_layer())
                {
                    if (lp->x != orig_layer_x || lp->y != orig_layer_y || lp->scale_x != orig_layer_sx || lp->scale_y != orig_layer_sy)
                        e.viewer->push_command(std::make_shared<CommandScale>(lp->id, orig_layer_x, orig_layer_y, orig_layer_sx, orig_layer_sy, lp->x, lp->y, lp->scale_x, lp->scale_y));
                }
            }
            drag_mode = DragMode::None;
            e.viewer->notify_view_changed();
            return 1;
        }

        int on_key_press(InternalImageViewer *viewer, int key) override
        {
            if (key == FL_Delete || key == FL_BackSpace)
            {
                std::vector<int> to_delete;
                for (int id : viewer->selection_ids())
                {
                    int idx = viewer->get_document()->get_layer_index(id);
                    if (idx >= 0)
                        to_delete.push_back(idx);
                }
                std::sort(to_delete.rbegin(), to_delete.rend());
                for (int idx : to_delete)
                    viewer->push_command(std::make_shared<CommandDelete>(idx, viewer->get_document()->get_layer(idx)));
                viewer->clear_selection();
                return 1;
            }
            double nudge = Fl::event_state(FL_SHIFT) ? 10.0 : 1.0;
            double dx = 0.0, dy = 0.0;
            if (key == FL_Left)
                dx = -nudge / viewer->scale();
            else if (key == FL_Right)
                dx = nudge / viewer->scale();
            else if (key == FL_Up)
                dy = -nudge / viewer->scale();
            else if (key == FL_Down)
                dy = nudge / viewer->scale();

            if ((dx != 0 || dy != 0) && viewer->get_selected_image_layer())
            {
                auto l = viewer->get_selected_image_layer();
                viewer->push_command(std::make_shared<CommandMove>(l->id, l->x, l->y, l->x + dx, l->y + dy));
                return 1;
            }
            return 0;
        }
    };

    class CropTool : public ViewerToolState
    {
        double crop_start_x = 0, crop_start_y = 0;
        double crop_end_x = 0, crop_end_y = 0;
        bool dragging = false;

    public:
        int on_mouse_down(const ViewerMouseEvent &e) override
        {
            if (auto lp = e.viewer->get_selected_image_layer())
            {
                e.viewer->world_to_layer_local(*lp, e.world_x, e.world_y, crop_start_x, crop_start_y);
                crop_end_x = crop_start_x;
                crop_end_y = crop_start_y;
                dragging = true;
            }
            return 1;
        }
        int on_mouse_drag(const ViewerMouseEvent &e, double dx, double dy) override
        {
            if (dragging)
            {
                if (auto lp = e.viewer->get_selected_image_layer())
                    e.viewer->world_to_layer_local(*lp, e.world_x, e.world_y, crop_end_x, crop_end_y);
                e.viewer->invalidate();
            }
            return 1;
        }
        int on_mouse_up(const ViewerMouseEvent &e) override
        {
            if (dragging && e.viewer->get_selected_image_layer())
            {
                auto l = e.viewer->get_selected_image_layer();
                double min_lx = std::min(crop_start_x, crop_end_x), min_ly = std::min(crop_start_y, crop_end_y);
                double max_lx = std::max(crop_start_x, crop_end_x), max_ly = std::max(crop_start_y, crop_end_y);

                double iw = l->image->data_w(), ih = l->image->data_h();
                double cx = std::clamp((min_lx - l->x) / l->scale_x, 0.0, iw);
                double cy = std::clamp((min_ly - l->y) / l->scale_y, 0.0, ih);
                double cw = std::clamp((max_lx - min_lx) / l->scale_x, 0.0, iw - cx);
                double ch = std::clamp((max_ly - min_ly) / l->scale_y, 0.0, ih - cy);

                if (cw > 1.0 && ch > 1.0)
                {
                    Rect2D cur = layer_effective_rect(*l);
                    double C_x = cur.x + cur.w * 0.5, C_y = cur.y + cur.h * 0.5;
                    double C_ux = (l->x + cx * l->scale_x) + (cw * l->scale_x) * 0.5;
                    double C_uy = (l->y + cy * l->scale_y) + (ch * l->scale_y) * 0.5;

                    Point2D W = Transform::local_to_world(C_ux, C_uy, C_x, C_y, l->rotation_angle, l->flip_h, l->flip_v);
                    e.viewer->push_command(std::make_shared<CommandCrop>(l->id, l->crop_x, l->crop_y, l->crop_w, l->crop_h,
                                                                         cx, cy, cw, ch, l->x, l->y, l->x + (W.x - C_ux), l->y + (W.y - C_uy)));
                }
            }
            dragging = false;
            e.viewer->invalidate();
            return 1;
        }

        void on_render_overlay(InternalImageViewer *v, int cx, int cy, int cw, int ch) override
        {
            if (dragging && v->get_selected_image_layer())
            {
                auto l = v->get_selected_image_layer();
                double lw = l->original_w * l->scale_x, lh = l->original_h * l->scale_y;
                if (l->crop_w >= 0)
                {
                    lw = l->crop_w * l->scale_x;
                    lh = l->crop_h * l->scale_y;
                }
                double rot_cx = (l->x + (l->crop_w >= 0 ? l->crop_x * l->scale_x : 0)) + lw * 0.5;
                double rot_cy = (l->y + (l->crop_h >= 0 ? l->crop_y * l->scale_y : 0)) + lh * 0.5;

                double min_lx = std::min(crop_start_x, crop_end_x), min_ly = std::min(crop_start_y, crop_end_y);
                double max_lx = std::max(crop_start_x, crop_end_x), max_ly = std::max(crop_start_y, crop_end_y);
                Point2D pts[4] = {{min_lx, min_ly}, {max_lx, min_ly}, {max_lx, max_ly}, {min_lx, max_ly}};

                fl_color(ThemeManager::get_palette().fg_main);
                fl_line_style(FL_DASH, 2);
                fl_begin_loop();
                for (int j = 0; j < 4; ++j)
                {
                    Point2D w_pt = Transform::local_to_world(pts[j].x, pts[j].y, rot_cx, rot_cy, l->rotation_angle, l->flip_h, l->flip_v);
                    int sx, sy;
                    v->world_to_screen(w_pt.x, w_pt.y, sx, sy);
                    fl_vertex(sx, sy);
                }
                fl_end_loop();
                fl_line_style(0);
            }
        }
    };

    class EyedropperTool : public ViewerToolState
    {
    public:
        int on_mouse_down(const ViewerMouseEvent &e) override
        {
            e.viewer->sample_color(e.mx, e.my, e.world_x, e.world_y);
            return 1;
        }
        int on_mouse_drag(const ViewerMouseEvent &e, double dx, double dy) override
        {
            e.viewer->sample_color(e.mx, e.my, e.world_x, e.world_y);
            return 1;
        }
    };
}