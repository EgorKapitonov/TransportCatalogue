#include "map_renderer.h"

namespace map_renderer {

    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    svg::Point SphereProjector::operator()(transport_catalogue::detail::geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }
    /*---------- MapRenderer ----------*/
    MapRenderer::MapRenderer(RenderSettings& render_settings) 
        : render_settings_(render_settings) {}
    RenderSettings MapRenderer::GetRenderSettings() const {
        return render_settings_;
    }
    SphereProjector  MapRenderer::GetSphereProjector(const std::vector<transport_catalogue::detail::geo::Coordinates>& coords) const {
        return SphereProjector(coords.begin(), coords.end(), render_settings_.width, render_settings_.height, render_settings_.padding);
    }
    void MapRenderer::InitSphereProjector(std::vector<transport_catalogue::detail::geo::Coordinates> coords) {
        sphere_projector_ = SphereProjector(coords.begin(), coords.end(), render_settings_.width, render_settings_.height, render_settings_.padding);
    }
    int MapRenderer::GetPaletteSize() const {
        return render_settings_.color_palette.size();
    }
    svg::Color MapRenderer::GetColor(int line_number) const {
        return render_settings_.color_palette[line_number];
    }
    void MapRenderer::SetLineProperties(svg::Polyline& polyline, [[maybe_unused]] int line_number) const {
        using namespace std::literals;

        polyline.SetStrokeColor(GetColor(line_number));
        polyline.SetFillColor("none"s);
        polyline.SetStrokeWidth(render_settings_.line_width);
        polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    }

