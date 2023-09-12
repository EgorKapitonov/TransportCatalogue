#include "serialization.h"

namespace serialization {

    template <typename It>
    uint32_t CalculateId(It start, It end, std::string_view name) {

        auto stop_it = std::find_if(start, end, [&name](const transport_catalogue::Stop stop) {return stop.name_stop == name; });
        return std::distance(start, stop_it);
    }

    transport_catalogue_protobuf::TransportCatalogue TransportCatalogueSerialization(const transport_catalogue::TransportCatalogue& transport_catalogue) {

        transport_catalogue_protobuf::TransportCatalogue transport_catalogue_proto;

        const auto& stops = transport_catalogue.GetStops();
        const auto& buses = transport_catalogue.GetBuses();
        const auto& distances = transport_catalogue.GetDistance();

        int id = 0;
        for (const auto& stop : stops) {

            transport_catalogue_protobuf::Stop stop_proto;

            stop_proto.set_id(id);
            stop_proto.set_name(stop.name_stop);
            stop_proto.set_latitude(stop.latitude);
            stop_proto.set_longitude(stop.longitude);

            *transport_catalogue_proto.add_stops() = std::move(stop_proto);

            ++id;
        }

        for (const auto& bus : buses) {

            transport_catalogue_protobuf::Bus bus_proto;

            bus_proto.set_name(bus.name_bus);

            for (auto stop : bus.stops_bus) {
                uint32_t stop_id = CalculateId(stops.cbegin(),
                    stops.cend(),
                    stop->name_stop);
                bus_proto.add_stops(stop_id);
            }

            bus_proto.set_is_roundtrip(bus.is_roundtrip);
            bus_proto.set_route_length(bus.route_length);

            *transport_catalogue_proto.add_buses() = std::move(bus_proto);
        }

        for (const auto& [pair_stops, pair_distance] : distances) {

            transport_catalogue_protobuf::Distance distance_proto;

            distance_proto.set_start(CalculateId(stops.cbegin(),
                stops.cend(),
                pair_stops.first->name_stop));

            distance_proto.set_end(CalculateId(stops.cbegin(),
                stops.cend(),
                pair_stops.second->name_stop));

            distance_proto.set_distance(pair_distance);

            *transport_catalogue_proto.add_distances() = std::move(distance_proto);
        }

        return transport_catalogue_proto;
    }


    transport_catalogue::TransportCatalogue TransportCatalogueDeserialization(const transport_catalogue_protobuf::TransportCatalogue& transport_catalogue_proto) {

        transport_catalogue::TransportCatalogue transport_catalogue;

        const auto& stops_proto = transport_catalogue_proto.stops();
        const auto& buses_proto = transport_catalogue_proto.buses();
        const auto& distances_proto = transport_catalogue_proto.distances();

        for (const auto& stop : stops_proto) {

            transport_catalogue::Stop tc_stop;

            tc_stop.name_stop = stop.name();
            tc_stop.latitude = stop.latitude();
            tc_stop.longitude = stop.longitude();

            transport_catalogue.AddStop(std::move(tc_stop));
        }

        const auto& tc_stops = transport_catalogue.GetStops();

        std::vector<transport_catalogue::Distance> distances;
        for (const auto& distance : distances_proto) {

            transport_catalogue::Distance tc_distance;

            tc_distance.A = transport_catalogue.FindStop(tc_stops[distance.start()].name_stop);
            tc_distance.B = transport_catalogue.FindStop(tc_stops[distance.end()].name_stop);

            tc_distance.distance = distance.distance();

            distances.push_back(tc_distance);
        }

        transport_catalogue.AddDistance(distances);

        for (const auto& bus_proto : buses_proto) {

            transport_catalogue::Bus tc_bus;

            tc_bus.name_bus = bus_proto.name();

            for (auto stop_id : bus_proto.stops()) {
                auto name = tc_stops[stop_id].name_stop;
                tc_bus.stops_bus.push_back(transport_catalogue.FindStop(name));
            }

            tc_bus.is_roundtrip = bus_proto.is_roundtrip();
            tc_bus.route_length = bus_proto.route_length();

            transport_catalogue.AddBus(std::move(tc_bus));
        }

        return transport_catalogue;
    }

