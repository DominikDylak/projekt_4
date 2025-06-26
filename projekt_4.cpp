
#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <algorithm>
#include <ctime>
#include <set>
#include <map>

using namespace Gdiplus;

#pragma comment (lib, "Gdiplus.lib")

struct Passenger {
    int destinationFloor;
    int weight = 70;
};

struct Floor {
    std::vector<Passenger> waitingPassengers;
};

const int FLOOR_COUNT = 5;
Floor buildingFloors[FLOOR_COUNT];
std::map<int, std::vector<std::pair<DWORD, Passenger>>> exitingPassengers;

enum Direction { IDLE, UP, DOWN };

class Elevator {
public:
    int currentFloor = 0;
    std::vector<Passenger> passengers;
    std::vector<int> queue;
    const int maxWeight = 600;
    bool isIdle = false;
    DWORD idleStartTime = 0;
    Direction direction = IDLE;

    int getTotalWeight() {
        int sum = 0;
        for (auto& p : passengers) sum += p.weight;
        return sum;
    }

    int getPassengerCount() {
        return static_cast<int>(passengers.size());
    }

    bool hasWaitingPassengers() {
        for (int i = 0; i < FLOOR_COUNT; ++i) {
            if (!buildingFloors[i].waitingPassengers.empty()) {
                return true;
            }
        }
        return false;
    }

    int findNearestWaitingFloor() {
        int minDistance = FLOOR_COUNT + 1;
        int nearest = -1;
        for (int i = 0; i < FLOOR_COUNT; ++i) {
            if (!buildingFloors[i].waitingPassengers.empty()) {
                int dist = abs(currentFloor - i);
                if (dist < minDistance) {
                    minDistance = dist;
                    nearest = i;
                }
            }
        }
        return nearest;
    }

    void move() {
        if (!queue.empty()) {
            isIdle = false;
            int target = queue.front();

            if (target > currentFloor) {
                currentFloor++;
                direction = UP;
            }
            else if (target < currentFloor) {
                currentFloor--;
                direction = DOWN;
            }
            else {
                direction = IDLE;
            }

            unloadPassengers();
            bool reAdd = loadPassengers();

            if (target == currentFloor) {
                queue.erase(queue.begin());
                if (reAdd && std::find(queue.begin(), queue.end(), currentFloor) == queue.end()) {
                    queue.push_back(currentFloor);
                }
            }

            if (!queue.empty()) {
                if (queue.front() > currentFloor) direction = UP;
                else if (queue.front() < currentFloor) direction = DOWN;
                else direction = IDLE;
            }
            else {
                direction = IDLE;
            }
        }
        else {
            if (passengers.empty()) {
                int nearest = findNearestWaitingFloor();
                if (nearest != -1) {
                    queue.push_back(nearest);
                    direction = nearest > currentFloor ? UP : DOWN;
                    isIdle = false;
                }
                else {
                    if (!isIdle) {
                        idleStartTime = GetTickCount();
                        isIdle = true;
                    }
                    else {
                        if (GetTickCount() - idleStartTime >= 5000 && currentFloor != 0) {
                            queue.push_back(0);
                            direction = currentFloor > 0 ? DOWN : UP;
                        }
                    }
                }
            }
            else {
                isIdle = false;
                direction = passengers.front().destinationFloor > currentFloor ? UP : DOWN;
            }
        }
    }

    void addDestination(int floor) {
        if (std::find(queue.begin(), queue.end(), floor) == queue.end()) {
            queue.push_back(floor);
        }
    }

    void unloadPassengers() {
        std::vector<Passenger> remaining;
        DWORD now = GetTickCount();
        for (auto& p : passengers) {
            if (p.destinationFloor == currentFloor) {
                exitingPassengers[currentFloor].emplace_back(now, p);
            }
            else {
                remaining.push_back(p);
            }
        }
        passengers = remaining;
    }

    bool loadPassengers() {
        auto& waiting = buildingFloors[currentFloor].waitingPassengers;
        bool anyLeft = false;
        for (auto it = waiting.begin(); it != waiting.end();) {
            if (getTotalWeight() + it->weight <= maxWeight) {
                if (direction == IDLE ||
                    (direction == UP && it->destinationFloor > currentFloor) ||
                    (direction == DOWN && it->destinationFloor < currentFloor)) {
                    passengers.push_back(*it);
                    addDestination(it->destinationFloor);
                    it = waiting.erase(it);
                }
                else {
                    ++it;
                }
            }
            else {
                anyLeft = true;
                ++it;
            }
        }
        return anyLeft;
    }
};

Elevator elevator;
const int FLOOR_HEIGHT = 100;
HWND floorButtons[FLOOR_COUNT][FLOOR_COUNT];

