#pragma once
#include <FL/Fl_Native_File_Chooser.H>

namespace mui
{
    class NativeFileChooser
    {
    public:
        enum Type
        {
            BrowseFile = Fl_Native_File_Chooser::BROWSE_FILE,
            BrowseDirectory = Fl_Native_File_Chooser::BROWSE_DIRECTORY,
            BrowseMultiFile = Fl_Native_File_Chooser::BROWSE_MULTI_FILE,
            BrowseMultiDirectory = Fl_Native_File_Chooser::BROWSE_MULTI_DIRECTORY,
            SaveFile = Fl_Native_File_Chooser::BROWSE_SAVE_FILE,
            SaveDirectory = Fl_Native_File_Chooser::BROWSE_SAVE_DIRECTORY
        };

        enum Options
        {
            NoOptions = Fl_Native_File_Chooser::NO_OPTIONS,
            SaveAsConfirm = Fl_Native_File_Chooser::SAVEAS_CONFIRM,
            NewFolder = Fl_Native_File_Chooser::NEW_FOLDER,
            Preview = Fl_Native_File_Chooser::PREVIEW,
            UseFilterExt = Fl_Native_File_Chooser::USE_FILTER_EXT
        };

    private:
        Fl_Native_File_Chooser m_chooser;

    public:
        explicit NativeFileChooser(Type type = BrowseFile)
        {
            m_chooser.type(type);
        }

        NativeFileChooser &title(const char *t)
        {
            m_chooser.title(t);
            return *this;
        }

        NativeFileChooser &filter(const char *f)
        {
            m_chooser.filter(f);
            return *this;
        }

        NativeFileChooser &directory(const char *d)
        {
            m_chooser.directory(d);
            return *this;
        }

        NativeFileChooser &preset_file(const char *f)
        {
            m_chooser.preset_file(f);
            return *this;
        }

        NativeFileChooser &options(int o)
        {
            m_chooser.options(o);
            return *this;
        }
        bool show()
        {
            return m_chooser.show() == 0;
        }

        const char *filename() const { return m_chooser.filename(); }
        const char *filename(int i) const { return m_chooser.filename(i); }
        int count() const { return m_chooser.count(); }
        const char *error_message() const { return m_chooser.errmsg(); }
    };
}
