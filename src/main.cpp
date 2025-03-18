#include "glad/glad.h"
#include <GLFW/glfw3.h> // Include glfw3.h after OpenGL headers

#include "../dependencies/imgui/imgui.h"
#include "../dependencies/imgui/backends/imgui_impl_glfw.h"
#include "../dependencies/imgui/backends/imgui_impl_opengl3.h"
#include <iostream>  // For std::cerr and std::endl


#include "../dependencies/tinyFileDialogs/tinyFileDialogs.h"

#include "storage.h"
#include "font.h"
#include "map.h"


void loadTheme();





void HandleFileMenu() {
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New File")) {
            rooms.clear();
            doors.clear();
            waps.clear();
            room_categories.clear();
            currentRoomName = "";
            map_state = map_view;
        }

        if (ImGui::MenuItem("Load PRZMAP")) {
            loadPRZMAPFile(rooms, doors, waps);
        }

        if (ImGui::MenuItem("Save PRZMAP As")) {
            const char* file = tinyfd_saveFileDialog(
                "Save PRZMAP File As",
                "",
                1,  // Filters count
                (const char*[]){"*.przmap"},  // Filters
                NULL  // Default file name
            );

            if (file != NULL) {
                std::string filename(file);
                if (filename.find(".przmap") == std::string::npos) {
                    filename += ".przmap";
                }

                generatePRZMAPFile(filename, rooms, doors,waps);
            }
        }
        ImGui::EndMenu();
    }

    if(ImGui::BeginMenu("Add")){
        if (ImGui::MenuItem("Add Room")) {
            map_state = map_addRoomSelectName;
        }
        if (ImGui::MenuItem("Add Door")) {
            map_state = map_addDoorSelectName;
        }
        if (ImGui::MenuItem("Add WAP")) {
            map_state = map_addWAPSelectName;
        }
        ImGui::EndMenu();

    }
}


