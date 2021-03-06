/*
        A* shortest pathfinding visualization.
        Written by SilverbossTD (duycntsilverboss@gmail.com).

        Language: C++.
        Library: SDL2.
        Algorithm: A*.

        Basic Controls:
        - Create "Nodes":
                + Start: press 1 + left click.
                + End: press 2 + left click.
                + Wall: press 3 + left click.
        - Delete "Walls":
                + Press 3 + left click on the wall.
        - Start finding:
                + Press "Enter".
        - Clear the map:
                + Press "ESC".
*/

#include <SDL2/SDL.h>

#include <iostream>

#include <vector>

#include <algorithm>

#include <math.h>

#include "Globals.h"

#include "Node.h"

#include "Grid.h"

SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;

int
        frameCount,
        timerFPS,
        lastFrame,
        fps,
        lastTime,
        type = 0;

bool
        running = true,
        mouseDown = false,
        startFinding = false;

Grid grid;

SDL_Rect grid_cursor_ghost;

SDL_Event event;

Node start, target;

std::vector<Node> open, closed;
std::vector<Node> path;

inline void init() {
        open.clear();
        closed.clear();
        path.clear();

        grid = Grid(renderer, ROWS, COLS, SIZE);
        start = Node(renderer, 0, 0, SIZE);
        target = Node(renderer, ROWS - 1, COLS - 1, SIZE);
        grid_cursor_ghost = {(ROWS - 1) / 2 * SIZE, (COLS - 1) / 2 * SIZE, SIZE, SIZE};
}

inline auto heuristic(const Node &start, const Node &end) -> float {
        return std::hypot(end.x - start.x, end.y - start.y);
}

inline void zoom(int&& size) {
        SIZE += size;
        ROWS = SCREEN_WIDTH / SIZE;
        COLS = SCREEN_HEIGHT / SIZE;

        init();
}

void update() {
        if (open.empty() || !startFinding) return;

        Node current = open[0];

        for (std::size_t i = 1; i < open.size(); ++i)
                if (open[i].f < current.f)
                        current = open[i];

        if (current == target) {
                current = grid.getNode(target.x, target.y);

                while (current != start) {
                        path.push_back(current);
                        current = grid.getNode(current.parentX, current.parentY);
                }

                std::reverse(path.begin(), path.end());
                startFinding = false;
                return;
        }

        open.erase(std::remove(open.begin(), open.end(), current), open.end());
        closed.push_back(current);

        for (auto &neighbor : grid.getNeighbors(current)) {
                if (neighbor.isWall || (std::find(closed.begin(), closed.end(), neighbor) != closed.end()))
                        continue;

                int newCost = current.g + heuristic(current, neighbor);
                if (newCost < neighbor.g || (std::find(open.begin(), open.end(), neighbor) == open.end())) {
                        neighbor.g = newCost;
                        neighbor.h = heuristic(neighbor, target);
                        neighbor.f = neighbor.g + neighbor.h;

                        grid.setParent(neighbor, current);

                        if (std::find(open.begin(), open.end(), neighbor) == open.end())
                                open.push_back(neighbor);
                }
        }
}

void input() {
        while (SDL_PollEvent(&event)) {
                switch (event.type) {
                        case SDL_QUIT: running = false; break;
                        case SDL_MOUSEWHEEL:
                                if(event.wheel.y > 0 && !startFinding && SIZE < 100)
                                        zoom(10);
                                else if(event.wheel.y < 0 && !startFinding && SIZE >= 20)
                                        zoom(-10);
                                break;
                        case SDL_MOUSEMOTION:
                                grid_cursor_ghost.x = (event.motion.x / SIZE) * SIZE;
                                grid_cursor_ghost.y = (event.motion.y / SIZE) * SIZE;
                                if (mouseDown && type == 2) {
                                        if (grid.checkExist((event.motion.x / SIZE), (event.motion.y / SIZE))) break;
                                        grid.addWall((event.motion.x / SIZE), (event.motion.y / SIZE));
                                }
                                break;
                        case SDL_MOUSEBUTTONDOWN:
                                mouseDown = true;
                                open.clear();
                                closed.clear();
                                path.clear();
                                if (type == 0) start = Node(renderer, (event.motion.x / SIZE), (event.motion.y / SIZE), SIZE);
                                else if (type == 1) target = Node(renderer, (event.motion.x / SIZE), (event.motion.y / SIZE), SIZE);
                                else if (type == 2) {
                                        if (grid.checkExist((event.motion.x / SIZE), (event.motion.y / SIZE))) {
                                                grid.removeWall((event.motion.x / SIZE), (event.motion.y / SIZE));
                                                break;
                                        }
                                        grid.addWall((event.motion.x / SIZE), (event.motion.y / SIZE));
                                }
                                break;
                        case SDL_MOUSEBUTTONUP:
                                mouseDown = false;
                                break;
                        case SDL_KEYDOWN:
                                switch (event.key.keysym.sym) {
                                        case SDLK_RETURN:
                                                startFinding = true;
                                                open.push_back(start);
                                                break;
                                        case SDLK_ESCAPE:
                                                startFinding = false;
                                                open.clear();
                                                closed.clear();
                                                path.clear();
                                                grid.removeWalls();
                                                break;
                                        case SDLK_1: type = 0; break;
                                        case SDLK_2: type = 1; break;
                                        case SDLK_3: type = 2; break;
                                }
                                break;
                }
        }
}

void tick() {
        lastFrame = SDL_GetTicks();
        if (lastFrame >= lastTime + 1000) {
                lastTime = lastFrame;
                fps = frameCount;
                frameCount = 0;
        }
}

void draw() {
        SDL_SetRenderDrawColor(renderer, 192, 191, 192, 0xff);     // Draw grid_cursor_ghost.
        SDL_RenderFillRect(renderer, &grid_cursor_ghost);

        for (auto &node : closed) node.draw(255, 106, 73);
        for (auto &node : open) node.draw(36, 221, 96);
        for (auto &node : path) node.draw(85, 176, 254);

        grid.drawWalls();

        start.draw(63, 119, 255);
        target.draw(255, 34, 10);

        grid.draw();
}

void render() {
        SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
        SDL_RenderClear(renderer);

        ++frameCount;
        timerFPS = SDL_GetTicks() - lastFrame;
        if (timerFPS < (1000 / 60)) {
                SDL_Delay((1000 / 60) - timerFPS);
        }

        draw();

        SDL_RenderPresent(renderer);
}

int main(int argc, char const *argv[]) {
        if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
                std::cout << "SDL_Init() failed" << '\n';
        if (SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer) < 0)
                std::cout << "SDL_CreateWindowAndRenderer() failed" << '\n';
        SDL_SetWindowTitle(window, "Path Finding");

        init();

        while (running) {
                tick();
                update();
                input();
                render();
        };

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();

        return 0;
}
