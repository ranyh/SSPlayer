#ifndef __PLAYOS_LAYER_H__
#define __PLAYOS_LAYER_H__

#include "group.h"

namespace playos {

class Layer: public Group {
public:
    Layer(int x, int y, int width, int height, Element *parent = nullptr);
};

}

#endif
