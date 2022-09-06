#ifndef __PLAYOS_FONT_MANAGER_H__
#define __PLAYOS_FONT_MANAGER_H__

#include <memory>
#include <unordered_map>
#include <mutex>

#include "font.h"


namespace playos {

class FontManager {
private:
    FontManager() { };
    FontManager(const FontManager &) = delete;
    FontManager(FontManager &&) = delete;
    FontManager operator=(const FontManager &) = delete;

public:
    static FontManager &instance();

    std::shared_ptr<Font> addFont(const std::string &name, const std::string &fontPath);
    void addFont(const std::string &name, std::shared_ptr<Font> font);
    std::shared_ptr<Font> getFont(const std::string &name);

private:
    std::unordered_map<std::string, std::shared_ptr<Font>> m_fonts;
    std::mutex m_mutex;
    FT_Library library;
};

}

#endif
