#include "shape_collections.hpp"

using namespace std;

static point calc_next_pos(float *angle_so_far, float angle,
                           point prev_point, float link_len)
{
    point result = prev_point;

    *angle_so_far += angle;
    result.x += link_len * cosf(*angle_so_far);
    result.y += link_len * sinf(*angle_so_far);

    return result;
}

static unique_ptr<line> get_all_links(point start, float *angles,
                                      float *link_lens, uint num_links)
{
    line *links = new line[num_links];
    point prev = start;
    float angle = 0.f;

    for (uint i = 0; i < num_links; i++) {
        links[i].start = prev;
        links[i].end = calc_next_pos(&angle, angles[i], prev, link_lens[i]);
        prev = links[i].end;
    }

    return unique_ptr<line>(links);
}

bool system_planar_arm::is_line_allowed(line l)
{
    uint obsts_num = obstacles.get_num_circles();

    for (uint j = 0; j < obsts_num; j++) {
        circle obstacle = obstacles.get_circle(j)->get_data();
        if (intersect(&obstacle, &l))
                return false;
    }

    return true;
}

bool system_planar_arm::valid_cfg_internal(float *cfg_coords)
{
    unique_ptr<line> links = get_all_links(root, cfg_coords, link_len.get(),
                                           num_links);
    uint obsts_num = obstacles.get_num_circles();
    obstacles.apply_transforms();
    
    for (uint j = 0; j < obsts_num; j++) {
        circle obstacle = obstacles.get_circle(j)->get_data();
        for (uint i = 0; i < num_links; i++)
            if (intersect(&obstacle, links.get() + i))
                return false;
    }

    return true;
}

bool system_planar_arm::valid_cfg_seq_internal(float *cfg_1, float *cfg_2)
{
    uint num_inter_points = num_links;
    unique_ptr<line> links_1 = get_all_links(root, cfg_1, link_len.get(),
                                             num_links);

    auto incrs = unique_ptr<float>(new float[num_links]);
    for (uint i = 0; i < num_links; i++)
        incrs.get()[i] = (cfg_2[i] - cfg_1[i]) / ((float)num_inter_points + 1);

    auto cfg_prev = unique_ptr<float>(new float[num_links]);
    memcpy(cfg_prev.get(), cfg_1, num_links * sizeof(float));
    point p_prev = links_1.get()[num_links - 1].end;

    for (uint i = 0; i <= num_inter_points; i++) {
        auto cfg_cur = unique_ptr<float>(new float[num_links]);
        for (uint j = 0; j < num_links; j++)
            cfg_cur.get()[j] = cfg_prev.get()[j] + incrs.get()[j];
        
        unique_ptr<line> links = get_all_links(root, cfg_cur.get(),
                                               link_len.get(), num_links);
        point p_cur = links.get()[num_links - 1].end;
        if (!is_line_allowed({ p_prev, p_cur }))
            return false;
        
        cfg_prev.swap(cfg_cur);
        p_prev = p_cur;
    }

    return true;
}

static color robot_red = { 180, 0, 0 };
static color robot_green = { 34, 139, 34 };
static point zero_offset = { 0.f, 0.f };
const static float circle_rad = 5.f;

void system_planar_arm::pre_draw(float *q_vec)
{
    gfx_mgr_init();

    if (!q_vec && start.get() && finish.get()) {
        pre_draw(start.get());
        pre_draw(finish.get());
        return;
    }

    unique_ptr<line> links = get_all_links(root, q_vec, link_len.get(),
                                           num_links);

    if (!valid_cfg_internal(q_vec))
        set_draw_color(&robot_red);
    else
        set_draw_color(&robot_green);
    
    set_line_width(3.f);
    set_rot_angle(0);
    start_2d(gfx_mgr.get_space_ptr());

    circle node = { { 0.f, 0.f }, circle_rad };

    if (q_vec == start.get() || q_vec == finish.get())
        goto lines;


    set_offset(&root);
    draw_circle(&node);

    for (uint i = 0; i < num_links; i++) {
        set_offset(&links.get()[i].end);
        draw_circle(&node);
    }

lines:
    set_offset(&zero_offset);
    for (uint i = 0; i < num_links; i++)
        draw_line(links.get() + i);
}

bool system_planar_arm::event_handled_internally(SDL_Event *event)
{
    return false;
}

