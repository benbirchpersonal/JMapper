// #include "../dependencies/imgui/imgui.h"
// #include <algorithm>
// #include <string>
// #include <stack>
// #include "storage.h"
// // Define constants for grid size and resolution
// float GRID_SIZE = 1000.0f;  // 100m x 100m grid
// float GRID_RESOLUTION = 10.f;  // 10m per grid unit (fixed resolution)
// float BORDER_THICKNESS = 2.0f;  // Border thickness for the grid


// struct Camera {
// 	ImVec2 position = ImVec2(GRID_SIZE / 2.0f, GRID_SIZE / 2.0f);
// 	float zoom = 3.5f;
// 	float zoom_velocity = 0.0f;  // Zooming velocity
//     ImVec2 target_position = position; // Target position for camera centering

// };

// Camera camera;

// void CenterCameraOnItem(ImVec2 item_center) {
//     // Move the camera towards the target position smoothly
//     camera.target_position = item_center;  // Set the target position to the item center
// }

// void UpdateCameraPosition(float delta_time) {
//     // Lerp the camera position towards the target position with smooth interpolation
//     float lerp_speed = 5.0f;  // Speed of smooth camera movement
//     camera.position = ImVec2(
//         camera.position.x + (camera.target_position.x - camera.position.x) * lerp_speed * delta_time,
//         camera.position.y + (camera.target_position.y - camera.position.y) * lerp_speed * delta_time
//     );

//     // Clamp the camera position to the target position to prevent overshooting
//     if (std::abs(camera.target_position.x - camera.position.x) < 0.01f) {
//         camera.position.x = camera.target_position.x;
//     }

//     if (std::abs(camera.target_position.y - camera.position.y) < 0.01f) {
//         camera.position.y = camera.target_position.y;
//     }


//     if (ImGui::Begin("Helper Window")) {
//         // Display current camera position
//         ImVec2 current_pos = camera.position;
//         ImGui::Text("Current Position: (%.2f, %.2f)", current_pos.x, current_pos.y);
    
//         // Display target camera position
//         ImVec2 target_pos = camera.target_position;
//         ImGui::Text("Target Position: (%.2f, %.2f)", target_pos.x, target_pos.y);
    
//         // Optionally display zoom level
//         ImGui::Text("Zoom: %.2f", camera.zoom);
    
//         // Display selected item I
//             ImGui::Text("Selected Item ID: %d", selected_room_index);

//     }
//     ImGui::End();
    
// }





// ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) {
// 	return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y);
// }

// ImVec2 convert_grid_coord_to_screen_space_tl(ImVec2 input) {
// 	float step = GRID_RESOLUTION * camera.zoom;
// 	ImVec2 top_left = ImVec2(camera.position.x - GRID_SIZE / 2.0f * camera.zoom,
// 		camera.position.y - GRID_SIZE / 2.0f * camera.zoom);

// 	return top_left + ImVec2(input.x * step, input.y * step);
// }

// ImVec2 convert_grid_coord_to_screen_space_tr(ImVec2 input) {
// 	float step = GRID_RESOLUTION * camera.zoom;
// 	ImVec2 top_left = ImVec2(camera.position.x - GRID_SIZE / 2.0f * camera.zoom,
// 		camera.position.y - GRID_SIZE / 2.0f * camera.zoom);

// 	return top_left + ImVec2((input.x + 1) * step, input.y * step);
// }

// ImVec2 convert_grid_coord_to_screen_space_bl(ImVec2 input) {
// 	float step = GRID_RESOLUTION * camera.zoom;
// 	ImVec2 top_left = ImVec2(camera.position.x - GRID_SIZE / 2.0f * camera.zoom,
// 		camera.position.y - GRID_SIZE / 2.0f * camera.zoom);

// 	return top_left + ImVec2(input.x * step, (input.y + 1) * step);
// }

// ImVec2 convert_grid_coord_to_screen_space_br(ImVec2 input) {
// 	float step = GRID_RESOLUTION * camera.zoom;
// 	ImVec2 top_left = ImVec2(camera.position.x - GRID_SIZE / 2.0f * camera.zoom,
// 		camera.position.y - GRID_SIZE / 2.0f * camera.zoom);

// 	return top_left + ImVec2((input.x + 1) * step, (input.y + 1) * step);
// }


// bool is_point_in_room(const std::vector<Point>& coordinates, int x, int y) {
// 	for (const Point& p : coordinates) {
// 		if (p.x == x && p.y == y) {
// 			return true;
// 		}
// 	}
// 	return false;
// }


