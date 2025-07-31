#include "raylib.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"

// WINDOW / UTILS
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define DEGREE 0.0174533

// KEYS
#define KEY_W 83
#define KEY_S 87
#define KEY_A 65
#define KEY_D 68

// MAP
#define SIZE 10

// CAMERA / GRAPHICS
#define FOV 60
#define LEVEL_OF_DETAIL 10

int level[10][10] = {
    {1,1,1,1,1,1,1,1,1,1},
    {1,0,1,0,1,0,0,0,0,1},
    {1,0,1,0,0,0,0,0,0,1},
    {1,0,1,1,1,0,0,0,0,1},
    {1,0,0,0,0,0,0,3,0,1},
    {1,0,0,2,2,0,0,3,0,1},
    {1,0,0,2,0,0,0,0,0,1},
    {1,0,0,2,0,0,0,0,0,1},
    {1,0,0,2,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1},
};

Color colors[] = {
    {255, 0, 0},
    {0, 255, 0},
    {0, 0, 255}
};

struct Hit {
    float distance;
    int tile_value;
    bool side;
};

float distance_vv(Vector2 a, Vector2 b) {
    return sqrt(pow(a.x-b.x, 2) + pow(a.y-b.y, 2));
}

Color from_rgb(float r, float g, float b) {
    return (Color){r, g, b, 255};
}

Vector2 from_angle(float angle) {
    return (Vector2){cos(angle), sin(angle)};
}

struct Hit DDA(Vector2 pos, Vector2 dir) {

    Vector2 unit_step_size = { sqrt(1 + (dir.y / dir.x) * (dir.y / dir.x)), sqrt(1 + (dir.x / dir.y) * (dir.x / dir.y)) };
    Vector2 grid_pos = {(int) (pos.x),(int) (pos.y)};

    Vector2 step;
    Vector2 ray_length;

    if (dir.x < 0) {
        step.x = -1;
        ray_length.x = (pos.x - (grid_pos.x)) * unit_step_size.x;
    } else {
        step.x = 1;
        ray_length.x = ((grid_pos.x + 1) - pos.x) * unit_step_size.x;
    }

    if (dir.y < 0) {
        step.y = -1;
        ray_length.y = (pos.y - (grid_pos.y)) * unit_step_size.y;
    } else {
        step.y = 1;
        ray_length.y = ((grid_pos.y + 1) - pos.y) * unit_step_size.y;
    }

    float max_distance = 100;
    float distance = 0;
    bool side = false;
    while (distance < max_distance)
    {
        if (ray_length.x < ray_length.y)
        {
            grid_pos.x += step.x;
            distance = ray_length.x;
            ray_length.x += unit_step_size.x;
            side = true;
        }
        else
        {
            grid_pos.y += step.y;
            distance = ray_length.y;
            ray_length.y += unit_step_size.y;
            side = false;
        }

        if (level[(int) grid_pos.y][(int) grid_pos.x] != 0)
        {
            Vector2 poi = {pos.x + dir.x * distance, pos.y + dir.y * distance};
            float distance = distance_vv(poi, pos);
            int tile_value = level[(int) grid_pos.y][(int) grid_pos.x];
            return (struct Hit){distance, tile_value, side};
        }
    }

}

int main(void)
{
    Vector2 player_pos= {1.5,1.5};
    float player_angle = PI/2;

    float speed_scale = 1./30.;
    float turn_speed = 0.02;

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Raycaster Game");
    DisableCursor();
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        Vector2 dir = from_angle(player_angle);
        Vector2 perpendicular_dir = {-dir.y, dir.x};
        Vector2 movement_vector = {0,0};

        float mx = GetMouseDelta().x;
        player_angle += mx/500;

        if (IsKeyDown(KEY_W)) {
            movement_vector.x -= dir.x;
            movement_vector.y -= dir.y;
        } else if (IsKeyDown(KEY_S)) {
            movement_vector.x += dir.x;
            movement_vector.y += dir.y;
        }
        if (IsKeyDown(KEY_D)) {
            movement_vector.x += perpendicular_dir.x;
            movement_vector.y += perpendicular_dir.y;
        } else if (IsKeyDown(KEY_A)) {
            movement_vector.x -= perpendicular_dir.x;
            movement_vector.y -= perpendicular_dir.y;
        }

        movement_vector.x *= speed_scale;
        movement_vector.y *= speed_scale;

        player_pos.x += movement_vector.x;
        if (level[(int) player_pos.y][(int) player_pos.x] != 0) {
            player_pos.x -= movement_vector.x;
        }

        player_pos.y += movement_vector.y;
        if (level[(int) player_pos.y][(int) player_pos.x] != 0) {
            player_pos.y -= movement_vector.y;
        }

        player_angle = fmod(player_angle, 2*PI);

        BeginDrawing();
        ClearBackground(BLACK);

        DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT/2, BLACK);
        DrawRectangle(0, WINDOW_HEIGHT/2, WINDOW_WIDTH, WINDOW_HEIGHT/2, DARKGRAY);
        for (int i = 0; i < FOV*LEVEL_OF_DETAIL; i++) {
            float j = (float) i / LEVEL_OF_DETAIL;

            float ray_angle = player_angle - DEGREE*(FOV/2) + DEGREE*j;
            Vector2 ray_vector = from_angle(ray_angle);

            struct Hit hit_info = DDA(player_pos, ray_vector);
            float distance = hit_info.distance * cos(player_angle - ray_angle);
            int tile_value = hit_info.tile_value;
            bool side = hit_info.side;

            if (distance != 0) {

                float line_height = WINDOW_HEIGHT/distance;
                float y_start = -line_height / 2 + WINDOW_HEIGHT/2;
                float y_end = line_height/2 + WINDOW_HEIGHT/2;
                float height = y_end - y_start;

                float width = (float) WINDOW_WIDTH/(FOV*LEVEL_OF_DETAIL);

                float r = colors[tile_value-1].r;
                float g = colors[tile_value-1].g;
                float b = colors[tile_value-1].b;

                Color color = from_rgb(r/2,g/2,b/2);
                if (side) {color = from_rgb(r, g, b); }

                DrawRectangle(i*width, y_start, ceil(width), height, color);

            }

        }

        DrawRectangle(0, 0, 10*SIZE, 10*SIZE, WHITE);
        for (int x = 0; x < 10; x++) {
            for (int y = 0; y < 10; y++) {

                if (level[y][x] > 0) {
                    DrawRectangle(x*SIZE,y*SIZE,SIZE,SIZE, BLACK);
                }
            }
        }
        DrawCircle(player_pos.x*SIZE, player_pos.y*SIZE, 3, RED);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
