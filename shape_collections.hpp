#ifndef SHAPE_COLLECTIONS_H
#define SHAPE_COLLECTIONS_H

#include <gl_sdl_shape_obj.hpp>
#include <memory>
#include <vector>
#include <fstream>
#include "private_params.hpp"

class shape_manager {
private:
    space_2d space;
    std::vector<shape *> shapes;
    shape_manager_state<shape *> mgr_state;
public:
    bool handle_mouse(SDL_Event *event);
    void draw();
    void reserve_more_space(uint num_entities);
    void add_entity(shape *shape_to_push);
    space_2d *get_space_ptr() {return &space;}
};

class obstacle_list {
public:
    obstacle_list() {}
    void add_one(circle c);
    void save_as(std::ofstream &file);
    void apply_transforms();    /* To apply before running algorithm */
    shape_circle *get_circle(uint idx);
    uint get_num_circles();
    bool intersects_with(shape *shape);
    shape_manager *gfx_mgr;
    void fill_from_file(std::ifstream &file);
private:
    std::vector<std::unique_ptr<shape_circle>> circles;
};

class system_nd : public private_params_provider {
private:
    uint num_called = 0;
    uint num_called_seq = 0;
protected:
    virtual bool valid_cfg_internal(float *cfg_coords) = 0;
    virtual void pre_draw(float *q_vec) = 0;
    virtual void save_tool(std::ofstream &file) = 0;
    virtual bool valid_cfg_seq_internal(float *cfg_1, float *cfg_2) { return true; }
    virtual bool event_handled_internally(SDL_Event *event) { return false; }
    virtual void correct_moved_objects() {}
    shape_manager gfx_mgr;
public:
    bool handle_mouse(SDL_Event *event);
    bool valid_cfg(float *cfg_coords) {
        num_called++;
        return valid_cfg_internal(cfg_coords);
    }

    bool valid_cfg_seq(float *cfg_1, float *cfg_2) {
        num_called_seq++;
        return valid_cfg_seq_internal(cfg_1, cfg_2);
    }

    space_2d *get_space_ptr() { return gfx_mgr.get_space_ptr(); }

    void reset_counter() { num_called = 0; }
    void draw(float *q_vec);
    void save(std::string system_name);
    obstacle_list obstacles;
    float w = 400.0f;
    float h = 225.0f;
    virtual uint get_q_size() = 0;
    virtual float *get_dims_low() = 0;
    virtual float *get_dims_high() = 0;
    virtual float *get_start() = 0;
    virtual float *get_finish() = 0;
    virtual float get_lebesgue();
};

#define DEFAULT_RADIUS 2.0f

class system_2d : public system_nd {
protected:
    virtual bool valid_cfg_internal(float *cfg_coords) override;
    virtual void pre_draw(float *q_vec) override;
    virtual void save_tool(std::ofstream &file) override;
    virtual bool valid_cfg_seq_internal(float *cfg_1, float *cfg_2) override;
    float dims[2] = {w, h};
    float dims_low[2] = { 0, 0 };
public:
    shape_circle start;
    shape_circle finish;
    shape_circle cur;
    system_2d(circle start_pos, circle end_pos);
    system_2d() : system_2d({{0.f, 0.f}, DEFAULT_RADIUS},
                            {{0.f, 0.f}, DEFAULT_RADIUS}) {}
    system_2d(std::ifstream &file);
    virtual uint get_q_size() override;

    virtual float *get_dims_high() override { return dims; }
    virtual float *get_dims_low() override { return dims_low; }
    virtual float *get_start() override;
    virtual float *get_finish() override;
};

class system_planar_arm : public system_nd {
protected:
    virtual bool valid_cfg_internal(float *cfg_coords) override;
    virtual void pre_draw(float *q_vec) override; /* draw robot arm */
    virtual void save_tool(std::ofstream &file) override;
    virtual bool valid_cfg_seq_internal(float *cfg_1, float *cfg_2) override;
    virtual bool event_handled_internally(SDL_Event *event);
    virtual void correct_moved_objects();
    bool is_line_allowed(line l);

    uint num_links = 2;
    std::unique_ptr<float> link_len;
    std::unique_ptr<float> start;
    std::unique_ptr<float> finish;
    std::unique_ptr<float> limits_low;
    std::unique_ptr<float> limits_high;
    std::unique_ptr<shape_circle> start_shape;
    std::unique_ptr<shape_circle> finish_shape;
    point root = { 0.f, 0.f };

    std::vector<float *> params_float;
    std::vector<int *> params_int;
    std::vector<private_param_info<float>> info_float;
    std::vector<private_param_info<int>> info_int;
    void init();
    void gfx_mgr_init();

public:
    system_planar_arm(uint num_links, float universal_link_len = 60.f);
    system_planar_arm(uint num_links, float *links_length);
    system_planar_arm(std::ifstream &file);
    virtual uint get_q_size() override { return num_links; }
    virtual float *get_start() override { return start.get(); }
    virtual float *get_finish() override { return finish.get(); }
    virtual float *get_dims_high() override { return limits_high.get(); }
    virtual float *get_dims_low() override { return limits_low.get(); }
    virtual uint get_params_float(float ***params,
                                  private_param_info<float> **info) override;
    virtual uint get_params_int(int ***params,
                                private_param_info<int> **info) override;
};

system_nd *get_from_file(std::string path_name);

class graph {
public:
    uint q_size = 2;
    std::vector<std::vector<uint>> groups;  /* neighbors */
    std::vector<float> vertice_data;
    std::vector<uint> connected_components;
    float *get_vertice(uint idx);
    uint get_num_verts() { return groups.size(); }
    void add_vertice(float *data) {
        vertice_data.reserve(vertice_data.size() + q_size);
        for (uint i = 0; i < q_size; i++)
            vertice_data.push_back(data[i]);
        groups.push_back({});
        connected_components.push_back(connected_components.size());
    }

    void add_edge(uint id1, uint id2);
    bool same_component(uint id1, uint id2);

    graph() {}
    graph(uint config_size) : q_size(config_size) {}
};

void draw_2d_graph(space_2d *space, graph &g);
uint get_next_in_radius(graph *g, float r, uint start, float *ref);

std::vector<float> build_path(graph *g, system_nd *sys,
                              float *start, float *finish, float con_r_sq);
void draw_path(std::vector<float> &path);
void draw_pos(std::vector<float> &path, system_nd *sys, float percent);

#endif
