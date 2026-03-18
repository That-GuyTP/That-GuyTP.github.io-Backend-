// COPYRIGHT Thomas Peterson 2026
#define NOMINMAX
#include <windows.h>

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

#include "dp_program.h"

namespace {

constexpr int kPlayButtonId = 1001;
constexpr UINT_PTR kSimulationTimerId = 1;
constexpr UINT kSimulationFrameMs = 180;

constexpr int kUiMargin = 24;
constexpr int kInfoHeight = 96;

struct CellPos {
  int row;
  int col;
};

enum class Phase {
  Idle,
  Learning,
  Traversing,
  Complete
};

struct AppState {
  std::vector<std::vector<bool>> coins;
  std::vector<std::vector<int>> dp;
  std::vector<CellPos> learning_order;
  std::vector<CellPos> optimal_path;
  std::vector<Move> optimal_moves;
  std::vector<std::vector<bool>> traversed_mask;
  std::vector<std::vector<bool>> final_path_mask;

  CellPos active_learning_cell{-1, -1};
  size_t learning_index = 0;
  size_t traversal_index = 0;
  int coins_collected = 0;
  int optimal_coin_total = 0;
  Phase phase = Phase::Idle;
  std::string status_text = "Press Play to start.";

  HWND play_button = nullptr;
};

AppState* GetState(HWND hwnd) {
  return reinterpret_cast<AppState*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
}

std::vector<std::vector<bool>> BuildCoinMaze() {
  return {
      {true, true, false, true, true, false, true, true, false, true},
      {false, true, true, true, false, true, true, false, true, true},
      {true, false, true, false, true, true, false, true, true, false},
      {true, true, true, true, false, true, true, true, false, true},
      {false, true, false, true, true, false, true, true, true, true},
      {true, true, true, false, true, true, false, true, false, true},
      {true, false, true, true, true, false, true, true, true, true}
  };
}

std::vector<CellPos> BuildLearningOrder(int rows, int cols) {
  std::vector<CellPos> order;
  order.reserve(static_cast<size_t>(rows * cols));

  order.push_back({0, 0});
  for (int r = 1; r < rows; ++r) {
    order.push_back({r, 0});
  }
  for (int c = 1; c < cols; ++c) {
    order.push_back({0, c});
  }
  for (int r = 1; r < rows; ++r) {
    for (int c = 1; c < cols; ++c) {
      order.push_back({r, c});
    }
  }

  return order;
}

std::vector<CellPos> BuildPathFromMoves(const std::vector<Move>& moves) {
  std::vector<CellPos> path;
  path.reserve(moves.size() + 1);
  int row = 0;
  int col = 0;
  path.push_back({row, col});

  for (Move move : moves) {
    if (move == Right) {
      ++col;
    } else {
      ++row;
    }
    path.push_back({row, col});
  }
  return path;
}

void RebuildMasks(AppState* state) {
  const int rows = static_cast<int>(state->coins.size());
  const int cols = static_cast<int>(state->coins[0].size());
  state->traversed_mask.assign(rows, std::vector<bool>(cols, false));
  state->final_path_mask.assign(rows, std::vector<bool>(cols, false));
}

void StartSimulation(AppState* state) {
  const int rows = static_cast<int>(state->coins.size());
  const int cols = static_cast<int>(state->coins[0].size());

  state->dp.assign(rows, std::vector<int>(cols, -1));
  state->learning_order = BuildLearningOrder(rows, cols);
  state->learning_index = 0;
  state->traversal_index = 0;
  state->active_learning_cell = {-1, -1};
  state->optimal_path.clear();
  state->optimal_moves.clear();
  state->coins_collected = 0;
  state->optimal_coin_total = 0;
  state->phase = Phase::Learning;
  state->status_text = "Learning the dynamic-programming table...";
  RebuildMasks(state);
}

void ApplyLearningStep(AppState* state) {
  if (state->learning_index >= state->learning_order.size()) {
    return;
  }

  const CellPos cell = state->learning_order[state->learning_index];
  state->active_learning_cell = cell;

  const int row = cell.row;
  const int col = cell.col;
  const int coin = state->coins[row][col] ? 1 : 0;

  if (row == 0 && col == 0) {
    state->dp[row][col] = coin;
  } else if (row == 0) {
    state->dp[row][col] = state->dp[row][col - 1] + coin;
  } else if (col == 0) {
    state->dp[row][col] = state->dp[row - 1][col] + coin;
  } else {
    state->dp[row][col] =
        std::max(state->dp[row - 1][col], state->dp[row][col - 1]) + coin;
  }

  ++state->learning_index;

  std::ostringstream oss;
  oss << "Learning table cell " << state->learning_index << " / "
      << state->learning_order.size() << "...";
  state->status_text = oss.str();
}

void BeginTraversal(AppState* state) {
  RCS solution = solve_robot_coin(state->coins);
  state->optimal_coin_total = solution.n;
  state->optimal_moves = solution.moves;
  state->optimal_path = BuildPathFromMoves(state->optimal_moves);
  state->phase = Phase::Traversing;
  state->traversal_index = 0;
  state->coins_collected = state->coins[0][0] ? 1 : 0;
  state->active_learning_cell = {-1, -1};

  if (!state->optimal_path.empty()) {
    const CellPos start = state->optimal_path.front();
    state->traversed_mask[start.row][start.col] = true;
  }

  std::ostringstream oss;
  oss << "Traversing optimal path... coins: " << state->coins_collected;
  state->status_text = oss.str();
}

void ApplyTraversalStep(AppState* state) {
  if (state->optimal_path.empty()) {
    state->phase = Phase::Complete;
    state->status_text = "No path available.";
    return;
  }

  if (state->traversal_index + 1 < state->optimal_path.size()) {
    ++state->traversal_index;
    const CellPos cell = state->optimal_path[state->traversal_index];
    state->traversed_mask[cell.row][cell.col] = true;
    if (state->coins[cell.row][cell.col]) {
      ++state->coins_collected;
    }

    std::ostringstream oss;
    oss << "Traversing step " << state->traversal_index << " / "
        << (state->optimal_path.size() - 1) << " | coins: "
        << state->coins_collected;
    state->status_text = oss.str();
    return;
  }

  for (const CellPos& cell : state->optimal_path) {
    state->final_path_mask[cell.row][cell.col] = true;
  }
  state->phase = Phase::Complete;

  std::ostringstream oss;
  oss << "Complete. Final path: " << (state->optimal_path.size() - 1)
      << " steps, " << state->coins_collected << " coins.";
  state->status_text = oss.str();
}

COLORREF BlendToGreen(int dp_value) {
  if (dp_value <= 0) {
    return RGB(238, 238, 238);
  }

  const int capped = std::min(dp_value, 14);
  const int tint = 190 - (capped * 10);
  const int green = 230 - (capped * 4);
  return RGB(tint, green, tint);
}

void DrawLegend(HDC hdc, int left, int top) {
  struct LegendItem {
    COLORREF color;
    const char* text;
  };

  const LegendItem items[] = {
      {RGB(255, 206, 84), "Coin"},
      {RGB(252, 164, 62), "Learning Cell"},
      {RGB(93, 194, 255), "Final Path"},
      {RGB(251, 95, 95), "Robot"}
  };

  int y = top;
  for (const auto& item : items) {
    HBRUSH brush = CreateSolidBrush(item.color);
    RECT box{left, y, left + 16, y + 16};
    FillRect(hdc, &box, brush);
    DeleteObject(brush);

    Rectangle(hdc, box.left, box.top, box.right, box.bottom);
    TextOutA(hdc, left + 24, y + 2, item.text, lstrlenA(item.text));
    y += 22;
  }
}

void DrawMaze(HDC hdc, const RECT& client, const AppState* state) {
  const int rows = static_cast<int>(state->coins.size());
  const int cols = static_cast<int>(state->coins[0].size());
  const int usable_width = (client.right - client.left) - (kUiMargin * 2);
  const int usable_height =
      (client.bottom - client.top) - (kUiMargin * 2) - kInfoHeight;

  const int cell_size = std::max(
      26, std::min(usable_width / cols, usable_height / rows));
  const int grid_width = cell_size * cols;
  const int grid_height = cell_size * rows;

  const int grid_left = (client.right - grid_width) / 2;
  const int grid_top = kUiMargin + kInfoHeight;

  SetBkMode(hdc, TRANSPARENT);

  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < cols; ++c) {
      RECT cell_rect{
          grid_left + (c * cell_size),
          grid_top + (r * cell_size),
          grid_left + ((c + 1) * cell_size),
          grid_top + ((r + 1) * cell_size)
      };

      COLORREF base = RGB(245, 245, 245);
      if (state->dp[r][c] >= 0) {
        base = BlendToGreen(state->dp[r][c]);
      }

      HBRUSH cell_brush = CreateSolidBrush(base);
      FillRect(hdc, &cell_rect, cell_brush);
      DeleteObject(cell_brush);
      Rectangle(hdc, cell_rect.left, cell_rect.top, cell_rect.right,
                cell_rect.bottom);

      if (state->traversed_mask[r][c] && state->phase != Phase::Complete) {
        RECT inner{
            cell_rect.left + 4, cell_rect.top + 4, cell_rect.right - 4,
            cell_rect.bottom - 4
        };
        HBRUSH traversed = CreateSolidBrush(RGB(163, 232, 173));
        FillRect(hdc, &inner, traversed);
        DeleteObject(traversed);
      }

      if (state->final_path_mask[r][c]) {
        RECT inner{
            cell_rect.left + 4, cell_rect.top + 4, cell_rect.right - 4,
            cell_rect.bottom - 4
        };
        HBRUSH path = CreateSolidBrush(RGB(152, 220, 255));
        FillRect(hdc, &inner, path);
        DeleteObject(path);
      }

      if (state->coins[r][c]) {
        const int radius = std::max(5, cell_size / 6);
        const int center_x = (cell_rect.left + cell_rect.right) / 2;
        const int center_y = (cell_rect.top + cell_rect.bottom) / 2;

        HBRUSH coin = CreateSolidBrush(RGB(255, 206, 84));
        HBRUSH old_brush = reinterpret_cast<HBRUSH>(
            SelectObject(hdc, coin));
        Ellipse(hdc, center_x - radius, center_y - radius,
                center_x + radius, center_y + radius);
        SelectObject(hdc, old_brush);
        DeleteObject(coin);
      }

      if (state->dp[r][c] >= 0) {
        std::ostringstream oss;
        oss << state->dp[r][c];
        std::string label = oss.str();
        TextOutA(hdc, cell_rect.left + 3, cell_rect.top + 2, label.c_str(),
                 static_cast<int>(label.size()));
      }

      if (state->active_learning_cell.row == r &&
          state->active_learning_cell.col == c) {
        HPEN pen = CreatePen(PS_SOLID, 3, RGB(252, 164, 62));
        HPEN old_pen = reinterpret_cast<HPEN>(SelectObject(hdc, pen));
        HBRUSH old_brush = reinterpret_cast<HBRUSH>(
            SelectObject(hdc, GetStockObject(HOLLOW_BRUSH)));

        Rectangle(hdc, cell_rect.left + 2, cell_rect.top + 2,
                  cell_rect.right - 2, cell_rect.bottom - 2);

        SelectObject(hdc, old_brush);
        SelectObject(hdc, old_pen);
        DeleteObject(pen);
      }
    }
  }

  // Mark start/end coordinates.
  const char* start_label = "Start [0][0]";
  TextOutA(hdc, grid_left, grid_top - 18, start_label, lstrlenA(start_label));

  std::ostringstream end_text;
  end_text << "Finish [" << (rows - 1) << "][" << (cols - 1) << "]";
  const std::string end_label = end_text.str();
  TextOutA(hdc, grid_left + grid_width - 120, grid_top + grid_height + 4,
           end_label.c_str(), static_cast<int>(end_label.size()));

  if (!state->optimal_path.empty() &&
      (state->phase == Phase::Traversing || state->phase == Phase::Complete)) {
    const CellPos robot = state->optimal_path[state->traversal_index];
    const int cx = grid_left + (robot.col * cell_size) + (cell_size / 2);
    const int cy = grid_top + (robot.row * cell_size) + (cell_size / 2);
    const int radius = std::max(7, cell_size / 4);

    HBRUSH robot_brush = CreateSolidBrush(RGB(251, 95, 95));
    HBRUSH old_brush = reinterpret_cast<HBRUSH>(
        SelectObject(hdc, robot_brush));
    Ellipse(hdc, cx - radius, cy - radius, cx + radius, cy + radius);
    SelectObject(hdc, old_brush);
    DeleteObject(robot_brush);
  }
}

