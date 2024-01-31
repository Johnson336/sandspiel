
#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "./raylib/examples/shapes/raygui.h"
#include <vector>

int sandsize = 5;
int flowRate = 35;
int width;
int height;
std::vector<float> grid;
float hueValue = 1.0f;
bool colorLocked = false;
bool chooserOpen = false;
bool glowOn = false;
Rectangle chooserSize;
Rectangle lockBoxSize;
int shader_choice = 0;

const int NUM_SHADERS = 9;
const char *shader_names[NUM_SHADERS] = {
    "Bloom",      "Eratosthenes", "Cross Hatching", "Cross Stitching", "Depth",
    "Distortion", "Pixelizer",    "Posterization",  "Predator",
};

const char *shader_paths[NUM_SHADERS] = {
    "bloom.fs",           "eratosthenes.fs",  "cross_hatching.fs",
    "cross_stitching.fs", "depth.fs",         "distortion.fs",
    "pixelizer.fs",       "posterization.fs", "predator.fs",
};

Shader shaders[NUM_SHADERS];

void LoadShaders() {
  for (int i = 0; i < NUM_SHADERS; i++) {
    shaders[i] = LoadShader(0, shader_paths[i]);
  }
}

void InitGrid() {
  height = GetMonitorHeight(GetCurrentMonitor()) / sandsize;
  width = GetMonitorWidth(GetCurrentMonitor()) / sandsize;
  chooserSize = {(float)GetMonitorWidth(GetCurrentMonitor()) - 200, 0, 200,
                 400};
  lockBoxSize = (Rectangle){chooserSize.x - 15, chooserSize.y + 5, 10, 10};
  // fill grid with zeros
  // one dimensional array
  grid.clear();
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      grid.push_back(0);
    }
  }
}

void DrawChooser() {
  GuiColorBarHue(chooserSize, "Pick a color", &hueValue);
  GuiCheckBox(lockBoxSize, "Lock", &colorLocked);
}

float *getGrid(int x, int y) { return &grid[width * y + x]; }

void PrintGrid() {
  Vector2 mousePos = GetMousePosition();
  Vector2 mouseCoord = {floorf(mousePos.x / sandsize),
                        floorf(mousePos.y / sandsize)};
  DrawFPS(10, 10);
  DrawText(TextFormat("%2.0f x %2.0f Flow: %d Shader: %s", mouseCoord.x,
                      mouseCoord.y, flowRate,
                      (glowOn ? shader_names[shader_choice] : "None")),
           100, 10, 30, GREEN);
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      if (*getGrid(j, i)) {
        DrawRectangle(j * sandsize, i * sandsize, sandsize, sandsize,
                      ColorFromHSV(*getGrid(j, i), 1.0f, 1.0f));
      }
    }
  }
  if (chooserOpen) {
    DrawChooser();
  }
}

int checkThis(int x, int y) { return *getGrid(x, y); }

int checkDown(int x, int y) { return *getGrid(x, y + 1); }

int checkUp(int x, int y) { return *getGrid(x, y - 1); }

int checkLeft(int x, int y) { return *getGrid(x - 1, y + 1); }

int checkRight(int x, int y) { return *getGrid(x + 1, y + 1); }

void moveDown(int x, int y) {
  *getGrid(x, y + 1) = *getGrid(x, y);
  *getGrid(x, y) = 0;
}

void moveUp(int x, int y) {
  *getGrid(x, y - 1) = *getGrid(x, y);
  *getGrid(x, y) = 0;
}

void moveLeft(int x, int y) {
  *getGrid(x - 1, y + 1) = *getGrid(x, y);
  *getGrid(x, y) = 0;
}

void moveRight(int x, int y) {
  *getGrid(x + 1, y + 1) = *getGrid(x, y);
  *getGrid(x, y) = 0;
}

int checkDir(int x, int y, int dir) { return *getGrid(x + dir, y + 1); }

void moveDir(int x, int y, int dir) {
  *getGrid(x + dir, y + 1) = *getGrid(x, y);
  *getGrid(x, y) = 0;
}

void UpdateGrid() {
  // don't check the bottom row which would be height-1
  int i, j, ystart, yend, ydir, checkdir;
  for (i = height; i >= 0; --i) {
    (rand() % 2 ? (ystart = 0, yend = width, ydir = 1)
                : (ystart = width, yend = -1, ydir = -1));
    for (j = ystart; j != yend; j += ydir) {
      if (checkThis(j, i)) {
        // draw the grain of sand before doing any movements
        DrawRectangle(j * sandsize, i * sandsize, sandsize, sandsize,
                      ColorFromHSV(*getGrid(j, i), 1.0f, 1.0f));
        if (!checkDown(j, i)) {
          // nothing below, move down
          moveDown(j, i);
        } else {
          // hit a grain below, look to the side
          // flowRate / 100 chance to slide left or right
          checkdir = (rand() % 2) ? 1 : -1;
          if (rand() * 100 < flowRate) {
            if (!checkDir(j, i, checkdir)) {
              moveDir(j, i, checkdir);
            } else if (!checkDir(j, i, -checkdir)) {
              moveDir(j, i, -checkdir);
            }
          }
        }
      }
    }
  }
  if (chooserOpen) {
    DrawChooser();
  }
}

