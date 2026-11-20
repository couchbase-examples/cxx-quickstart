#include "search_index_definition.h"

couchbase::management::search::index searchIndexFromJson(const tao::json::value& definition){
    couchbase::management::search::index index;
    index.name = definition.at("name").get_string();
    index.params_json = tao::json::to_string(definition.at("params"));
    index.plan_params_json = tao::json::to_string(definition.at("planParams"));
    index.source_name = definition.at("sourceName").get_string();
    index.source_type = definition.at("sourceType").get_string();
    return index;
}

couchbase::management::search::index loadSearchIndexDefinition(const std::string& index_file){
    return searchIndexFromJson(tao::json::from_file(index_file));
}
