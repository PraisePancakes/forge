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
        v.dir = {0};
    });
};

void Render(world& w, forge::entity player) {
    auto v = w.view<Position, Velocity>();
    v.each([player](forge::entity e, auto& p, auto& v) {
        DrawCircle(p.pos.x, p.pos.y, 10, forge::to_id(e) == forge::to_id(player) ? BLACK : RED);
    });
}

void HandleKeyEvents(world& w, forge::entity player, float deltaTime) {
    auto& vel = w.get_component<Velocity>(player);
    //================== x  y
    int move_states[] = {0, 0};
    const float speed = 250.f;
    if (IsKeyDown(KEY_W)) move_states[1]--;
    if (IsKeyDown(KEY_S)) move_states[1]++;
    if (IsKeyDown(KEY_A)) move_states[0]--;
    if (IsKeyDown(KEY_D)) move_states[0]++;
    vel.dir.x += move_states[0];
    vel.dir.y += move_states[1];
    vel.dir = Vector2Scale(vel.dir, speed * deltaTime);
};

void HandleMouseEvenets(world& w, forge::entity player, float deltaTime) {

};

int main(void) {
    const int screenWidth = 800;
    const int screenHeight = 800;
    InitWindow(screenWidth, screenHeight, "sandbox");
    world registry;
    auto player = registry.make();
    auto enemy = registry.make();
    auto [p, v] = registry.add_component<Position, Velocity>(player, Position{screenWidth / 2, screenHeight / 2}, Velocity{0});
    registry.add_component<Position, Velocity>(enemy, Position{p.pos.x - 50, p.pos.y - 50}, Velocity{0});
    Camera2D camera{p.pos, p.pos, 0.0f, 1.0f};

    while (!WindowShouldClose()) {
        float delta = GetFrameTime();
        camera.offset = p.pos;
        camera.target = p.pos;
        std::cout << p.pos.x << " : " << p.pos.y << std::endl;
        Update(registry);
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode2D(camera);
        Render(registry, player);
        DrawText("Press ESC to exit", 20, 20, 20, BLACK);

        EndMode2D();
        EndDrawing();
        HandleKeyEvents(registry, player, delta);
        HandleMouseEvenets(registry, player, delta);
    }

    CloseWindow();
    return 0;
}