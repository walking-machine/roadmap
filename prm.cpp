#include "prm.hpp"

bool s_prm::continue_map_internal(graph *cur_set)
{
    /* generate vertices */
    if (!cur_set->get_num_verts()) {
        uint q_size = cur_set->q_size;
        float *data = new float[q_size];

        for (uint i = 0; i < n; i++) {
try_again:
            for (uint j = 0; j < q_size; j++)
                data[j] = ranges[j](generator);
        
            if (!sys->valid_cfg(data))
                goto try_again;

            cur_set->add_vertice(data);
        }

        delete data;
        return true;
    }

    if (internal_cnt >= n)
        return false;

    uint next_neigh = internal_cnt + 1;

    while (next_neigh < n) {
        uint neigh = get_next_in_radius(cur_set, r * r, next_neigh,
                                        cur_set->get_vertice(internal_cnt));
        next_neigh = neigh + 1;
        if (neigh < n)
            if (sys->valid_cfg_seq(cur_set->get_vertice(internal_cnt),
                                   cur_set->get_vertice(neigh)))
                cur_set->add_edge(internal_cnt, neigh);
    }

    return ++internal_cnt < n;
}

graph *s_prm::init_algo_internal(system_nd *new_sys)
{
    uint q_size = new_sys->get_q_size();
    float *dims = new_sys->get_dims();
    ranges.clear();

    for (uint i = 0; i < q_size; i++)
        ranges.push_back(std::uniform_real_distribution<float>(0.f, dims[i]));

    generator.seed(seed);
    internal_cnt = 0;

    return nullptr;
}