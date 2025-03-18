#include "../dependencies/imgui/imgui.h"
#include <algorithm>
#include <string>
#include <stack>
#include "storage.h"

// Define constants for grid size and resolution
float GRID_SIZE = 1000.0f;  // 100m x 100m grid
float GRID_RESOLUTION = 10.f;  // 10m per grid unit (fixed resolution)
float BORDER_THICKNESS = 2.0f;  // Border thickness for the grid

struct Camera {
    ImVec2 position = ImVec2(GRID_SIZE / 2.0f, GRID_SIZE / 2.0f);
    float zoom = 3.5f;
    float zoom_velocity = 0.0f;  // Zooming velocity
    ImVec2 target_position = position; // Target position for camera centering
};

Camera camera;

// Transformation matrix to convert grid coordinates to screen space
struct TransformMatrix {
    float scale;
    ImVec2 translation;

    ImVec2 transform(ImVec2 point) const {
        return ImVec2(point.x * scale + translation.x, point.y * scale + translation.y);
    }
};

TransformMatrix get_transform_matrix() {
    float scale = GRID_RESOLUTION * camera.zoom;
    ImVec2 translation = ImVec2(camera.position.x - GRID_SIZE / 2.0f * camera.zoom,
                                camera.position.y - GRID_SIZE / 2.0f * camera.zoom);
    return { scale, translation };
}

void CenterCameraOnItem(ImVec2 item_center) {
    camera.target_position = item_center;
}

ImVec2 InverseProjectWorldToScreen(ImVec2 world_pos) {
    TransformMatrix matrix = get_transform_matrix();

    // Reverse the transformation:
    ImVec2 screen_pos = ImVec2(
        (world_pos.x - matrix.translation.x) * matrix.scale,
        (world_pos.y - matrix.translation.y) * matrix.scale
    );

    return screen_pos;
}


ImVec2 ProjectMouseToGridSquare() {
    ImVec2 mouse_pos = ImGui::GetIO().MousePos;
    TransformMatrix matrix = get_transform_matrix();

    ImVec2 mouse_on_grid = ImVec2(mouse_pos.x - matrix.translation.x, mouse_pos.y - matrix.translation.y);
    int grid_x = static_cast<int>(mouse_on_grid.x / matrix.scale);
    int grid_y = static_cast<int>(mouse_on_grid.y / matrix.scale);

    return ImVec2(grid_x, grid_y);
}


ImVec2 ProjectMouse() {
    ImVec2 mouse_pos = ImGui::GetIO().MousePos;
    TransformMatrix matrix = get_transform_matrix();

    ImVec2 mouse_on_grid = ImVec2(mouse_pos.x - matrix.translation.x, mouse_pos.y - matrix.translation.y);

    return mouse_on_grid;
}

void UpdateCameraPosition(float delta_time) {
    float lerp_speed = 5.0f;
    camera.position = ImVec2(
        camera.position.x + (camera.target_position.x - camera.position.x) * lerp_speed * delta_time,
        camera.position.y + (camera.target_position.y - camera.position.y) * lerp_speed * delta_time
    );

    if (std::abs(camera.target_position.x - camera.position.x) < 0.01f) {
        camera.position.x = camera.target_position.x;
    }

    if (std::abs(camera.target_position.y - camera.position.y) < 0.01f) {
        camera.position.y = camera.target_position.y;
    }

    if (ImGui::Begin("Helper Window")) {
        ImVec2 current_pos = camera.position;
        ImGui::Text("Current Position: (%.2f, %.2f)", current_pos.x, current_pos.y);
    
        ImVec2 target_pos = camera.target_position;
        ImGui::Text("Target Position: (%.2f, %.2f)", target_pos.x, target_pos.y);
    
        ImGui::Text("Zoom: %.2f", camera.zoom);
    
        ImGui::Text("Selected Item ID: %d", selected_room_index);
    }
    ImGui::End();
}

bool is_point_in_room(const std::vector<Point>& coordinates, int x, int y) {
    for (const Point& p : coordinates) {
        if (p.x == x && p.y == y) {
            return true;
        }
    }
    return false;
}

ImVec2 calculate_room_center(const Room& room) {
    float avg_x = 0.0f;
    float avg_y = 0.0f;
    for (const Point& p : room.coordinates) {
        avg_x += static_cast<float>(p.x);
        avg_y += static_cast<float>(p.y);
    }
    avg_x /= room.coordinates.size();
    avg_y /= room.coordinates.size();

    TransformMatrix matrix = get_transform_matrix();
    return matrix.transform(ImVec2(avg_x, avg_y));
}

