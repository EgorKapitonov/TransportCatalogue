#pragma once

#include <deque>
#include <string>
#include <vector>
#include <iomanip>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <numeric>

#include "geo.h"

/*
	Класс транспортного справочника назовите TransportCatalogue
	Он должен иметь методы для выполнения следующих задач :
	-добавление маршрута в базу,
	-добавление остановки в базу,
	-поиск маршрута по имени,
	-поиск остановки по имени,
	-получение информации о маршруте.

	Рекомендуется в качестве значений использовать константные указатели:
	const Bus* и const Stop*, где Bus и Stop — название структур для маршрута и остановки соответственно.
*/

namespace transport_catalogue {

	struct Bus;

	// Stop - название структуры для остановки
	struct Stop {
		std::string name_stop;
		double latitude;
		double longitude;

		std::vector<Bus*> buses_vector;
	};

	// Bus — название структуры для маршрута
	struct Bus {
		std::string name_bus;
		std::vector<Stop*> stops_bus;

		bool is_roundtrip;
		size_t route_length;
	};

	// Distance - название структуры для дистанции
	struct Distance {
		const Stop* A;
		const Stop* B;
		int distance;
	};

	//Хешер для объединения элементов пары
	class DistanceHasher {
	public:
		size_t operator()(const std::pair<const Stop*, const Stop*> hasher_pair) const {
			auto hash_first = d_hasher_(hasher_pair.first);
			auto hash_second = d_hasher_(hasher_pair.second);
			return hash_first * 37 + hash_second;
		}
	private:
		std::hash<const void*> d_hasher_;
	};

	//Класс транспортного справочника
	class TransportCatalogue {
	public:

		void AddStop(Stop&& stop);																				// Метод добавления остановки в базу
		void AddBus(Bus&& bus);																					// Метод добавления маршрута в базу
		void AddDistance(const std::vector<Distance>& distance);												// Метод добавления дистанции в базу
		Stop* FindStop(std::string_view find_stop);																// Метод поиска остановки
		Bus* FindBus(std::string_view find_bus);																// Метод поиска маршрута
		double GetComputeDistance(const Bus* bus);																// Метод получает информацию о дистанции
		std::unordered_set<const Stop*> GetUniqStops(Bus* bus);
		std::unordered_set<const Bus*> GetUniqBuses(Stop* stop);
		size_t GetDistanceStop(const Stop* a, const Stop* b) const;
		size_t GetDistanceBus(Bus* bus);
		std::unordered_map<std::string_view, Bus*> GetBusnameToBus() const;
		std::unordered_map<std::string_view, Stop*> GetStopnameToStop() const;

		std::deque<Stop> GetStops() const;
		std::deque<Bus> GetBuses() const;
		std::unordered_map<std::pair<const Stop*, const Stop*>, int, DistanceHasher> GetDistance() const;

	private:
		std::deque<Stop> stops_;																				// Контейнер для хранения остановок
		std::deque<Bus> buses_;																					// Контейнер для хранения маршрута
		std::unordered_map<std::string_view, Stop*> stopname_to_stop_;											// Хеш таблица которая имя остановок переводит в указатель
		std::unordered_map<std::string_view, Bus*> busname_to_bus_;												// Хеш таблица которая имя маршрута переводит в указатель
		std::unordered_map<std::pair<const Stop*, const Stop*>, int, DistanceHasher> distance_to_stop_;			// Контейнер для расстояние между остановками
	};
} // End namespace transport_catalogue