void DrawScene(HDC hdc, const RECT& client, const AppState* state) {
  HBRUSH bg = CreateSolidBrush(RGB(250, 248, 243));
  FillRect(hdc, &client, bg);
  DeleteObject(bg);

  SetBkMode(hdc, TRANSPARENT);

  const std::string title = "Robot Coin Maze (Dynamic Programming Visualizer)";
  TextOutA(hdc, kUiMargin, 18, title.c_str(), static_cast<int>(title.size()));

  std::ostringstream info;
  info << state->status_text;
  const std::string status = info.str();
  TextOutA(hdc, kUiMargin, 48, status.c_str(), static_cast<int>(status.size()));

  const int rows = static_cast<int>(state->coins.size());
  const int cols = static_cast<int>(state->coins[0].size());
  const int shortest_steps = (rows - 1) + (cols - 1);

  std::ostringstream metrics;
  metrics << "Shortest possible steps: " << shortest_steps
          << " | Best coin count: ";
  if (state->phase == Phase::Idle || state->phase == Phase::Learning) {
    metrics << "-";
  } else {
    metrics << state->optimal_coin_total;
  }
  const std::string metrics_text = metrics.str();
  TextOutA(hdc, kUiMargin, 70, metrics_text.c_str(),
           static_cast<int>(metrics_text.size()));

  DrawLegend(hdc, client.right - 180, 24);
  DrawMaze(hdc, client, state);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM w_param,
                            LPARAM l_param) {
  AppState* state = GetState(hwnd);

  switch (message) {
    case WM_CREATE: {
      auto* new_state = new AppState();
      new_state->coins = BuildCoinMaze();
      new_state->dp.assign(new_state->coins.size(),
                           std::vector<int>(new_state->coins[0].size(), -1));
      RebuildMasks(new_state);
      new_state->play_button = CreateWindowExA(
          0, "BUTTON", "Play", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
          kUiMargin, 16, 110, 34, hwnd, reinterpret_cast<HMENU>(kPlayButtonId),
          reinterpret_cast<LPCREATESTRUCT>(l_param)->hInstance, nullptr);

      SetWindowLongPtr(hwnd, GWLP_USERDATA,
                       reinterpret_cast<LONG_PTR>(new_state));
      return 0;
    }

    case WM_COMMAND: {
      if (state == nullptr) {
        return 0;
      }

      if (LOWORD(w_param) == kPlayButtonId && HIWORD(w_param) == BN_CLICKED) {
        StartSimulation(state);
        EnableWindow(state->play_button, FALSE);
        SetTimer(hwnd, kSimulationTimerId, kSimulationFrameMs, nullptr);
        InvalidateRect(hwnd, nullptr, TRUE);
      }
      return 0;
    }

    case WM_TIMER: {
      if (state == nullptr) {
        return 0;
      }

      if (w_param == kSimulationTimerId) {
        if (state->phase == Phase::Learning) {
          if (state->learning_index < state->learning_order.size()) {
            ApplyLearningStep(state);
          } else {
            BeginTraversal(state);
          }
        } else if (state->phase == Phase::Traversing) {
          ApplyTraversalStep(state);
          if (state->phase == Phase::Complete) {
            KillTimer(hwnd, kSimulationTimerId);
            EnableWindow(state->play_button, TRUE);
          }
        }

        InvalidateRect(hwnd, nullptr, TRUE);
      }
      return 0;
    }

    case WM_ERASEBKGND:
      return 1;

    case WM_PAINT: {
      PAINTSTRUCT paint_struct;
      HDC hdc = BeginPaint(hwnd, &paint_struct);
      RECT client;
      GetClientRect(hwnd, &client);

      if (state == nullptr) {
        FillRect(hdc, &client,
                 reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));
        EndPaint(hwnd, &paint_struct);
        return 0;
      }

      HDC mem_dc = CreateCompatibleDC(hdc);
      HBITMAP mem_bitmap = CreateCompatibleBitmap(
          hdc, client.right - client.left, client.bottom - client.top);
      HBITMAP old_bitmap = reinterpret_cast<HBITMAP>(
          SelectObject(mem_dc, mem_bitmap));

      DrawScene(mem_dc, client, state);
      BitBlt(hdc, 0, 0, client.right - client.left, client.bottom - client.top,
             mem_dc, 0, 0, SRCCOPY);

      SelectObject(mem_dc, old_bitmap);
      DeleteObject(mem_bitmap);
      DeleteDC(mem_dc);

      EndPaint(hwnd, &paint_struct);
      return 0;
    }

    case WM_DESTROY:
      KillTimer(hwnd, kSimulationTimerId);
      if (state != nullptr) {
        delete state;
      }
      PostQuitMessage(0);
      return 0;
  }

  return DefWindowProc(hwnd, message, w_param, l_param);
}

}  // namespace

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int show_cmd) {
  const char* class_name = "RobotCoinMazeWindowClass";

  WNDCLASSA window_class{};
  window_class.lpfnWndProc = WindowProc;
  window_class.hInstance = instance;
  window_class.lpszClassName = class_name;
  window_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
  window_class.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

  RegisterClassA(&window_class);

  HWND window = CreateWindowExA(
      0, class_name, "Coin Row Robot", WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT, 980, 760, nullptr, nullptr, instance,
      nullptr);

  if (window == nullptr) {
    return 0;
  }

  ShowWindow(window, show_cmd);
  UpdateWindow(window);

  MSG message;
  while (GetMessage(&message, nullptr, 0, 0) > 0) {
    TranslateMessage(&message);
    DispatchMessage(&message);
  }

  return static_cast<int>(message.wParam);
}
