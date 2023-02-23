#include "shape_collections.hpp"
#include "imgui.h"

void show_private_params(system_nd *system)
{
    private_param_info<float> *info_f;
    float **params_f;
    uint num_f;
    
    private_param_info<int> *info_i;
    int **params_i;
    uint num_i;

    num_f = system->get_params_float(&params_f, &info_f);
    num_i = system->get_params_int(&params_i, &info_i);

    if (!num_f && !num_i)
        return;

    ImGui::Begin("System parameters");
    for (uint i = 0; i < num_f; i++) {
        float speed = info_f[i].range.max - info_f[i].range.min;
        speed /= 40.f;
        ImGui::DragFloat(info_f[i].name.c_str(), params_f[i], speed,
                         info_f[i].range.min, info_f[i].range.max);
    }

    for (uint i = 0; i < num_i; i++) {
        float speed = (float)info_f[i].range.max - (float)info_f[i].range.min;
        speed /= 40.f;
        ImGui::DragInt(info_i[i].name.c_str(), params_i[i], speed,
                       info_i[i].range.min, info_i[i].range.max);
    }
    ImGui::End();
    
}