// ImVec2 calculate_room_center(const Room& room) {
//     float avg_x = 0.0f;
//     float avg_y = 0.0f;
//     for (const Point& p : room.coordinates) {
//         avg_x += static_cast<float>(p.x);
//         avg_y += static_cast<float>(p.y);
//     }
//     avg_x /= room.coordinates.size();
//     avg_y /= room.coordinates.size();

//     return convert_grid_coord_to_screen_space_tl(ImVec2(avg_x, avg_y));  // Return center of room in screen space
// }


// void DrawGridAndRooms(ImDrawList* draw_list, const std::vector<Room>& rooms, const std::vector<Door>& doors, int selectedRoomIDX) {
//     // Apply zoom to resolution
//     float step;
//     if (camera.zoom < 0.5) {
//         step = GRID_RESOLUTION * camera.zoom * 10;
//     }
//     else if (camera.zoom < 1.5) {
//         step = GRID_RESOLUTION * camera.zoom * 5;
//     }
//     else {
//         step = GRID_RESOLUTION * camera.zoom * 1;
//     }

//     ImVec2 top_left = ImVec2(camera.position.x - GRID_SIZE / 2.0f * camera.zoom,
//         camera.position.y - GRID_SIZE / 2.0f * camera.zoom);
//     ImVec2 bottom_right = ImVec2(camera.position.x + GRID_SIZE / 2.0f * camera.zoom,
//         camera.position.y + GRID_SIZE / 2.0f * camera.zoom);

//     // Draw grid lines
//     int linecnt = -1;
//     for (float x = top_left.x; x <= bottom_right.x; x += step) {
//         linecnt++;
//         // Regular 50% opacity lines
//         ImU32 color = IM_COL32(200, 200, 200, 128);
//         draw_list->AddLine(ImVec2(x, top_left.y), ImVec2(x, bottom_right.y), color);

//         // Every 5th line with 75% opacity, relative to the border
//         if (linecnt % 5 == 0) {
//             color = IM_COL32(200, 200, 200, 191);  // 75% opacity
//             draw_list->AddLine(ImVec2(x, top_left.y), ImVec2(x, bottom_right.y), color);
//         }
//     }
//     linecnt = -1;
//     for (float y = top_left.y; y <= bottom_right.y; y += step) {
//         linecnt++;
//         // Regular 50% opacity lines
//         ImU32 color = IM_COL32(200, 200, 200, 128);
//         draw_list->AddLine(ImVec2(top_left.x, y), ImVec2(bottom_right.x, y), color);

//         // Every 5th line with 75% opacity, relative to the border
//         if (linecnt % 5 == 0) {
//             color = IM_COL32(200, 200, 200, 191);  // 75% opacity
//             draw_list->AddLine(ImVec2(top_left.x, y), ImVec2(bottom_right.x, y), color);
//         }
//     }

//     // Draw the thick border around the grid
//     draw_list->AddRect(top_left, bottom_right, IM_COL32(255, 255, 255, 255), 0.0f, 0, 1);



//     // Draw rooms
//     int x = 0;
//     for (const Room& room : rooms) {
//         x++;
//         if (room.coordinates.empty()) continue;

//         // Fill the room squares
//         for (const Point& p : room.coordinates) {
//             ImVec2 top_left_square = convert_grid_coord_to_screen_space_tl(ImVec2(static_cast<float>(p.x), static_cast<float>(p.y)));
//             ImVec2 bottom_right_square = convert_grid_coord_to_screen_space_br(ImVec2(static_cast<float>(p.x), static_cast<float>(p.y)));
//             draw_list->AddRectFilled(top_left_square, bottom_right_square, IM_COL32(100, 100, 100, 230)); // 0.9 opacity
//         }

//         // Draw the room outline by checking neighbors
//         for (const Point& p : room.coordinates) {
//             // Check each neighbor (left, right, up, down)
//             if (ImGui::IsMouseHoveringRect(convert_grid_coord_to_screen_space_bl(ImVec2(p.x, p.y)), convert_grid_coord_to_screen_space_tr(ImVec2(p.x, p.y)))) {
//                 // Get the current mouse position
//                 ImVec2 mouse_pos = ImGui::GetMousePos();

//                 // Offset the position slightly so it's not directly on top of the pointer
//                 ImVec2 text_pos = ImVec2(mouse_pos.x + 10.0f, mouse_pos.y + 10.0f);

//                 // Draw the room's name near the mouse pointer
//                 draw_list->AddText(text_pos, IM_COL32(255, 255, 255, 255), room.name.c_str());
//             }

