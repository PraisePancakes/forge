#include <raylib.h>
#include <raymath.h>

#include <forge/forge.hpp>

struct Position {
    Vector2 pos{0, 0};
};
struct Velocity {
    Vector2 dir{0, 0};
};

using world = forge::registry<Position, Velocity>;

void Update(world& w) {
    auto v = w.view<Position, Velocity>();
    v.each([](auto& p, auto& v) {
        p.pos = Vector2Add(p.pos, v.dir);
        DrawCircle(p.pos.x, p.pos.y, 10, BLACK);
        v.dir = {0};
    });
};

void HandleKeyEvents(world& w, forge::entity player) {
    auto& vel = w.get_component<Velocity>(player);
    if (IsKeyDown(KEY_W)) {
        vel.dir = {0, -1};
    }
    if (IsKeyDown(KEY_A)) {
        vel.dir = {-1, 0};
    }
    if (IsKeyDown(KEY_D)) {
        vel.dir = {1, 0};
    }
    if (IsKeyDown(KEY_S)) {
        vel.dir = {0, 1};
    }
};

int main(void) {
    const int screenWidth = 800;
    const int screenHeight = 800;
    InitWindow(screenWidth, screenHeight, "sandbox");
    world registry;
    auto player = registry.make();
    registry.add_component<Position, Velocity>(player, Position{screenWidth / 2, screenHeight / 2}, Velocity{0});
    while (!WindowShouldClose()) {
        BeginDrawing();
        HandleKeyEvents(registry, player);
        Update(registry);

        ClearBackground(RAYWHITE);
        DrawText("Press ESC to exit", 20, 20, 20, BLACK);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}