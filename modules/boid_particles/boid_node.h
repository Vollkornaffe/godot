#ifndef BOID_NODE_H
#define BOID_NODE_H

#include "scene/2d/node_2d.h"

class BoidNode : public Node2D {
    GDCLASS(BoidNode, Node2D);

protected:
    static void _bind_methods() {

        ClassDB::bind_method(D_METHOD("update"), &BoidNode::update);

    }

public:

    enum Parameter {
        PARAM_MAX,
    };

    void update() {


    }

};

#endif //BOID_NODE_H