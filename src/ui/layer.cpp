#include "layer.h"

#include <assert.h>


namespace playos {

Layer::Layer(int x, int y, int width, int height, Element *parent):
    Group(parent)
{
    setGeometry(x, y, width, height);
}

}