// New helper function to compute the WAP’s screen position.
ImVec2 CalculateWAPScreenPosition(const WAP& wap, const TransformMatrix& matrix, const std::vector<Room>& rooms) {
    // Compute the grid cell's top-left and bottom-right corners in screen space.
    float cell_tl_x = matrix.transform(ImVec2(static_cast<float>(wap.location.x), static_cast<float>(wap.location.y))).x;
    float cell_tl_y = matrix.transform(ImVec2(static_cast<float>(wap.location.x), static_cast<float>(wap.location.y))).y;
    float cell_br_x = matrix.transform(ImVec2(static_cast<float>(wap.location.x + 1), static_cast<float>(wap.location.y + 1))).x;
    float cell_br_y = matrix.transform(ImVec2(static_cast<float>(wap.location.x + 1), static_cast<float>(wap.location.y + 1))).y;

    // Default: center the WAP (a 4x4 square) in the cell.
    const float wap_size = 4.0f;
    float pos_x = (cell_tl_x + cell_br_x) * 0.5f - wap_size * 0.5f;
    float pos_y = (cell_tl_y + cell_br_y) * 0.5f - wap_size * 0.5f;

    // If this grid cell belongs to a room, try to stick the WAP to a wall.
    // We'll iterate over all rooms; if the wap location is found, check for missing neighbors.
    for (const Room& room : rooms) {
        // Check if this wap's grid coordinate is part of the room.
        Point wap_pt = { wap.location.x, wap.location.y };
        if (std::find(room.coordinates.begin(), room.coordinates.end(), wap_pt) != room.coordinates.end()) {
            // Check for missing neighbors in a priority order: left, top, right, bottom.
            bool left_missing   = !is_point_in_room(room.coordinates, wap.location.x - 1, wap.location.y);
            bool top_missing    = !is_point_in_room(room.coordinates, wap.location.x,     wap.location.y - 1);
            bool right_missing  = !is_point_in_room(room.coordinates, wap.location.x + 1, wap.location.y);
            bool bottom_missing = !is_point_in_room(room.coordinates, wap.location.x,     wap.location.y + 1);
            
            if (left_missing) {
                pos_x = cell_tl_x;  // Stick to left edge.
            } else if (top_missing) {
                pos_y = cell_tl_y;  // Stick to top edge.
            } else if (right_missing) {
                pos_x = cell_br_x - wap_size;  // Stick to right edge.
            } else if (bottom_missing) {
                pos_y = cell_br_y - wap_size;  // Stick to bottom edge.
            }
            break; // Use the first room found that contains this wap.
        }
    }
    return ImVec2(pos_x, pos_y);
}

