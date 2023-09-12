#pragma once

#include "router.h"
#include "domain.h"
#include "transport_catalogue.h"

#include <unordered_map>
#include <variant>
#include <iterator>

namespace transport_catalogue {
    namespace detail {
        namespace router {

            using namespace graph;

            const int HOUR = 60;
            const int KILOMETER = 1000;

            struct StopEdge {
                std::string_view name;
                double time = 0;
            };

            struct BusEdge {
                std::string_view bus_name;
                size_t span_count = 0;
                double time = 0;
            };
            struct RoutingSettings {
                double bus_wait_time = 0.0; // время ожидания автобуса на остановке, в минутах.
                double bus_velocity = 0.0;  // скорость автобуса, в км/ч.
            };

            struct RouterByStop {
                VertexId bus_wait_start;    
                VertexId bus_wait_end;
            };

            struct RouteInfo {
                double total_time = 0.0;
                std::vector<std::variant<StopEdge, BusEdge>> edges;
            };

            class TransportRouter {
            public:
                void SetRoutingSettings(RoutingSettings routing_settings);
                const RoutingSettings& GetRoutingSettings() const;

                void BuildRouter(TransportCatalogue& transport_catalogue);

                const DirectedWeightedGraph<double>& GetGraph() const;
                const Router<double>& GetRouter() const;
                const std::variant<StopEdge, BusEdge>& GetEdge(EdgeId id) const;

                std::optional<RouterByStop> GetRouterByStop(Stop* stop) const;
                std::optional<RouteInfo> GetRouteInfo(VertexId start, VertexId end) const;

                const std::unordered_map<Stop*, RouterByStop>& GetStopToVertex() const;
                const std::unordered_map<EdgeId, std::variant<StopEdge, BusEdge>>& GetEdgeIdToEdge() const;

                std::deque<Stop*> GetStopsPtr(TransportCatalogue& transport_catalogue);
                std::deque<Bus*> GetBusPtr(TransportCatalogue& transport_catalogue);

                void AddEdgeToStop();
                void AddEdgeToBus(TransportCatalogue& transport_catalogue);

                void SetStops(const std::deque<Stop*>& stops);
                void SetGraph(TransportCatalogue& transport_catalogue);

                Edge<double> MakeEdgeToBus(Stop* start, Stop* end, const double distance) const;

                template <typename Iterator>
                void ParseBusToEdges(Iterator first, Iterator last, const TransportCatalogue& transport_catalogue, const Bus* bus);

            private:

                std::unordered_map<Stop*, RouterByStop> stop_to_router_;
                std::unordered_map<EdgeId, std::variant<StopEdge, BusEdge>> edge_id_to_edge_;

                std::unique_ptr<DirectedWeightedGraph<double>> graph_;
                std::unique_ptr<Router<double>> router_;

                RoutingSettings routing_settings_;
            };

            template <typename Iterator>
            void TransportRouter::ParseBusToEdges(Iterator first, Iterator last, const TransportCatalogue& transport_catalogue, const Bus* bus) {

                for (auto it = first; it != last; ++it) {
                    size_t distance = 0;
                    size_t span = 0;

                    for (auto it2 = std::next(it); it2 != last; ++it2) {
                        distance += transport_catalogue.GetDistanceStop(*prev(it2), *it2);
                        ++span;

                        EdgeId id = graph_->AddEdge(MakeEdgeToBus(*it, *it2, distance));

                        edge_id_to_edge_[id] = BusEdge{ bus->name_bus, span, graph_->GetEdge(id).weight };
                    }
                }
            }
        }
    }
}