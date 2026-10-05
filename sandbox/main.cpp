#include <raylib.h>
#include <raymath.h>

#include <forge/forge.hpp>

struct Position {
    Vector2 pos{0, 0};
};
struct Velocity {
    Vector2 dir{0, 0};
};

struct Tag {
    std::string tag;
};

struct Health {
    int health{0};
};

using world = forge::registry<Position, Velocity, Health, Tag>;

void Update(world& w, float deltaTime) {
    auto v = w.view<Position, Velocity>();
    v.each([deltaTime](auto& p, auto& v) {
        p.pos = Vector2Add(
            p.pos,
            Vector2Scale(v.dir, deltaTime));
    });
}

void Render(world& w) {
    auto v = w.view<Position, Health>();
    v.each([&w](forge::entity e, auto& p, auto& health) {
        DrawCircle(p.pos.x, p.pos.y, 10, w.get_component<Tag>(e).tag == "Player" ? BLACK : RED);
        DrawRectangle(p.pos.x, p.pos.y - 35, 50, 20, health.health >= 80 ? GREEN : health.health >= 50 && health.health < 80 ? YELLOW
                                                                                                                             : RED);
    });

    auto v1 = w.view<Position>().exclude<Health>();
    v1.each([](auto& p) {
        DrawCircle(p.pos.x, p.pos.y, 5, BLACK);
    });
}

void HandleKeyEvents(world& w, forge::entity player) {
    auto& vel = w.get_component<Velocity>(player);
    Vector2 direction{0, 0};
    if (IsKeyDown(KEY_W)) direction.y--;
    if (IsKeyDown(KEY_S)) direction.y++;
    if (IsKeyDown(KEY_A)) direction.x--;
    if (IsKeyDown(KEY_D)) direction.x++;
    if (Vector2Length(direction) > 0.0f)
        direction = Vector2Normalize(direction);
    constexpr float speed = 250.0f;
    vel.dir = Vector2Scale(direction, speed);
}
void HandleMouseEvents(world& w, forge::entity player, Camera2D camera) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        auto [pos, vel] = w.get_component<Position, Velocity>(player);
        Vector2 mouse_world = GetScreenToWorld2D(GetMousePosition(), camera);
        Vector2 mouse_dir = Vector2Normalize(Vector2Subtract(mouse_world, pos.pos));
        auto bullet = w.make();
        constexpr float bullet_speed = 500.0f;
        w.add_component<Position, Velocity>(
            bullet,
            Position{.pos = pos.pos},
            Velocity{.dir = Vector2Scale(mouse_dir, bullet_speed)});
    }
}

int main(void) {
    const int screenWidth = 800;
    const int screenHeight = 800;
    InitWindow(screenWidth, screenHeight, "sandbox");
    world registry;
    auto player = registry.make();
    auto enemy = registry.make();
    auto [p, v, _, _] = registry.add_component<Position, Velocity, Tag, Health>(player, Position{screenWidth / 2, screenHeight / 2}, Velocity{0}, Tag{.tag = "Player"}, Health{100});
    registry.add_component<Position, Velocity, Tag, Health>(enemy, Position{p.pos.x - 50, p.pos.y - 50}, Velocity{0}, Tag{.tag = "enemy"}, Health{100});
    Camera2D camera{p.pos, p.pos, 0.0f, 1.0f};

    while (!WindowShouldClose()) {
        float delta = GetFrameTime();
        Update(registry, delta);
        HandleKeyEvents(registry, player);
        HandleMouseEvents(registry, player, camera);
        camera.target = p.pos;
        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode2D(camera);
        Render(registry);
        DrawText("Press ESC to exit", camera.target.x - screenWidth / 2, camera.target.y - screenHeight / 2, 20, BLACK);
        EndMode2D();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}