//             for (const auto& dir : { ImVec2(-1, 0), ImVec2(1, 0), ImVec2(0, -1), ImVec2(0, 1) }) {
//                 int neighbor_x = p.x + static_cast<int>(dir.x);
//                 int neighbor_y = p.y + static_cast<int>(dir.y);

//                 // If the neighbor is not part of the room, draw an edge
//                 if (!is_point_in_room(room.coordinates, neighbor_x, neighbor_y)) {
//                     ImVec2 start, end;

//                     // Determine the correct corners based on direction
//                     if (dir.x == -1) {  // Left
//                         start = convert_grid_coord_to_screen_space_tl(ImVec2(static_cast<float>(p.x), static_cast<float>(p.y))); // Top-left of current
//                         end = convert_grid_coord_to_screen_space_bl(ImVec2(static_cast<float>(p.x), static_cast<float>(p.y))); // Bottom-left of current
//                     }
//                     else if (dir.x == 1) {  // Right
//                         start = convert_grid_coord_to_screen_space_tr(ImVec2(static_cast<float>(p.x), static_cast<float>(p.y))); // Top-right of current
//                         end = convert_grid_coord_to_screen_space_br(ImVec2(static_cast<float>(p.x), static_cast<float>(p.y))); // Bottom-right of current
//                     }
//                     else if (dir.y == -1) {  // Up
//                         start = convert_grid_coord_to_screen_space_tl(ImVec2(static_cast<float>(p.x), static_cast<float>(p.y))); // Top-left of current
//                         end = convert_grid_coord_to_screen_space_tr(ImVec2(static_cast<float>(p.x), static_cast<float>(p.y))); // Top-right of current
//                     }
//                     else if (dir.y == 1) {  // Down
//                         start = convert_grid_coord_to_screen_space_bl(ImVec2(static_cast<float>(p.x), static_cast<float>(p.y))); // Bottom-left of current
//                         end = convert_grid_coord_to_screen_space_br(ImVec2(static_cast<float>(p.x), static_cast<float>(p.y))); // Bottom-right of current
//                     }

//                     // Draw the line representing the boundary
//                     draw_list->AddLine(start, end, IM_COL32(255, 255, 255, 255), 2.0f);  // White outline
//                 }
//             }
//         }

//         // Draw room name at the center of the room
//         if (!room.name.empty()) {

//             // Center the camera on the room's center when selected
//             if(x == selectedRoomIDX){
//                 ImVec2 room_center = calculate_room_center(room);  // Calculate center of room
//                 CenterCameraOnItem(room_center);
//             }
            


//             // Calculate center of the room
//             float avg_x = 0.0f;
//             float avg_y = 0.0f;
//             for (const Point& p : room.coordinates) {
//                 avg_x += static_cast<float>(p.x);
//                 avg_y += static_cast<float>(p.y);
//             }
//             avg_x /= room.coordinates.size();
//             avg_y /= room.coordinates.size();

//             ImVec2 room_center = convert_grid_coord_to_screen_space_tl(ImVec2(avg_x, avg_y));

//             // Adjust the center position for text drawing
//             room_center.x += 5.0f;  // Offset slightly for better visibility
//             room_center.y += 5.0f;

//             // Check if the mouse is hovering over the room
//             ImVec2 room_top_left = convert_grid_coord_to_screen_space_tl(ImVec2(avg_x, avg_y));
//             ImVec2 room_bottom_right = convert_grid_coord_to_screen_space_br(ImVec2(avg_x, avg_y));


//             // Draw the room name
//           //  draw_list->AddText(room_center, IM_COL32(255, 255, 255, 255), room.name.c_str());
//         }
//     }
    

//     // Draw doors as red edges
//     for (const Door& door : doors) {
//         // Convert door locations to screen space
//         ImVec2 loc1_top_left = convert_grid_coord_to_screen_space_tl(ImVec2(static_cast<float>(door.location1.x), static_cast<float>(door.location1.y)));
//         ImVec2 loc1_bottom_right = convert_grid_coord_to_screen_space_br(ImVec2(static_cast<float>(door.location1.x), static_cast<float>(door.location1.y)));
//         ImVec2 loc2_top_left = convert_grid_coord_to_screen_space_tl(ImVec2(static_cast<float>(door.location2.x), static_cast<float>(door.location2.y)));
//         ImVec2 loc2_bottom_right = convert_grid_coord_to_screen_space_br(ImVec2(static_cast<float>(door.location2.x), static_cast<float>(door.location2.y)));

