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

        ADD_GROUP("BoundaryBehaviour", "");
        ADD_PROPERTYI(PropertyInfo(Variant::REAL, "range"), "set_param", "get_param", PARAM_BOUNDARY_RANGE);
        ADD_PROPERTYI(PropertyInfo(Variant::REAL, "force"), "set_param", "get_param", PARAM_BOUNDARY_FORCE);

        BIND_ENUM_CONSTANT(PARAM_BOUNDARY_RANGE);
        BIND_ENUM_CONSTANT(PARAM_BOUNDARY_FORCE);
        BIND_ENUM_CONSTANT(PARAM_MAX);
}