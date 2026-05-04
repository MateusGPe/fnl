#pragma once

#include <string>
#include <memory>
#include "../Image.hpp"

namespace mui
{
    enum class BlendMode
    {
        Normal,
        Multiply,
        Screen,
        Overlay
    };

    struct Point2D
    {
        double x, y;
    };
    struct Rect2D
    {
        double x, y, w, h;
    };
    enum class DocumentMode
    {
        InfiniteCanvas,
        FixedCanvas
    };

    enum class ViewerTool
    {
        Select,
        Pan,
        Move,
        Crop,
        Eyedropper
    };

    enum class LayerType
    {
        Base,
        Image
    };
}

#include "Layers/Layer.hpp"

namespace mui
{

    struct ImageLayer : public Layer
    {
        std::shared_ptr<Image> image;
        double x = 0.0;
        double y = 0.0;
        int original_w = 0;
        int original_h = 0;
        double scale_x = 1.0;
        double scale_y = 1.0;
        double rotation_angle = 0.0;
        std::shared_ptr<Image> thumbnail = nullptr;
        double crop_x = 0.0;
        double crop_y = 0.0;
        double crop_w = -1.0;
        double crop_h = -1.0;
        bool flip_h = false;
        bool flip_v = false;

        ImageLayer() = default;

        ImageLayer(std::shared_ptr<Image> img, std::string n, double x, double y, int ow, int oh, double sx, double sy, double rot, double a, BlendMode bm, bool vis, bool lck, std::shared_ptr<Image> thumb)
        {
            this->image = img;
            this->name = n;
            this->x = x;
            this->y = y;
            this->original_w = ow;
            this->original_h = oh;
            this->scale_x = sx;
            this->scale_y = sy;
            this->rotation_angle = rot;
            this->alpha = a;
            this->blend_mode = bm;
            this->visible = vis;
            this->locked = lck;
        }

        Rect2D get_effective_bounds() const
        {
            Rect2D r = {x, y, original_w * scale_x, original_h * scale_y};
            if (crop_w >= 0 && crop_h >= 0)
            {
                r.x += crop_x * scale_x;
                r.y += crop_y * scale_y;
                r.w = crop_w * scale_x;
                r.h = crop_h * scale_y;
            }
            return r;
        }

        Point2D get_center() const
        {
            Rect2D r = get_effective_bounds();
            return {r.x + r.w * 0.5, r.y + r.h * 0.5};
        }

        void render(int target_w, int target_h, double view_x, double view_y, double scale) override {}
        bool hit_test(double world_x, double world_y) const override { return false; }
        void get_bounds(double &min_x, double &min_y, double &max_x, double &max_y) const override {}
        std::shared_ptr<Layer> clone() const override { return std::make_shared<ImageLayer>(*this); }
        LayerType get_type() const override { return LayerType::Image; }
    };
}