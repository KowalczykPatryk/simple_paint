#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "imgui.h"
#include "imgui-SFML.h"
#include "ImGuiFileDialog.h"
#include <iostream>
#include <cmath>
#include <cstdint> // for uint8_t to convert from float in ImVec4 to uint8_t for sf::Color

/*
Ideas for further development:
- Add undo/redo functionality
- Add clear canvas after new button pressed
- Two color line is saved two the file correctly with gradient but live preview is not implemented
- Element have to added in reversed order to be drawn correctly (last added on top)
- Implement polimorphism for shapes and store them in one vector
- Add more shapes (triangle, ellipse, polygon)
- Add ability to select, move, delete shapes

*/

struct Line {
    ImVec2 start;
    ImVec2 end;
    ImVec4 borderColor;
    float thickness;
};

struct TwoColorLine{
    ImVec2 start;
    ImVec2 end;
    ImVec4 borderColor;
    float thickness;
    ImVec4 fillColor;

};

struct Rectangle {
    ImVec2 min; // top-left
    ImVec2 max; // bottom-right
    ImVec4 borderColor;
    float thickness;
};

struct FilledRectangle{
    ImVec2 min; // top-left
    ImVec2 max; // bottom-right
    ImVec4 borderColor;
    float thickness;
    ImVec4 fillColor;
};

struct Circle {
    ImVec2 center;
    float radius;
    ImVec4 borderColor;
    ImVec4 fillColor;
    float thickness;
};



class Settings
{
public:
	void set_active_shape(int index);
    bool is_shape_active(int index) const;
    void set_border_color(const ImVec4& color);
    ImVec4& get_border_color();
    void set_fill_color(const ImVec4& color);
    ImVec4& get_fill_color();
    void turn_off_tool_options();
	void turn_on_tool_options();
    bool is_tool_options_shown() const;
    void set_use_border_and_fill_color(bool value);
    bool* get_use_border_and_fill_color();
    void set_thickness_value(int index, float value);
    float* get_thickness_value(int index);
private:
	std::array<bool, 4> shapes_states = { false, false, false, false };
	std::array<float, 3> thickness_values = { 1.0f, 1.0f, 1.0f };
    ImVec4 border_color = { 0.0f, 1.0f, 0.0f, 1.0f };
    ImVec4 fill_color = { 1.0f, 0.0f, 0.0f, 1.0f };
	bool show_tool_options = false;
    bool Use_border_AND_fill_color = false;
};

void Settings::set_active_shape(int index)
{
    for (auto& shape_state : shapes_states)
    {
        shape_state = false;
    }
	shapes_states[index] = true;
}
bool Settings::is_shape_active(int index) const 
{ 
    return shapes_states[index]; 
}
void Settings::set_border_color(const ImVec4& color) 
{ 
    border_color = color; 
}
ImVec4& Settings::get_border_color() 
{ 
    return border_color; 
}
void Settings::set_fill_color(const ImVec4& color) 
{ 
    fill_color = color; 
}
ImVec4& Settings::get_fill_color() 
{ 
    return fill_color; 
}
void Settings::turn_off_tool_options()
{
	show_tool_options = false;
}
void Settings::turn_on_tool_options()
{
	show_tool_options = true;
}
bool Settings::is_tool_options_shown() const
{
	return show_tool_options;
}
void Settings::set_use_border_and_fill_color(bool value) 
{ 
    Use_border_AND_fill_color = value; 
}
bool* Settings::get_use_border_and_fill_color() 
{ 
    return &Use_border_AND_fill_color; 
}
void Settings::set_thickness_value(int index, float value)
{
    thickness_values[index] = value;
}
float* Settings::get_thickness_value(int index)
{ 
    return &(thickness_values[index]); 
}



