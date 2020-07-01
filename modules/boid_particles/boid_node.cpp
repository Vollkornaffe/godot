#include "boid_node.h"

void BoidNode::set_param(Parameter p_param, float p_value) {
    parameters[p_param] = p_value;
}
float BoidNode::get_param(Parameter p_param) const {
    return parameters[p_param];
}

void BoidNode::_bind_methods() {

        ClassDB::bind_method(D_METHOD("init_particles"), &BoidNode::init_particles);
        ClassDB::bind_method(D_METHOD("step"), &BoidNode::step);
        ClassDB::bind_method(D_METHOD("set_param", "param", "value"), &BoidNode::set_param);
        ClassDB::bind_method(D_METHOD("get_param", "param"), &BoidNode::get_param);

        ADD_GROUP("Boundary", "");
        ADD_PROPERTYI(PropertyInfo(Variant::REAL, "boundary_range"), "set_param", "get_param", PARAM_BOUNDARY_RANGE);
        ADD_PROPERTYI(PropertyInfo(Variant::REAL, "boundary_force"), "set_param", "get_param", PARAM_BOUNDARY_FORCE);

        ADD_GROUP("Boid", "");
        ADD_PROPERTYI(PropertyInfo(Variant::REAL, "boid_detection_range"), "set_param", "get_param", PARAM_BOID_DETECTION_RANGE);
        ADD_PROPERTYI(PropertyInfo(Variant::REAL, "boid_detection_angle"), "set_param", "get_param", PARAM_BOID_DETECTION_ANGLE);
        ADD_PROPERTYI(PropertyInfo(Variant::REAL, "boid_avoiding"), "set_param", "get_param", PARAM_BOID_AVOIDING);
        ADD_PROPERTYI(PropertyInfo(Variant::REAL, "boid_aligning"), "set_param", "get_param", PARAM_BOID_ALIGNING);
        ADD_PROPERTYI(PropertyInfo(Variant::REAL, "boid_clumping"), "set_param", "get_param", PARAM_BOID_CLUMPING);

        ADD_GROUP("Other", "");
        ADD_PROPERTYI(PropertyInfo(Variant::REAL, "linear_drag"), "set_param", "get_param", PARAM_LINEAR_DRAG);
}