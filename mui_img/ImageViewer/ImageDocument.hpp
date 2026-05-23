#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include "Types.hpp"

namespace mui
{
    // A pure data class representing the image state without any knowledge of the UI
    class ImageDocument
    {
    private:
        int next_layer_id_ = 0;
        std::vector<std::shared_ptr<Layer>> layers_;
        DocumentMode mode_ = DocumentMode::InfiniteCanvas;
        int canvas_width_ = 800;
        int canvas_height_ = 600;
        std::unordered_map<int, int> layer_id_to_index_;

        void rebuild_id_map()
        {
            layer_id_to_index_.clear();
            for (size_t i = 0; i < layers_.size(); ++i)
            {
                if (layers_[i])
                {
                    layer_id_to_index_[layers_[i]->id] = i;
                }
            }
        }

    public:
        ImageDocument() = default;
        ImageDocument(const ImageDocument &other)
            : next_layer_id_(other.next_layer_id_),
              mode_(other.mode_),
              canvas_width_(other.canvas_width_),
              canvas_height_(other.canvas_height_)
        {
            layers_.reserve(other.layers_.size());
            for (const auto &layer : other.layers_)
            {
                if (layer)
                    layers_.push_back(layer->clone());
            }
            rebuild_id_map();
        }

        ~ImageDocument() = default;

        DocumentMode mode() const { return mode_; }
        void mode(DocumentMode m) { mode_ = m; }
        int canvas_width() const { return canvas_width_; }
        int canvas_height() const { return canvas_height_; }
        void canvas_size(int w, int h)
        {
            canvas_width_ = w;
            canvas_height_ = h;
        }

        void add_layer(std::shared_ptr<Layer> layer)
        {
            if (layer->id == -1)
                layer->id = next_layer_id_++;
            layer_id_to_index_[layer->id] = layers_.size();
            layers_.push_back(layer);
        }

        void clear_layers()
        {
            layers_.clear();
            layer_id_to_index_.clear();
        }
        void remove_layer(int index)
        {
            if (index >= 0 && index < (int)layers_.size())
            {
                layers_.erase(layers_.begin() + index);
                rebuild_id_map();
            }
        }
        void insert_layer(int index, std::shared_ptr<Layer> layer)
        {
            layers_.insert(layers_.begin() + index, layer);
            rebuild_id_map();
        }
        void swap_layers(int i, int j)
        {
            std::swap(layers_[i], layers_[j]);
            rebuild_id_map();
        }
        std::shared_ptr<Layer> get_layer(int index) const { return layers_[index]; }
        size_t layer_count() const { return layers_.size(); }

        const std::vector<std::shared_ptr<Layer>> &layers() const { return layers_; }

        int get_layer_index(int id) const
        {
            auto it = layer_id_to_index_.find(id);
            return (it != layer_id_to_index_.end()) ? it->second : -1;
        }
    };
}