int main()
{
    sf::RenderWindow window(sf::VideoMode({ 1800,900 }), "Paint", sf::Style::Titlebar | sf::Style::Close);
    sf::Clock deltaClock;

    sf::Texture edit_texture("broom.png");
    sf::Sprite draw_line_sprite(edit_texture);

    sf::Texture square_texture("square.png");
    sf::Sprite draw_square_sprite(square_texture);

    sf::Texture window_texture("window.png");
    sf::Sprite draw_window_sprite(window_texture);

    sf::Texture circle_texture("circle.png");
    sf::Sprite draw_circle_sprite(circle_texture);

    window.setFramerateLimit(60);
    std::ignore = ImGui::SFML::Init(window);

    ImGuiIO& io = ImGui::GetIO();
    ImFont* font = io.Fonts->AddFontFromFileTTF("RobotoLight.ttf", 22.0f);
    std::ignore = ImGui::SFML::UpdateFontTexture();

    Settings settings;

    bool openExit = false;
    bool showAbout = false;

    bool drawing = false;
    ImVec2 start_point;
    ImVec2 current_point;
	std::vector<Line> drawn_lines;
	std::vector<TwoColorLine> drawn_two_color_lines;    
	std::vector<FilledRectangle> drawn_filled_rectangles;
	std::vector<Rectangle> drawn_rectangles;
	std::vector<Circle> drawn_circles;


    //pseudo canvas for drawing shapes for saving later if needed
    sf::RenderTexture renderTexture({ 1800, 870 });// same size as real canvas
    renderTexture.clear(sf::Color::Black);
    
    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            ImGui::SFML::ProcessEvent(window, *event);
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
        }
        ImGui::SFML::Update(window, deltaClock.restart());


        ImGui::SetNextWindowPos(ImVec2(0, 30), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(1800, 870), ImGuiCond_Once);
        ImGui::Begin("canvas", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove| ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar);

                ImVec2 origin = ImGui::GetCursorScreenPos(); 
                ImVec2 canvas_size = ImGui::GetContentRegionAvail();

                ImDrawList* draw_list = ImGui::GetWindowDrawList();

                draw_list->AddRectFilled(origin, ImVec2(origin.x + canvas_size.x, origin.y + canvas_size.y), IM_COL32(0, 0, 0, 255));
                ImGui::InvisibleButton("canvas", canvas_size);

                ImVec2& mouse_pos = ImGui::GetIO().MousePos;


                //each frame this logic checks whether mouse was pressed or/and released and saves the shape coordinates
                if (ImGui::IsItemHovered()) {
                    if (ImGui::IsMouseClicked(0)) {
                        drawing = true;
                        start_point = mouse_pos;
                    }
                    else if (ImGui::IsMouseReleased(0)) {
                        drawing = false;
						current_point = mouse_pos;
                        if (settings.is_shape_active(0) && !settings.get_use_border_and_fill_color()) //line
                        {
							drawn_lines.emplace_back(Line{ start_point, current_point, settings.get_border_color(), *(settings.get_thickness_value(0)) });
                        }
                        else if (settings.is_shape_active(0) && settings.get_use_border_and_fill_color()) //two color line
                        {
							drawn_two_color_lines.emplace_back(TwoColorLine{ start_point, current_point, settings.get_border_color(), *(settings.get_thickness_value(0)), settings.get_fill_color() });
                        }
                        else if (settings.is_shape_active(1)) //rectangle
                        {
                            drawn_rectangles.emplace_back(Rectangle{ start_point, current_point, settings.get_border_color(), *(settings.get_thickness_value(0)) });
                        }
                        else if (settings.is_shape_active(2)) // filled rectangle
                        {
                            drawn_filled_rectangles.emplace_back(FilledRectangle{ start_point, current_point, settings.get_border_color(), *(settings.get_thickness_value(1)), settings.get_fill_color() });
						}
                        else if (settings.is_shape_active(3)) //circle
						{
                            float radius = (float)(sqrt(pow(current_point.x - start_point.x, 2) + pow(current_point.y - start_point.y, 2)) / 2);
							drawn_circles.emplace_back(Circle{ start_point, radius, settings.get_border_color(), settings.get_fill_color(), *(settings.get_thickness_value(2)) });
                        }
                    }
                }

				//each frame this logic draws all the saved shapes
                for (auto& line : drawn_lines) {
					auto [x, y, z, w] = line.borderColor;
                    draw_list->AddLine(line.start, line.end, IM_COL32((int)(x*255), (int)(y*255), (int)(z*255), (int)(w*255)),line.thickness);

                    uint8_t r = static_cast<uint8_t>(line.borderColor.x * 255.0f);
                    uint8_t g = static_cast<uint8_t>(line.borderColor.y * 255.0f);
                    uint8_t b = static_cast<uint8_t>(line.borderColor.z * 255.0f);
                    uint8_t a = static_cast<uint8_t>(line.borderColor.w * 255.0f);

                    // VertexArray z dwoma wierzcho³kami definuj¹cymi liniê
                    sf::VertexArray verts(sf::PrimitiveType::Lines, 2);
                    verts[0].position = sf::Vector2f(line.start.x, line.start.y);
                    verts[0].color = sf::Color(r, g, b, a);
                    verts[1].position = sf::Vector2f(line.end.x, line.end.y);
                    verts[1].color = sf::Color(r, g, b, a);

                    renderTexture.draw(verts);
                }


                for (auto& two_color_line : drawn_two_color_lines) {
                    auto [x, y, z, w] = two_color_line.borderColor;
                    draw_list->AddLine(two_color_line.start, two_color_line.end, IM_COL32((int)(x * 255), (int)(y * 255), (int)(z * 255), (int)(w * 255)), two_color_line.thickness);

                    uint8_t r = static_cast<uint8_t>(two_color_line.borderColor.x * 255.0f);
                    uint8_t g = static_cast<uint8_t>(two_color_line.borderColor.y * 255.0f);
                    uint8_t b = static_cast<uint8_t>(two_color_line.borderColor.z * 255.0f);
                    uint8_t a = static_cast<uint8_t>(two_color_line.borderColor.w * 255.0f);

                    // VertexArray z dwoma wierzcho³kami definuj¹cymi liniê
                    sf::VertexArray verts(sf::PrimitiveType::Lines, 2);
                    verts[0].position = sf::Vector2f(two_color_line.start.x, two_color_line.start.y);
                    verts[0].color = sf::Color(r, g, b, a);
                    verts[1].position = sf::Vector2f(two_color_line.end.x, two_color_line.end.y);

                    r = static_cast<uint8_t>(two_color_line.fillColor.x * 255.0f);
                    g = static_cast<uint8_t>(two_color_line.fillColor.y * 255.0f);
                    b = static_cast<uint8_t>(two_color_line.fillColor.z * 255.0f);
                    a = static_cast<uint8_t>(two_color_line.fillColor.w * 255.0f);

                    verts[1].color = sf::Color(r, g, b, a);

                    renderTexture.draw(verts);
                }


                for (auto& rect : drawn_rectangles) {
                    auto [x, y, z, w] = rect.borderColor;
                    draw_list->AddRect(rect.min, rect.max, IM_COL32((int)(x * 255), (int)(y * 255), (int)(z * 255), (int)(w * 255)),0,0, rect.thickness);

                    sf::RectangleShape rectangle;
                    rectangle.setPosition(sf::Vector2f(rect.min.x, rect.min.y));
                    rectangle.setSize(sf::Vector2f(rect.max.x - rect.min.x, rect.max.y - rect.min.y));
                    rectangle.setFillColor(sf::Color::Transparent);
                    rectangle.setOutlineColor(sf::Color(rect.borderColor.x * 255, rect.borderColor.y * 255, rect.borderColor.z * 255));
                    rectangle.setOutlineThickness(rect.thickness);
                    renderTexture.draw(rectangle);
                }

                for (auto& filled_rect : drawn_filled_rectangles) {
                    auto [x, y, z, w] = filled_rect.borderColor;
                    draw_list->AddRectFilled(filled_rect.min, filled_rect.max, IM_COL32((int)(x * 255), (int)(y * 255), (int)(z * 255), (int)(w * 255)));

                    sf::RectangleShape rectangle;
                    rectangle.setPosition(sf::Vector2f(filled_rect.min.x, filled_rect.min.y));
                    rectangle.setSize(sf::Vector2f(filled_rect.max.x - filled_rect.min.x, filled_rect.max.y - filled_rect.min.y));
                    rectangle.setFillColor(sf::Color(filled_rect.fillColor.x * 255, filled_rect.fillColor.y * 255, filled_rect.fillColor.z * 255));
                    rectangle.setOutlineColor(sf::Color(filled_rect.borderColor.x * 255, filled_rect.borderColor.y * 255, filled_rect.borderColor.z * 255));
                    rectangle.setOutlineThickness(filled_rect.thickness);
                    renderTexture.draw(rectangle);
                }


                for (auto& circle : drawn_circles) {
                    auto [x, y, z, w] = circle.borderColor;
                    draw_list->AddCircle(circle.center, circle.radius, IM_COL32((int)(x * 255), (int)(y * 255), (int)(z * 255), (int)(w * 255)), 0, circle.thickness);

                    sf::CircleShape shape(circle.radius);
                    shape.setPosition({ circle.center.x - circle.radius, circle.center.y - circle.radius });
                    shape.setFillColor(sf::Color(circle.fillColor.x * 255, circle.fillColor.y * 255, circle.fillColor.z * 255));
                    shape.setOutlineColor(sf::Color(circle.borderColor.x * 255, circle.borderColor.y * 255, circle.borderColor.z * 255));
                    shape.setOutlineThickness(circle.thickness);
                    renderTexture.draw(shape);
                }


				//each frame adding for being drawn the preview line while mouse is being held down
                if (drawing) {
                    current_point = mouse_pos;
					auto [x, y, z, w] = settings.get_border_color();
                    if (settings.is_shape_active(0))
                    {
                        draw_list->AddLine(start_point, current_point, IM_COL32((int)(x*255), (int)(y*255), (int)(z*255), (int)(w*255)), 2.0f);
                    }
                    else if (settings.is_shape_active(1))
                    {
                        draw_list->AddRect(start_point, current_point, IM_COL32((int)(x * 255), (int)(y * 255), (int)(z * 255), (int)(w * 255)), 0, 0, *(settings.get_thickness_value(0)));
                    }
                    else if (settings.is_shape_active(2))
                    {
                        draw_list->AddRectFilled(start_point, current_point, IM_COL32((int)(x * 255), (int)(y * 255), (int)(z * 255), (int)(w * 255)));
					}
                    else if (settings.is_shape_active(3))
                    {
                        float radius = (float)(sqrt(pow(current_point.x - start_point.x, 2) + pow(current_point.y - start_point.y, 2)) / 2);
                        draw_list->AddCircle(start_point, radius, IM_COL32((int)(x * 255), (int)(y * 255), (int)(z * 255), (int)(w * 255)), 0, *(settings.get_thickness_value(2)));
					}
                }

        ImGui::End();

        ImGui::PushFont(font);

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New")) 
                {
                
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Open"))
                {
                    ImGuiFileDialog::Instance()->OpenDialog("Open", "Open", ".png");
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Save As.."))
                {
                    ImGuiFileDialog::Instance()->OpenDialog("Save_As", "Save as", ".png");
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) 
                {
					openExit = true;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View"))
            {
				std::string tool_options_label = settings.is_tool_options_shown() ? "Hide Tool Options##View" : "Show Tool Options##View";
                if (ImGui::MenuItem(tool_options_label.c_str())) 
                {
					settings.is_tool_options_shown() ? settings.turn_off_tool_options() : settings.turn_on_tool_options();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Help"))
            {
                if (ImGui::MenuItem("About")) 
                {
					showAbout = true;
                }
                ImGui::EndMenu();
            }
        ImGui::EndMainMenuBar();
        }
        if (openExit) ImGui::OpenPopup("exit_popup");
        ImGui::SetNextWindowPos(ImVec2(675, 337), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(450, 225), ImGuiCond_Always);
        if (ImGui::BeginPopupModal("exit_popup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Are you sure you want to exit?");
            ImGui::Separator();
            if (ImGui::Button("Yes", ImVec2(120, 0)))
                window.close();
            ImGui::SameLine();
            if (ImGui::Button("No", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
                openExit = false;
            }
            ImGui::EndPopup();
        }

        if (showAbout) ImGui::OpenPopup("About Author");
        ImGui::SetNextWindowPos(ImVec2(675, 337), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(450, 225), ImGuiCond_Always);
        if (ImGui::BeginPopupModal("About Author", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Patryk Kowalczyk");
            ImGui::Separator();
            if (ImGui::Button("Ok", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
            showAbout = false;
        }

        ImGui::Begin("Tools");
            if (ImGui::ImageButton("draw_line_button", draw_line_sprite, { 60, 60 }, settings.is_shape_active(0) ? sf::Color::Green : sf::Color::White))
            {
                settings.set_active_shape(0);
            }
            ImGui::SameLine();
            if (ImGui::ImageButton("draw_square_button", draw_square_sprite, { 60, 60 }, settings.is_shape_active(1) ? sf::Color::Green : sf::Color::White))
            {
                settings.set_active_shape(1);
            }
            if (ImGui::ImageButton("draw_window_button", draw_window_sprite, { 60, 60 }, settings.is_shape_active(2) ? sf::Color::Green : sf::Color::White))
            {
                settings.set_active_shape(2);
            }
            ImGui::SameLine();
            if (ImGui::ImageButton("draw_circle_button", draw_circle_sprite, { 60, 60 }, settings.is_shape_active(3) ? sf::Color::Green : sf::Color::White))
            {
                settings.set_active_shape(3);
            }
            ImGui::Separator();
            ImGui::Text("Border COLOR:");
            ImGui::PushID("BorderGroup");
			    std::string border_r_label = "R: " + std::to_string(static_cast<int>(settings.get_border_color().x * 255));
				std::string border_g_label = "G: " + std::to_string(static_cast<int>(settings.get_border_color().y * 255));
				std::string border_b_label = "B: " + std::to_string(static_cast<int>(settings.get_border_color().z * 255));
			    ImGui::Button(border_r_label.c_str(), {60, 30}); ImGui::SameLine();
			    ImGui::Button(border_g_label.c_str(), {60, 30}); ImGui::SameLine();
			    ImGui::Button(border_b_label.c_str(), {60, 30}); ImGui::SameLine();
            ImGui::PopID();
            if (ImGui::ColorButton("border_color", settings.get_border_color(), 0, {30, 30}))
            {
                ImGui::OpenPopup("border_color_popup");
            }
            if (ImGui::BeginPopup("border_color_popup"))
            {
                ImGui::ColorPicker4("BorderColor", (float*)&settings.get_border_color());
				ImGui::EndPopup();
			}
            ImGui::Text("Fill COLOR:");
			ImGui::PushID("FillGroup");
			    std::string fill_r_label = "R: " + std::to_string(static_cast<int>(settings.get_fill_color().x * 255));
				std::string fill_g_label = "G: " + std::to_string(static_cast<int>(settings.get_fill_color().y * 255));
				std::string fill_b_label = "B: " + std::to_string(static_cast<int>(settings.get_fill_color().z * 255));
                ImGui::Button(fill_r_label.c_str(), {60, 30}); ImGui::SameLine();
                ImGui::Button(fill_g_label.c_str(), {60, 30}); ImGui::SameLine();
                ImGui::Button(fill_b_label.c_str(), {60, 30}); ImGui::SameLine();
			ImGui::PopID();
            if (ImGui::ColorButton("fill_color", settings.get_fill_color(), 0, {30,30}))
            {
				ImGui::OpenPopup("fill_color_popup");
            }
            if (ImGui::BeginPopup("fill_color_popup"))
            {
                ImGui::ColorPicker4("FillColor", (float*)&settings.get_fill_color());
				ImGui::EndPopup();
            }
            if (settings.is_tool_options_shown() && (settings.is_shape_active(0)|| settings.is_shape_active(1)|| settings.is_shape_active(2) || settings.is_shape_active(3)))
            {
                ImGui::Separator();
                if (settings.is_shape_active(0))
                {
                    ImGui::Checkbox("Use border AND fill color", settings.get_use_border_and_fill_color());
                }
                else if (settings.is_shape_active(1))
                {
                    ImGui::Text("Rectangle outline thickness");
                    ImGui::SliderFloat("##rectangle", settings.get_thickness_value(0), 1.0f, 10.0f);
                }
                else if (settings.is_shape_active(2))
                {
                    ImGui::Text("Rectangle outline thickness");
                    ImGui::SliderFloat("##filled_rectangle", settings.get_thickness_value(1), 1.0f, 10.0f);
                }
                else if (settings.is_shape_active(3))
                {
                    ImGui::Text("Circle outline thickness");
                    ImGui::SliderFloat("##circle", settings.get_thickness_value(2), 1.0f, 10.0f);
                }
			}
        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(450, 225));
        ImGui::SetNextWindowSize(ImVec2(900, 450));
        if (ImGuiFileDialog::Instance()->Display("Open"))
        {
            if (ImGuiFileDialog::Instance()->IsOk()) std::cout << ImGuiFileDialog::Instance()->GetFilePathName() << std::endl;
            ImGuiFileDialog::Instance()->Close();
        }
        if (ImGuiFileDialog::Instance()->Display("Save_As"))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filename = ImGuiFileDialog::Instance()->GetFilePathName();

                renderTexture.display();
                sf::Image screenshot = renderTexture.getTexture().copyToImage();
                screenshot.saveToFile(filename.c_str());
            }
            ImGuiFileDialog::Instance()->Close();
        }
        ImGui::PopFont();

        window.clear();
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}
