#ifndef BOID_NODE_H
#define BOID_NODE_H

#include "scene/2d/node_2d.h"

class BoidNode2D : Node2D {
    GDCLASS(BoidNode2D, Node2D);

protected:
    static void _bind_methods() {

        //ClassDB::bind_method(D_METHOD());

    }

public:

    enum Parameter {

    };

    void update() {


    }

};

#endif //BOID_NODE_H