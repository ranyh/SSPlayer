#include "font_manager.h"

#include <sys/stat.h>

namespace playos {

FontManager &FontManager::instance()
{
    static FontManager manager;

    return manager;
}

std::shared_ptr<Font> FontManager::addFont(const std::string &name, const std::string &fontPath)
{
    struct stat st;
    if (stat(fontPath.c_str(), &st) != 0 || S_ISDIR(st.st_mode)) {
        m_mutex.unlock();
        printf("[ERROR] %s is not a file\n", fontPath.c_str());
        return nullptr;
    }

    auto font = std::make_shared<Font>(name, fontPath);
    if (font->isFont()) {
        addFont(name, font);
        return font;
    } else {
        printf("[ERROR] file %s is not a font file\n", fontPath.c_str());
    }

    return nullptr;
}

void FontManager::addFont(const std::string &name, std::shared_ptr<Font> font)
{
    m_mutex.lock();
    m_fonts[name] = font;
    m_mutex.unlock();
}

std::shared_ptr<Font> FontManager::getFont(const std::string &name)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_fonts[name];
}

}