void DrawGridAndRooms(ImDrawList* draw_list, const std::vector<Room>& rooms, const std::vector<Door>& doors, const std::vector<WAP>& waps, int selectedRoomIDX) {
    TransformMatrix matrix = get_transform_matrix();

    // Calculate the step (grid resolution) based on the zoom level
    float step = GRID_RESOLUTION * camera.zoom;
    if (camera.zoom < 0.5f) {
        step *= 10;
    } else if (camera.zoom < 1.5f) {
        step *= 5;
    } else {
        step *= 1;
    }

    ImVec2 top_left = matrix.transform(ImVec2(0, 0));
    ImVec2 bottom_right = matrix.transform(ImVec2(GRID_SIZE / GRID_RESOLUTION, GRID_SIZE / GRID_RESOLUTION));

    // Draw grid lines with dynamic step
    int linecnt = -1;
    for (float x = top_left.x; x <= bottom_right.x; x += step) {
        linecnt++;
        ImU32 color = IM_COL32(200, 200, 200, grid_opacity * 120);
        draw_list->AddLine(ImVec2(x, top_left.y), ImVec2(x, bottom_right.y), color);
        if (linecnt % 5 == 0) {
            color = IM_COL32(200, 200, 200, grid_opacity * 180);
            draw_list->AddLine(ImVec2(x, top_left.y), ImVec2(x, bottom_right.y), color);
        }
        if (linecnt % 10 == 0) {
            color = IM_COL32(200, 200, 200, grid_opacity * 255);
            draw_list->AddLine(ImVec2(x, top_left.y), ImVec2(x, bottom_right.y), color);
        }
    }
    linecnt = -1;
    for (float y = top_left.y; y <= bottom_right.y; y += step) {
        linecnt++;
        ImU32 color = IM_COL32(200, 200, 200, grid_opacity * 120);
        draw_list->AddLine(ImVec2(top_left.x, y), ImVec2(bottom_right.x, y), color);
        if (linecnt % 5 == 0) {
            color = IM_COL32(200, 200, 200, grid_opacity * 180);
            draw_list->AddLine(ImVec2(top_left.x, y), ImVec2(bottom_right.x, y), color);
        }
        if (linecnt % 10 == 0) {
            color = IM_COL32(200, 200, 200, grid_opacity * 255);
            draw_list->AddLine(ImVec2(top_left.x, y), ImVec2(bottom_right.x, y), color);
        }
    }
    draw_list->AddRect(top_left, bottom_right, IM_COL32(255, 255, 255, 255), 0.0f, 0, 1);

    // Draw rooms
    int x = 0;
    for (const Room& room : rooms) {
        x++;
        if (room.coordinates.empty()) continue;
        for (const Point& p : room.coordinates) {
            ImVec2 tl = matrix.transform(ImVec2(static_cast<float>(p.x), static_cast<float>(p.y)));
            ImVec2 br = matrix.transform(ImVec2(static_cast<float>(p.x + 1), static_cast<float>(p.y + 1)));
            draw_list->AddRectFilled(tl, br, IM_COL32(
                static_cast<int>(room.color.x * 255), 
                static_cast<int>(room.color.y * 255), 
                static_cast<int>(room.color.z * 255), 
                100 // Set alpha as needed
            ));
        }
        for (const Point& p : room.coordinates) {
            if (ImGui::IsMouseHoveringRect(matrix.transform(ImVec2(p.x, p.y)), matrix.transform(ImVec2(p.x + 1, p.y + 1)))) {
                ImVec2 mouse_pos = ImGui::GetMousePos();
                ImVec2 text_pos = ImVec2(mouse_pos.x + 10.0f, mouse_pos.y + 10.0f);
                draw_list->AddText(text_pos, IM_COL32(255, 255, 255, 255), room.name.c_str());
            }
        }

        // Draw the room outline by checking neighbors
for (const Point& p : room.coordinates) {
    // Check each neighbor (left, right, up, down)
    if (ImGui::IsMouseHoveringRect(matrix.transform(ImVec2(p.x, p.y)), matrix.transform(ImVec2(p.x + 1, p.y + 1)))) {
        // Get the current mouse position
        ImVec2 mouse_pos = ImGui::GetMousePos();

        // Offset the position slightly so it's not directly on top of the pointer
        ImVec2 text_pos = ImVec2(mouse_pos.x + 10.0f, mouse_pos.y + 10.0f);

        // Draw the room's name near the mouse pointer
        draw_list->AddText(text_pos, IM_COL32(255, 255, 255, 255), room.name.c_str());
    }

    for (const auto& dir : { ImVec2(-1, 0), ImVec2(1, 0), ImVec2(0, -1), ImVec2(0, 1) }) {
        int neighbor_x = p.x + static_cast<int>(dir.x);
        int neighbor_y = p.y + static_cast<int>(dir.y);

        // If the neighbor is not part of the room, draw an edge
        if (!is_point_in_room(room.coordinates, neighbor_x, neighbor_y)) {
            ImVec2 start, end;

            // Determine the correct corners based on direction
            if (dir.x == -1) {  // Left
                start = matrix.transform(ImVec2(static_cast<float>(p.x), static_cast<float>(p.y))); // Top-left of current
                end = matrix.transform(ImVec2(static_cast<float>(p.x), static_cast<float>(p.y + 1))); // Bottom-left of current
            }
            else if (dir.x == 1) {  // Right
                start = matrix.transform(ImVec2(static_cast<float>(p.x + 1), static_cast<float>(p.y))); // Top-right of current
                end = matrix.transform(ImVec2(static_cast<float>(p.x + 1), static_cast<float>(p.y + 1))); // Bottom-right of current
            }
            else if (dir.y == -1) {  // Up
                start = matrix.transform(ImVec2(static_cast<float>(p.x), static_cast<float>(p.y))); // Top-left of current
                end = matrix.transform(ImVec2(static_cast<float>(p.x + 1), static_cast<float>(p.y))); // Top-right of current
            }
            else if (dir.y == 1) {  // Down
                start = matrix.transform(ImVec2(static_cast<float>(p.x), static_cast<float>(p.y + 1))); // Bottom-left of current
                end = matrix.transform(ImVec2(static_cast<float>(p.x + 1), static_cast<float>(p.y + 1))); // Bottom-right of current
            }

            // Draw the line representing the boundary
            draw_list->AddLine(start, end, IM_COL32(
                static_cast<int>(room.color.x * 255), 
                static_cast<int>(room.color.y * 255), 
                static_cast<int>(room.color.z * 255), 
                255 // Set alpha as needed
            ), 2.0f);  // White outline
        }
    }
}


        ImVec2 room_center = calculate_room_center(room);
        draw_list->AddText(room_center, IM_COL32_WHITE, room.name.c_str());

    }


    // Draw doors
    for (const Door& door : doors) {
        ImVec2 loc1_tl = matrix.transform(ImVec2(door.location1.x, door.location1.y));
        ImVec2 loc1_br = matrix.transform(ImVec2(door.location1.x + 1, door.location1.y + 1));
        ImVec2 loc2_tl = matrix.transform(ImVec2(door.location2.x, door.location2.y));
        ImVec2 loc2_br = matrix.transform(ImVec2(door.location2.x + 1, door.location2.y + 1));
        if (door.location1.x == door.location2.x) {
            ImVec2 start = loc1_br;
            ImVec2 end = loc2_tl;
            draw_list->AddLine(start, end, IM_COL32(255, 0, 0, 255), 4.0f);
        } else if (door.location1.y == door.location2.y) {
            ImVec2 start, end;
            if (door.location1.x < door.location2.x) {
                start = matrix.transform(ImVec2(door.location1.x + 1, door.location1.y));
                end = matrix.transform(ImVec2(door.location2.x, door.location2.y + 1));
            } else {
                start = matrix.transform(ImVec2(door.location1.x, door.location1.y));
                end = matrix.transform(ImVec2(door.location2.x + 1, door.location2.y + 1));
            }
            draw_list->AddLine(start, end, IM_COL32(255, 0, 0, 255), 4.0f);
        }
    }


    for (const WAP& wap : waps) {
        ImVec2 wap_pos = CalculateWAPScreenPosition(wap, matrix, rooms);
        draw_list->AddRectFilled(wap_pos, ImVec2(wap_pos.x + 4.0f, wap_pos.y + 4.0f), IM_COL32(0, 255, 0, 255));
    }
}


