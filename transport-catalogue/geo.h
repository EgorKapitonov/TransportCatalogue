#pragma once

const int RADIUS_EARTH = 6371000;
const double NUMBER_PI = 3.1415926535;

namespace transport_catalogue {
    namespace detail {
        namespace geo {

            struct Coordinates {
                double lat; // Широта
                double lng; // Долгота
                bool operator==(const Coordinates& other) const {
                    return lat == other.lat && lng == other.lng;
                }
                bool operator!=(const Coordinates& other) const {
                    return !(*this == other);
                }
            };

            double ComputeDistance(Coordinates from, Coordinates to);
        } // namespace geo
    } // namespace detail
}  // namespace transport_catalogue