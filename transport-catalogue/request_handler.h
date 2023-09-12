#pragma once

#include "domain.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "json_builder.h"
#include "transport_router.h"

using namespace transport_catalogue;
using namespace detail;
using namespace transport_catalogue::detail::json;
using namespace transport_catalogue::detail::router;
using namespace map_renderer;

namespace request_handler {

    class RequestHandler {
    public:
        RequestHandler() = default;

        std::optional<RouteInfo> GetRouteInfo(std::string_view start, std::string_view end, TransportCatalogue& catalogue, TransportRouter& routing) const;

        std::vector<detail::geo::Coordinates> GetStopsCoordinates(TransportCatalogue& catalogue) const;
        std::vector<std::string_view> GetSortBusesNames(TransportCatalogue& catalogue) const;

        BusQuery QueryBus(TransportCatalogue& catalogue, std::string_view text);
        StopQuery QueryStop(TransportCatalogue& catalogue, std::string_view text);

        Node ExecuteMakeNodeStop(int id_request, const StopQuery& stop_query);
        Node ExecuteMakeNodeBus(int id_request, const BusQuery& bus_query);
        Node ExecuteMakeNodeMap(int id_request, TransportCatalogue& catalogue, RenderSettings render_settings);
        void ExecuteQueries(TransportCatalogue& catalogue, std::vector<StatRequest>& stat_requests, RenderSettings& render_settings, RoutingSettings& routing_settings);
        void ExecuteRenderMap(MapRenderer& map_catalogue, TransportCatalogue& catalogue) const;
        Node ExecuteMakeNodeRoute(StatRequest& request, TransportCatalogue& catalogue, TransportRouter& routing);

        const Document& GetDocument();

    private:
        Document document_out_;
    };
}