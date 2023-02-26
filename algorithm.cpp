#include "algorithm.hpp"

bool algorithm::continue_map(graph *cur_set) {
    if (!sys)
        return false;
    return continue_map_internal(cur_set);
}

graph *algorithm::init_algo(system_nd *new_sys) {
    sys = new_sys;
    uint q_size = new_sys->get_q_size();
    float *dims_low = new_sys->get_dims_low();
    float *dims_high = new_sys->get_dims_high();
    ranges.clear();

    for (uint i = 0; i < q_size; i++)
        ranges.push_back(std::uniform_real_distribution<float>(dims_low[i], dims_high[i]));

    generator->seed(seed);
    graph *g = init_algo_internal(new_sys);
    return g ? g : new graph(sys->get_q_size());
}