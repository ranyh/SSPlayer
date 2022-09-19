#ifndef __PLAYOS_ICON_H__
#define __PLAYOS_ICON_H__

#include "text.h"


namespace playos {

class Icon: public Text {
public:
    Icon(Element *parent, const std::string &name);
    static void init(const std::string &resourceDir);

    void setSize(int size) {
        setFontSize(size);
        Element::setSize(size, size);
    }

    int getSize() {
        return getFontSize();
    }

protected:
    std::vector<Text::Char> createChars() override;

private:
    std::string m_name;

    static std::unordered_map<std::string, uint32_t> names;
    static std::once_flag load;
};

}

#endif
