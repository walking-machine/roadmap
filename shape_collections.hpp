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
protected:
    virtual bool valid_cfg_internal(float *cfg_coords) = 0;
    virtual void pre_draw(float *q_vec) = 0;
    virtual void save_tool(std::ofstream &file) = 0;
    shape_manager gfx_mgr;
public:
    bool handle_mouse(SDL_Event *event) { return gfx_mgr.handle_mouse(event); }
    bool valid_cfg(float *cfg_coords) {
        num_called++;
        return valid_cfg_internal(cfg_coords);
    }

    space_2d *get_space_ptr() { return gfx_mgr.get_space_ptr(); }

    void reset_counter() { num_called = 0; }
    void draw(float *q_vec);
    void save(std::string system_name);
    obstacle_list obstacles;
    float w = 400.0f;
    float h = 225.0f;
    virtual uint get_q_size() = 0;
};

#define DEFAULT_RADIUS 2.0f

class system_2d : public system_nd {
protected:
    virtual bool valid_cfg_internal(float *cfg_coords) override;
    virtual void pre_draw(float *q_vec) override;
    virtual void save_tool(std::ofstream &file) override;
public:
    shape_circle start;
    shape_circle finish;
    shape_circle cur;
    system_2d(circle start_pos, circle end_pos);
    system_2d() : system_2d({{0.f,0.f}, DEFAULT_RADIUS},
                            {{0.f, 0.f}, DEFAULT_RADIUS}) {}
    system_2d(std::ifstream &file);
    virtual uint get_q_size() override;
};

system_nd *get_from_file(std::string path_name);

class graph {
public:
    uint q_size = 2;
    std::vector<std::vector<uint>> groups;  /* neighbors */
    std::vector<float> vertice_data;
    float *get_vertice(uint idx);
    uint get_num_verts() { return groups.size(); }
    void add_vertice(float *data) {
        vertice_data.reserve(vertice_data.size() + q_size);
        for (uint i = 0; i < q_size; i++)
            vertice_data.push_back(data[i]);
        groups.push_back({});
    }

    void add_edge(uint id1, uint id2) {
        groups[id1].push_back(id2);
        groups[id2].push_back(id1);
    }

    graph() {}
    graph(uint config_size) : q_size(config_size) {}
};

void draw_2d_graph(space_2d *space, graph &g);

class algorithm {
protected:
    uint closest_neighbor(float *q_point, graph *cur_set);  /* Do a naive search */
    system_nd *sys = NULL;
    virtual bool continue_map_internal(graph *cur_set) = 0;
public:
    bool continue_map(graph *cur_set) {
        if (!sys)
            return false;
        return continue_map_internal(cur_set);
    }

    graph *init_algo(system_nd *new_sys) {
        sys = new_sys;
        return new graph(sys->get_q_size());
    }
};

#endif