void DrawCursorInfo(const std::vector<Room>& rooms, const std::vector<Door>& doors, const std::vector<WAP>& waps) {
    // Get the grid square the mouse is over (in grid coordinates)
    ImVec2 gridSquare = ProjectMouseToGridSquare();
    std::string infoText;

    // Check if the mouse is inside any room
    for (const Room& room : rooms) {
        for (const Point& p : room.coordinates) {
            if (p.x == static_cast<int>(gridSquare.x) && p.y == static_cast<int>(gridSquare.y)) {
                infoText += "Room: " + room.name + " ";
                break;
            }
        }
    }

    // Check for any WAP in this grid square
    for (const WAP& wap : waps) {
        if (wap.location.x == static_cast<int>(gridSquare.x) && wap.location.y == static_cast<int>(gridSquare.y)) {
            infoText += "WAP: " + wap.ssid + " ";
        }
    }

    // Check for any Door whose one endpoint is in this grid square
    for (const Door& door : doors) {
        if ((door.location1.x == static_cast<int>(gridSquare.x) && door.location1.y == static_cast<int>(gridSquare.y)) ||
            (door.location2.x == static_cast<int>(gridSquare.x) && door.location2.y == static_cast<int>(gridSquare.y))) {
            infoText += "Door: " + door.name + " ";
        }
    }

    if (!infoText.empty()) {
        // Get the current mouse position (screen space)
        ImVec2 mousePos = ImGui::GetIO().MousePos;
        // Offset the text slightly from the cursor
        ImVec2 textPos(mousePos.x + 15.0f, mousePos.y + 15.0f);

        // Measure the size of the text
        ImVec2 textSize = ImGui::CalcTextSize(infoText.c_str());
        float padding = 4.0f; // Adjust the padding as desired

        // Create a background rectangle behind the text
        ImVec2 bgMin(textPos.x - padding, textPos.y - padding);
        ImVec2 bgMax(textPos.x + textSize.x + padding, textPos.y + textSize.y + padding);
        ImDrawList* draw_list = ImGui::GetForegroundDrawList();
        draw_list->AddRectFilled(bgMin, bgMax, IM_COL32(0, 0, 0, 150));  // Black background with 150/255 opacity

        // Draw the info text over the background (in yellow)
        draw_list->AddText(textPos, IM_COL32(255, 255, 0, 255), infoText.c_str());
    }
}