    transport_catalogue_protobuf::Color ColorSerialize(const svg::Color& tc_color) {

        transport_catalogue_protobuf::Color color_proto;

        if (std::holds_alternative<std::monostate>(tc_color)) {
            color_proto.set_none(true);

        }
        else if (std::holds_alternative<svg::Rgb>(tc_color)) {
            svg::Rgb rgb = std::get<svg::Rgb>(tc_color);

            color_proto.mutable_rgb()->set_red_(rgb.red);
            color_proto.mutable_rgb()->set_green_(rgb.green);
            color_proto.mutable_rgb()->set_blue_(rgb.blue);

        }
        else if (std::holds_alternative<svg::Rgba>(tc_color)) {
            svg::Rgba rgba = std::get<svg::Rgba>(tc_color);

            color_proto.mutable_rgba()->set_red_(rgba.red);
            color_proto.mutable_rgba()->set_green_(rgba.green);
            color_proto.mutable_rgba()->set_blue_(rgba.blue);
            color_proto.mutable_rgba()->set_opacity_(rgba.opacity);

        }
        else if (std::holds_alternative<std::string>(tc_color)) {
            color_proto.set_string_color(std::get<std::string>(tc_color));
        }

        return color_proto;
    }

    svg::Color ColorDeserealize(const transport_catalogue_protobuf::Color& color_proto) {

        svg::Color color;

        if (color_proto.has_rgb()) {
            svg::Rgb rgb;

            rgb.red = color_proto.rgb().red_();
            rgb.green = color_proto.rgb().green_();
            rgb.blue = color_proto.rgb().blue_();

            color = rgb;

        }
        else if (color_proto.has_rgba()) {
            svg::Rgba rgba;

            rgba.red = color_proto.rgba().red_();
            rgba.green = color_proto.rgba().green_();
            rgba.blue = color_proto.rgba().blue_();
            rgba.opacity = color_proto.rgba().opacity_();

            color = rgba;

        }
        else { color = color_proto.string_color(); }

        return color;
    }

    transport_catalogue_protobuf::RenderSettings RenderSettingsSerialization(const map_renderer::RenderSettings& render_settings) {

        transport_catalogue_protobuf::RenderSettings render_settings_proto;

        render_settings_proto.set_width_(render_settings.width);
        render_settings_proto.set_height_(render_settings.height);
        render_settings_proto.set_padding_(render_settings.padding);
        render_settings_proto.set_line_width_(render_settings.line_width);
        render_settings_proto.set_stop_radius_(render_settings.stop_radius);
        render_settings_proto.set_bus_label_font_size_(render_settings.bus_label_font_size);

        transport_catalogue_protobuf::Point bus_label_offset_proto;
        bus_label_offset_proto.set_x(render_settings.bus_label_offset.first);
        bus_label_offset_proto.set_y(render_settings.bus_label_offset.second);

        *render_settings_proto.mutable_bus_label_offset_() = std::move(bus_label_offset_proto);

        render_settings_proto.set_stop_label_font_size_(render_settings.stop_label_font_size);

        transport_catalogue_protobuf::Point stop_label_offset_proto;
        stop_label_offset_proto.set_x(render_settings.stop_label_offset.first);
        stop_label_offset_proto.set_y(render_settings.stop_label_offset.second);

        *render_settings_proto.mutable_stop_label_offset_() = std::move(stop_label_offset_proto);
        *render_settings_proto.mutable_underlayer_color_() = std::move(ColorSerialize(render_settings.underlayer_color));
        render_settings_proto.set_underlayer_width_(render_settings.underlayer_width);

        const auto& colors = render_settings.color_palette;
        for (const auto& color : colors) {
            *render_settings_proto.add_color_palette_() = std::move(ColorSerialize(color));
        }

        return render_settings_proto;
    }