ImVec4 currColor;
int selected_category = -1;
void HandleRoomCreation() {
    static char room_name[64] = "";


    switch (map_state) {

    case map_addRoom:
        if (ImGui::Button("Add Room")) {
            map_state = map_addRoomSelectName;
        }
        break;

    case map_addRoomSelectName:
        ImGui::Text("Enter Room Name:");
        ImGui::InputText("Room Name", room_name, IM_ARRAYSIZE(room_name));

        if(ImGui::BeginCombo("Room Category", selected_category == -1 ? "Select Category" : room_categories[selected_category].c_str())){
            for (int i = 0; i < (int)room_categories.size(); i++) {
                bool is_selected = (selected_category == i);
                if (ImGui::Selectable(room_categories[i].c_str(), is_selected)) {
                    selected_category = i;
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        
        ImGui::ColorPicker4("Room color", (float*)&currColor.x);
        if (ImGui::Button("Confirm Room Name and Color")) {
            Room new_room(std::string(room_name), room_categories[selected_category].c_str(),currColor);
            rooms.push_back(new_room);
            currentRoomName = room_name;
            map_state = map_addRoomSelectSquares;
        }
        break;

    case map_addRoomSelectSquares:
        ImGui::Text("Select Room Squares");

        if (ImGui::Button("Finish Room") && isValid(rooms.back())) {
            map_state = map_view;
            break;
        }
        if (ImGui::Button("cancel")) {
            rooms.erase(rooms.end() - 1);
            map_state = map_view;
            break;
        }

        if (ImGui::GetIO().KeyCtrl)
            break;

        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) || ImGui::IsWindowFocused(ImGuiHoveredFlags_AnyWindow))
            break;



        if (ImGui::GetIO().MouseDown[ImGuiMouseButton_Left]) {
            ImVec2 currSquare = ProjectMouseToGridSquare();
            rooms.back().addCoordinate(currSquare.x, currSquare.y);
        }
        if (ImGui::GetIO().MouseDown[ImGuiMouseButton_Right]) {
            ImVec2 currSquare = ProjectMouseToGridSquare();
            rooms.back().removeCoordinate(currSquare.x, currSquare.y);
        }
       
        break;
    }
}

void DrawRoomSelector() {
    ImGui::Text("Select Room");
    for (int i = 0; i < (int)rooms.size(); i++) {
        if (ImGui::Selectable(rooms[i].name.c_str(), selected_room_index == i)) {
            selected_room_index = i;
        }

        if (ImGui::BeginPopupContextItem()) {  // Right-click context menu
            if (ImGui::MenuItem("Edit Room")) {
                selected_room_index = i;
                map_state = map_editRoom;
            }
            if (ImGui::MenuItem("Delete Room")) {
                rooms.erase(rooms.begin() + i);
                if (selected_room_index == i) selected_room_index = -1;
            }
            ImGui::EndPopup();
        }
    }
}


void DrawDoorSelector() {
    ImGui::Text("Select Door");
    for (int i = 0; i < (int)doors.size(); i++) {
        if (ImGui::Selectable(doors[i].name.c_str(), selected_door_index == i)) {
            selected_door_index = i;
        }

        if (ImGui::BeginPopupContextItem()) {  // Right-click context menu
            if (ImGui::MenuItem("Edit Door")) {
                selected_door_index = i;
                map_state = map_editDoor;
            }
            if (ImGui::MenuItem("Delete Door")) {
                doors.erase(doors.begin() + i);
                if (selected_door_index == i) selected_door_index = -1;
            }
            ImGui::EndPopup();
        }
    }
}


void HandleDoorCreation() {
    static char door_name[64] = "";
    static int selected_room1_index = -1;
    static int selected_room2_index = -1;

    switch (map_state) {
    case map_addDoorSelectName:
        ImGui::Text("Enter Door Name:");
        ImGui::InputText("Door Name", door_name, IM_ARRAYSIZE(door_name));

        if (rooms.empty()) {
            ImGui::Text("No rooms available.");
        }
        else {
            std::vector<const char*> room_names;
            for (const auto& room : rooms) {
                room_names.push_back(room.name.c_str());
            }

            ImGui::Combo("Room 1", &selected_room1_index, room_names.data(), room_names.size());
            ImGui::Combo("Room 2", &selected_room2_index, room_names.data(), room_names.size());

            if (ImGui::Button("Confirm Door Name") && selected_room1_index != selected_room2_index) {
                doors.emplace_back(door_name, rooms[selected_room1_index].name, rooms[selected_room2_index].name);
                map_state = map_addDoorSelectSquares;
            }

            if (ImGui::Button("cancel")) {
                map_state = map_view;
                break;
            }
        }
        break;

    case map_addDoorSelectSquares:
        ImGui::Text("Select Two Squares for Door");
        if (ImGui::Button("Finish Door") && doors.back().location1.x != -1 && doors.back().location2.x != -1) {
            map_state = map_view;
            break;
        }
        if (ImGui::Button("cancel")) {
            doors.erase(doors.end() - 1);
            map_state = map_view;
            break;
        }
        if (ImGui::GetIO().KeyCtrl)
            break;

        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) || ImGui::IsWindowFocused(ImGuiHoveredFlags_AnyWindow))
            break;

        if (ImGui::GetIO().MouseDown[ImGuiMouseButton_Left]) {
            ImVec2 currSquare = ProjectMouseToGridSquare();
            if (currSquare.x >= 0 && currSquare.y >= 0) {
                doors.back().addCoordinate(currSquare.x, currSquare.y);
            }
        }

        if (ImGui::GetIO().MouseDown[ImGuiMouseButton_Right]) {
            doors.back().removeCoordinates();
        }

        
        break;
    }
}

void HandleRoomEditing(int selected_room_index) {

    if (selected_room_index >= 0 && selected_room_index < rooms.size()) {
        Room& selectedRoom = rooms[selected_room_index];

        static char new_room_name[64];
        strcpy(new_room_name, selectedRoom.name.c_str());

        ImGui::InputText("Room Name", new_room_name, IM_ARRAYSIZE(new_room_name));
        ImGui::ColorPicker4("Room color", (float*)&rooms[selected_room_index].color.x);
        if (ImGui::Button("Update Name")) {
            selectedRoom.name = new_room_name;
        }

        ImGui::Text("Edit Room Coordinates");


        if(!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) && !ImGui::IsWindowFocused(ImGuiHoveredFlags_AnyWindow)) {
            if (ImGui::GetIO().MouseDown[ImGuiMouseButton_Left]) {
                ImVec2 currSquare = ProjectMouseToGridSquare();
                selectedRoom.addCoordinate(currSquare.x, currSquare.y);
            }
    
            if (ImGui::GetIO().MouseDown[ImGuiMouseButton_Right]) {
                ImVec2 currSquare = ProjectMouseToGridSquare();
                selectedRoom.removeCoordinate(currSquare.x, currSquare.y);
            }

        }
        

        if (ImGui::Button("Delete Room")) {
            rooms.erase(rooms.begin() + selected_room_index);
            selected_room_index = -1;
        }
    }

    if(ImGui::Button("Done"))
        map_state = map_view;
}

