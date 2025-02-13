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

std::vector<std::string> parseEnvironmentVariables(const std::vector<std::string>& keys);
bool checkScopeAndColExists(couchbase::bucket& bucket, const std::string& scope_name, const std::string& col_name);
bool checkSearchEnabled(couchbase::cluster& cluster, int min_nodes);
std::tuple<couchbase::cluster, couchbase::bucket, couchbase::scope, couchbase::collection> connectCluster(const std::string& DB_CONN_STRING,
    const std::string& DB_USERNAME,
    const std::string& DB_PASSWD,
    const std::string& BUCKET_NAME,
    const std::string& SCOPE_NAME,
    const std::string& COL_NAME);
std::tuple<couchbase::cluster, couchbase::bucket, couchbase::scope, couchbase::collection> InitCluster();

//Create and Update
int UpsertFromFile(couchbase::collection& col, const std::string& doc_id, const std::string& file_path);
int UpsertFromString(couchbase::collection& col, const std::string& doc_id, const std::string& json_doc);
int InsertFromFile(couchbase::collection& col, const std::string& doc_id, const std::string& file_path);
int InsertFromString(couchbase::collection& col, const std::string& doc_id, const std::string& json_doc);

// Read
tao::json::value Read(couchbase::collection& col, const std::string& doc_id);

// Delete
int Delete(couchbase::collection& col, const std::string& doc_id);

// Query
couchbase::query_result Query(couchbase::scope& scope, std::string& query, const couchbase::query_options& opts = couchbase::query_options{});

// Search Index
bool searchIndexExists(couchbase::scope_search_index_manager& sc_manager, const std::string& index_name);
std::string CreateSearchIndex(couchbase::scope& scope, const std::string& index_file);
std::vector<std::string> SearchByName(couchbase::scope& scope, const std::string& index, const std::string& name, const std::string& field, int limit=-1);
std::vector<std::string> Filter(couchbase::scope& scope, const std::string& index_name, int limit=-1, int offset=-1);