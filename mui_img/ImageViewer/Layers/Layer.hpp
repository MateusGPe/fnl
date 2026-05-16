#pragma once

#include <string>
#include <memory>
#include "../Types.hpp"

namespace mui
{
    class Layer
    {
    public:
        std::string name;
        int id = -1;
        int parent_id = -1;
        bool visible = true;
        bool locked = false;
        double alpha = 1.0;
        BlendMode blend_mode = BlendMode::Normal;

        virtual ~Layer() = default;
        virtual void render(int target_w, int target_h, double view_x, double view_y, double scale) = 0;
        virtual bool hit_test(double world_x, double world_y) const = 0;
        virtual void get_bounds(double &min_x, double &min_y, double &max_x, double &max_y) const = 0;
        virtual std::shared_ptr<Layer> clone() const = 0;
        virtual LayerType get_type() const = 0;

    };
}