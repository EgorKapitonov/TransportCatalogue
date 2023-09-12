#include "json_reader.h"

namespace transport_catalogue {
    namespace detail {
        namespace json {

            JsonReader::JsonReader(Document document)
                : document_(std::move(document)) {}
            JsonReader::JsonReader(std::istream& input) 
                : document_(json::Load(input)) {}

            void JsonReader::ParseNodeBase(const Node& root, TransportCatalogue& catalogue) {
                Array base_requests;
                Dict req_map;
                Node req_node;

                std::vector<Node> buses;
                std::vector<Node> stops;

                if (root.IsArray()) {
                    base_requests = root.AsArray();

                    for (Node& node : base_requests) {
                        if (node.IsDict()) {
                            req_map = node.AsDict();

                            try {
                                req_node = req_map.at("type");
                                if (req_node.IsString()) {

                                    if (req_node.AsString() == "Bus") {
                                        buses.push_back(req_map);

                                    }
                                    else if (req_node.AsString() == "Stop") {
                                        stops.push_back(req_map);

                                    }
                                    else {
                                        std::cout << "Error: base_requests have bad type";
                                    }
                                }

                            }
                            catch (...) {
                                std::cout << "Error: base_requests not have type value";
                            }
                        }
                    }

                    for (auto stop : stops) {
                        catalogue.AddStop(ParseNodeStop(stop));
                    }

                    for (auto stop : stops) {
                        catalogue.AddDistance(ParseNodeDistances(stop, catalogue));
                    }

                    for (auto bus : buses) {
                        catalogue.AddBus(ParseNodeBus(bus, catalogue));
                    }

                }
                else {
                    std::cout << "base_requests is not array";
                }
            }
            void JsonReader::ParceNodeRender(const Node& node, map_renderer::RenderSettings& render_settings) {
                Dict rend_map;
                Array bus_lab_offset;
                Array stop_lab_offset;
                Array arr_color;
                Array arr_palette;
                uint8_t red;
                uint8_t green;
                uint8_t blue;
                double opacity;

                if (node.IsDict()) {
                    rend_map = node.AsDict();
                    try {
                        render_settings.width = rend_map.at("width").AsDouble();
                        render_settings.height = rend_map.at("height").AsDouble();
                        render_settings.padding = rend_map.at("padding").AsDouble();
                        render_settings.line_width = rend_map.at("line_width").AsDouble();
                        render_settings.stop_radius = rend_map.at("stop_radius").AsDouble();
                        render_settings.bus_label_font_size = rend_map.at("bus_label_font_size").AsInt();

                        if (rend_map.at("bus_label_offset").IsArray()) {
                            bus_lab_offset = rend_map.at("bus_label_offset").AsArray();
                            render_settings.bus_label_offset = std::make_pair(bus_lab_offset[0].AsDouble(),
                                bus_lab_offset[1].AsDouble());
                        }

                        render_settings.stop_label_font_size = rend_map.at("stop_label_font_size").AsInt();

                        if (rend_map.at("stop_label_offset").IsArray()) {
                            stop_lab_offset = rend_map.at("stop_label_offset").AsArray();
                            render_settings.stop_label_offset = std::make_pair(stop_lab_offset[0].AsDouble(),
                                stop_lab_offset[1].AsDouble());
                        }

                        if (rend_map.at("underlayer_color").IsString()) {
                            render_settings.underlayer_color = svg::Color(rend_map.at("underlayer_color").AsString());

                        } else if (rend_map.at("underlayer_color").IsArray()) {
                            arr_color = rend_map.at("underlayer_color").AsArray();
                            red = arr_color[0].AsInt();
                            green = arr_color[1].AsInt();
                            blue = arr_color[2].AsInt();

                            if (arr_color.size() == 4) {
                                opacity = arr_color[3].AsDouble();
                                render_settings.underlayer_color = svg::Color(svg::Rgba(red, green, blue, opacity));
                            }
                            else if (arr_color.size() == 3) {
                                render_settings.underlayer_color = svg::Color(svg::Rgb(red, green, blue));
                            }
                        }

                        render_settings.underlayer_width = rend_map.at("underlayer_width").AsDouble();

                        if (rend_map.at("color_palette").IsArray()) {
                            arr_palette = rend_map.at("color_palette").AsArray();

                            for (Node color_palette : arr_palette) {

                                if (color_palette.IsString()) {
                                    render_settings.color_palette.push_back(svg::Color(color_palette.AsString()));
                                }
                                else if (color_palette.IsArray()) {
                                    arr_color = color_palette.AsArray();
                                    red = arr_color[0].AsInt();
                                    green = arr_color[1].AsInt();
                                    blue = arr_color[2].AsInt();

                                    if (arr_color.size() == 4) {
                                        opacity = arr_color[3].AsDouble();
                                        render_settings.color_palette.push_back(svg::Color(svg::Rgba(red, green, blue, opacity)));
                                    }
                                    else if (arr_color.size() == 3) {
                                        render_settings.color_palette.push_back(svg::Color(svg::Rgb(red, green, blue)));
                                    }
                                }
                            }
                        }
                    } catch (...) {
                        std::cout << "unable to parsse init settings";
                    }
                }
                else {
                    std::cout << "render_settings is not map";
                }
            }
            void JsonReader::ParseNode(const Node& root, TransportCatalogue& catalogue, std::vector<StatRequest>& stat_request, map_renderer::RenderSettings& render_settings, router::RoutingSettings& routing_settings) {
                Dict root_dictionary;

                if (root.IsDict()) {
                    root_dictionary = root.AsDict();
                    try {
                        ParseNodeBase(root_dictionary.at("base_requests"), catalogue);
                    } catch (...) {
                        std::cout << "base_requests is empty";
                    }
                    try {
                        ParceNodeStat(root_dictionary.at("stat_requests"), stat_request);
                    }
                    catch (...) {
                        std::cout << "stat_requests is empty";
                    }
                    try {
                        ParceNodeRender(root_dictionary.at("render_settings"), render_settings);
                    } catch (...) {
                        std::cout << "render_settings is empty";
                    }
                    try {
                        ParceNodeRouting(root_dictionary.at("routing_settings"), routing_settings);
                    }
                    catch (...) {
                        std::cout << "routing_settings is empty";
                    }
                } else {
                    std::cout << "root is not map";
                }
            }
            void JsonReader::ParceNodeStat(const Node& node, std::vector<StatRequest>& stat_request) {
                Array stat_requests;
                Dict req_map;
                StatRequest req;

                if (node.IsArray()) {
                    stat_requests = node.AsArray();

                    for (Node req_node : stat_requests) {

                        if (req_node.IsDict()) {
                            req_map = req_node.AsDict();
                            req.id = req_map.at("id").AsInt();
                            req.type = req_map.at("type").AsString();

                            if ((req.type == "Bus") || (req.type == "Stop")) {
                                req.name = req_map.at("name").AsString();
                                req.from = "";
                                req.to = "";
                            } else {
                                req.name = "";
                                req.name = "";
                                if (req.type == "Route") {
                                    req.from = req_map.at("from").AsString();
                                    req.to = req_map.at("to").AsString();
                                }
                                else {
                                    req.from = "";
                                    req.to = "";
                                }
                            }
                            stat_request.push_back(req);
                        }
                    }
                } else {
                    std::cout << "base_requests is not array";
                }
            }
            void JsonReader::Parse(TransportCatalogue& catalogue, std::vector<StatRequest>& stat_request, map_renderer::RenderSettings& render_settings, router::RoutingSettings& routing_settings) {
                ParseNode(document_.GetRoot(), catalogue, stat_request, render_settings, routing_settings);
            }

