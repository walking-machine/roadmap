#ifndef INTERFACE_H
#define INTERFACE_H

#include <gl_sdl_shape_obj.hpp>

class interface_bg {
private:
    rect rects[4];
    void calc_dims();
    space_2d space;
    rect drawing_zone;
public:
    void draw();
    void init_drawing_space(space_2d *draw_space);
    bool handle_mouse(SDL_Event *event);
    interface_bg() {
        calc_dims();
    }
};

#endif
