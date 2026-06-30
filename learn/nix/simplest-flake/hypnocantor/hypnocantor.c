#define A_NOPREFIX
#include <a.h>
#include <math.h>
#include <raylib.h>

////// Perlin //////

// Took a lot of stuff from here:
// https://rtouti.github.io/graphics/perlin-noise-algorithm

// Perlin's original permutation table (may as well)
int P[] = { 151, 160, 137,  91,  90,  15, 131,  13, 201,  95,  96,  53, 194, 233,   7, 225,
            140,  36, 103,  30,  69, 142,   8,  99,  37, 240,  21,  10,  23, 190,   6, 148,
            247, 120, 234,  75,   0,  26, 197,  62,  94, 252, 219, 203, 117,  35,  11,  32,
             57, 177,  33,  88, 237, 149,  56,  87, 174,  20, 125, 136, 171, 168,  68, 175,
             74, 165,  71, 134, 139,  48,  27, 166,  77, 146, 158, 231,  83, 111, 229, 122,
             60, 211, 133, 230, 220, 105,  92,  41,  55,  46, 245,  40, 244, 102, 143,  54,
             65,  25,  63, 161,   1, 216,  80,  73, 209,  76, 132, 187, 208,  89,  18, 169,
            200, 196, 135, 130, 116, 188, 159,  86, 164, 100, 109, 198, 173, 186,   3,  64,
             52, 217, 226, 250, 124, 123,   5, 202,  38, 147, 118, 126, 255,  82,  85, 212,
            207, 206,  59, 227,  47,  16,  58,  17, 182, 189,  28,  42, 223, 183, 170, 213,
            119, 248, 152,   2,  44, 154, 163,  70, 221, 153, 101, 155, 167,  43, 172,   9,
            129,  22,  39, 253,  19,  98, 108, 110,  79, 113, 224, 232, 178, 185, 112, 104,
            218, 246,  97, 228, 251,  34, 242, 193, 238, 210, 144,  12, 191, 179, 162, 241,
             81,  51, 145, 235, 249,  14, 239, 107,  49, 192, 214,  31, 181, 199, 106, 157,
            184,  84, 204, 176, 115, 121,  50,  45, 127,   4, 150, 254, 138, 236, 205,  93,
            222, 114,  67,  29,  24,  72, 243, 141, 128, 195,  78,  66, 215,  61, 156, 180 };

vec2f32 gridpointVector2(int gridId) {
  switch (gridId & 0b11) {
    case 0: return (vec2f32){ 1.0,  1.0};
    case 1: return (vec2f32){-1.0,  1.0};
    case 2: return (vec2f32){-1.0, -1.0};
    case 3: return (vec2f32){ 1.0, -1.0};
    default: impossible();
  }
}

f32 perlinEase(f32 t) {
  return ((6*t - 15)*t + 10)*t*t*t;
}
f32 perlinLerp(f32 t, f32 a, f32 b) {
  return a + t*(b-a);
}

f32 perlin2D(float x, float y) {
  // gridpoint vector indices
  int xi = (int)floorf(x);
  int yi = (int)floorf(y);

  f32 xf = x - (f32)xi;
  f32 yf = y - (f32)yi;

  // vectors within gridcell
  vec2f32 trf = (vec2f32){xf-1.0, yf-1.0};
  vec2f32 tlf = (vec2f32){xf    , yf-1.0};
  vec2f32 brf = (vec2f32){xf-1.0, yf    };
  vec2f32 blf = (vec2f32){xf    , yf    };

  // gridpoint hash
  int tri = P[(P[(xi+1) % 256]+yi+1) % 256];
  int tli = P[(P[(xi  ) % 256]+yi+1) % 256];
  int bri = P[(P[(xi+1) % 256]+yi  ) % 256];
  int bli = P[(P[(xi  ) % 256]+yi  ) % 256];

  // dot products
  f32 trd = dot(trf, gridpointVector2(tri));
  f32 tld = dot(tlf, gridpointVector2(tli));
  f32 brd = dot(brf, gridpointVector2(bri));
  f32 bld = dot(blf, gridpointVector2(bli));

  // interpolate
  float u = perlinEase(xf);
  float v = perlinEase(yf);

  return perlinLerp(u,
         perlinLerp(v, bld, tld),
         perlinLerp(v, brd, trd));
}

float perlinOctaves2D(f32 x, f32 y, int nOctaves) {
  f32 out = 0.0;
  f32 A = 1.0;
  f32 f = 1.0;
  for (int octave = 0; octave < nOctaves; octave++) {
    out += A * perlin2D(x * f, y * f);
    A *= 0.5;
    f *= 2.0;
  }
  return out;
}

////// Main //////

void DrawFps(float updateTime_s) {
  float dt_spf = GetFrameTime();
  char msg[12] = {0};
  snprintf(msg, sizeof(msg), "%.1f FPS", 1.0/dt_spf);
  DrawText(msg, 8, 6, 16, GRAY);
  snprintf(msg, sizeof(msg), "%.2f us/f", 1.0e6 * updateTime_s);
  DrawText(msg, 8, 6+16+6, 16, GRAY);
}

Vector2 points[81];
int main() {
  // initialize
  int monitorWd = 800;
  int monitorHt = 600;
  InitWindow(monitorWd, monitorHt, "Hypnoline");
  HideCursor();
  ToggleFullscreen();
  { // re-initialize
    int monitorId = GetCurrentMonitor();
    monitorWd = GetMonitorWidth(monitorId);
    monitorHt = GetMonitorHeight(monitorId);
    debug(monitorWd);
    debug(monitorHt);
  }
  defer { CloseWindow(); }
  SetTargetFPS(60);
  { // initialize x-coords of hypnowave
    f32 dx_points = monitorWd / (lengthof(points)-1);
    for (int i = 0; i < lengthof(points); i++) {
      Vector2* pt = &points[i];
      pt->x = i*dx_points;
    }
  }
  while (!WindowShouldClose()) {
    float t0_s = GetTime();
    BeginDrawing();
    defer { EndDrawing(); }

    ClearBackground((Color){0,0,0,255});

    for (int i = 0; i < lengthof(points); i++) {
      Vector2* pt = &points[i];
      f32 dy = perlinOctaves2D(
                pt->x/(monitorWd/4.0) + t0_s,
                t0_s*0.2,
                4);
      pt->y = monitorHt/2.0 + (monitorHt/6)*dy;
    }
    DrawLineStrip(points, lengthof(points), (Color){0x80, 0xC0, 0xFF, 0xFF});

    float t_s = GetTime();
    DrawFps(t_s - t0_s);

  }
  return 0;
}
