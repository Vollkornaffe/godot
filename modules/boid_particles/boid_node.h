#ifndef BOID_NODE_H
#define BOID_NODE_H

#include "scene/2d/node_2d.h"

class BoidNode : public Node2D {
    GDCLASS(BoidNode, Node2D);

protected:
    static void _bind_methods() {

    }

public:

    enum Parameter {
        PARAM_MAX,
    };

};

#endif //BOID_NODE_H