static float angle_from_vector(point p1, point p2)
{
    vect v = { p2.x - p1.x, p2.y - p1.y };
    return atan2f(v.y, v.x);
}

static void recalculate_angles(float *angles, shape_circle *nodes,
                               uint num_links, point start)
{
    point prev_point = start;
    float prev_angle = 0.f;

    for (uint i = 0; i < num_links; i++) {
        float angle = angle_from_vector(prev_point, nodes[i].get_offset());
        angles[i] = angle - prev_angle;
        prev_angle = angle;
        prev_point = nodes[i].get_offset();
    }
}

void system_planar_arm::correct_moved_objects()
{
    recalculate_angles(start.get(), start_shape.get(), num_links, root);
    recalculate_angles(finish.get(), finish_shape.get(), num_links, root);

    gfx_mgr_init();
}

void system_planar_arm::init()
{
    link_len.reset(new float[num_links]);
    start.reset(new float[num_links]);
    finish.reset(new float[num_links]);
    limits_low.reset(new float[num_links]);
    limits_high.reset(new float[num_links]);
    params_float.reserve(num_links);
    info_float.reserve(num_links);

    for (uint i = 0; i < num_links; i++) {
        start.get()[i] = 0.f;
        finish.get()[i] = M_PI_4;
        limits_low.get()[i] = -M_PI_2;
        limits_high.get()[i] = M_PI_2;

        private_param_info<float> info { {0.1f, 200.0f},
                                         "Link " + std::to_string(i + 1) };
        params_float.push_back(link_len.get() + i);
        info_float.push_back(info);
    }

    start_shape.reset(new shape_circle[num_links]);
    finish_shape.reset(new shape_circle[num_links]);

    gfx_mgr.reserve_more_space(2 * num_links);

    for (uint i = 0; i < num_links; i++) {
        gfx_mgr.add_entity(&start_shape.get()[i]);
        gfx_mgr.add_entity(&finish_shape.get()[i]);
    }

    obstacles.gfx_mgr = &gfx_mgr;
    rect r {0.1f, 0.1f, 0.64f, 0.36f};
    use_rectangle(gfx_mgr.get_space_ptr(), &r, w);
}

void system_planar_arm::gfx_mgr_init()
{
    unique_ptr<line> start_links = get_all_links(root, start.get(), link_len.get(),
                                                 num_links);
    unique_ptr<line> finish_links = get_all_links(root, finish.get(),
                                                  link_len.get(), num_links);

    for (uint i = 0; i < num_links; i++) {
        start_shape.get()[i].set_origin(start_links.get()[i].end);
        finish_shape.get()[i].set_origin(finish_links.get()[i].end);
    }
}

system_planar_arm::system_planar_arm(uint num_links, float universal_link_len)
{
    this->num_links = num_links;
    init();
    for (uint i = 0; i < num_links; i++)
        link_len.get()[i] = universal_link_len;
    gfx_mgr_init();
}

system_planar_arm::system_planar_arm(uint num_links, float *links_length)
{
    this->num_links = num_links;
    init();
    for (uint i = 0; i < num_links; i++)
        link_len.get()[i] = links_length[i];
    gfx_mgr_init();
}

system_planar_arm::system_planar_arm(ifstream &file)
{
    file >> num_links;
    init();
    for (uint i = 0; i < num_links; i++)
        file >> link_len.get()[i];
    for (uint i = 0; i < num_links; i++)
        file >> start.get()[i];
    for (uint i = 0; i < num_links; i++)
        file >> finish.get()[i];
    gfx_mgr_init();
}

static void write_row(ofstream &file, unique_ptr<float> &data, uint n)
{
    for (uint i = 0; i < n; i++)
        file << data.get()[i] << (i == n - 1 ? "\n" : " ");
}

void system_planar_arm::save_tool(ofstream &file)
{
    file << num_links << "\n";
    write_row(file, link_len, num_links);
    write_row(file, start, num_links);
    write_row(file, finish, num_links);
}

uint system_planar_arm::
get_params_float(float ***params, private_param_info<float> **info)
{
    *params = params_float.data();
    *info = info_float.data();

    return params_float.size();
}
uint system_planar_arm::
get_params_int(int ***params, private_param_info<int> **info)
{
    *params = params_int.data();
    *info = info_int.data();

    return params_int.size();
}