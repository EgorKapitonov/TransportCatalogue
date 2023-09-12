#pragma once

#include "transport_catalogue.h"
#include <algorithm>
#include <vector>

namespace transport_catalogue {
    namespace detail {

        struct StatRequest {
            int id;
            std::string type;
            std::string name;
            std::string from;
            std::string to;
        };

        struct BusQuery {
            std::string_view name;
            bool not_found;
            int stops_on_route;
            int unique_stops;
            int route_length;
            double curvature;
        };

        struct StopQuery {
            std::string_view name;
            bool not_found;
            std::vector <std::string> buses_name;
        };
    }
}//end namespace transport_catalogue