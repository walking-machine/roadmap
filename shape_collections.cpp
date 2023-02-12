#include "shape_collections.hpp"
#include <limits>
#include <algorithm>
#include <queue>

bool shape_manager::handle_mouse(SDL_Event *event)
{
    if (event->type != SDL_MOUSEMOTION &&
        event->type != SDL_MOUSEBUTTONUP &&
        event->type != SDL_MOUSEBUTTONDOWN)
        return false;

    mgr_state.num_shapes = shapes.size();
    mgr_state.shapes = shapes.data();

    assign_random_colors<shape *>(&mgr_state);
    return try_drag_all_shapes<shape *>(event, &mgr_state, &space);
}

void shape_manager::draw()
{
    mgr_state.num_shapes = shapes.size();
    mgr_state.shapes = shapes.data();
    start_2d(&space);
    draw_all_shapes(&mgr_state);
}

void shape_manager::reserve_more_space(uint num_entities)
{
    shapes.reserve(shapes.size() + num_entities);
}

void shape_manager::add_entity(shape *shape_to_push)
{
    shapes.push_back(shape_to_push);
}

void obstacle_list::add_one(circle c)
{
    circles.push_back(std::unique_ptr<shape_circle>(new shape_circle(c.center, c.radius)));
    gfx_mgr->add_entity(circles.back().get());
}

static circle circle_from_file(std::ifstream &file)
{
    circle c;
    file >> c.center.x >> c.center.y >> c.radius;
    return c;
}

static void write_circle_to_file(circle &data, std::ofstream &file)
{
    file << data.center.x << " " << data.center.y << " " << data.radius << "\n";
}

void obstacle_list::save_as(std::ofstream &file)
{
    file << circles.size() << "\n";
    apply_transforms();

    for (auto &c : circles) {
        circle data = c->get_data();
        write_circle_to_file(data, file);
    }
}

void obstacle_list::apply_transforms()
{
    for (auto &c : circles)
        c->apply_transform();
}

shape_circle *obstacle_list::get_circle(uint idx)
{
    return circles[idx].get();
}

uint obstacle_list::get_num_circles()
{
    return circles.size();
}

bool obstacle_list::intersects_with(shape *shape)
{
    for (auto &c : circles) {
        if (c->intersects_with(shape))
            return true;
    }

    return false;
}

void obstacle_list::fill_from_file(std::ifstream &file)
{
    uint num_circles;

    file >> num_circles;
    circles.reserve(num_circles);

    for (uint i = 0; i < num_circles; i++) {
        circle c = circle_from_file(file);
        add_one(c);
    }
}

void system_nd::draw(float *q_vec)
{
    pre_draw(q_vec);
    gfx_mgr.draw();
}

void system_nd::save(std::string system_name)
{
    std::ofstream file(system_name);

    file << get_q_size() << "\n";
    save_tool(file);
    obstacles.save_as(file);
}

bool system_nd::handle_mouse(SDL_Event *event)
{
    bool handled = event_handled_internally(event);
    handled = handled ? true : gfx_mgr.handle_mouse(event);

    if (handled)
        correct_moved_objects();
    
    return handled;
}

system_2d::system_2d(circle start_pos, circle end_pos) :
                     start(start_pos.radius), finish(start_pos.radius),
                     cur(start_pos.radius)
{
    gfx_mgr.reserve_more_space(3);
    gfx_mgr.add_entity(&start);
    gfx_mgr.add_entity(&finish);
    gfx_mgr.add_entity(&cur);
    start.move(start_pos.center);
    cur.move(start_pos.center);
    finish.move(end_pos.center);

    /* TODO : move to base class */
    rect r {0.1f, 0.1f, 0.64f, 0.36f};
    use_rectangle(gfx_mgr.get_space_ptr(), &r, w);
    obstacles.gfx_mgr = &gfx_mgr;
}

void system_2d::pre_draw(float *q_vec)
{
    set_line_width(3.f);
    if (q_vec) {
        cur.reset_transform();
        cur.move({q_vec[0], q_vec[1]});
        cur.set_fill_in(valid_cfg(q_vec));
        cur.set_draw_border(true);
        start.set_enabled(false);
        finish.set_enabled(false);
        cur.set_enabled(true);
    } else {
        start.set_fill_in(false);
        finish.set_fill_in(false);
        start.set_draw_border(true);
        finish.set_draw_border(true);
        start.set_enabled(true);
        finish.set_enabled(true);
        cur.set_enabled(false);
    }
}

