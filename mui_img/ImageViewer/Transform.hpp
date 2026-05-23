#pragma once

#include "Types.hpp"
#include <cmath>
#include <array>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace mui
{
    struct GizmoHandles
    {
        std::array<Point2D, 4> corners;
        Point2D rot_handle;
    };

    class Transform
    {
    public:
        static inline Point2D rotate_point(double px, double py, double cx, double cy, double angle_deg)
        {
            double rad = angle_deg * M_PI / 180.0;
            double c = std::cos(rad);
            double s = std::sin(rad);
            double tx = px - cx;
            double ty = py - cy;
            return {cx + (tx * c - ty * s), cy + (tx * s + ty * c)};
        }

        static inline Point2D world_to_local(double wx, double wy, double cx, double cy, double angle_deg, bool flip_h, bool flip_v)
        {
            double rad = -angle_deg * M_PI / 180.0;
            double c = std::cos(rad);
            double s = std::sin(rad);
            double tx = wx - cx;
            double ty = wy - cy;
            double rx = tx * c - ty * s;
            double ry = tx * s + ty * c;
            return {cx + (flip_h ? -rx : rx), cy + (flip_v ? -ry : ry)};
        }

        static inline Point2D local_to_world(double lx, double ly, double cx, double cy, double angle_deg, bool flip_h, bool flip_v)
        {
            double tx = lx - cx;
            double ty = ly - cy;
            if (flip_h)
                tx = -tx;
            if (flip_v)
                ty = -ty;
            double rad = angle_deg * M_PI / 180.0;
            double c = std::cos(rad);
            double s = std::sin(rad);
            return {cx + (tx * c - ty * s), cy + (tx * s + ty * c)};
        }

        static inline Rect2D get_rotated_bounds(double x, double y, double w, double h, double angle_deg)
        {
            double cx = x + w * 0.5;
            double cy = y + h * 0.5;
            double rad = angle_deg * M_PI / 180.0;
            double c = std::cos(rad);
            double s = std::sin(rad);
            double hw = w * 0.5;
            double hh = h * 0.5;
            double pts[4][2] = {{-hw, -hh}, {hw, -hh}, {hw, hh}, {-hw, hh}};
            double min_x = 1e9, min_y = 1e9, max_x = -1e9, max_y = -1e9;
            for (int j = 0; j < 4; ++j)
            {
                double rx = pts[j][0] * c - pts[j][1] * s;
                double ry = pts[j][0] * s + pts[j][1] * c;
                min_x = std::min(min_x, rx);
                max_x = std::max(max_x, rx);
                min_y = std::min(min_y, ry);
                max_y = std::max(max_y, ry);
            }
            return {cx + min_x, cy + min_y, max_x - min_x, max_y - min_y};
        }

        static inline Point2D get_bounds_center(const Rect2D &b)
        {
            return {b.x + b.w * 0.5, b.y + b.h * 0.5};
        }

        static inline std::array<Point2D, 4> get_bounds_corners(const Rect2D &b, double cx, double cy, double angle_deg, bool flip_h, bool flip_v)
        {
            return {
                local_to_world(b.x, b.y, cx, cy, angle_deg, flip_h, flip_v),
                local_to_world(b.x + b.w, b.y, cx, cy, angle_deg, flip_h, flip_v),
                local_to_world(b.x + b.w, b.y + b.h, cx, cy, angle_deg, flip_h, flip_v),
                local_to_world(b.x, b.y + b.h, cx, cy, angle_deg, flip_h, flip_v)};
        }

        static inline std::array<Point2D, 4> get_layer_corners(const ImageLayer &l)
        {
            Rect2D b = l.get_effective_bounds();
            return get_bounds_corners(b, b.x + b.w * 0.5, b.y + b.h * 0.5, l.rotation_angle, l.flip_h, l.flip_v);
        }

        static inline GizmoHandles get_gizmo_handles(const ImageLayer &l, double rot_offset_world)
        {
            GizmoHandles g;
            g.corners = get_layer_corners(l);

            double w_mid_x = (g.corners[0].x + g.corners[1].x) / 2.0;
            double w_mid_y = (g.corners[0].y + g.corners[1].y) / 2.0;
            double w_dx = g.corners[1].x - g.corners[0].x;
            double w_dy = g.corners[1].y - g.corners[0].y;

            double len = std::sqrt(w_dx * w_dx + w_dy * w_dy);
            if (len > 1e-6)
            {
                w_dx /= len;
                w_dy /= len;
            }
            else
            {
                w_dx = 1;
                w_dy = 0;
            }
            g.rot_handle = {w_mid_x + w_dy * rot_offset_world, w_mid_y - w_dx * rot_offset_world};
            return g;
        }
    };
}