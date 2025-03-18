#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <set>
#include <stack>
#include <stdio.h>
// Include TinyFileDialogs (if needed for file dialogs)
#include "../dependencies/tinyFileDialogs/tinyFileDialogs.h"
#include "globals.h"

// Basic structure definitions

//
// VALIDATION FUNCTION
//
bool isValid(Room& r) {
    if (r.coordinates.empty()) {
        return false;
    }
    std::set<Point> visited;
    std::stack<Point> to_visit;
    to_visit.push(r.coordinates[0]);
    while (!to_visit.empty()) {
        Point current = to_visit.top();
        to_visit.pop();
        if (visited.find(current) != visited.end())
            continue;
        visited.insert(current);
        static const int directions[4][2] = { {0,1}, {0,-1}, {1,0}, {-1,0} };
        for (int i = 0; i < 4; i++) {
            Point neighbor = { current.x + directions[i][0], current.y + directions[i][1] };
            if (std::find(r.coordinates.begin(), r.coordinates.end(), neighbor) != r.coordinates.end() &&
                visited.find(neighbor) == visited.end()) {
                to_visit.push(neighbor);
            }
        }
    }
    return visited.size() == r.coordinates.size();
}

//
// PARSING FUNCTIONS
//
bool parseRoomDefinitions(const std::string& line, std::vector<Room>& rooms, std::vector<std::string>& categories) {
    size_t nameStart = line.find("\"");
    size_t nameEnd = line.find("\"", nameStart + 1);
    if (nameStart == std::string::npos || nameEnd == std::string::npos)
        return false;

    std::string roomName = line.substr(nameStart + 1, nameEnd - nameStart - 1);

    size_t categoryStart = line.find(":\"", nameEnd);
    size_t categoryEnd = line.find("\"", categoryStart + 2);
    if (categoryStart == std::string::npos || categoryEnd == std::string::npos)
        return false;

    std::string roomCategory = line.substr(categoryStart + 2, categoryEnd - categoryStart - 2);
    
    if (std::find(categories.begin(), categories.end(), roomCategory) == categories.end()) {
        categories.push_back(roomCategory);
    }

    ImVec4 roomColor = ImVec4(1.0f, 1.0f, 1.0f, 0.3f);

    size_t colorStart = line.find("color(", categoryEnd);
    if (colorStart != std::string::npos) {
        size_t colorEnd = line.find(")", colorStart);
        if (colorEnd != std::string::npos) {
            std::string colorString = line.substr(colorStart + 6, colorEnd - colorStart - 6);
            float r, g, b, a;
            if (sscanf(colorString.c_str(), "%f,%f,%f,%f", &r, &g, &b, &a) == 4) {
                roomColor = ImVec4(r, g, b, a);
            }
        }
    }

    Room room(roomName, roomCategory, roomColor);

    size_t coordStart = line.find("[", categoryEnd);
    size_t coordEnd = line.find("]", coordStart);
    if (coordStart == std::string::npos || coordEnd == std::string::npos)
        return false;

    std::string coords = line.substr(coordStart + 1, coordEnd - coordStart - 1);
    size_t pos = 0;
    while ((pos = coords.find("{")) != std::string::npos) {
        size_t endPos = coords.find("}", pos);
        if (endPos == std::string::npos)
            break;
        std::string coord = coords.substr(pos + 1, endPos - pos - 1);
        int x, y;
        if (sscanf(coord.c_str(), "%d,%d", &x, &y) == 2) {
            room.addCoordinate(x, y);
        }
        coords.erase(0, endPos + 1);
    }
    rooms.push_back(room);
    return true;
}


// Parse door definitions: "doorName":"room1"->"room2"[{x1,y1}{x2,y2}]
bool parseDoorDefinitions(const std::string& line, std::vector<Door>& doors) {
    std::istringstream stream(line);
    std::string doorName, room1, room2;
    int x1, y1, x2, y2;
    size_t first_quote = line.find("\"");
    size_t second_quote = line.find("\"", first_quote + 1);
    if (first_quote == std::string::npos || second_quote == std::string::npos)
        return false;
    doorName = line.substr(first_quote + 1, second_quote - first_quote - 1);

    size_t room1_start = line.find("\"", second_quote + 1);
    size_t room1_end = line.find("\"", room1_start + 1);
    if (room1_start == std::string::npos || room1_end == std::string::npos)
        return false;
    room1 = line.substr(room1_start + 1, room1_end - room1_start - 1);

    size_t room2_start = line.find("\"", room1_end + 3);
    size_t room2_end = line.find("\"", room2_start + 1);
    if (room2_start == std::string::npos || room2_end == std::string::npos)
        return false;
    room2 = line.substr(room2_start + 1, room2_end - room2_start - 1);

    size_t coord_start1 = line.find("{", room2_end);
    size_t coord_end1 = line.find("}", coord_start1);
    size_t coord_start2 = line.find("{", coord_end1);
    size_t coord_end2 = line.find("}", coord_start2);
    if (coord_start1 == std::string::npos || coord_end1 == std::string::npos ||
        coord_start2 == std::string::npos || coord_end2 == std::string::npos)
        return false;
    if (sscanf(line.substr(coord_start1 + 1, coord_end1 - coord_start1 - 1).c_str(), "%d,%d", &x1, &y1) != 2)
        return false;
    if (sscanf(line.substr(coord_start2 + 1, coord_end2 - coord_start2 - 1).c_str(), "%d,%d", &x2, &y2) != 2)
        return false;
    doors.push_back(Door(doorName, room1, room2, { x1, y1 }, { x2, y2 }));
    return true;
}

