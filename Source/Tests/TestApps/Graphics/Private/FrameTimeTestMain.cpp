/*
 * This program was copied from:
 * https://discourse.libsdl.org/t/sdl2-0-6-stuttering-problem/23220/2
 *
 * Instructions:
 * The graph shows full frame time (green) and SDL_RenderPresent time (red).
 * The horizontal blue line is the 1/60 millisecond mark. You’d expect the green line to
 * be near that if vsync is enabled and the refresh rate is at 60 Hz. Additionally, in
 * its big version (toggle by pressing B), the graph shows event time (pink), update
 * time (white), clear time (yellow), draw time (light blue). Red, pink, white, yellow,
 * and light blue are stacked on top of each other and add up to the green line.
 *
 * Other controls:
 *
 * Space: Toggle pause
 * 1: Toggle frame graph line
 * 2: Toggle event graph line
 * 3: Toggle update graph line
 * 4: Toggle clear graph line
 * 5: Toggle draw graph line
 * 6: Toggle swap graph line
 * i: Toggle interpolation of square (like your sample code does)
 * r: Add random delay after updating square position
 * p: Toggle between millisecond and microsecond counters
 * b: Toggle graph size
 * Escape: Quit
 *
 * Try setting the environment variable SDL_RENDER_VSYNC to 1 to force vsync on. You can
 * also try the other renderers with the environment variable SDL_RENDER_DRIVER.
 * Possible values are: opengl, opengles, opengles2, direct3d, direct3d11, software.
 * (Depends on the platform and if SDL was built with that renderer enabled.)
 */

#include <iostream>
#include <string>
#include <deque>
#include <cmath>
#include <algorithm>
#include <vector>

#include "Ignore.h"

#include "SDL.h"

#define FRAMETIME_COUNT 200

class Player {
public:
    Player();

    void update(const double tick_rate);
    void draw(SDL_Renderer * renderer, const double alpha, int width, int height);

    double x, y, px, py, vx, vy, ax, ay;
};

struct FrameTime {
    double full, event, update, clear, draw, swap;
};

class Window {
public:
    Window();

    void init();
    void quit();

    void run();
    void drawgraph();
    void settitle();

    void clear();
    void swap();

    SDL_Window * handle;
    SDL_Renderer * renderer;
    int width, height;
    bool pause;
    bool interpolate;
    bool highprecision;
    bool randomdelay;
    bool biggraph;
    bool showfullline;
    bool showeventline;
    bool showupdateline;
    bool showclearline;
    bool showdrawline;
    bool showswapline;
    Player player;

    std::deque<FrameTime> frametime;
    std::vector<SDL_Point> full_line;
    std::vector<SDL_Point> event_line;
    std::vector<SDL_Point> update_line;
    std::vector<SDL_Point> clear_line;
    std::vector<SDL_Point> draw_line;
    std::vector<SDL_Point> swap_line;
};

static Uint32 xstate = 905309021;
static Uint32 xorshift() {
    xstate ^= xstate << 13;
    xstate ^= xstate >> 17;
    xstate ^= xstate << 5;
    return xstate;
}

Player::Player() : x(0.1), y(0.9), vx(0.35), vy(0), ax(0), ay(-0.5)
{}

void Player::update(const double tick_rate) {
    px = x;
    py = y;

    vx += ax * tick_rate;
    vy += ay * tick_rate;

    x += vx * tick_rate;
    y += vy * tick_rate;

    if (x < 0 || x > 1) {
        x = x < 0 ? -x : x - (x - 1);
        vx = -vx;
    }
    if (y < 0 || y > 1) {
        y = y < 0 ? -y : y - (y - 1);
        vy = -vy;
    }
}

void Player::draw(SDL_Renderer * renderer, const double alpha, int width, int height) {
    const double interp_x = x * alpha + px * (1.0 - alpha);
    const double interp_y =  1 - (y * alpha + py * (1.0 - alpha));
    const int int_x = static_cast<int>(floor(interp_x * (width - 20) + 0.5));
    const int int_y = static_cast<int>(floor(interp_y * (height - 20) + 0.5));
    const SDL_Rect draw_rect = {int_x, int_y, 20, 20};

    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    SDL_RenderFillRect(renderer, &draw_rect);
}

