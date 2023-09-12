#include "request_handler.h"

namespace request_handler {

    struct EdgeInfoGetter {

        Node operator()(const StopEdge& edge_info) {
            using namespace std::literals;

            return Builder{}.StartDict()
                .Key("type").Value("Wait")
                .Key("stop_name").Value(std::string(edge_info.name))
                .Key("time").Value(edge_info.time)
                .EndDict()
                .Build();
        }

        Node operator()(const BusEdge& edge_info) {
            using namespace std::literals;

            return Builder{}.StartDict()
                .Key("type").Value("Bus")
                .Key("bus").Value(std::string(edge_info.bus_name))
                .Key("span_count").Value(static_cast<int>(edge_info.span_count))
                .Key("time").Value(edge_info.time)
                .EndDict()
                .Build();
        }
    };
    std::optional<RouteInfo> RequestHandler::GetRouteInfo(std::string_view start, std::string_view end, TransportCatalogue& catalogue, TransportRouter& routing) const {
        return routing.GetRouteInfo(routing.GetRouterByStop(catalogue.FindStop(start))->bus_wait_start, routing.GetRouterByStop(catalogue.FindStop(end))->bus_wait_start);
    }
    std::vector<detail::geo::Coordinates> RequestHandler::GetStopsCoordinates(TransportCatalogue& catalogue) const {
        std::vector <detail::geo::Coordinates> stops_coordinates;
        auto buses = catalogue.GetBusnameToBus();

        for (auto& [busname, bus] : buses) {
            for (auto& stop : bus->stops_bus) {
                detail::geo::Coordinates coordinates;
                coordinates.lat = stop->latitude;
                coordinates.lng = stop->longitude;

                stops_coordinates.push_back(coordinates);
            }
        }
        return stops_coordinates;
	}
    std::vector<std::string_view> RequestHandler::GetSortBusesNames(TransportCatalogue& catalogue) const {
        std::vector<std::string_view> buses_names;

        auto buses = catalogue.GetBusnameToBus();
        if (buses.size() > 0) {
            for (auto& [busname, bus] : buses) {
                buses_names.push_back(busname);
            }

            std::sort(buses_names.begin(), buses_names.end());
            return buses_names;
        } else {
            return {};
        }
    }

    BusQuery RequestHandler::QueryBus(TransportCatalogue& catalogue, std::string_view text) {
        BusQuery bus_info;
        Bus* bus = catalogue.FindBus(text);

        if (bus != nullptr) {
            bus_info.name = bus->name_bus;
            bus_info.not_found = false;
            bus_info.stops_on_route = static_cast<int>(bus->stops_bus.size());
            bus_info.unique_stops = static_cast<int>(catalogue.GetUniqStops(bus).size());
            bus_info.route_length = static_cast<int>(bus->route_length);
            bus_info.curvature = double(catalogue.GetDistanceBus(bus) / catalogue.GetComputeDistance(bus));
        } else {
            bus_info.name = text;
            bus_info.not_found = true;
        }
        return bus_info;
    }
    StopQuery RequestHandler::QueryStop(TransportCatalogue& catalogue, std::string_view text) {
        std::unordered_set<const Bus*> unique_buses;
        StopQuery stop_info;
        Stop* stop = catalogue.FindStop(text);

        if (stop != NULL) {

            stop_info.name = stop->name_stop;
            stop_info.not_found = false;
            unique_buses = catalogue.GetUniqBuses(stop);

            if (unique_buses.size() > 0) {
                for (const Bus* bus : unique_buses) {
                    stop_info.buses_name.push_back(bus->name_bus);
                }
                std::sort(stop_info.buses_name.begin(), stop_info.buses_name.end());
            }
        } else {
            stop_info.name = text;
            stop_info.not_found = true;
        }
        return stop_info;
    }

