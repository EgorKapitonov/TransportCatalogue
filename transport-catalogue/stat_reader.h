#pragma once

#include "transport_catalogue.h"

namespace transport_catalogue {
	namespace detail {

		void QueryBus(TransportCatalogue& catalogue, std::string_view text, std::ostream& output);
		void QueryStop(TransportCatalogue& catalogue, std::string_view text, std::ostream& output);
		void Query(TransportCatalogue& catalogue, std::string_view text, std::ostream& output);
		void Output(TransportCatalogue& catalogue, std::istream& input, std::ostream& output);
	}
}