            Stop JsonReader::ParseNodeStop(Node& node) {
                Stop stop;
                Dict stop_node;

                if (node.IsDict()) {

                    stop_node = node.AsDict();
                    stop.name_stop = stop_node.at("name").AsString();
                    stop.latitude = stop_node.at("latitude").AsDouble();
                    stop.longitude = stop_node.at("longitude").AsDouble();
                }

                return stop;
            }
            Bus JsonReader::ParseNodeBus(Node& node, TransportCatalogue& catalogue) {
                Bus bus;
                Dict bus_node;
                Array bus_stops;

                if (node.IsDict()) {
                    bus_node = node.AsDict();
                    bus.name_bus = bus_node.at("name").AsString();
                    bus.is_roundtrip = bus_node.at("is_roundtrip").AsBool();

                    try {
                        bus_stops = bus_node.at("stops").AsArray();

                        for (Node stop : bus_stops) {
                            bus.stops_bus.push_back(catalogue.FindStop(stop.AsString()));
                        }

                        if (!bus.is_roundtrip) {
                            size_t size = bus.stops_bus.size() - 1;

                            for (size_t i = size; i > 0; i--) {
                                bus.stops_bus.push_back(bus.stops_bus[i - 1]);
                            }
                        }

                    } catch (...) {
                        std::cout << "Error: base_requests: bus: stops is empty" << std::endl;
                    }
                }
                return bus;
            }
            std::vector<Distance> JsonReader::ParseNodeDistances(Node& node, TransportCatalogue& catalogue) {
                std::vector<Distance> distances;
                Dict stop_node;
                Dict stop_road_map;
                std::string begin_name;
                std::string last_name;
                int distance;

                if (node.IsDict()) {
                    stop_node = node.AsDict();
                    begin_name = stop_node.at("name").AsString();

                    try {
                        stop_road_map = stop_node.at("road_distances").AsDict();

                        for (auto [key, value] : stop_road_map) {
                            last_name = key;
                            distance = value.AsInt();
                            distances.push_back({ catalogue.FindStop(begin_name),
                                                 catalogue.FindStop(last_name),
                                                 distance });
                        }
                    }

                    catch (...) {
                        std::cout << "Error: Road invalide" << std::endl;
                    }
                }
                return distances;
            }