Window::Window() : handle(0), renderer(0), pause(false), interpolate(true), highprecision(false), randomdelay(false), biggraph(false),
    showfullline(true), showeventline(true), showupdateline(true), showclearline(true), showdrawline(true), showswapline(true),
    full_line(FRAMETIME_COUNT), event_line(FRAMETIME_COUNT), update_line(FRAMETIME_COUNT), clear_line(FRAMETIME_COUNT), draw_line(FRAMETIME_COUNT), swap_line(FRAMETIME_COUNT)
{}

void Window::init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
        throw "Could not init SDL";

    handle = SDL_CreateWindow("Loading...", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 500, SDL_WINDOW_RESIZABLE);
    if (!handle)
        throw "Could not create window";

    renderer = SDL_CreateRenderer(handle, -1, 0);
    if (!renderer)
        throw "Could not create renderer";

    SDL_GetWindowSize(handle, &width, &height);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

//    SDL_SetWindowFullscreen(handle, SDL_WINDOW_FULLSCREEN);

    settitle();

    clear();
    swap();
}

void Window::quit() {
    if (renderer)
        SDL_DestroyRenderer(renderer);
    if (handle)
        SDL_DestroyWindow(handle);
    if (SDL_WasInit(SDL_INIT_EVERYTHING))
        SDL_Quit();
}

void Window::drawgraph() {
    double graph_x_step = width / (double)(FRAMETIME_COUNT - 1);
    int graph_height = biggraph ? height : (int)floor(std::max(height * 0.15, 10.));
    int graph_top_y = height - graph_height;
    int graph_mid_y = height - graph_height / 2;

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 100);
    SDL_Rect bg = {0, graph_top_y, width, graph_height};
    SDL_RenderFillRect(renderer, &bg);
    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 200);
    SDL_RenderDrawLine(renderer, 0, graph_top_y, width, graph_top_y);
    SDL_SetRenderDrawColor(renderer, 100, 100, 220, 200);
    SDL_RenderDrawLine(renderer, 0, graph_mid_y, width, graph_mid_y);

#define TRANSFORM_X(I) ((int)floor((I) * graph_x_step + 0.5))
#define TRANSFORM_Y(S) (height - 1 - (int)floor(((S) / (1. / 60. * 2)) * graph_height + 0.5))

    size_t samplecount = frametime.size();
    if (samplecount < 2)
        return;

    for (size_t i = 0; i < samplecount; i++) {
        FrameTime & times = frametime.at(i);

        full_line[i].x = TRANSFORM_X(i);
        event_line[i].x = full_line[i].x;
        update_line[i].x = full_line[i].x;
        clear_line[i].x = full_line[i].x;
        draw_line[i].x = full_line[i].x;
        swap_line[i].x = full_line[i].x;

        full_line[i].y = TRANSFORM_Y(times.full);

        double s = times.swap;
        swap_line[i].y = TRANSFORM_Y(s);
        s += times.draw;
        draw_line[i].y = TRANSFORM_Y(s);
        if (swap_line[i].y == draw_line[i].y)
            draw_line[i].y--;
        s += times.clear;
        clear_line[i].y = TRANSFORM_Y(s);
        if (draw_line[i].y == clear_line[i].y)
            clear_line[i].y--;
        s += times.update;
        update_line[i].y = TRANSFORM_Y(s);
        if (clear_line[i].y == update_line[i].y)
            update_line[i].y--;
        s += times.event;
        event_line[i].y = TRANSFORM_Y(s);
        if (update_line[i].y == event_line[i].y)
            event_line[i].y--;
    }

    if (biggraph) {
        if (showeventline) {
            SDL_SetRenderDrawColor(renderer, 255, 102, 255, 200);
            SDL_RenderDrawLines(renderer, event_line.data(), (int)samplecount);
        }
        if (showupdateline) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 200);
            SDL_RenderDrawLines(renderer, update_line.data(), (int)samplecount);
        }
        if (showclearline) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 200);
            SDL_RenderDrawLines(renderer, clear_line.data(), (int)samplecount);
        }
        if (showdrawline) {
            SDL_SetRenderDrawColor(renderer, 200, 200, 255, 200);
            SDL_RenderDrawLines(renderer, draw_line.data(), (int)samplecount);
        }
    }

    if (showswapline) {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 200);
        SDL_RenderDrawLines(renderer, swap_line.data(), (int)samplecount);
    }
    if (showfullline) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 200);
        SDL_RenderDrawLines(renderer, full_line.data(), (int)samplecount);
    }
}

