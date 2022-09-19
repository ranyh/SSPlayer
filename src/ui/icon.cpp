#include "icon.h"

#define ICON_MAP_FILE "/font/icon-name.txt"

#include <fstream>
#include <iostream>

namespace playos {

std::unordered_map<std::string, uint32_t> Icon::names;
std::once_flag Icon::load;

Icon::Icon(Element *parent, const std::string &name):
        Text(parent, name),
        m_name(name)
{
    setAlignment(Text::AlignCenter);
    setForceVCenter(true);
    setFont(FontManager::instance().getFont("mdi"));
}

void Icon::init(const std::string &resourceDir)
{
    std::call_once(load, [&]() {
        std::ifstream ifs;
        std::string line;

        ifs.open(resourceDir + ICON_MAP_FILE, std::ios_base::in);
        while (!ifs.eof()) {
            ifs >> line;

            int index = line.find(':');
            if (index != line.npos)
                names[line.substr(0, index)] = std::strtol(line.substr(index+1).c_str(), nullptr, 16);
        }
    });
}

std::vector<Text::Char> Icon::createChars()
{
    auto buffer = font()->getWord(names[text()]);

    if (!buffer) {
        return {};
    }

    std::vector<Text::Char> ret;
    ret.push_back(std::move(createChar(buffer)));

    return ret;
}

}
