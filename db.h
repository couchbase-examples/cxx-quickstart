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
