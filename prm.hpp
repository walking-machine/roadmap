#ifndef PRM_H
#define PRM_H
#include "algorithm.hpp"

class s_prm : public algorithm {
protected:
    uint n;
    virtual bool continue_map_internal(graph *cur_set) override;
    virtual graph *init_algo_internal(system_nd *new_sys) override;
    uint internal_cnt;
public:
    void set_num_points(uint num_points) { n = num_points; }
    uint get_num_points() { return n; }
    virtual float get_connection_radius(system_nd *sys) override;
    float r_multi;
    float base_r;
    s_prm(uint num_points, float r_multi) : n(num_points),
                                             internal_cnt(0),
                                             r_multi(r_multi) {}
    s_prm(uint num_points, float r_multi,
          sampler *generator) : algorithm(generator), n(num_points),
                                internal_cnt(0), r_multi(r_multi) {}
};

#endif