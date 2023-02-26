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
    float r;
    s_prm(uint num_points, float lebesgue) : n(num_points),
                                             internal_cnt(0),
                                             r(lebesgue) {}
    s_prm(uint num_points, float lebesgue,
          sampler *generator) : algorithm(generator), n(num_points),
                                internal_cnt(0), r(lebesgue) {}
};

#endif