#include "raylib.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"

// WINDOW / UTILS
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define DEGREE 0.0174533

// MAP
#define TILE_SIZE 5

// CAMERA / GRAPHICS
#define FOV 75
#define LEVEL_OF_DETAIL 30

#define MAP_SIZE 24

int level[MAP_SIZE][MAP_SIZE]=
{
  {4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,7,7,7,7,7,7,7,7},
  {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,7},
  {4,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
  {4,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
  {4,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,7},
  {4,0,4,0,0,0,0,5,5,5,5,5,5,5,5,5,7,7,0,7,7,7,7,7},
  {4,0,5,0,0,0,0,5,0,5,0,5,0,5,0,5,7,0,0,0,7,7,7,1},
  {4,0,6,0,0,0,0,5,0,0,0,0,0,0,0,5,7,0,0,0,0,0,0,7},
  {4,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,7,7,1},
  {4,0,8,0,0,0,0,5,0,0,0,0,0,0,0,5,7,0,0,0,0,0,0,7},
  {4,0,0,0,0,0,0,5,0,0,0,0,0,0,0,5,7,0,0,0,7,7,7,1},
  {4,0,0,0,0,0,0,5,5,5,5,0,5,5,5,5,7,7,7,7,7,7,7,1},
  {6,6,6,6,6,6,6,6,6,6,6,0,6,6,6,6,6,6,6,6,6,6,6,6},
  {7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
  {6,6,6,6,6,6,0,6,6,6,6,0,6,6,6,6,6,6,6,6,6,6,6,6},
  {4,4,4,4,4,4,0,4,4,4,6,0,6,2,2,2,2,2,2,2,3,3,3,3},
  {4,0,0,0,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,0,0,0,2},
  {4,0,0,0,0,0,0,0,0,0,0,0,6,2,0,0,5,0,0,2,0,0,0,2},
  {4,0,0,0,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,2,0,2,2},
  {4,0,6,0,6,0,0,0,0,4,6,0,0,0,0,0,5,0,0,0,0,0,0,2},
  {4,0,0,5,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,2,0,2,2},
  {4,0,6,0,6,0,0,0,0,4,6,0,6,2,0,0,5,0,0,2,0,0,0,2},
  {4,0,0,0,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,0,0,0,2},
  {4,4,4,4,4,4,4,4,4,4,1,1,1,2,2,2,2,2,2,3,3,3,3,3}
};

Color colors[] = {
    {255, 0, 0},     // Red
    {0, 255, 0},     // Green
    {0, 0, 255},     // Blue
    {100, 130, 0},   // Olive
    {255, 0, 255},   // Magenta
    {0, 255, 255},   // Cyan
    {255, 128, 0},   // Orange
    {128, 0, 255},   // Purple
    {0, 128, 128},   // Teal
    {255, 255, 0}   // Yellow
};

struct Hit {
    float distance;
    float distance_along_wall;
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
            float daw;

            if (!side) { daw = fabs(poi.x - grid_pos.x); }
            else { daw = fabs(poi.y - grid_pos.y); }

            float distance = distance_vv(poi, pos);
            int tile_value = level[(int) grid_pos.y][(int) grid_pos.x];

            return (struct Hit){distance, daw, tile_value, side};
        }
    }

}


int main(void)
{
    float bob = 0;
    float framerate = 0;
    float elapsed = 0;

    Vector2 player_pos= {1.5,1.5};
    float player_angle = PI/2;

    float speed_scale = 1./30.;
    float sprint_scale = 2./30.;
    float turn_speed = 0.02;

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Raycaster Game");
    DisableCursor();
    SetTargetFPS(60);

    Texture hands = LoadTexture("assets/hands.png");
    Texture bricks = LoadTexture("assets/bricks.png");
    Color* brick_colours = LoadImageColors(LoadImage("assets/bricks.png"));

    while (!WindowShouldClose())
    {
        elapsed += GetFrameTime();
        framerate = 1.0/GetFrameTime();
        printf("%f\n", framerate);
        bob = 10*sin(10*elapsed);

        Vector2 movement_vector = {0,0};
        Vector2 dir = from_angle(player_angle);
        Vector2 perpendicular_dir = {-dir.y, dir.x};

        float mx = GetMouseDelta().x;
        player_angle += mx/500;


        if (IsKeyDown(KEY_W)) {
            movement_vector.x += dir.x;
            movement_vector.y += dir.y;
        } else if (IsKeyDown(KEY_S)) {
            movement_vector.x -= dir.x;
            movement_vector.y -= dir.y;
        }

        if (IsKeyDown(KEY_D)) {
            movement_vector.x += perpendicular_dir.x;
            movement_vector.y += perpendicular_dir.y;
        } else if (IsKeyDown(KEY_A)) {
            movement_vector.x -= perpendicular_dir.x;
            movement_vector.y -= perpendicular_dir.y;
        }

        if (IsKeyDown(KEY_R)) {
            movement_vector.x *= sprint_scale;
            movement_vector.y *= sprint_scale;
        } else {
            movement_vector.x *= speed_scale;
            movement_vector.y *= speed_scale;
        }

        if (movement_vector.x == 0 && movement_vector.y == 0) {
            bob = 0;
        }

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

        DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT/2, WHITE);
        DrawRectangle(0, WINDOW_HEIGHT/2, WINDOW_WIDTH, WINDOW_HEIGHT/2, DARKGRAY);
        for (int i = 0; i < FOV*LEVEL_OF_DETAIL; i++) {
            float j = (float) i / LEVEL_OF_DETAIL;

            float ray_angle = player_angle - DEGREE*(FOV/2) + DEGREE*j;
            Vector2 ray_vector = from_angle(ray_angle);

            struct Hit hit_info = DDA(player_pos, ray_vector);
            float distance = hit_info.distance * cos(player_angle - ray_angle);
            float distance_along_wall = hit_info.distance_along_wall;
            int tile_value = hit_info.tile_value;
            bool side = hit_info.side;

            if (distance != 0) {

                float line_height = WINDOW_HEIGHT/distance;
                float y_start = -line_height / 2 + WINDOW_HEIGHT/2;
                float y_end = line_height/2 + WINDOW_HEIGHT/2;
                float height = y_end - y_start;

                float width = (float) WINDOW_WIDTH/(FOV*LEVEL_OF_DETAIL);

                float index = (int) (distance_along_wall*64);
                // float r = colors[tile_value-1].r;
                // float g = colors[tile_value-1].g;
                // float b = colors[tile_value-1].b;

                // Color color = from_rgb(r,g,b);
                // if (!side) {color = from_rgb(r/2,g/2,b/2); }
                
                for (int j = 0; j < 64; j++) {
                    float pixel_height = (height/64.);

                    Color color = brick_colours[64 * (int)(j) + (int) index];
                    DrawRectangle(i*width, j*pixel_height + y_start + bob, ceil(width), ceil(pixel_height), color);
                }


            }

        }

        DrawRectangle(0, 0, MAP_SIZE*TILE_SIZE, MAP_SIZE*TILE_SIZE, WHITE);
        for (int x = 0; x < MAP_SIZE; x++) {
            for (int y = 0; y < MAP_SIZE; y++) {

                if (level[y][x] > 0) {
                    DrawRectangle(x*TILE_SIZE,y*TILE_SIZE,TILE_SIZE,TILE_SIZE, BLACK);
                }
            }
        }

        DrawCircle(player_pos.x*TILE_SIZE, player_pos.y*TILE_SIZE, 2, RED);
        DrawTextureEx(hands, (Vector2){15-bob,15-bob}, 0, 2.5, WHITE);             // Always draw a texture
        EndDrawing();

    }

    CloseWindow();
    return 0;
}