//         // Determine if the door is horizontal or vertical
//         if (door.location1.x == door.location2.x) { // Vertical door (same x-coordinate)
//             // Door is vertical, draw a line along the shared vertical edge
//             ImVec2 start = loc1_bottom_right;  // Bottom-right of the first block
//             ImVec2 end = loc2_top_left;        // Top-left of the second block
//             draw_list->AddLine(start, end, IM_COL32(255, 0, 0, 255), 4.0f);  // Bold red line
//         }
//         else if (door.location1.y == door.location2.y) { // Horizontal door (same y-coordinate)
//             // Door is horizontal, draw a line along the shared horizontal edge
//             ImVec2 start, end;

//             if (door.location1.x < door.location2.x) {
//                 // Door is drawn from left to right
//                 start = convert_grid_coord_to_screen_space_tr(ImVec2(static_cast<float>(door.location1.x), static_cast<float>(door.location1.y)));  // Top-left of the first block
//                 end = convert_grid_coord_to_screen_space_bl(ImVec2(static_cast<float>(door.location2.x), static_cast<float>(door.location2.y)));    // Top-right of the second block
//             }
//             else {
//                 // Door is drawn from right to left
//                 start = convert_grid_coord_to_screen_space_tl(ImVec2(static_cast<float>(door.location1.x), static_cast<float>(door.location1.y)));  // Top-right of the first block
//                 end = convert_grid_coord_to_screen_space_bl(ImVec2(static_cast<float>(door.location2.x), static_cast<float>(door.location2.y)));    // Top-left of the second block
//             }

//             draw_list->AddLine(start, end, IM_COL32(255, 0, 0, 255), 4.0f);  // Bold red line
//         }
//     }
// }



// ImVec2 ProjectMouseToGridSquare() {
//     // Get the mouse position in screen space
//     ImVec2 mouse_pos = ImGui::GetIO().MousePos;

//     // Calculate the offset from the camera's position to the top-left corner of the grid
//     ImVec2 grid_top_left = ImVec2(camera.position.x - GRID_SIZE / 2.0f * camera.zoom,
//         camera.position.y - GRID_SIZE / 2.0f * camera.zoom);

//     // Get the grid step size based on the resolution and zoom level
//     float step = GRID_RESOLUTION * camera.zoom;

//     // Calculate the position of the mouse in grid space
//     ImVec2 mouse_on_grid = ImVec2(mouse_pos.x - grid_top_left.x, mouse_pos.y - grid_top_left.y);

//     // Calculate the indices of the grid square the mouse is hovering over
//     int grid_x = static_cast<int>(mouse_on_grid.x / step);
//     int grid_y = static_cast<int>(mouse_on_grid.y / step);

//     // Return the grid coordinates of the square the mouse is over
//     return ImVec2(grid_x, grid_y);
// }


// // Smooth scrolling with inertia for panning and zooming
// ImVec2 camera_velocity = ImVec2(0.0f, 0.0f);  // Velocity for panning
// float scroll_velocity = 0.0f;  // Velocity for zooming
// float CAMERA_INERTIA = 0.90f;  // Reduced inertia for faster stop
// float SCROLL_INERTIA = 0.75f;  // Reduced inertia for snappier zooming
// float ZOOM_SMOOTHING = 0.2f;  // Increased smoothing for faster zoom response
// float PAN_SPEED_MULTIPLIER = 5;  // 5x speed increase for panning
// float SCROLL_SPEED_MULTIPLIER = 0.2f;  // Lower value for better responsiveness
// // Define the zoom min and max bounds
// const float MIN_ZOOM = 0.2f;
// const float MAX_ZOOM = 10.0f;

// float EaseZoom(float zoom, float delta) {
//     float new_zoom = zoom + (delta * delta * (3.0f - 2.0f * delta)); // Simple cubic easing
//     return std::clamp(new_zoom, MIN_ZOOM, MAX_ZOOM); // Clamp zoom to the specified range
// }
// // Define constants for zooming limit

// void HandleInput() {
//     float delta_time = ImGui::GetIO().DeltaTime;

//     // Panning: Drag with mouse
//     if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && ImGui::GetIO().KeyCtrl) {
//         camera_velocity = ImVec2(ImGui::GetIO().MouseDelta.x * 20.0f, ImGui::GetIO().MouseDelta.y * 20.0f);  // 20x speed increase
//     }
//     else {
//         // Apply inertia to smooth out panning
//         camera_velocity.x *= CAMERA_INERTIA;
//         camera_velocity.y *= CAMERA_INERTIA;
//     }
//     camera.position.x += camera_velocity.x * delta_time * PAN_SPEED_MULTIPLIER;
//     camera.position.y += camera_velocity.y * delta_time * PAN_SPEED_MULTIPLIER;

