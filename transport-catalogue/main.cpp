#include <fstream>
#include <iostream>

#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "serialization.h"

using namespace std;
using namespace transport_catalogue;
using namespace map_renderer;
using namespace request_handler;

using namespace transport_catalogue::detail::json;
using namespace transport_catalogue::detail::router;

using namespace serialization;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {

    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    TransportCatalogue transport_catalogue;

    RenderSettings render_settings;
    RoutingSettings routing_settings;

    SerializationSettings serialization_settings;

    JsonReader json_reader;
    vector<StatRequest> stat_request;

    if (mode == "make_base"sv) {

        json_reader = JsonReader(cin);

        json_reader.ParseNodeMakeBase(transport_catalogue,
            render_settings,
            routing_settings,
            serialization_settings);

        ofstream out_file(serialization_settings.file_name, ios::binary);
        CatalogueSerialization(transport_catalogue, render_settings, routing_settings, out_file);

    }
    else if (mode == "process_requests"sv) {

        json_reader = JsonReader(cin);

        json_reader.ParseNodeProcessRequests(stat_request, serialization_settings);

        ifstream in_file(serialization_settings.file_name, ios::binary);
        Catalogue catalogue = CatalogueDeserialization(in_file);

        RequestHandler request_handler;

        request_handler.ExecuteQueries(catalogue.transport_catalogue_,
            stat_request,
            catalogue.render_settings_,
            catalogue.routing_settings_);

        Print(request_handler.GetDocument(), cout);

    }
    else {
        PrintUsage();
        return 1;
    }
}