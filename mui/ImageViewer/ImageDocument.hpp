#pragma once

#include <vector>
#include <memory>
#include "Layers/Layer.hpp"
#include "Types.hpp"

namespace mui
{
    class ViewerCommand;

    class ImageDocument
    {
    private:
        std::vector<std::shared_ptr<Layer>> layers_;
        std::vector<std::shared_ptr<ViewerCommand>> undo_stack_;
        std::vector<std::shared_ptr<ViewerCommand>> redo_stack_;
        int selected_layer_index_ = -1;
        DocumentMode mode_ = DocumentMode::InfiniteCanvas;
        int canvas_width_ = 800;
        int canvas_height_ = 600;

    public:
        ImageDocument() = default;
        ~ImageDocument() = default;

        DocumentMode mode() const { return mode_; }
        void mode(DocumentMode m) { mode_ = m; }
        int canvas_width() const { return canvas_width_; }
        int canvas_height() const { return canvas_height_; }
        void canvas_size(int w, int h) { canvas_width_ = w; canvas_height_ = h; }

        void add_layer(std::shared_ptr<Layer> layer)
        {
            layers_.push_back(layer);
        }

        void clear_layers() { layers_.clear(); }
        void remove_layer(int index)
        {
            if (index >= 0 && index < (int)layers_.size())
                layers_.erase(layers_.begin() + index);
        }
        void insert_layer(int index, std::shared_ptr<Layer> layer) { layers_.insert(layers_.begin() + index, layer); }
        void swap_layers(int i, int j) { std::swap(layers_[i], layers_[j]); }
        std::shared_ptr<Layer> get_layer(int index) const { return layers_[index]; }
        size_t layer_count() const { return layers_.size(); }

        void undo() {}

        const std::vector<std::shared_ptr<Layer>> &layers() const { return layers_; }
    };
}