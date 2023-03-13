#ifndef PRIVATE_PARAMS_H
#define PRIVATE_PARAMS_H

#include <string>

template <class T>
struct range_1d {
    T min;
    T max;
};

template <class T>
struct private_param_info {
    range_1d<T> range;
    std::string name;
};

class private_params_provider {
public:
    virtual uint get_params_float(float ***params,
                                  private_param_info<float> **info)
        { return 0; }
    virtual uint get_params_int(int ***params,
                                private_param_info<int> **info)
        { return 0; }
};

void show_private_params(private_params_provider *system);

#endif