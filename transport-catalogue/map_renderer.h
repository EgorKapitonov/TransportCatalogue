#pragma once

#include "geo.h"
#include "svg.h"
#include "domain.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>

using namespace transport_catalogue;

namespace map_renderer {

    inline const double EPSILON = 1e-6;

    bool IsZero(double value);

    class SphereProjector {
    public:
        
        SphereProjector() = default;

        // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width, double max_height, double padding);

        // Проецирует широту и долготу в координаты внутри SVG-изображения
        svg::Point operator()(transport_catalogue::detail::geo::Coordinates coords) const;

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

    // Словарь для управления визуализацией карты 
    struct RenderSettings {
        double width;                                           // Ширина изображения в пикселях.
        double height;                                          // Высота изображения в пикселях.
        double padding;                                         // Отступ краёв карты от границ SVG-документа.
        double line_width;                                      // Толщина линий, которыми рисуются автобусные маршруты.
        double stop_radius;                                     // Радиус окружностей, которыми обозначаются остановки.
        int bus_label_font_size;                                // Размер текста, которым написаны названия автобусных маршрутов.
        std::pair<double, double> bus_label_offset;             // Смещение надписи с названием маршрута относительно координат конечной остановки на карте.
        int stop_label_font_size;                               // Размер текста, которым отображаются названия остановок.
        std::pair<double, double>  stop_label_offset;           // Смещение названия остановки относительно её координат на карте.
        svg::Color underlayer_color;                            // Цвет подложки под названиями остановок и маршрутов.
        double underlayer_width;                                // Толщина подложки под названиями остановок и маршрутов.
        std::vector<svg::Color> color_palette;                  // Цветовая палитра.
    };

    class MapRenderer {
    public:
        MapRenderer(RenderSettings& render_settings);
        RenderSettings GetRenderSettings() const;
        SphereProjector GetSphereProjector(const std::vector<transport_catalogue::detail::geo::Coordinates>& coords) const;
        void InitSphereProjector(std::vector<transport_catalogue::detail::geo::Coordinates> points);
        int GetPaletteSize() const;
        svg::Color GetColor(int line_number) const;
        void SetLineProperties(svg::Polyline& polyline, int line_number) const;

        /*---Bus Names---*/
        void SetBusTextCommonProperty(svg::Text& text, const std::string& name, svg::Point position) const;
        void SetBusTextAdditionalProperty(svg::Text& text, const std::string& name, svg::Point position) const;
        void SetBusTextColorProperty(svg::Text& text, const std::string& name, int palette, svg::Point position) const;

        /*---Stop Symbols---*/
        void SetStopsCircleProperty(svg::Circle& circle, svg::Point position) const;

        /*---Stop Names---*/
        void SetStopsTextCommonProperty(svg::Text& text, const std::string& name, svg::Point position) const;
        void SetStopsTextAdditionalProperty(svg::Text& text, const std::string& name, svg::Point position) const;
        void SetStopsTextColorProperty(svg::Text& text, const std::string& name, svg::Point position) const;

        void AddLine(std::vector<std::pair<Bus*, int>>& buses_palette);
        void AddBusesName(std::vector<std::pair<Bus*, int>>& buses_palette);
        void AddStopsCircle(std::vector<Stop*>& stops_name);
        void AddStopsName(std::vector<Stop*>& stops_name);

        void GetStreamMap(std::ostream& stream);

    private:
        RenderSettings& render_settings_; // Словарь для управления визуализацией карты
        SphereProjector sphere_projector_;
        svg::Document map_;
    };

    template <typename PointInputIt>
    SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width, double max_height, double padding)
        : padding_(padding) {

        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        }
        else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        }
        else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }
} // namespace map_renderer