bool system_2d::valid_cfg_internal(float *cfg_coords)
{
    circle c = {{ cfg_coords[0], cfg_coords[1] }, start.get_data().radius };
    for (uint i = 0; i < obstacles.get_num_circles(); i++)
        if (obstacles.get_circle(i)->intersects_another_circle(&c))
            return false;

    return true;
}

bool system_2d::valid_cfg_seq_internal(float *cfg_1, float *cfg_2)
{
    float x1 = cfg_1[0];
    float x2 = cfg_2[0];
    float y1 = cfg_1[1];
    float y2 = cfg_2[1];

    float v_x = -(y2 - y1);
    float v_y = x2 - x1;

    float len = sqrtf(v_x * v_x + v_y * v_y);

    v_x = v_x * start.get_data().radius / len;
    v_y = v_y * start.get_data().radius / len;

    point p1 = { x1 - v_x, y1 - v_y };
    point p2 = { x1 + v_x, y1 + v_y };
    point p3 = { x2 - v_x, y2 - v_y };
    point p4 = { x2 + v_x, y2 + v_y };

    tri tri_1 = { p1, p2, p4 };
    tri tri_2 = { p2, p4, p3 };

    for (uint i = 0; i < obstacles.get_num_circles(); i++)
        if (obstacles.get_circle(i)->intersects_tri(&tri_1) ||
            obstacles.get_circle(i)->intersects_tri(&tri_2))
            return false;

    return true;
}

void system_2d::save_tool(std::ofstream &file)
{
    start.apply_transform();
    finish.apply_transform();
    circle c1 = start.get_data();
    circle c2 = finish.get_data();

    write_circle_to_file(c1, file);
    write_circle_to_file(c2, file);
}

uint system_2d::get_q_size() { return 2; }

system_2d::system_2d(std::ifstream &file) : system_2d()
{
    circle c1 = circle_from_file(file);
    circle c2 = circle_from_file(file);
    circle c3 = { { 0.f, 0.f }, c1.radius };

    start = shape_circle(c1);
    finish = shape_circle(c2);
    cur = shape_circle(c3);
}

float *system_2d::get_start()
{
    float *data = new float[2];
    start.apply_transform();
    circle c = start.get_data();
    data[0] = c.center.x;
    data[1] = c.center.y;
    return data;
}

float *system_2d::get_finish()
{
    float *data = new float[2];
    finish.apply_transform();
    circle c = finish.get_data();
    data[0] = c.center.x;
    data[1] = c.center.y;
    return data;
}

system_nd *get_from_file(std::string path_name)
{
    std::ifstream file(path_name);
    if (!file.is_open())
        return NULL;
    
    system_nd *sys_nd = NULL;
    
    uint num_dims;
    file >> num_dims;
    switch (num_dims) {
    case 2:
        sys_nd = new system_2d(file);
        break;
    case 3:
    case 4:
    case 5:
    case 6:
        sys_nd = new system_planar_arm(file);
        break;
    default:
        return NULL;
    }

    sys_nd->obstacles.fill_from_file(file);
    return sys_nd;
}

float *graph::get_vertice(uint idx)
{
    float *all_data = vertice_data.data();
    return all_data + idx * q_size;
}

static uint get_component(std::vector<uint> &ccs, uint id)
{
    uint root = id;
    while (ccs[root] != root)
        root = ccs[root];
    
    /* Reduce depth */
    while (ccs[id] != root) {
        uint next = id;
        ccs[id] = root;
        id = next;
    }

    return root;
}

void graph::add_edge(uint id1, uint id2)
{
    groups[id1].push_back(id2);
    groups[id2].push_back(id1);

    uint cc1 = get_component(connected_components, id1);
    uint cc2 = get_component(connected_components, id2);

    uint cc = cc1 < cc2 ? cc1 : cc2;
    connected_components[cc1] = cc;
    connected_components[cc2] = cc;
}

bool graph::same_component(uint id1, uint id2)
{
    return get_component(connected_components, id1) ==
           get_component(connected_components, id2);
}

