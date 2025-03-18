#include <vector>
#include "../dependencies/imgui/imgui.h"
#include <string>




// Struct to represent a point on the map (coordinates)
struct Point {
    int x = -1;
    int y = -1;

    bool operator<(const Point& other) const {
        return (x < other.x) || (x == other.x && y < other.y);
    }

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

// Room class
class Room {
public:
    std::string name;
    std::string category;
    std::vector<Point> coordinates;
    ImVec4 color {100,100,100,50};

    Room(const std::string& name, const std::string& category, ImVec4 color) : name(name) , category(category), color(color) {}

    void addCoordinate(int x, int y) {
        Point new_point = { x, y };
        if (x < 0 || y < 0) return;

        // Check if the coordinate already exists in the vector
        if (std::find(coordinates.begin(), coordinates.end(), new_point) == coordinates.end()) {
            coordinates.push_back(new_point);  // Add the new point only if it's not a duplicate
        }
    }

    void removeCoordinate(int x, int y) {
        Point target = { x, y };
        coordinates.erase(std::remove(coordinates.begin(), coordinates.end(), target), coordinates.end());
    }

};

// Door class
class Door {
    public:
        std::string name;
        std::string room1;
        std::string room2;
        Point location1;
        Point location2;
    
        Door(const std::string& name, const std::string& room1, const std::string& room2)
            : name(name), room1(room1), room2(room2) {
            location1 = Point();
            location2 = Point();
        }
    
        Door(const std::string& name, const std::string& room1, const std::string& room2,const Point& room1loc, const Point& room2loc)
            : name(name), room1(room1), room2(room2),location1(room1loc), location2(room2loc) {
        }
    
        void addCoordinate(int x, int y) {
            if (location1.x == -1) {
                location1 = { x,y };
            }
            else
                location2 = { x,y };
        }
    
        void removeCoordinates() {
            location1 = { -1,-1 };
            location2 = { -1,-1 };
        }
    };
    // New structure for WAPs
    struct WAP {
        std::string ssid;
        Point location;
    };



std::vector<Room> rooms = std::vector<Room>();
std::vector<Door> doors = std::vector<Door>();
std::vector<WAP> waps = std::vector<WAP>();
std::vector<std::string> room_categories = std::vector<std::string>();
std::string currentRoomName;
char wap_name[64];
ImFont* fnt;

int selected_wap_index;

enum MapState {
    map_addRoom,
    map_addRoomSelectName,
    map_addRoomSelectSquares,
    map_addDoorSelectName,
    map_addDoorSelectSquares,
    map_addWAPSelectName,
    map_addWAPSelectSquare,
    map_editRoom,
    map_editDoor,
    map_moveWAP,
    map_view,
    map_addCategory
};

MapState map_state = map_view;


int selected_room_index = -1; // Holds the index of the selected room
int selected_door_index = -1; // Holds the index of the selected room


// Add these declarations:
ImVec2 camera_velocity = ImVec2(0.0f, 0.0f);  // Velocity for panning
float scroll_velocity = 0.0f;  // Velocity for zooming
float CAMERA_INERTIA = 0.90f;  // Reduced inertia for faster stop
float SCROLL_INERTIA = 0.75f;  // Reduced inertia for snappier zooming
float ZOOM_SMOOTHING = 0.2f;  // Increased smoothing for faster zoom response
float PAN_SPEED_MULTIPLIER = 5.0f;  // 5x speed increase for panning
float SCROLL_SPEED_MULTIPLIER = 0.2f;  // Lower value for better responsiveness
const float MIN_ZOOM = 0.2f;  // Minimum zoom level
const float MAX_ZOOM = 10.0f;  // Maximum zoom level


float grid_opacity;