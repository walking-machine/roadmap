#ifndef ALGO_UTILS_H
#define ALGO_UTILS_H

static float unit_ball_volume(uint dim)
{
    if (dim == 0)
        return 1.f;

    if (dim == 1)
        return 2.f;

    return (2.f * M_PI / (float)dim) * unit_ball_volume(dim - 2);
}

#endif