void draw_2d_graph(space_2d *space, graph &g)
{
    start_2d(space);
    if (g.q_size != 2)
        return;

    set_line_width(2.f);
    color graph_color = { 20, 20, 20, 255};
    set_draw_color(&graph_color);
    point zero_offset = {0.f, 0.f};
    set_offset(&zero_offset);
    set_rot_angle(0);

    uint num_verts = g.groups.size();

    for (uint i = 0; i < num_verts; i++) {
        float *first_data = g.get_vertice(i);
        point p1 = { first_data[0], first_data[1] };

        for (auto j : g.groups[i]) {
            float *second_data = g.get_vertice(j);
            point p2 = { second_data[0], second_data[1] };
            line edge = { p1, p2 };
            draw_line(&edge);
        }
    }

    for (uint i = 0; i < num_verts; i++) {
        set_draw_color(&colors[get_component(g.connected_components, i) % 18]);
        float x = g.vertice_data[i * 2];
        float y = g.vertice_data[i * 2 + 1];
        circle vert = {{x, y}, 2.f};
        draw_circle(&vert);
    }
}

static float get_dist_sq_nd(float *v1, float *v2, uint size)
{
    float dist_sq = 0.f;

    for (uint j = 0; j < size; j++)
        dist_sq += powf(v1[j] - v2[j], 2.f);

    return dist_sq;
}

uint get_next_in_radius(graph *g, float r_sq, uint start, float *ref)
{
    uint n = g->get_num_verts();
    uint q_size = g->q_size;

    for (uint i = start; i < n; i++) {
        float *data = g->get_vertice(i);
        float dist_sq = 0.f;

        for (uint j = 0; j < q_size; j++)
            dist_sq += powf(data[j] - ref[j], 2.f);

        if (dist_sq <= r_sq)
            return i;
    }

    return n;
}

struct dijkstra_node {
    uint idx;
    uint through;
    float cost;
};

class dijkstra_greater {
public:
    const bool
    operator() (const dijkstra_node &node_1, const dijkstra_node &node_2)
    {
        return node_1.cost > node_2.cost;
    }
};

float get_graph_dist(graph *g, uint id1, uint id2)
{
    float *data_1 = g->get_vertice(id1);
    float *data_2 = g->get_vertice(id2);

    return get_dist_sq_nd(data_1, data_2, g->q_size);
}

static std::vector<uint> dijkstra_path(graph *g, uint start, uint finish)
{
    uint n = g->get_num_verts();
    dijkstra_node def = { n, n, std::numeric_limits<float>::max() };
    std::vector<dijkstra_node> cur_best_path(n, def);
    std::vector<bool> was_visited(g->get_num_verts(), false);
    std::priority_queue<dijkstra_node, std::vector<dijkstra_node>,
                        dijkstra_greater> verts_to_explore;

    verts_to_explore.push({start, start, 0.f});
    bool found = false;

    while (!verts_to_explore.empty()) {
        dijkstra_node node = verts_to_explore.top();
        verts_to_explore.pop();
        if (was_visited[node.idx])
            continue;

        for (uint id : g->groups[node.idx]) {
            dijkstra_node new_node = {id, node.idx,
                                      node.cost + get_graph_dist(g, node.idx, id)};
            if (was_visited[id] || new_node.cost >= cur_best_path[id].cost)
                continue;
            
            cur_best_path[id] = new_node;
            
            if (id == finish) {
                found = true;
                break;
            }

            verts_to_explore.push(new_node);
        }

        was_visited[node.idx] = true;
        if (found)
            break;
    }

    if (!found)
        return {};

    std::vector<uint> full_path;
    uint cur_id = finish;
    while (cur_id != start) {
        full_path.push_back(cur_id);
        cur_id = cur_best_path[cur_id].through;
    }

    full_path.push_back(start);

    std::reverse(full_path.begin(), full_path.end());

    return full_path;
}

struct vert_dist {
    uint vert_id;
    float dist;
};

bool comp_verts(vert_dist &vd1, vert_dist &vd2)
{
    return vd1.dist < vd2.dist;
}

