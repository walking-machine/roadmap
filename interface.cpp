#include "interface.hpp"
#include <gl_sdl_utils.hpp>

void interface_bg::calc_dims()
{
    float bw = 0.05f;
    float sw = 0.25f;

    float h = get_h_to_w_aspect();
    float w = 1.f;

    rect space_rect = { 0.f, 0.f, 1.f, h };
    use_rectangle(&space, &space_rect, 1.f);

    rects[0] = { 0.f, 0.f, w, bw };
    rects[1] = { 0.f, 0.f, bw, h };
    rects[2] = { 0.f, h - bw, w, bw };
    rects[3] = { w - sw, 0.f, sw, h };

    drawing_zone = { bw, bw, w - sw - bw, h - 2 * bw };
}

void interface_bg::draw()
{
    calc_dims();
    start_2d(&space);
    point zero_offset = {0.f, 0.f};
    set_offset(&zero_offset);
    set_rot_angle(0);
    color light_gray = { 192, 192, 201, 255 };
    color nice_blue = { 0, 114, 206, 255 };

    set_draw_color(&light_gray);
    for (uint i = 0; i < 4; i++)
        draw_rect(&rects[i]);

    set_line_width(3.0f);
    set_draw_color(&nice_blue);
    draw_rect_border(&drawing_zone);
}

bool interface_bg::handle_mouse(SDL_Event *event)
{
    if (event->type != SDL_MOUSEBUTTONDOWN &&
        event->type != SDL_MOUSEBUTTONUP &&
        event->type != SDL_MOUSEMOTION)
        return false;

    calc_dims();

    point sdl_point = { (float)event->motion.x, (float)event->motion.y };
    SDL_Window *window = SDL_GetWindowFromID(event->motion.windowID);
    point screen_point = sdl_point_to_space_2d(window, &space, sdl_point);
    
    return !point_in_rect(screen_point, &drawing_zone);
}

void interface_bg::init_drawing_space(space_2d *draw_space)
{
    calc_dims();
    use_rectangle(draw_space, &drawing_zone, draw_space->w_int);
}
