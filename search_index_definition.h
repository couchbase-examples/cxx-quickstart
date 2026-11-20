#pragma once
#include <couchbase/management/search_index.hxx>
#include <tao/json.hpp>
#include <string>

// Maps a Search index definition exported from the Couchbase UI/REST API
// (the format of hotel_search_index.json) to the SDK's index type.
// Pure: no cluster access, so it can be unit-tested offline.
couchbase::management::search::index searchIndexFromJson(const tao::json::value& definition);

// Reads a Search index definition file and maps it with searchIndexFromJson.
couchbase::management::search::index loadSearchIndexDefinition(const std::string& index_file);