std::vector<float> build_path(graph *g, system_nd *sys,
                              float *start, float *finish, float con_r_sq)
{
    uint n = g->get_num_verts();
    std::vector<vert_dist> vds_start(n);
    std::vector<vert_dist> vds_finish(n);

    for (uint i = 0; i < n; i++) {
        float *data = g->get_vertice(i);
        vds_start[i] = { i, get_dist_sq_nd(data, start, g->q_size) };
        vds_finish[i] = { i, get_dist_sq_nd(data, finish, g->q_size) };
    }

    std::sort(vds_start.begin(), vds_start.end(), comp_verts);
    std::sort(vds_finish.begin(), vds_finish.end(), comp_verts);

    bool found = false;
    uint start_neigh = 0;
    uint end_neigh = 0;

    for (uint i = 0; i < n; i++) {
        if (vds_start[i].dist > con_r_sq)
            break;

        start_neigh = vds_start[i].vert_id;
        found = false;
        if (!sys->valid_cfg_seq(start, g->get_vertice(start_neigh)))
            continue;

        for (uint j = 0; j < n; j++) {
            if (vds_finish[j].dist > con_r_sq)
                break;

            end_neigh = vds_finish[j].vert_id;

            if (!g->same_component(start_neigh, end_neigh))
                continue;
            if (!sys->valid_cfg_seq(finish, g->get_vertice(end_neigh)))
                continue;
            found = true;
            break;
        }

        if (found)
            break;
    }

    if (!found)
        return {};
    
    auto id_path = dijkstra_path(g, start_neigh, end_neigh);
    std::vector<float> path((id_path.size() + 2) * g->q_size);

    for (uint j = 0; j < g->q_size; j++) {
        path[j] = start[j];
        path[path.size() - g->q_size + j] = finish[j];
    }

    for (uint i = 0; i < id_path.size(); i++) {
        uint step = g->q_size;
        float *data = g->get_vertice(id_path[i]);

        for (uint j = 0; j < step; j++)
            path[step + i * step + j] = data[j];
    }

    return path;
}

void draw_path(std::vector<float> &path)
{
    uint num_verts = path.size() / 2;

    set_line_width(2.f);
    color path_color = { 208, 20, 20, 255};
    set_draw_color(&path_color);
    point zero_offset = {0.f, 0.f};
    set_offset(&zero_offset);
    set_rot_angle(0);

    for (uint i = 0; i < num_verts; i++) {
        point cur = { path[i * 2], path[i * 2 + 1] };

        if (i > 0) {
            point prev = { path[i * 2 - 2], path[i * 2 - 1] };
            line edge = {prev, cur};
            draw_line(&edge);
        }

        circle vert = {cur, 2.f};
        draw_circle(&vert);
    }
}

static std::vector<float> get_costs_norm(std::vector<float> &path, uint q_size)
{
    std::vector<float> costs(path.size() / q_size);
    float overall_cost = 0.f;

    for (uint i = 0; i < path.size() - q_size; i += q_size) {
        overall_cost += sqrtf(get_dist_sq_nd(path.data() + i,
                              path.data() + i + q_size, q_size));
        costs[i / q_size] = overall_cost;
    }

    for (float &cost : costs) {
        cost /= overall_cost;
    }

    return costs;
}

static void interpolate(float *p1, float *p2, float t, float *dest, uint q_size)
{
    for (uint j = 0; j < q_size; j++)
        dest[j] = p1[j] + t * (p2[j] - p1[j]);
}

void draw_pos(std::vector<float> &path, system_nd *sys, float percent)
{
    std::vector<float> costs = get_costs_norm(path, sys->get_q_size());
    uint start_id = costs.size() - 2;
    uint q_size = sys->get_q_size();

    for (uint i = 0; i <= costs.size() - 2; i++) {
        if (costs[i] > percent) {
            start_id = i;
            break;
        }
    }

    uint end_id = start_id + 1;
    float start_percent = start_id ? costs[start_id - 1] : 0.f;
    float end_percent = costs[start_id];

    float internal_percent = (percent - start_percent) /
                             (end_percent - start_percent);

    float *pos = new float[3];
    interpolate(path.data() + q_size * start_id, path.data() + q_size * end_id,
                internal_percent, pos, q_size);
    sys->draw(pos);

    delete pos;
}