void HandleDoorEditing(int selected_door_index) {

    if (selected_door_index >= 0 && selected_door_index < doors.size()) {
        Door& selectedDoor = doors[selected_door_index];

        static char new_door_name[64];
        strcpy(new_door_name, selectedDoor.name.c_str());

        ImGui::InputText("Door Name", new_door_name, IM_ARRAYSIZE(new_door_name));
        if (ImGui::Button("Update Name")) {
            selectedDoor.name = new_door_name;
        }

        ImGui::Text("Edit Door Location");


        if(!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) && !ImGui::IsWindowFocused(ImGuiHoveredFlags_AnyWindow)) {
            if (ImGui::GetIO().MouseDown[ImGuiMouseButton_Left]) {
                ImVec2 currSquare = ProjectMouseToGridSquare();
                selectedDoor.addCoordinate(currSquare.x, currSquare.y);
            }
    
            if (ImGui::GetIO().MouseDown[ImGuiMouseButton_Right]) {
                selectedDoor.removeCoordinates();
            }
        }

       
        if (ImGui::Button("Delete Door")) {
            doors.erase(doors.begin() + selected_door_index);
            selected_door_index = -1;
        }

        
    }

    if(ImGui::Button("Done"))
        map_state = map_view;
}


void DrawWAPSelector() {
    if (!waps.empty()) {
        ImGui::Text("Select WAP");
        for (int i = 0; i < (int)waps.size(); i++) {
            if (ImGui::Selectable(waps[i].ssid.c_str(), selected_wap_index == i)) {
                selected_wap_index = i;
            }

            if (ImGui::BeginPopupContextItem()) {  // Right-click context menu
                if (ImGui::MenuItem("move WAP")) {
                    selected_wap_index = i;
                    map_state = map_moveWAP;
                }
                if (ImGui::MenuItem("Delete WAP")) {
                    waps.erase(waps.begin() + i);
                    if (selected_wap_index == i) selected_wap_index = -1;
                }
                ImGui::EndPopup();
            }
        }
    }
}

void DrawViewMenu(){
    if (ImGui::BeginMenu("View")) {
        ImGui::SliderFloat("Grid Opacity",&grid_opacity, 0,1);
    
        ImGui::EndMenu();
    }
}


