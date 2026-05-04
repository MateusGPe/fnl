#pragma once
#include <FL/Fl_Image.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_SVG_Image.H>
#include <FL/Fl_Shared_Image.H>
#include <stdexcept>
#include <string>
#include <utility>
#include <memory>

namespace mui
{
    struct ImageDeleter
    {
        void operator()(Fl_Image *ptr) const
        {
            if (!ptr)
                return;
            if (auto *shared = dynamic_cast<Fl_Shared_Image *>(ptr))
            {
                shared->release();
            }
            else
            {
                delete ptr;
            }
        }
    };

    class Image
    {
    private:
        std::unique_ptr<Fl_Image, ImageDeleter> m_ptr;
        std::string m_filepath;

    public:
        explicit Image(const char *filepath) : m_filepath(filepath)
        {
            fl_register_images();
            Fl_Shared_Image *shared = Fl_Shared_Image::get(filepath);

            Fl_Image* raw_ptr = nullptr;
            if (shared)
            {
                raw_ptr = shared->copy();
                shared->release();
            }

            if (!raw_ptr || raw_ptr->d() == 0 || raw_ptr->w() == 0)
            {
                delete raw_ptr;
                throw std::runtime_error("MUI: Failed to load image: " + m_filepath);
            }
            m_ptr.reset(raw_ptr);
        }
        
        explicit Image(Fl_Image *raw_copy) : m_ptr(raw_copy), m_filepath("(from raw pointer)") {}

        Image(const Image &) = delete;
        Image &operator=(const Image &) = delete;

        Image(Image &&other) noexcept = default;
        Image &operator=(Image &&other) noexcept = default;

        Fl_Image *handle() const { return m_ptr.get(); }

        int width() const { return m_ptr ? m_ptr->w() : 0; }
        int height() const { return m_ptr ? m_ptr->h() : 0; }
        int d() const { return m_ptr ? m_ptr->d() : 0; }
        int data_w() const { return m_ptr ? m_ptr->data_w() : 0; }
        int data_h() const { return m_ptr ? m_ptr->data_h() : 0; }
        void scale(int target_w, int target_h, bool proportional = true, bool can_expand = false)
        {
            if (!m_ptr)
                throw std::runtime_error("MUI: Cannot scale empty image.");
            m_ptr->scale(target_w, target_h, proportional, can_expand);
        }
        Image copy_and_scale(int target_w, int target_h) const
        {
            if (!m_ptr)
                throw std::runtime_error("MUI: Cannot copy and scale empty image.");
            Fl_Image *scaled = m_ptr->copy(target_w, target_h);
            return Image(scaled);
        }

        Image copy() const
        {
            if (!m_ptr)
                throw std::runtime_error("MUI: Cannot copy empty image.");
            return Image(m_ptr->copy());
        }

        void inactive()
        {
            if (m_ptr)
                m_ptr->inactive();
        }

        void draw(int x, int y, int w = 0, int h = 0, int cx = 0, int cy = 0)
        {
            if (m_ptr)
                m_ptr->draw(x, y, w, h, cx, cy);
        }
        void tint(Fl_Color color, float amount)
        {
            if (m_ptr)
                m_ptr->color_average(color, amount);
        }
        void grayscale()
        {
            if (m_ptr)
                m_ptr->desaturate();
        }
    };
}