#include "shape_collections.hpp"
#include <iostream>

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
        cur.set_fill_in(false);
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
    /* TODO : implement */
    return false;
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

    start = shape_circle(c1);
    finish = shape_circle(c2);
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

void draw_2d_graph(space_2d *space, graph &g)
{
    start_2d(space);
    if (g.q_size != 2)
        return;

    set_line_width(3.f);
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
        float x = g.vertice_data[i * 2];
        float y = g.vertice_data[i * 2 + 1];
        circle vert = {{x, y}, 4.f};
        draw_circle(&vert);
    }
}
