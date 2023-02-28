#include "prm.hpp"
#include "algo_utils.h"

static float calc_base_radius(float lebesgue, uint dim)
{
    float dim_inv = 1.f / (float)dim;

    return 2.f * powf(1.f + dim_inv, dim_inv) *
           powf(lebesgue / unit_ball_volume(dim), dim_inv);
}

bool s_prm::continue_map_internal(graph *cur_set)
{
    float r = r_multi * base_r;
    /* generate vertices */
    if (!cur_set->get_num_verts()) {
        uint q_size = cur_set->q_size;
        float *data = new float[q_size];

        for (uint i = 0; i < n; i++) {
try_again:
            for (uint j = 0; j < q_size; j++)
                data[j] = generator->generate(ranges[j]);
        
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
    internal_cnt = 0;
    base_r = calc_base_radius(new_sys->get_lebesgue(), new_sys->get_q_size());

    return nullptr;
}

float s_prm::get_connection_radius(system_nd *sys)
{
    return calc_base_radius(sys->get_lebesgue(), sys->get_q_size());
}