    map_renderer::RenderSettings RenderSettingsDeserialization(const transport_catalogue_protobuf::RenderSettings& render_settings_proto) {

        map_renderer::RenderSettings render_settings;

        render_settings.width = render_settings_proto.width_();
        render_settings.height = render_settings_proto.height_();
        render_settings.padding = render_settings_proto.padding_();
        render_settings.line_width = render_settings_proto.line_width_();
        render_settings.stop_radius = render_settings_proto.stop_radius_();
        render_settings.bus_label_font_size = render_settings_proto.bus_label_font_size_();

        render_settings.bus_label_offset.first = render_settings_proto.bus_label_offset_().x();
        render_settings.bus_label_offset.second = render_settings_proto.bus_label_offset_().y();

        render_settings.stop_label_font_size = render_settings_proto.stop_label_font_size_();

        render_settings.stop_label_offset.first = render_settings_proto.stop_label_offset_().x();
        render_settings.stop_label_offset.second = render_settings_proto.stop_label_offset_().y();

        render_settings.underlayer_color = ColorDeserealize(render_settings_proto.underlayer_color_());
        render_settings.underlayer_width = render_settings_proto.underlayer_width_();

        for (const auto& color_proto : render_settings_proto.color_palette_()) {
            render_settings.color_palette.push_back(ColorDeserealize(color_proto));
        }

        return render_settings;
    }

    transport_catalogue_protobuf::RoutingSettings RoutingSettingsSerialization(const RoutingSettings& routing_settings) {

        transport_catalogue_protobuf::RoutingSettings routing_settings_proto;

        routing_settings_proto.set_bus_wait_time(routing_settings.bus_wait_time);
        routing_settings_proto.set_bus_velocity(routing_settings.bus_velocity);

        return routing_settings_proto;
    }

    RoutingSettings RoutingSettingsDeserialization(const transport_catalogue_protobuf::RoutingSettings& routing_settings_proto) {

        RoutingSettings routing_settings;

        routing_settings.bus_wait_time = routing_settings_proto.bus_wait_time();
        routing_settings.bus_velocity = routing_settings_proto.bus_velocity();

        return routing_settings;
    }

    void CatalogueSerialization(const transport_catalogue::TransportCatalogue& transport_catalogue,
        const map_renderer::RenderSettings& render_settings,
        const RoutingSettings& routing_settings,
        std::ostream& out) {

        transport_catalogue_protobuf::Catalogue catalogue_proto;

        transport_catalogue_protobuf::TransportCatalogue transport_catalogue_proto = TransportCatalogueSerialization(transport_catalogue);
        transport_catalogue_protobuf::RenderSettings render_settings_proto = RenderSettingsSerialization(render_settings);
        transport_catalogue_protobuf::RoutingSettings routing_settings_proto = RoutingSettingsSerialization(routing_settings);

        *catalogue_proto.mutable_transport_catalogue() = std::move(transport_catalogue_proto);
        *catalogue_proto.mutable_render_settings() = std::move(render_settings_proto);
        *catalogue_proto.mutable_routing_settings() = std::move(routing_settings_proto);

        catalogue_proto.SerializePartialToOstream(&out);
    }

    Catalogue CatalogueDeserialization(std::istream& in) {

        transport_catalogue_protobuf::Catalogue catalogue_proto;
        auto success_parsing_catalogue_from_istream = catalogue_proto.ParseFromIstream(&in);

        if (!success_parsing_catalogue_from_istream) {
            throw std::runtime_error("cannot parse serialized file from istream");
        }

        return { TransportCatalogueDeserialization(catalogue_proto.transport_catalogue()),
                RenderSettingsDeserialization(catalogue_proto.render_settings()),
                RoutingSettingsDeserialization(catalogue_proto.routing_settings()) };
    }

}