void Window::clear() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
}

void Window::swap() {
    SDL_RenderPresent(renderer);
}

void Window::settitle() {
    static std::string title(128, 0);

    SDL_RendererInfo info;
    SDL_GetRendererInfo(renderer, &info);
    title.assign(info.name);
    title.append(interpolate ? ", interpolating position" : ", not interpolating");
    title.append(highprecision ? ", microsec counter" : ", millisec counter");
    if (randomdelay)
        title.append(", random delay");

    SDL_SetWindowTitle(handle, title.c_str());
}

static double Timer(bool highprecision) {
    if (highprecision) {
        static double period = 1. / SDL_GetPerformanceFrequency();
        return SDL_GetPerformanceCounter() * period;
    }
    return SDL_GetTicks() / 1000.0;
}

void Window::run() {
    SDL_Event e;
    FrameTime ft;

    int run = 1;

    double time_last_event = Timer(highprecision);
    double accumulator = 0.0;
    double tick_rate = 1.0 / 60.0;

    while (run) {
        double time_frame_start = Timer(highprecision);

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                run = 0;
            } else if (e.type == SDL_KEYUP) {
                Uint32 sym = e.key.keysym.sym;
                if (sym == SDLK_ESCAPE) {
                    run = 0;
                } else if (sym == SDLK_SPACE) {
                    pause = !pause;
                } else if (sym == SDLK_1) {
                    showfullline = !showfullline;
                } else if (sym == SDLK_2) {
                    showeventline = !showeventline;
                } else if (sym == SDLK_3) {
                    showupdateline = !showupdateline;
                } else if (sym == SDLK_4) {
                    showclearline = !showclearline;
                } else if (sym == SDLK_5) {
                    showdrawline = !showdrawline;
                } else if (sym == SDLK_6) {
                    showswapline = !showswapline;
                } else if (sym == SDLK_i) {
                    interpolate = !interpolate;
                    settitle();
                } else if (sym == SDLK_p) {
                    highprecision = !highprecision;
                    double difference = Timer(highprecision) - Timer(!highprecision);
                    time_frame_start += difference;
                    time_last_event += difference;
                    settitle();
                } else if (sym == SDLK_r) {
                    randomdelay = !randomdelay;
                    settitle();
                } else if (sym == SDLK_b) {
                    biggraph = !biggraph;
                }
            } else if (e.type == SDL_WINDOWEVENT) {
                if (e.window.event == SDL_WINDOWEVENT_RESIZED || e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    width = e.window.data1;
                    height = e.window.data2;
                }
            }
        }

        const double time_event = Timer(highprecision);
        double last_frame_time = time_event - time_last_event;
        time_last_event = time_event;

        if (!pause) {
            if (last_frame_time > 0.25)
                last_frame_time = 0.25;

            accumulator += last_frame_time;

            while (accumulator >= tick_rate) {
                player.update(tick_rate);
                accumulator -= tick_rate;
            }
        }

        const double alpha = accumulator / tick_rate;

        if (randomdelay) {
            double delay_end = Timer(highprecision) + (xorshift() % 16 + 5) / 1000.;
            while (delay_end >= Timer(highprecision)) {
            }
        }

        const double time_update = Timer(highprecision);

        clear();

        const double time_clear = Timer(highprecision);

        player.draw(renderer, interpolate ? alpha : 1, width, height);
        drawgraph();

        const double time_draw = Timer(highprecision);

        swap();

        const double time_swap = Timer(highprecision);

        if (pause)
            continue;

        ft.full = time_swap - time_frame_start;
        ft.event = time_event - time_frame_start;
        ft.update = time_update - time_event;
        ft.clear = time_clear - time_update;
        ft.draw = time_draw - time_clear;
        ft.swap = time_swap - time_draw;

        if (frametime.size() >= FRAMETIME_COUNT)
            frametime.pop_front();
        frametime.push_back(ft);
    }
}

int main(int argc, char * argv[]) {
    AM::ignore(argc);
    AM::ignore(argv);

    Uint64 t = SDL_GetPerformanceCounter();
    xstate = (Uint32)(t | (t >> 32));

    Window window;

    try {
        window.init();
        window.run();
    } catch(const char * msg) {
        std::cout << msg << std::endl;
    } catch(...) {
        std::cout << "Unexpected exception" << std::endl;
    }

    window.quit();

    return 0;
}
