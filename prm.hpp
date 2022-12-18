#ifndef PRM_H
#define PRM_H
#include "shape_collections.hpp"
#include <random>

class s_prm : public algorithm {
protected:
    uint n;
    std::mt19937 generator;
    std::vector<std::uniform_real_distribution<float>> ranges;
    virtual bool continue_map_internal(graph *cur_set) override;
    virtual graph *init_algo_internal(system_nd *new_sys) override;
    uint seed;
    uint internal_cnt;
public:
    void set_num_points(uint num_points) { n = num_points; }
    uint get_num_points() { return n; }
    float r;
    s_prm(uint num_points, float connection_r,
          uint gen_seed = 1) : n(num_points), generator(gen_seed),
                               seed(gen_seed), internal_cnt(0),
                               r(connection_r) {}
};

#endif