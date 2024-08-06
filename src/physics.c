#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "raylib.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
const Color COLORS[10] = {YELLOW, ORANGE, PINK, RED,       MAROON,
                          GOLD,   GREEN,  LIME, DARKGREEN, SKYBLUE};

/*
 * TODO:
 * Ensure objects can't get stuck in each other boundaries
 * Ensure objects can also collide with non circle objects
 *  - Implements this for the walls, instead of the current implementation.
 * Add ability to draw objects
 * */

const int MAX_CIRCLES = 1000;
const float DT = 0.016 / 8.0;
const int SCREENWIDTH = 1280 * 2;
const int SCREENHEIGHT = 1300;

const int R = 10;
const float ELASTICITY = 0.9;
const bool CONT_X = false;

struct Circle {
  int x;
  int y;
  int r;

  int v[2];
  int m;
  float elasticity;
};

bool collision(struct Circle circ1, struct Circle circ2) {
  return (circ1.x - circ2.x) * (circ1.x - circ2.x) +
             (circ1.y - circ2.y) * (circ1.y - circ2.y) <
         (circ1.r + circ2.r) * (circ1.r + circ2.r);
};

void gravity(struct Circle *circ) { circ->v[1] -= 10; };

void wind(struct Circle *circ, float speed) { circ->v[0] += speed; };

void bounce(struct Circle *circ1, struct Circle *circ2) {
  // get dot product of differences for the first circle
  float diff_x = circ1->x - circ2->x;
  float diff_y = circ1->y - circ2->y;
  float diff_one = (circ1->v[0] - circ2->v[0]) * (diff_x);
  float diff_two = (circ1->v[1] - circ2->v[1]) * (diff_y);
  float dot_one = diff_one + diff_two;
  float norm_one = (pow(diff_x, 2) + pow(diff_y, 2));
  float factor_one = (float)circ2->m * 2 / (circ1->m + circ2->m);

  float dif_one_two = (circ2->v[0] - circ1->v[0]) * (circ2->x - circ1->x);
  float dif_two_two = (circ2->v[1] - circ1->v[1]) * (circ2->y - circ1->y);
  float dot_two = dif_one_two + dif_two_two;
  float diff_x_two = circ2->x - circ1->x;
  float diff_y_two = circ2->y - circ1->y;
  float norm_two = (pow(diff_x_two, 2) + pow(diff_y_two, 2));
  float factor_two = (float)circ1->m * 2 / (circ1->m + circ2->m);

  if (norm_two < 1e-4) {
    norm_two += 0.1;
  };
  if (norm_one < 1e-4) {
    norm_one += 0.1;
  };

  circ1->x += diff_x * DT;
  circ1->y += diff_y * DT;
  circ2->x -= diff_x * DT;
  circ2->y -= diff_y * DT;
  circ1->v[0] -= factor_one * dot_one / norm_one * diff_x * circ1->elasticity *
                 circ2->elasticity;
  circ1->v[1] -= factor_one * dot_one / norm_one * diff_y * circ1->elasticity *
                 circ2->elasticity;
  circ2->v[0] -= factor_two * dot_two / norm_two * diff_x_two *
                 circ1->elasticity * circ2->elasticity;
  circ2->v[1] -= factor_two * dot_two / norm_two * diff_y_two *
                 circ1->elasticity * circ2->elasticity;
};

void validatePos(struct Circle *circ) {
  // Define a small threshold to move the particle away from the boundary
  const int radius = circ->r;
  const float elasticity = circ->elasticity;

  if (circ->x >= SCREENWIDTH) {
    if (CONT_X) {
      circ->x = radius; // Adjust position slightly
    } else {
      circ->x = SCREENWIDTH - radius; // Adjust position slightly
      circ->v[0] *= -elasticity;      // Reverse and scale velocity
    };
  } else if (circ->x <= 0) {
    if (CONT_X) {
      circ->x = SCREENWIDTH - radius; // Adjust position slightly
    } else {
      circ->x = radius;          // Adjust position slightly
      circ->v[0] *= -elasticity; // Reverse and scale velocity
    };
  }

  if (circ->y >= SCREENHEIGHT) {
    circ->y = SCREENHEIGHT - radius; // Adjust position slightly
    circ->v[1] *= -elasticity;       // Reverse and scale velocity
  } else if (circ->y <= 0) {
    circ->y = radius;          // Adjust position slightly
    circ->v[1] *= -elasticity; // Reverse and scale velocity
  }
}
void move(struct Circle *circ) {
  validatePos(circ);

  circ->x += circ->v[0] * DT;
  circ->y -= circ->v[1] * DT;
};