// Zoom so that the mouse pointer remains fixed in world space.
void HandleZoomWithMouse(float delta_time) {
    float scroll_input = ImGui::GetIO().MouseWheel;
    if (scroll_input != 0.0f) {
        // Get current mouse position (screen space)
        float s_x = ImGui::GetIO().MousePos.x;
        float s_y = ImGui::GetIO().MousePos.y;

        // Save old zoom and compute old scale (grid units to screen)
        float old_zoom = camera.zoom;
        float scale_old = GRID_RESOLUTION * old_zoom;
        // Compute old translation (top-left of grid in screen space)
        float trans_old_x = camera.position.x - (GRID_SIZE / 2.0f * old_zoom);
        float trans_old_y = camera.position.y - (GRID_SIZE / 2.0f * old_zoom);

        // World coordinate under mouse before zooming: w = (s - translation) / scale_old
        float w_x = (s_x - trans_old_x) / scale_old;
        float w_y = (s_y - trans_old_y) / scale_old;

        // --- Update zoom ---
        float factor = 0.05f;  // sensitivity (adjust for less sensitivity)
        float new_zoom = old_zoom * (1.0f + scroll_input * factor);
        if (new_zoom < MIN_ZOOM) {
            new_zoom = MIN_ZOOM;
        }
        if (new_zoom > MAX_ZOOM) {
            new_zoom = MAX_ZOOM;
        }
        camera.zoom = new_zoom;

        // --- Compute new transformation ---
        float scale_new = GRID_RESOLUTION * new_zoom;
        // New translation should satisfy: w = (s - new_translation) / scale_new
        float trans_new_x = s_x - (w_x * scale_new);
        float trans_new_y = s_y - (w_y * scale_new);

        // The camera position is defined as: position = new_translation + (GRID_SIZE/2 * new_zoom)
        camera.position.x = trans_new_x + (GRID_SIZE / 2.0f * new_zoom);
        camera.position.y = trans_new_y + (GRID_SIZE / 2.0f * new_zoom);
    }
}


void HandleInput() {
    float delta_time = ImGui::GetIO().DeltaTime;

    // ---- Panning ----
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && ImGui::GetIO().KeyCtrl) {
        // Calculate new velocity from mouse delta (component‑wise)
        float new_vel_x = ImGui::GetIO().MouseDelta.x * 20.0f;
        float new_vel_y = ImGui::GetIO().MouseDelta.y * 20.0f;
        camera_velocity.x = new_vel_x;
        camera_velocity.y = new_vel_y;
    } else {
        // Apply inertia (smooth panning)
        camera_velocity.x *= CAMERA_INERTIA;
        camera_velocity.y *= CAMERA_INERTIA;
    }
    camera.position.x += camera_velocity.x * delta_time * PAN_SPEED_MULTIPLIER;
    camera.position.y += camera_velocity.y * delta_time * PAN_SPEED_MULTIPLIER;

    // ---- Center on selected room (if any) ----
    if (selected_room_index != -1 && !rooms.empty()) {
        Room& sel_room = rooms[selected_room_index];
        ImVec2 room_center = calculate_room_center(sel_room);
        CenterCameraOnItem(room_center);
    }

    // ---- Zooming about the mouse pointer ----
    HandleZoomWithMouse(delta_time);
}



void DrawMouseGridInfo() {
    ImVec2 grid_square = ProjectMouseToGridSquare();
    ImVec2 mouse_proj = ProjectMouse();
    char grid_info[64];
    snprintf(grid_info, sizeof(grid_info), "Grid: (%d, %d)", static_cast<int>(grid_square.x), static_cast<int>(grid_square.y));

    char camera_info[128];
    snprintf(camera_info, sizeof(camera_info), "Pan: (%.2f, %.2f) | Zoom: %.2f | cursor: (%.2f, %.2f)", camera.position.x, camera.position.y, camera.zoom,mouse_proj.x, mouse_proj.y );


    ImDrawList* draw_list = ImGui::GetForegroundDrawList();
    ImU32 text_color = IM_COL32(255, 255, 255, 255);
    ImVec2 text_pos(10.0f, ImGui::GetIO().DisplaySize.y - 50.0f);

    ImVec2 grid_text_size = ImGui::CalcTextSize(grid_info);
    ImVec2 camera_text_size = ImGui::CalcTextSize(camera_info);

    draw_list->AddText(ImVec2(text_pos.x + 5, text_pos.y + 5), text_color, grid_info);
    draw_list->AddText(ImVec2(text_pos.x + 5, text_pos.y + 5 + grid_text_size.y + 5.0f), text_color, camera_info);
}

void Render(std::vector<Room> &rooms, std::vector<Door>& doors, std::vector<WAP>& waps ,int selectedRoomIDX) {
    ImDrawList* draw_list = ImGui::GetBackgroundDrawList();

    HandleInput();
    DrawGridAndRooms(draw_list, rooms, doors,waps, selectedRoomIDX);

    DrawMouseGridInfo();
    DrawCursorInfo(rooms, doors, waps);

}