void DrawElevator(Graphics& graphics) {
    Pen pen(Color(255, 0, 0, 0));
    SolidBrush brush(Color(255, 100, 100, 255));
    graphics.DrawRectangle(&pen, 100, 50, 100, FLOOR_HEIGHT * FLOOR_COUNT);

    FontFamily fontFamily(L"Arial");
    Font floorFont(&fontFamily, 12);
    SolidBrush labelBrush(Color(255, 0, 0, 0));
    for (int i = 0; i < FLOOR_COUNT; ++i) {
        int yLine = 50 + i * FLOOR_HEIGHT;
        graphics.DrawLine(&pen, 100, yLine, 200, yLine);
        WCHAR floorLabel[16];
        wsprintf(floorLabel, L"%d", FLOOR_COUNT - 1 - i);
        graphics.DrawString(floorLabel, -1, &floorFont, PointF(70.0f, static_cast<REAL>(yLine + 10)), &labelBrush);
    }

    SolidBrush waitingBrush(Color(255, 150, 150, 150));
    for (int i = 0; i < FLOOR_COUNT; ++i) {
        int yBase = 50 + (FLOOR_COUNT - 1 - i) * FLOOR_HEIGHT + 5;
        int xStart = 210;
        int count = static_cast<int>(buildingFloors[i].waitingPassengers.size());
        for (int j = 0; j < count; ++j) {
            graphics.FillEllipse(&waitingBrush, xStart + j * 12, yBase, 10, 10);
        }
    }

    int y = 50 + (FLOOR_COUNT - 1 - elevator.currentFloor) * FLOOR_HEIGHT;
    graphics.FillRectangle(&brush, 100, y, 100, 80);

    SolidBrush inElevatorBrush(Color(255, 50, 100, 50));
    for (size_t i = 0; i < elevator.passengers.size(); ++i) {
        int px = 110 + static_cast<int>(i) % 5 * 12;
        int py = y + 5 + (static_cast<int>(i) / 5) * 12;
        graphics.FillEllipse(&inElevatorBrush, px, py, 10, 10);
    }

    WCHAR countBuf[100];
    wsprintf(countBuf, L"%d os.", elevator.getPassengerCount());
    Font smallFont(&fontFamily, 12);
    SolidBrush countBrush(Color(255, 0, 0, 0));
    graphics.DrawString(countBuf, -1, &smallFont, PointF(110.0f, static_cast<REAL>(y + 20)), &countBrush);

    WCHAR buf[100];
    wsprintf(buf, L"Waga: %d kg", elevator.getTotalWeight());
    Font font(&fontFamily, 16);
    SolidBrush textBrush(Color(255, 0, 0, 0));
    graphics.DrawString(buf, -1, &font, PointF(220.0f, 60.0f), &textBrush);

    SolidBrush exitingBrush(Color(255, 200, 50, 50));
    DWORD now = GetTickCount();
    for (int i = 0; i < FLOOR_COUNT; ++i) {
        int yBase = 50 + (FLOOR_COUNT - 1 - i) * FLOOR_HEIGHT + 5;
        int xStart = 80;
        auto& exiting = exitingPassengers[i];
        int j = 0;
        for (auto it = exiting.begin(); it != exiting.end();) {
            if (now - it->first <= 3000) {
                graphics.FillEllipse(&exitingBrush, xStart - j * 12, yBase, 10, 10);
                ++it;
                ++j;
            }
            else {
                it = exiting.erase(it);
            }
        }
    }
}

void CreateFloorButtons(HWND hWnd) {
    const int baseX = 250;
    const int baseY = 100;
    for (int i = 0; i < FLOOR_COUNT; ++i) {
        wchar_t label[16];
        wsprintf(label, L"Piętro %d", i);
        CreateWindow(L"STATIC", label, WS_VISIBLE | WS_CHILD,
            baseX + i * 100, baseY - 20, 80, 20, hWnd, NULL, NULL, NULL);

        for (int j = 0; j < FLOOR_COUNT; ++j) {
            if (i != j) {
                wsprintf(label, L"Do %d", j);
                floorButtons[i][j] = CreateWindow(L"BUTTON", label,
                    WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                    baseX + i * 100, baseY + j * 30, 80, 25,
                    hWnd, (HMENU)(1000 + i * FLOOR_COUNT + j), NULL, NULL);
            }
        }
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        SetTimer(hWnd, 1, 1000, NULL);
        CreateFloorButtons(hWnd);
        break;
    case WM_TIMER:
        elevator.move();
        InvalidateRect(hWnd, NULL, TRUE);
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) >= 1000 && LOWORD(wParam) < 1000 + FLOOR_COUNT * FLOOR_COUNT) {
            int id = LOWORD(wParam) - 1000;
            int from = id / FLOOR_COUNT;
            int to = id % FLOOR_COUNT;
            buildingFloors[from].waitingPassengers.push_back({ to });
            elevator.addDestination(from);
        }
        break;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        Graphics graphics(hdc);
        graphics.Clear(Color(255, 255, 255));
        DrawElevator(graphics);
        EndPaint(hWnd, &ps);
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"ElevatorWindowClass";

    RegisterClass(&wc);

    HWND hWnd = CreateWindowEx(0, L"ElevatorWindowClass", L"Symulator windy - GDI+", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 650, NULL, NULL, hInstance, NULL);

    ShowWindow(hWnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    GdiplusShutdown(gdiplusToken);
    return 0;
}