void solvePhysics(struct Circle *circles, int circ_num, bool gravity_bool,
                  float wind_speed) {
  int m;
  for (m = 0; m < 8; m++) {
    int i, j;

    for (i = 0; i < circ_num; i++) {
      for (j = i; j < circ_num; j++) {
        if (collision(circles[i], circles[j]) && i != j) {
          bounce(&circles[i], &circles[j]);
        };
      };
    };
    for (i = 0; i < circ_num; i++) {
      if (gravity_bool) {
        gravity(&circles[i]);
      };
      wind(&circles[i], wind_speed);
      move(&circles[i]);
    };
  };
};

int main(void) {
  // Initialization
  //--------------------------------------------------------------------------------------

  InitWindow(SCREENWIDTH, SCREENHEIGHT, "THE physics simulation");
  char text_count[128];
  char text_fps[16];
  char text_speed[128];
  char text_momentum[128];

  SetTargetFPS(60); // Set our game to run at 60 frames-per-second
  //--------------------------------------------------------------------------------------

  struct Circle circles[MAX_CIRCLES];
  int circ_num = 0;
  int y = 100;
  int x = 1200;
  int v1 = 2000;
  int v2 = 0;

  Rectangle wind_slider = {2300, 50, 200, 70};
  Rectangle gravity_button = {2300, 140, 200, 60};
  bool gravity_bool = true;
  float wind_speed = 0;
  // Main game loop
  while (!WindowShouldClose()) // Detect window close button or ESC key
  {

    GuiSlider(wind_slider, "-10", "10", &wind_speed, -10.0, 10.0);
    GuiToggle(gravity_button, "Gravity", &gravity_bool);
    if (circ_num < MAX_CIRCLES) {
      struct Circle circ = {
          x , y -  (20 * R * circ_num % 2), R, {v1, v2}, R * 10, ELASTICITY};
      circles[circ_num] = circ;
      if (IsMouseButtonDown(1)) {
        int x_mouse, y_mouse;
        x_mouse = GetMouseX();
        y_mouse = GetMouseY();
        struct Circle circ = {x_mouse,  y_mouse,         R,
                              {v1, v2}, 4.2 * R * R * R, ELASTICITY};
        circles[circ_num] = circ;
      };
      circ_num++;
    };

    BeginDrawing();

    ClearBackground(RAYWHITE);

    float avg_speed = 0;
    float avg_momentum = 0;
    solvePhysics(circles, circ_num, gravity_bool, wind_speed);

    int i;
    for (i = 0; i < circ_num; i++) {
      /*DrawCircle(circles[i].x, circles[i].y, circles[i].r, COLORS[i % 10]);*/
      DrawCircle(circles[i].x, circles[i].y, circles[i].r, RED);
      avg_speed += sqrt(circles[i].v[0] * circles[i].v[0] +
                        circles[i].v[1] * circles[i].v[1]);
      avg_momentum += sqrt(circles[i].v[0] * circles[i].v[0] +
                           circles[i].v[1] * circles[i].v[1]) *
                      circles[i].m;
    };

    int FPS = GetFPS();
    sprintf(text_fps, "FPS: %d", FPS);
    sprintf(text_count, "Number of Particles: %d", circ_num);
    sprintf(text_speed, "Average Speed of Particles: %.0f",
            avg_speed / MAX_CIRCLES);
    sprintf(text_momentum, "Average Momentum of Particles: %.0f",
            avg_momentum / MAX_CIRCLES);
    DrawText(text_count, 10, 10, 50, BLACK);
    DrawText(text_fps, 1400, 10, 50, BLACK);
    DrawText(text_speed, 10, 60, 50, BLACK);
    DrawText(text_momentum, 10, 110, 50, BLACK);

    ShowCursor();
    EndDrawing();
    //----------------------------------------------------------------------------------
  }

  // De-Initialization
  //--------------------------------------------------------------------------------------
  CloseWindow(); // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}