    Node RequestHandler::ExecuteMakeNodeStop(int id_request, const StopQuery& stop_query) {
        Node result;
        Array buses;
        Builder builder;

        std::string str_not_found = "not found";

        if (stop_query.not_found) {
            builder.StartDict()
                .Key("request_id").Value(id_request)
                .Key("error_message").Value(str_not_found)
                .EndDict();

            result = builder.Build();
        } else {
            builder.StartDict()
                .Key("request_id").Value(id_request)
                .Key("buses").StartArray();

            for (std::string bus_name : stop_query.buses_name) {
                builder.Value(bus_name);
            }
            builder.EndArray().EndDict();
            result = builder.Build();
        }
        return result;
    }
    Node RequestHandler::ExecuteMakeNodeBus(int id_request, const BusQuery& bus_query) {
        Node result;
        std::string str_not_found = "not found";

        if (bus_query.not_found) {
            result = Builder{}.StartDict()
                .Key("request_id").Value(id_request)
                .Key("error_message").Value(str_not_found)
                .EndDict()
                .Build();
        } else {
            result = Builder{}.StartDict()
                .Key("request_id").Value(id_request)
                .Key("curvature").Value(bus_query.curvature)
                .Key("route_length").Value(bus_query.route_length)
                .Key("stop_count").Value(bus_query.stops_on_route)
                .Key("unique_stop_count").Value(bus_query.unique_stops)
                .EndDict()
                .Build();
        }
        return result;
    }
    Node RequestHandler::ExecuteMakeNodeMap(int id_request, TransportCatalogue& catalogue, RenderSettings render_settings) {
        Node result;
        std::ostringstream map_stream;
        std::string map_str;

        MapRenderer map_catalogue(render_settings);
        map_catalogue.InitSphereProjector(GetStopsCoordinates(catalogue));
        ExecuteRenderMap(map_catalogue, catalogue);
        map_catalogue.GetStreamMap(map_stream);
        map_str = map_stream.str();

        result = Builder{}.StartDict()
            .Key("request_id").Value(id_request)
            .Key("map").Value(map_str)
            .EndDict()
            .Build();

        return result;
    }
    void RequestHandler::ExecuteQueries(TransportCatalogue& catalogue, std::vector<StatRequest>& stat_requests, RenderSettings& render_settings, RoutingSettings& routing_settings) {
        std::vector<Node> result_request;
        TransportRouter routing;

        routing.SetRoutingSettings(routing_settings);
        routing.BuildRouter(catalogue);

        for (StatRequest req : stat_requests) {
            if (req.type == "Stop") {
                result_request.push_back(ExecuteMakeNodeStop(req.id, QueryStop(catalogue, req.name)));
            } else if (req.type == "Bus") {
                result_request.push_back(ExecuteMakeNodeBus(req.id, QueryBus(catalogue, req.name)));
            } else if (req.type == "Map") {
                result_request.push_back(ExecuteMakeNodeMap(req.id, catalogue, render_settings));
            } else if (req.type == "Route") {
                result_request.push_back(ExecuteMakeNodeRoute(req, catalogue, routing));
            }
        }
        document_out_ = Document{ Node(result_request) };
    }
    void RequestHandler::ExecuteRenderMap(MapRenderer& map_catalogue, TransportCatalogue& catalogue) const {
        std::vector<std::pair<Bus*, int>> buses_palette;
        std::vector<Stop*> stops_sort;
        int palette_size = 0;
        int palette_index = 0;

        palette_size = map_catalogue.GetPaletteSize();
        if (palette_size == 0) {
            std::cout << "color palette is empty";
            return;
        }
        auto buses = catalogue.GetBusnameToBus();
        if (buses.size() > 0) {

            for (std::string_view bus_name : GetSortBusesNames(catalogue)) {
                Bus* bus_info = catalogue.FindBus(bus_name);
                if (bus_info) {
                    if (bus_info->stops_bus.size() > 0) {
                        buses_palette.push_back(std::make_pair(bus_info, palette_index));
                        palette_index++;
                        if (palette_index == palette_size) {
                            palette_index = 0;
                        }
                    }
                }
            }
            if (buses_palette.size() > 0) {
                map_catalogue.AddLine(buses_palette);
                map_catalogue.AddBusesName(buses_palette);
            }
        }
        auto stops = catalogue.GetStopnameToStop();
        if (stops.size() > 0) {
            std::vector<std::string_view> stops_name;

            for (auto& [stop_name, stop] : stops) {
                if (stop->buses_vector.size() > 0) {
                    stops_name.push_back(stop_name);
                }
            }

            std::sort(stops_name.begin(), stops_name.end());

            for (std::string_view stop_name : stops_name) {
                Stop* stop = catalogue.FindStop(stop_name);
                if (stop) {
                    stops_sort.push_back(stop);
                }
            }

            if (stops_sort.size() > 0) {
                map_catalogue.AddStopsCircle(stops_sort);
                map_catalogue.AddStopsName(stops_sort);
            }
        }
    }

    const Document& RequestHandler::GetDocument() {
        return document_out_;
    }

    Node RequestHandler::ExecuteMakeNodeRoute(StatRequest& request, TransportCatalogue& catalogue, TransportRouter& routing) {
        const auto& route_info = GetRouteInfo(request.from, request.to, catalogue, routing);

        if (!route_info) {
            return Builder{}.StartDict()
                .Key("request_id").Value(request.id)
                .Key("error_message").Value("not found")
                .EndDict()
                .Build();
        }

        Array items;
        for (const auto& item : route_info->edges) {
            items.emplace_back(std::visit(EdgeInfoGetter{}, item));
        }

        return Builder{}.StartDict()
            .Key("request_id").Value(request.id)
            .Key("total_time").Value(route_info->total_time)
            .Key("items").Value(items)
            .EndDict()
            .Build();
    }
}