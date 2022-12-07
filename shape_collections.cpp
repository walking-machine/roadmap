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

void obstacle_list::save_as(std::string file_name)
{}

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

void draw_interface(system_2d *sys) {}

void system_nd::draw(float *q_vec)
{
    pre_draw(q_vec);
    gfx_mgr.draw();
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
    finish.move(end_pos.center);

    /* TODO : move to base class */
    rect r {0.1f, 0.1f, 0.64f, 0.36f};
    use_rectangle(gfx_mgr.get_space_ptr(), &r, w);
    obstacles.gfx_mgr = &gfx_mgr;
}

void system_2d::pre_draw(float *q_vec)
{
    if (q_vec) {
        cur.reset_transform();
        cur.move({q_vec[0], q_vec[1]});
        /* hide start and finish ? */
    } else {
        /* hide current position */
    }
}

bool system_2d::valid_cfg_internal(float *cfg_coords)
{
    /* TODO : implement */
    return false;
}