// Parse WAP definitions: "ssid"[{x,y}]
bool parseWAPDefinitions(const std::string& line, std::vector<WAP>& waps) {
    size_t first_quote = line.find("\"");
    size_t second_quote = line.find("\"", first_quote + 1);
    if (first_quote == std::string::npos || second_quote == std::string::npos)
        return false;
    std::string ssid = line.substr(first_quote + 1, second_quote - first_quote - 1);

    size_t open_bracket = line.find("[", second_quote);
    size_t close_bracket = line.find("]", open_bracket);
    if (open_bracket == std::string::npos || close_bracket == std::string::npos)
        return false;
    std::string coords = line.substr(open_bracket + 1, close_bracket - open_bracket - 1);
    size_t pos = 0;
    
    // Fix: Parse the coordinates enclosed in curly braces
    while ((pos = coords.find("{")) != std::string::npos) {
        size_t endPos = coords.find("}", pos);
        if (endPos == std::string::npos)
            break; // Malformed entry
        std::string coord = coords.substr(pos + 1, endPos - pos - 1);
        int x, y;
        if (sscanf(coord.c_str(), "%d,%d", &x, &y) == 2) {
            WAP wap;
            wap.ssid = ssid;
            wap.location = { x, y };
            waps.push_back(wap);
        }
        coords.erase(0, endPos + 1);
    }
    return true;
}

bool parsePRZMAPFile(const std::string& filename, std::vector<Room>& rooms, std::vector<Door>& doors, std::vector<WAP>& waps, std::vector<std::string>& categories) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }
    std::string line;
    bool inRoomDefinitions = false;
    bool inDoorDefinitions = false;
    bool inWAPDefinitions = false;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        if (line.find("RoomDefinitions") != std::string::npos) {
            inRoomDefinitions = true;
            inDoorDefinitions = false;
            inWAPDefinitions = false;
            continue;
        }
        if (line.find("DoorDefinitions") != std::string::npos) {
            inDoorDefinitions = true;
            inRoomDefinitions = false;
            inWAPDefinitions = false;
            continue;
        }
        if (line.find("WAPs:") != std::string::npos) {
            inWAPDefinitions = true;
            inRoomDefinitions = false;
            inDoorDefinitions = false;
            continue;
        }

        if (inRoomDefinitions) {
            if (!parseRoomDefinitions(line, rooms, categories)) {
                std::cerr << "Failed to parse room definition: " << line << std::endl;
                return false;
            }
        } else if (inDoorDefinitions) {
            if (!parseDoorDefinitions(line, doors)) {
                std::cerr << "Failed to parse door definition: " << line << std::endl;
                return false;
            }
        } else if (inWAPDefinitions) {
            if (!parseWAPDefinitions(line, waps)) {
                std::cerr << "Failed to parse WAP definition: " << line << std::endl;
                return false;
            }
        }
    }
    file.close();
    return true;
}


// Generate a .PRZMAP file including the new WAPs section

void generatePRZMAPFile(const std::string& filename, const std::vector<Room>& rooms, const std::vector<Door>& doors, const std::vector<WAP>& waps) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return;
    }
    file << "RoomDefinitions:\n";
    for (const auto& room : rooms) {
        file << "\"" << room.name << "\":\"" << room.category << "\"[";
        for (const auto& coord : room.coordinates) {
            file << "{" << coord.x << "," << coord.y << "}";
        }
        file << "] color(" << room.color.x << "," << room.color.y << "," << room.color.z << "," << room.color.w << ")\n";
    }
    file << "DoorDefinitions:\n";
    for (const auto& door : doors) {
        file << "\"" << door.name << "\":\"" << door.room1 << "\"->\"" << door.room2 << "\"[{" 
             << door.location1.x << "," << door.location1.y << "},{" << door.location2.x << "," << door.location2.y << "}]\n";
    }
    file << "WAPs:\n";
    for (const auto& wap : waps) {
        file << "\"" << wap.ssid << "\"[{" << wap.location.x << "," << wap.location.y << "}]\n";
    }
    file.close();
}

void loadPRZMAPFile(std::vector<Room>& rooms, std::vector<Door>& doors, std::vector<WAP>& waps) {
    // Clear existing data
    rooms.clear();
    doors.clear();
    waps.clear();

    // Use TinyFileDialogs to open a file dialog for .przmap files.
    const char* selectedFile = tinyfd_openFileDialog(
        "Open PRZMAP File",    // Dialog title
        "",                    // Default path (empty for current directory)
        1,                     // Number of filter patterns
        (const char*[]){"*.przmap"},  // Filter pattern(s)
        NULL,                  // No custom description
        0                      // No multi-selection flag
    );

    if (!selectedFile) {
        std::cerr << "No file selected." << std::endl;
        return;
    }

    std::string filename(selectedFile);
    // Call your parsing function
    if (!parsePRZMAPFile(filename, rooms, doors, waps, room_categories)) {
        std::cerr << "Failed to load PRZMAP file: " << filename << std::endl;
    } else {
        std::cout << "Successfully loaded PRZMAP file: " << filename << std::endl;
    }
}

