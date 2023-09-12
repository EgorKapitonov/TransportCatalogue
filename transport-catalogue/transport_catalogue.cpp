#include "transport_catalogue.h"

namespace transport_catalogue {

	// Метод добавления остановки в базу
	void TransportCatalogue::AddStop(Stop&& stop) {
		stops_.push_back(std::move(stop));
		Stop* buffer = &stops_.back();
		stopname_to_stop_.insert({ buffer->name_stop, buffer });
	}

	// Метод добавления маршрута в базу
	void TransportCatalogue::AddBus(Bus&& bus) {
		buses_.push_back(std::move(bus));
		Bus* buffer = &buses_.back();
		busname_to_bus_.insert({ buffer->name_bus, buffer });

		for (Stop* stop : buffer->stops_bus) {
			stop->buses_vector.push_back(buffer);
		}

		buffer->route_length = GetDistanceBus(buffer);
	}

	// Метод добавления дистанции в базу
	void TransportCatalogue::AddDistance(const std::vector<Distance>& distances) {
		for (auto distance : distances) {
			auto dist_stops = std::make_pair(distance.A, distance.B);
			distance_to_stop_.insert({ dist_stops, distance.distance });
		}
	}

	// Метод поиска остановки
	Stop* TransportCatalogue::FindStop(std::string_view find_stop) {
		if (stopname_to_stop_.count(find_stop)) {
			return stopname_to_stop_.at(find_stop);
		}
		else {
			return nullptr;
		}
	}

	// Метод поиска маршрута
	Bus* TransportCatalogue::FindBus(std::string_view find_bus) {
		if (busname_to_bus_.count(find_bus)) {
			return busname_to_bus_.at(find_bus);
		}
		else {
			return nullptr;
		}
	}

	// Метод получает информацию о дистанции
	double TransportCatalogue::GetComputeDistance(const Bus* bus) {
		return transform_reduce(next(bus->stops_bus.begin()),
			bus->stops_bus.end(), bus->stops_bus.begin(), 0.0, std::plus<>{},
			[](const Stop* lhs, const Stop* rhs) {
				return detail::geo::ComputeDistance({ (*lhs).latitude,
										(*lhs).longitude }, { (*rhs).latitude,
															  (*rhs).longitude });
			});
	}

	std::unordered_set<const Stop*>  TransportCatalogue::GetUniqStops(Bus* bus) {
		std::unordered_set<const Stop*> unique_stops;

		unique_stops.insert(bus->stops_bus.begin(), bus->stops_bus.end());
		return unique_stops;
	}

	std::unordered_set<const Bus*> TransportCatalogue::GetUniqBuses(Stop* stop) {
		std::unordered_set<const Bus*> unique_stops;

		unique_stops.insert(stop->buses_vector.begin(), stop->buses_vector.end());
		return unique_stops;
	}

	size_t TransportCatalogue::GetDistanceStop(const Stop* a, const Stop* b) const{
		auto distance = std::make_pair(a, b);
		auto reverse_distance = std::make_pair(b, a);

		if (distance_to_stop_.find(distance) != distance_to_stop_.end()) {
			return distance_to_stop_.at(distance);

		}
		else if (distance_to_stop_.find(reverse_distance) != distance_to_stop_.end()) {
			return distance_to_stop_.at(reverse_distance);
		}
		else {
			return 0;
		}
		
	}

	size_t TransportCatalogue::GetDistanceBus(Bus* bus) {
		size_t distance = 0;
		auto size_stops_bus = bus->stops_bus.size() - 1;

		for (int i = 0; i < static_cast<int>(size_stops_bus); i++) {
			distance += GetDistanceStop(bus->stops_bus[i], bus->stops_bus[i + 1]);
		}
		return distance;
	}
	std::unordered_map<std::string_view, Bus*> TransportCatalogue::GetBusnameToBus() const {
		return busname_to_bus_;
	}
	std::unordered_map<std::string_view, Stop*> TransportCatalogue::GetStopnameToStop() const {
		return stopname_to_stop_;
	}

	std::deque<Stop> TransportCatalogue::GetStops() const {
		return stops_;
	}

	std::deque<Bus> TransportCatalogue::GetBuses() const {
		return buses_;
	}
	std::unordered_map<std::pair<const Stop*, const Stop*>, int, DistanceHasher> TransportCatalogue::GetDistance() const {
		return distance_to_stop_;
	}
} // End namespace transport_catalogue