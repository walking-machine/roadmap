#ifndef SHAPE_COLLECTIONS_H
#define SHAPE_COLLECTIONS_H

#include <gl_sdl_shape_obj.hpp>
#include <memory>
#include <vector>
#include <fstream>

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

class system_nd {
private:
    uint num_called = 0;
    uint num_called_seq = 0;
protected:
    virtual bool valid_cfg_internal(float *cfg_coords) = 0;
    virtual void pre_draw(float *q_vec) = 0;
    virtual void save_tool(std::ofstream &file) = 0;
    virtual bool valid_cfg_seq_internal(float *cfg_1, float *cfg_2) { return true; }
    shape_manager gfx_mgr;
public:
    bool handle_mouse(SDL_Event *event) { return gfx_mgr.handle_mouse(event); }
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
    virtual float *get_dims() = 0;
    virtual float *get_start() = 0;
    virtual float *get_finish() = 0;
};

#define DEFAULT_RADIUS 2.0f

class system_2d : public system_nd {
protected:
    virtual bool valid_cfg_internal(float *cfg_coords) override;
    virtual void pre_draw(float *q_vec) override;
    virtual void save_tool(std::ofstream &file) override;
    float dims[2] = {w, h};
public:
    shape_circle start;
    shape_circle finish;
    shape_circle cur;
    system_2d(circle start_pos, circle end_pos);
    system_2d() : system_2d({{0.f,0.f}, DEFAULT_RADIUS},
                            {{0.f, 0.f}, DEFAULT_RADIUS}) {}
    system_2d(std::ifstream &file);
    virtual uint get_q_size() override;

    virtual float *get_dims() override { return dims; }
    virtual float *get_start() override;
    virtual float *get_finish() override;
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
                              float *start, float *finish);
void draw_path(std::vector<float> &path);
void draw_pos(std::vector<float> &path, system_nd *sys, float percent);

class algorithm {
protected:
    uint closest_neighbor(float *q_point, graph *cur_set);  /* Do a naive search */
    system_nd *sys = NULL;
    virtual bool continue_map_internal(graph *cur_set) = 0;
    virtual graph *init_algo_internal(system_nd *new_sys) { return nullptr; }
public:
    bool continue_map(graph *cur_set) {
        if (!sys)
            return false;
        return continue_map_internal(cur_set);
    }

    graph *init_algo(system_nd *new_sys) {
        sys = new_sys;
        graph *g = init_algo_internal(new_sys);
        return g ? g : new graph(sys->get_q_size());
    }
};

#endif