char category_name[100];
bool categoryNameInputOpen = false;
void DrawUI() {
    // Static variable for WAP name used during new WAP creation.
    static char wap_name[64] = "";

    // Draw toolbar
    if (ImGui::BeginMainMenuBar()) {
        HandleFileMenu();
        DrawViewMenu();
        ImGui::EndMainMenuBar();
    }

    ImGui::Begin("Map Editor");
    
    // --- Top Toolbar ---
    if (map_state == map_view) {
        // Room category combo box with + and - buttons
        ImGui::Text("Room Categories");
        if (ImGui::Button("+")) {
            categoryNameInputOpen = true;
            ImGui::OpenPopup("Add Category");  // Open popup when button is clicked
        }
    
        // Make popup to choose category name
        if (ImGui::BeginPopupModal("Add Category", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Enter Category Name:");
            ImGui::InputText("Category Name", (char*)category_name, IM_ARRAYSIZE(category_name));
            if (ImGui::Button("OK", ImVec2(120, 0))) { 
                room_categories.push_back(category_name);
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    
        if (ImGui::BeginListBox("Room Categories")) {
            for (int i = 0; i < (int)room_categories.size(); i++) {
                if (ImGui::Selectable(room_categories[i].c_str(), selected_category == i)) {
                    selected_category = i;
                }
                if (ImGui::BeginPopupContextItem()) {  // Right-click context menu
                    if (ImGui::MenuItem("Edit Category")) {
                        selected_category = i;
                    }
                    if (ImGui::MenuItem("Delete Category")) {
                        room_categories.erase(room_categories.begin() + i);
                        if (selected_category == i) selected_category = -1;
                    }
                    ImGui::EndPopup();
                }
            }
            ImGui::EndListBox();
        }
    }
    

    // --- Branch based on map state ---
    switch (map_state) {
        // Room creation states
        case map_addRoomSelectName:
        case map_addRoomSelectSquares:
            HandleRoomCreation();
            break;
        case map_editRoom:
            HandleRoomEditing(selected_room_index); // Pass the selected room index
            break;

        // Door creation states
        case map_addDoorSelectName:
        case map_addDoorSelectSquares:
            HandleDoorCreation();
            break;
        case map_editDoor:
            HandleDoorEditing(selected_door_index); // Pass the selected door index
            break;

        // WAP creation states
        case map_addWAPSelectName: {
            ImGui::Text("Enter WAP SSID:");
            ImGui::InputText("WAP SSID", wap_name, IM_ARRAYSIZE(wap_name));
            if (ImGui::Button("Confirm WAP Name")) {
                map_state = map_addWAPSelectSquare;
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                map_state = map_view;
            }
            break;
        }
        case map_addWAPSelectSquare: {
            ImGui::Text("Click on the map to place the WAP");
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                ImVec2 gridSquare = ProjectMouseToGridSquare();
                WAP newWap;
                newWap.ssid = std::string(wap_name);
                newWap.location.x = static_cast<int>(gridSquare.x);
                newWap.location.y = static_cast<int>(gridSquare.y);
                waps.push_back(newWap);
                map_state = map_view;
            }
            if (ImGui::Button("Cancel")) {
                map_state = map_view;
            }
            break;
        }
        case map_moveWAP: {
            ImGui::Text("Click on the map to move the selected WAP");
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                ImVec2 gridSquare = ProjectMouseToGridSquare();
                if (selected_wap_index >= 0 && selected_wap_index < (int)waps.size()) {
                    waps[selected_wap_index].location.x = static_cast<int>(gridSquare.x);
                    waps[selected_wap_index].location.y = static_cast<int>(gridSquare.y);
                    map_state = map_view;
                }
            }
            if (ImGui::Button("Cancel")) {
                map_state = map_view;
            }
            break;
        }

        default:
            break;
    }

    // --- In View Mode, show selectors ---
    if (map_state == map_view) {
        DrawRoomSelector(); // Draw the room selector
        ImGui::Separator();
        DrawDoorSelector(); // Draw the door selector
        ImGui::Separator();
        DrawWAPSelector();  // Draw the WAP selector
    }

    ImGui::End();
}

// Main code
int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);



    // Create a windowed mode window and OpenGL context
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "JMapper", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader (e.g. GLAD)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize OpenGL loader!" << std::endl;
        return -1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;


    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");


    ImVec2 screen_size = ImGui::GetIO().DisplaySize;
    ImVec2 window_pos((screen_size.x / 5) * 4, 0.0f);
    ImVec2 window_size(screen_size.x / 5.0f, screen_size.y);
    ImGui::SetNextWindowPos(window_pos);
    ImGui::SetNextWindowSize(window_size);
    loadTheme();
    // Main loop




    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Poll and handle events
        glfwPollEvents();

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        Render(rooms, doors, waps, selected_room_index);
        
        DrawUI();

        // Rendering
        ImGui::Render();
        glClearColor(0.06f, 0.05f, 0.07f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
void loadTheme() {
    ImGuiStyle* style = &ImGui::GetStyle();
    fnt = ImGui::GetIO().Fonts->AddFontFromMemoryTTF((void*)Montserrat_Medium, 330872, 18);
    ImGui::GetIO().Fonts->Build();

    style->WindowPadding = ImVec2(15, 15);
    style->WindowRounding = 5.0f;
    style->FramePadding = ImVec2(5, 5);
    style->FrameRounding = 4.0f;
    style->ItemSpacing = ImVec2(12, 8);
    style->ItemInnerSpacing = ImVec2(8, 6);
    style->IndentSpacing = 25.0f;
    style->ScrollbarSize = 15.0f;
    style->ScrollbarRounding = 9.0f;
    style->GrabMinSize = 5.0f;
    style->GrabRounding = 3.0f;

    style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
    style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.0f);
    style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.06f, 0.05f, 0.07f, 0.70f);

    style->Colors[ImGuiCol_PopupBg] = ImVec4(0.06f, 0.05f, 0.07f, 0.70f);
    style->Colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.f);
    style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
    style->Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_TitleBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);

    style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_Header] = ImVec4(0.12f, 0.11f, 0.14f, 1.00f);  // Default background
    style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.25f, 0.29f, 1.00f);  // Hovered with border
    style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);  // Active (selected) background
    
    // Border for selected headers
    style->FrameRounding = 5.0f;
    style->Colors[ImGuiCol_Border] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);  // Lighter border
    
    // More subtle hover effects for list items
    style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    
    // Update selected graph plot styles
    style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
}
