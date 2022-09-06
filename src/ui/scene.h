#ifndef __PLAYOS_SCENE_H__
#define __PLAYOS_SCENE_H__

#include "layer.h"

#include <vector>
#include <mutex>


namespace playos {

class Scene: public Layer {
public:
    Scene();
    virtual ~Scene();
    void init(UIContext *context, int width, int height);

protected:
    virtual void onInit(UIContext *context) = 0;

private:
    std::once_flag m_initFlag;
};

}

#endif