            void JsonReader::ParceNodeRouting(const Node& node, router::RoutingSettings& route_set) {
                Dict route;

                if (node.IsDict()) {
                    route = node.AsDict();

                    try {
                        route_set.bus_wait_time = route.at("bus_wait_time").AsDouble();
                        route_set.bus_velocity = route.at("bus_velocity").AsDouble();
                    }
                    catch (...) {
                        std::cout << "unable to parse routing settings";
                    }
                }
                else {
                    std::cout << "routing settings is not map";
                }
            }
            void JsonReader::ParseNodeSerialization(const Node& node, serialization::SerializationSettings& serialization_set) {

                Dict serialization;

                if (node.IsDict()) {
                    serialization = node.AsDict();

                    try {
                        serialization_set.file_name = serialization.at("file").AsString();

                    }
                    catch (...) {
                        std::cout << "unable to parse serialization settings";
                    }

                }
                else {
                    std::cout << "serialization settings is not map";
                }
            }

            void JsonReader::ParseNodeMakeBase(TransportCatalogue& catalogue, map_renderer::RenderSettings& render_settings, router::RoutingSettings& routing_settings, serialization::SerializationSettings& serialization_settings) {
                Dict root_dictionary;

                if (document_.GetRoot().IsDict()) {
                    root_dictionary = document_.GetRoot().AsDict();

                    try {
                        ParseNodeBase(root_dictionary.at("base_requests"), catalogue);

                    }
                    catch (...) {}

                    try {
                        ParceNodeRender(root_dictionary.at("render_settings"), render_settings);

                    }
                    catch (...) {}

                    try {
                        ParceNodeRouting(root_dictionary.at("routing_settings"), routing_settings);

                    }
                    catch (...) {}

                    try {
                        ParseNodeSerialization(root_dictionary.at("serialization_settings"), serialization_settings);

                    }
                    catch (...) {}

                }
                else {
                    std::cout << "root is not map";
                }
            }

            void JsonReader::ParseNodeProcessRequests(std::vector<StatRequest>& stat_request, serialization::SerializationSettings& serialization_settings) {
                Dict root_dictionary;

                if (document_.GetRoot().IsDict()) {
                    root_dictionary = document_.GetRoot().AsDict();

                    try {
                        ParceNodeStat(root_dictionary.at("stat_requests"), stat_request);

                    }
                    catch (...) {}

                    try {
                        ParseNodeSerialization(root_dictionary.at("serialization_settings"), serialization_settings);

                    }
                    catch (...) {}

                }
                else {
                    std::cout << "root is not map";
                }
            }

        }//end namespace json
    }//end namespace detail
}//end namespace transport_catalogue