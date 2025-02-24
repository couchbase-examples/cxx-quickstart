#pragma once
#include <couchbase/cluster.hxx>
#include <couchbase/codec/tao_json_serializer.hxx>
#include <couchbase/match_query.hxx>
#include <couchbase/term_query.hxx>
#include <couchbase/conjunction_query.hxx>
#include <couchbase/boolean_field_query.hxx>
#include <tao/json.hpp>
#include <tao/json/contrib/traits.hpp>
#include <iostream>

//Create and Update
int Upsert(couchbase::collection& col, const std::string& doc_id, const std::string& file_path, bool file_flag = false);
int Insert(couchbase::collection& col, const std::string& doc_id, const std::string& file_path, bool file_flag = false);

// Read
tao::json::value Read(couchbase::collection& col, const std::string& doc_id);

// Delete
int Delete(couchbase::collection& col, const std::string& doc_id);

// Query
std::vector<std::string> Query(couchbase::scope& scope);

// Search Index
bool searchIndexExists(couchbase::scope_search_index_manager& sc_manager, const std::string& index_name);
std::string CreateSearchIndex(couchbase::scope& scope, const std::string& index_file);
std::vector<std::string> SearchByName(couchbase::scope& scope, const std::string& index, const std::string& name, const std::string& field, int limit=-1);
std::vector<std::string> Filter(couchbase::scope& scope, const std::string& index_name, int limit=-1, int offset=-1);