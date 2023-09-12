#include "stat_reader.h"

namespace transport_catalogue {
    namespace detail {

        void QueryBus(TransportCatalogue& catalogue, std::string_view text, std::ostream& output) {
            auto entry = 4;
            text = text.substr(entry);

            Bus* bus = catalogue.FindBus(text);
            if (bus != nullptr) {
                output << "Bus " << bus->name_bus << ": "
                    << bus->stops_bus.size() << " stops on route, "
                    << (catalogue.GetUniqStops(bus)).size() << " unique stops, "
                    << catalogue.GetDistanceBus(bus) << " route length, "
                    << std::setprecision(6) << double(catalogue.GetDistanceBus(bus) / catalogue.GetComputeDistance(bus))
                    << " curvature" << std::endl;
            }
            else {
                output << "Bus " << text << ": not found" << std::endl;
            }
        }

        void QueryStop(TransportCatalogue& catalogue, std::string_view text, std::ostream& output) {
            auto entry = 5;
            text = text.substr(entry);

            std::unordered_set<const Bus*> unique_buses;
            std::unordered_set<std::string_view> unique_buses_name;
            std::vector <std::string> bus_name_v;

            Stop* stop = catalogue.FindStop(text);

            if (stop != NULL) {
                unique_buses = catalogue.GetUniqBuses(stop);

                if (unique_buses.size() == 0) {
                    output << "Stop " << text << ": no buses" << std::endl;
                }
                else {
                    output << "Stop " << text << ": buses ";

                    for (const Bus* bus : unique_buses) {
                        bus_name_v.push_back(bus->name_bus);
                    }

                    std::sort(bus_name_v.begin(), bus_name_v.end());

                    for (std::string_view bus_name : bus_name_v) {
                        output << bus_name;
                        output << " ";
                    }
                    output << std::endl;
                }
            }
            else {
                output << "Stop " << text << ": not found" << std::endl;
            }
        }

        void Query(TransportCatalogue& catalogue, std::string_view text, std::ostream& output) {
            if (text.substr(0, 3) == "Bus") {
                QueryBus(catalogue, text, output);
            }
            else if (text.substr(0, 4) == "Stop") {
                QueryStop(catalogue, text, output);
            }
            else {
                output << "Error query" << std::endl;
            }
        }

        void Output(TransportCatalogue& catalogue, std::istream& input, std::ostream& output) {
            std::string text;
            std::getline(input, text);

            std::string str;
            std::vector<std::string> query;
            auto amount = stoi(text);

            for (int i = 0; i < amount; ++i) {
                std::getline(input, str);
                query.push_back(str);
            }

            for (auto& it : query) {
                Query(catalogue, it, output);
            }
        }
    }
}