    /*---Bus Names---*/
    void MapRenderer::SetBusTextCommonProperty(svg::Text& text, const std::string& name, svg::Point position) const {
        using namespace std::literals;

        // Общие свойства обоих объектов:
        text.SetPosition(position);                                 // x и y — координаты соответствующей остановки;
        text.SetOffset({ render_settings_.bus_label_offset.first, 
                        render_settings_.bus_label_offset.second });// смещение dx и dy равно настройке bus_label_offset;
        text.SetFontSize(render_settings_.bus_label_font_size);     // размер шрифта font-size равен настройке bus_label_font_size;
        text.SetFontFamily("Verdana");                              // название шрифта font-family — "Verdana";
        text.SetFontWeight("bold");                                 // толщина шрифта font - weight — "bold".
        text.SetData(name);                                         // содержимое — название автобуса.
        
    }
    void MapRenderer::SetBusTextAdditionalProperty(svg::Text& text, const std::string& name, svg::Point position) const {
        SetBusTextCommonProperty(text, name, position);
 
        // Дополнительные свойства подложки:
        text.SetFillColor(render_settings_.underlayer_color);       // цвет заливки fill равны настройке underlayer_color;
        text.SetStrokeColor(render_settings_.underlayer_color);     // цвет линий stroke равны настройке underlayer_color;
        text.SetStrokeWidth(render_settings_.underlayer_width);     // толщина линий stroke-width равна настройке underlayer_width;
        text.SetStrokeLineCap(svg::StrokeLineCap::ROUND);           // формы конца линии stroke-linecap равны round.
        text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);         // формы соединений stroke-linejoin равны round.
        
    }
    void MapRenderer::SetBusTextColorProperty(svg::Text& text, const std::string& name, int palette, svg::Point position) const {
        SetBusTextCommonProperty(text, name, position);

        // Дополнительное свойство самой надписи:
        text.SetFillColor(GetColor(palette));                       // wвет заливки fill равен цвету соответствующего автобусного маршрута из палитры.
    }

    /*---Stop Symbols---*/
    void MapRenderer::SetStopsCircleProperty(svg::Circle& circle, svg::Point position) const {
        using namespace std::literals;

        // Атрибуты окружности:
        circle.SetCenter(position);                         // координаты центра cx и cy — координаты соответствующей остановки на карте;
        circle.SetRadius(render_settings_.stop_radius);     // радиус r равен настройке stop_radius из словаря render_settings;
        circle.SetFillColor("white");                       // цвет заливки fill — "white".
    }

    /*---Stop Names---*/
    void MapRenderer::SetStopsTextCommonProperty(svg::Text& text, const std::string& name, svg::Point position) const {
        using namespace std::literals;

        // Общие свойства обоих объектов:
        text.SetPosition(position);                                     // x и y — координаты соответствующей остановки;
        text.SetOffset({ render_settings_.stop_label_offset.first,
                         render_settings_.stop_label_offset.second });  // смещение dx и dy равно настройке stop_label_offset;
        text.SetFontSize(render_settings_.stop_label_font_size);        // размер шрифта font-size равен настройке stop_label_font_size;
        text.SetFontFamily("Verdana");                                  // название шрифта font-family — "Verdana";
        text.SetData(name);                                             // содержимое — название остановки.
    }
    void MapRenderer::SetStopsTextAdditionalProperty(svg::Text& text, const std::string& name, svg::Point position) const {
        using namespace std::literals;
        SetStopsTextCommonProperty(text, name, position);

        // Дополнительные свойства подложки:
        text.SetFillColor(render_settings_.underlayer_color);           // цвет заливки fill равны настройке underlayer_color;
        text.SetStrokeColor(render_settings_.underlayer_color);         // цвет линий stroke равны настройке underlayer_color;
        text.SetStrokeWidth(render_settings_.underlayer_width);         // толщина линий stroke-width равна настройке underlayer_width;
        text.SetStrokeLineCap(svg::StrokeLineCap::ROUND);               // формы конца линии stroke-linecap равны "round".
        text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);             // формы соединений stroke-linejoin равны "round".
        
    }
    void MapRenderer::SetStopsTextColorProperty(svg::Text& text, const std::string& name, svg::Point position) const {
        using namespace std::literals;
        SetStopsTextCommonProperty(text, name, position);

        // Дополнительное свойство самой надписи:
        text.SetFillColor("black");                                     // цвет заливки fill — "black".
    }

    void MapRenderer::AddLine(std::vector<std::pair<Bus*, int>>& buses_palette) {
        std::vector<transport_catalogue::detail::geo::Coordinates> stops_coordinates;

        for (auto [bus, palette] : buses_palette) {

            for (Stop* stop : bus->stops_bus) {
                transport_catalogue::detail::geo::Coordinates coordinates;
                coordinates.lat = stop->latitude;
                coordinates.lng = stop->longitude;

                stops_coordinates.push_back(coordinates);
            }

            svg::Polyline bus_line;
            bool bus_empty = true;

            for (auto& coord : stops_coordinates) {
                bus_empty = false;
                bus_line.AddPoint(sphere_projector_(coord));
            }

            if (!bus_empty) {
                SetLineProperties(bus_line, palette);
                map_.Add(bus_line);
            }
            stops_coordinates.clear();
        }
    }
    void MapRenderer::AddBusesName(std::vector<std::pair<Bus*, int>>& buses_palette) {
        std::vector<transport_catalogue::detail::geo::Coordinates> stops_coordinates;
        bool bus_empty = true;

        for (auto [bus, palette] : buses_palette) {

            for (Stop* stop : bus->stops_bus) {
                transport_catalogue::detail::geo::Coordinates coordinates;
                coordinates.lat = stop->latitude;
                coordinates.lng = stop->longitude;

                stops_coordinates.push_back(coordinates);
                if (bus_empty) bus_empty = false;
            }

            svg::Text route_name_roundtrip;
            svg::Text route_title_roundtrip;
            svg::Text route_name_notroundtrip;
            svg::Text route_title_notroundtrip;

            if (!bus_empty) {

                if (bus->is_roundtrip) {
                    SetBusTextAdditionalProperty(route_name_roundtrip, std::string(bus->name_bus), sphere_projector_(stops_coordinates[0]));
                    map_.Add(route_name_roundtrip);

                    SetBusTextColorProperty(route_title_roundtrip, std::string(bus->name_bus), palette, sphere_projector_(stops_coordinates[0]));
                    map_.Add(route_title_roundtrip);
                } else {
                    SetBusTextAdditionalProperty(route_name_roundtrip, std::string(bus->name_bus), sphere_projector_(stops_coordinates[0]));
                    map_.Add(route_name_roundtrip);

                    SetBusTextColorProperty(route_title_roundtrip, std::string(bus->name_bus), palette, sphere_projector_(stops_coordinates[0]));
                    map_.Add(route_title_roundtrip);

                    if (stops_coordinates[0] != stops_coordinates[stops_coordinates.size() / 2]) {
                        SetBusTextAdditionalProperty(route_name_notroundtrip, std::string(bus->name_bus), sphere_projector_(stops_coordinates[stops_coordinates.size() / 2]));
                        map_.Add(route_name_notroundtrip);

                        SetBusTextColorProperty(route_title_notroundtrip, std::string(bus->name_bus), palette, sphere_projector_(stops_coordinates[stops_coordinates.size() / 2]));
                        map_.Add(route_title_notroundtrip);
                    }
                }
            }
            bus_empty = false;
            stops_coordinates.clear();
        }
    }
    void MapRenderer::AddStopsCircle(std::vector<Stop*>& stops_name) {
        std::vector<transport_catalogue::detail::geo::Coordinates> stops_coordinates;
        svg::Circle icon;

        for (Stop* stop_info : stops_name) {

            if (stop_info) {
                transport_catalogue::detail::geo::Coordinates coordinates;
                coordinates.lat = stop_info->latitude;
                coordinates.lng = stop_info->longitude;

                SetStopsCircleProperty(icon,sphere_projector_(coordinates));
                map_.Add(icon);
            }
        }
    }
    void MapRenderer::AddStopsName(std::vector<Stop*>& stops_name) {
        std::vector<transport_catalogue::detail::geo::Coordinates> stops_coordinates;
        svg::Text svg_stop_name;
        svg::Text svg_stop_name_title;

        for (Stop* stop_info : stops_name) {

            if (stop_info) {
                transport_catalogue::detail::geo::Coordinates coordinates;
                coordinates.lat = stop_info->latitude;
                coordinates.lng = stop_info->longitude;

                SetStopsTextAdditionalProperty(svg_stop_name, stop_info->name_stop, sphere_projector_(coordinates));
                map_.Add(svg_stop_name);

                SetStopsTextColorProperty(svg_stop_name_title, stop_info->name_stop, sphere_projector_(coordinates));
                map_.Add(svg_stop_name_title);
            }
        }
    }

    void MapRenderer::GetStreamMap(std::ostream& stream) {
        map_.Render(stream);
    }
}