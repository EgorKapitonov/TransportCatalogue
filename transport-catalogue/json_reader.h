#pragma once

#include "json.h"
#include "domain.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "serialization.h"

namespace transport_catalogue {
    namespace detail {
        namespace json {
            class JsonReader {
            public:
                JsonReader() = default;
                JsonReader(Document document);
                JsonReader(std::istream& input);

                void ParseNodeBase(const Node& root, TransportCatalogue& catalogue);
                void ParceNodeRender(const Node& node, map_renderer::RenderSettings& render_settings);
                void ParseNode(const Node& root, TransportCatalogue& catalogue, std::vector<StatRequest>& stat_request, map_renderer::RenderSettings& render_settings, router::RoutingSettings& routing_settings);
                void ParceNodeStat(const Node& node, std::vector<StatRequest>& stat_request);
                void Parse(TransportCatalogue& catalogue, std::vector<StatRequest>& stat_request, map_renderer::RenderSettings& render_settings, router::RoutingSettings& routing_settings);
                void ParceNodeRouting(const Node& node, router::RoutingSettings& route_set);

                Stop ParseNodeStop(Node& node);
                Bus ParseNodeBus(Node& node, TransportCatalogue& catalogue);
                std::vector<Distance> ParseNodeDistances(Node& node, TransportCatalogue& catalogue);
                void ParseNodeSerialization(const Node& node, serialization::SerializationSettings& serialization_set);

                void ParseNodeMakeBase(TransportCatalogue& catalogue, map_renderer::RenderSettings& render_settings, router::RoutingSettings& routing_settings, serialization::SerializationSettings& serialization_settings);
                void ParseNodeProcessRequests(std::vector<StatRequest>& stat_request, serialization::SerializationSettings& serialization_settings);


            private:
                Document document_;
            };
        }//end namespace json
    }//end namespace detail
}//end namespace transport_catalogue