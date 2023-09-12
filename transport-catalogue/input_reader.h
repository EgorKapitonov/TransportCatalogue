#pragma once

#include <string>
#include <iostream>
#include <algorithm>

#include "transport_catalogue.h"

namespace transport_catalogue {
	namespace detail {

		Stop SplitStop(std::string& text);
		std::vector<Distance> SplitDistance(TransportCatalogue& catalogue, std::string text);
		Bus SplitBus(TransportCatalogue& catalogue, std::string_view text);
		void Input(TransportCatalogue& catalogue, std::istream& input);
	}
}