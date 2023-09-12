#include "input_reader.h"

namespace transport_catalogue {
    namespace detail {

        Stop SplitStop(std::string& text) {
            auto twopoint = text.find(':');
            auto comma = text.find(',');

            Stop stop;
            auto entry_length = 5;
            auto distance = 2;

            stop.name_stop = text.substr(entry_length, twopoint - entry_length);
            stop.latitude = stod(text.substr(twopoint + distance, comma - twopoint - distance));
            stop.longitude = stod(text.substr(comma + distance));

            return stop;
        }

        std::vector<Distance> SplitDistance(TransportCatalogue& catalogue, std::string text) {
            std::vector<Distance> result;
            auto twopoint = text.find(':');
            auto entry_length = 5;
            auto space = 2;

            std::string name = text.substr(entry_length, twopoint - entry_length);
            text = text.substr(text.find(',') + 1);
            text = text.substr(text.find(',') + space);

            while (text.find(',') != std::string::npos) {

                int distance = stoi(text.substr(0, text.find('m')));
                std::string distance_stop = text.substr(text.find('m') + entry_length);
                distance_stop = distance_stop.substr(0, distance_stop.find(','));

                result.push_back({ catalogue.FindStop(name), catalogue.FindStop(distance_stop), distance });

                text = text.substr(text.find(',') + space);
            }
            std::string last_name = text.substr(text.find('m') + entry_length);
            int distance = stoi(text.substr(0, text.find('m')));

            result.push_back({ catalogue.FindStop(name), catalogue.FindStop(last_name), distance });
            return result;
        }

        Bus SplitBus(TransportCatalogue& catalogue, std::string_view text) {
            auto twopoint = text.find(':');

            Bus bus;
            auto entry_length = 4;
            auto distance = 2;

            bus.name_bus = text.substr(entry_length, twopoint - entry_length);

            text = text.substr(twopoint + distance);
            auto more = text.find('>');

            if (more == std::string_view::npos) {
                auto tire = text.find('-');

                while (tire != std::string_view::npos) {
                    bus.stops_bus.push_back(catalogue.FindStop(text.substr(0, tire - 1)));

                    text = text.substr(tire + distance);
                    tire = text.find('-');
                }

                bus.stops_bus.push_back(catalogue.FindStop(text.substr(0, tire - 1)));
                size_t size_ = bus.stops_bus.size() - 1;

                for (size_t i = size_; i > 0; i--) {
                    bus.stops_bus.push_back(bus.stops_bus[i - 1]);
                }
            }
            else {
                while (more != std::string_view::npos) {
                    bus.stops_bus.push_back(catalogue.FindStop(text.substr(0, more - 1)));

                    text = text.substr(more + distance);
                    more = text.find('>');
                }

                bus.stops_bus.push_back(catalogue.FindStop(text.substr(0, more - 1)));
            }
            return bus;
        }

        void Input(TransportCatalogue& catalogue, std::istream& input) {
            std::string text;
            std::getline(input, text);

            if (text != "") {
                std::string str;
                std::vector<std::string> buses;
                std::vector<std::string> stops;
                int amount = stoi(text);
                auto bus_distance = 3;

                for (int i = 0; i < amount; ++i) {
                    std::getline(input, str);

                    if (str != "") {
                        auto space_pos = str.find_first_not_of(' ');
                        str = str.substr(space_pos);

                        if (str.substr(0, bus_distance) != "Bus") {
                            stops.push_back(str);
                        }
                        else {
                            buses.push_back(str);
                        }
                    }
                }
                for (auto stop : stops) {
                    catalogue.AddStop(SplitStop(stop));
                }
                for (auto stop : stops) {
                    catalogue.AddDistance(SplitDistance(catalogue, stop));
                }
                for (auto& bus : buses) {
                    catalogue.AddBus(SplitBus(catalogue, bus));
                }
            }
        }
    }
}