#ifndef __PLAYOS_IMAGE_H__
#define __PLAYOS_IMAGE_H__

#include <string>

#include "element.h"

namespace playos {

class Image: public Element {
private:
    std::string m_uri;
public:
    Image(const std::string &uri);

    void update(float dt) override;
    void draw() override;
};

}

#endif