//     // Center camera on selected room
//     if (selected_room_index != -1) {
//         Room& selected_room = rooms[selected_room_index];
//         ImVec2 room_center = calculate_room_center(selected_room);
//         CenterCameraOnItem(room_center);  // Center camera on the selected room center
//     }

//     // Zooming: Mouse wheel scroll with inertia
//     if (ImGui::GetIO().MouseWheel != 0.0f) {
//         float scroll_input = ImGui::GetIO().MouseWheel;
//         scroll_velocity += scroll_input * SCROLL_SPEED_MULTIPLIER * camera.zoom;
//     }

//     // Apply inertia to zoom
//     camera.zoom_velocity *= ZOOM_SMOOTHING;
//     camera.zoom_velocity += scroll_velocity;
//     camera.zoom += camera.zoom_velocity * delta_time;

//     // Clamp zoom to avoid limits being exceeded
//     if (camera.zoom < MIN_ZOOM) {
//         camera.zoom = MIN_ZOOM;
//         camera.zoom_velocity = 0.0f;  // Reset velocity when reaching min zoom
//     }
//     if (camera.zoom > MAX_ZOOM) {
//         camera.zoom = MAX_ZOOM;
//         camera.zoom_velocity = 0.0f;  // Reset velocity when reaching max zoom
//     }

//     // Reset scroll velocity after applying
//     scroll_velocity = 0.0f;
// }

// void drawGridCornersAndDots(ImDrawList* draw_list, const ImVec2& grid_origin, float grid_size, float zoom_level) {
//     // Calculate top-left and bottom-right corners
//     ImVec2 top_left = grid_origin;
//     ImVec2 bottom_right = grid_origin + ImVec2(grid_size, grid_size);
    
//     // Apply zoom to the corners
//     top_left.x *= zoom_level;
//     top_left.y *= zoom_level;
//     bottom_right.x *= zoom_level;
//     bottom_right.y *= zoom_level;

//     // Draw the green dot at the top-left corner
//     draw_list->AddCircleFilled(top_left, 5.0f, IM_COL32(0, 255, 0, 255)); // Green
    
//     // Draw the red dot at the bottom-right corner
//     draw_list->AddCircleFilled(bottom_right, 5.0f, IM_COL32(255, 0, 0, 255)); // Red
// }





// void DrawMouseGridInfo() {
//     // Get the grid square the mouse is hovering over
//     ImVec2 grid_square = ProjectMouseToGridSquare();

//     // Prepare the grid info text
//     char grid_info[64];
//     snprintf(grid_info, sizeof(grid_info), "Grid: (%d, %d)", static_cast<int>(grid_square.x), static_cast<int>(grid_square.y));

//     // Prepare the camera info text
//     char camera_info[128];
//     snprintf(camera_info, sizeof(camera_info), "Pan: (%.2f, %.2f) | Zoom: %.2f", camera.position.x, camera.position.y, camera.zoom);

//     // Get the current draw list
//     ImDrawList* draw_list = ImGui::GetForegroundDrawList();

//     // Set text color
//     ImU32 text_color = IM_COL32(255, 255, 255, 255); // White color

//     // Define the position for the text in the bottom-left corner
//     ImVec2 text_pos(10.0f, ImGui::GetIO().DisplaySize.y - 50.0f); // Adjust this for margin from screen edges

//     // Measure the text sizes to make a background that fits both
//     ImVec2 grid_text_size = ImGui::CalcTextSize(grid_info);
//     ImVec2 camera_text_size = ImGui::CalcTextSize(camera_info);


//     // Draw the grid info text
//     draw_list->AddText(ImVec2(text_pos.x + 5, text_pos.y + 5), text_color, grid_info);  // Add some padding for the text

//     // Draw the camera info text below the grid info
//     draw_list->AddText(ImVec2(text_pos.x + 5, text_pos.y + 5 + grid_text_size.y + 5.0f), text_color, camera_info);  // Add padding for the second line
// }

// void Render(std::vector<Room> &rooms, std::vector<Door>& doors,int selectedRoomIDX) {
//     ImDrawList* draw_list = ImGui::GetBackgroundDrawList();

//     HandleInput();
//     DrawGridAndRooms(draw_list, rooms, doors, selectedRoomIDX);

//     // Draw the mouse grid info in the bottom-left corner
//     DrawMouseGridInfo();
// }