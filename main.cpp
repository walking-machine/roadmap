#include <gl_sdl_utils.hpp>
#include <gl_sdl_2d.hpp>
#include <memory>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>
#include "shape_collections.hpp"
#include "interface.hpp"

static float aspect = 1.0f;

#define ARRAY_SIZE(_x) (sizeof(_x) / sizeof(*_x))

system_2d problem({{10.f, 10.f}, 10.f}, {{200.f, 200.f}, 10.f});
interface_bg interface;

/* Temporary variables */
float cur_pos[] = {0.f, 0.f};
bool use_cur = false;

int res_init()
{
    return init_2d();
}

#ifdef __EMSCRIPTEN__
    EM_JS(int, get_canvas_width, (), { return canvas.width; });
    EM_JS(int, get_canvas_height, (), { return canvas.height; });
#endif

int display()
{
    interface.init_drawing_space(problem.get_space_ptr());
    if (use_cur)
        problem.draw(cur_pos);
    else
        problem.draw(NULL);

    interface.draw();

    return 0;
}

static void reset_viewport_to_window(SDL_Window *window)
{
    int w, h;
#ifndef __EMSCRIPTEN__
    SDL_GL_GetDrawableSize(window, &w, &h);
    std::cout << "viewport: " << w << ":" << h << "\n";
#else
    w = get_canvas_width();
    h = get_canvas_height();
#endif
    aspect = (float)w / (float)h;
    glViewport(0, 0, w, h);
}

static bool handle_mouse(SDL_Event *event)
{
    if (interface.handle_mouse(event))
        return true;
    return problem.handle_mouse(event);
}

static bool handle_keyboard(SDL_Event *event)
{
    return false;
}

float circle_r;

int main(int, char**)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        std::cout << "SDL could not start, error: " << SDL_GetError() << "\n";
        return -1;
    }

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL |
                                                     SDL_WINDOW_SHOWN);

    SDL_Window* window = SDL_CreateWindow("Roadmap", SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED, 1280, 720,
                                          window_flags);

    auto gl_context = create_context(window);
    if (!gl_context) {
        std::cout << "SDL could not create a context, error: " << SDL_GetError() << "\n";
        return -1;
    }

    GLenum glew_ret = glewInit();
    if (glew_ret != GLEW_OK) {
        std::cout << "glew could not start, error: " << (unsigned long) glew_ret << "\n";
        return -1;
    }

    if (SDL_GL_MakeCurrent(window, gl_context)) {
        std::cout << "SDL could make context current, error: " << SDL_GetError() << "\n";
        return -1;
    }

#ifndef __EMSCRIPTEN__
    if (SDL_GL_SetSwapInterval(1))  /* Enable vsync */ {
        std::cout << "SDL could set swap interval, error: " << SDL_GetError() << "\n";
        return -1;
    }
#endif

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsLight();
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    const char *glsl_version = "#version 300 es";
    ImGui_ImplOpenGL3_Init(glsl_version);

    res_init();
    reset_viewport_to_window(window);

    bool done = false;
    while (!done)
    {
#ifdef __EMSCRIPTEN__
    emscripten_sleep(0);
#endif
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT &&
                event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID(window))
                done = true;

            if (!io.WantCaptureMouse)
                handle_mouse(&event);
            if (!io.WantCaptureKeyboard)
                handle_keyboard(&event);
            ImGui_ImplSDL2_ProcessEvent(&event);
        }

        glClearColor(1.f, 1.f, 1.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        int ret = display();
        if (ret)
            return ret;
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Geometry control");
        ImGui::DragFloat("R", &circle_r);
        if (ImGui::Button("Circle")) {
            problem.obstacles.add_one({{0, 0}, circle_r});
        }
        ImGui::Text("Current Position");
        ImGui::DragFloat2("Position", cur_pos, 1.f, 0.0f, 200.f);
        ImGui::Checkbox("Use Position", &use_cur);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}