bool mouseOverChooser() {
  Vector2 mousePos = GetMousePosition();

  return (CheckCollisionPointRec(mousePos, chooserSize) ||
          CheckCollisionPointRec(mousePos, lockBoxSize));
}

int brushSize = 5;

void SetMouseGrid(float c) {
  Vector2 mousePos = GetMousePosition();
  Vector2 mc = {floorf(mousePos.x / sandsize), floorf(mousePos.y / sandsize)};
  for (int i = -brushSize / 2; i < brushSize / 2; i++) {
    for (int j = -brushSize / 2; j < brushSize / 2; j++) {
      *getGrid(mc.x + i, mc.y + j) = c;
    }
  }
}

void PrintMouse() {
  Vector2 mp = GetMousePosition();
  Vector2 mc = {floorf(mp.x / sandsize), floorf(mp.y / sandsize)};
  if (!chooserOpen || !mouseOverChooser()) {

    DrawRectangle(mc.x * sandsize - (((float)brushSize / 2) * sandsize),
                  mc.y * sandsize - (((float)brushSize / 2) * sandsize),
                  brushSize * sandsize, brushSize * sandsize, GRAY);
  }

  DrawFPS(10, 10);

  DrawText(TextFormat("Flow: %d Shader: %s Press 'C' to open color picker",
                      flowRate,
                      (glowOn ? shader_names[shader_choice] : "None")),
           100, 10, 30, GREEN);
}

void ProcessInput() {
  sandsize += GetMouseWheelMove();
  if (sandsize < 1)
    sandsize = 1;
  if (sandsize > 20)
    sandsize = 20;
  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
      ((chooserOpen && !mouseOverChooser()) || !chooserOpen)) {
    SetMouseGrid(hueValue);
    if (!colorLocked) {

      hueValue += 0.25f;
      if (hueValue > 360.0f)
        hueValue = 1.0f;
    }

    // grid[(width * mouseCoord.y) + mouseCoord.x] = 1;
  } else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) &&
             ((chooserOpen && !mouseOverChooser()) || !chooserOpen)) {
    SetMouseGrid(0);
  }
  if (IsKeyPressed(KEY_RIGHT)) {
    shader_choice++;
    if (shader_choice >= NUM_SHADERS)
      shader_choice = 0;
  }
  if (IsKeyPressed(KEY_LEFT)) {
    shader_choice--;
    if (shader_choice < 0)
      shader_choice = NUM_SHADERS - 1;
  }
  if (IsKeyPressed(KEY_Q)) {
    InitGrid();
  }
  if (IsKeyPressed(KEY_C)) {
    chooserOpen = !chooserOpen;
  }
  if (IsKeyPressed(KEY_G)) {
    glowOn = !glowOn;
  }
  if (IsKeyPressed(KEY_O)) {
    flowRate += 5;
  }
  if (IsKeyPressed(KEY_P)) {
    flowRate -= 5;
  }
  if (IsKeyPressed(KEY_MINUS)) {
    brushSize--;
  }
  if (IsKeyPressed(KEY_EQUAL)) {
    brushSize++;
  }
}

int main() {
  // SetWindowState(FLAG_WINDOW_HIGHDPI);
  InitWindow(1440, 900, "Sandspiel");
  SetWindowState(FLAG_WINDOW_UNDECORATED);
  SetWindowState(FLAG_FULLSCREEN_MODE);
  // SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()) * 2);
  InitGrid();
  LoadShaders();

  RenderTexture2D target =
      LoadRenderTexture(width * sandsize, height * sandsize);

  while (!WindowShouldClose()) {
    BeginTextureMode(target);
    ClearBackground(BLACK);
    ProcessInput();
    UpdateGrid();
    EndTextureMode();

    BeginDrawing();
    if (glowOn) {

      BeginShaderMode(shaders[shader_choice]);
      ClearBackground(BLACK);
    }
    DrawTextureRec(target.texture,
                   (Rectangle){0, 0, (float)target.texture.width,
                               (float)-target.texture.height},
                   (Vector2){0, 0}, WHITE);
    if (glowOn) {

      EndShaderMode();
    }
    //  PrintGrid();
    PrintMouse();
    EndDrawing();
  }
  UnloadRenderTexture(target);
  for (int i = 0; i < NUM_SHADERS; i++) {
    UnloadShader(shaders[i]);
  }
  CloseWindow();
}
