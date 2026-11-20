#include <gtest/gtest.h>
#include "search_index_definition.h"

TEST(SearchIndexDefinitionTest, MapsTopLevelFields){
    auto definition = tao::json::from_string(R"({
        "name": "my_index",
        "type": "fulltext-index",
        "sourceType": "gocbcore",
        "sourceName": "travel-sample",
        "planParams": {"indexPartitions": 1, "numReplicas": 0},
        "params": {"doc_config": {"mode": "scope.collection.type_field"}}
    })");

    auto index = searchIndexFromJson(definition);

    EXPECT_EQ(index.name, "my_index");
    EXPECT_EQ(index.type, "fulltext-index");
    EXPECT_EQ(index.source_name, "travel-sample");
    EXPECT_EQ(index.source_type, "gocbcore");
    ASSERT_TRUE(index.params_json.has_value());
    ASSERT_TRUE(index.plan_params_json.has_value());
    EXPECT_EQ(tao::json::from_string(*index.params_json), definition.at("params"));
    EXPECT_EQ(tao::json::from_string(*index.plan_params_json), definition.at("planParams"));
    EXPECT_FALSE(index.uuid.has_value());
}

TEST(SearchIndexDefinitionTest, MissingRequiredFieldThrows){
    auto definition = tao::json::from_string(R"({"name": "no_source", "params": {}, "planParams": {}})");
    EXPECT_THROW(searchIndexFromJson(definition), std::out_of_range);
}

// The shipped definitions must stay loadable; main.cpp and the integration tests depend on them.
TEST(SearchIndexDefinitionTest, LoadsShippedDefinitionFiles){
    for(const auto& [file, name] : std::vector<std::pair<std::string, std::string>>{
            {"hotel_search_index.json", "hotel_search1"},
            {"test_index.json", "hotel_search_test"},
            {"test_index2.json", "hotel_search_test2"}}){
        auto index = loadSearchIndexDefinition(file);
        EXPECT_EQ(index.name, name) << file;
        EXPECT_EQ(index.source_name, "travel-sample") << file;
        EXPECT_EQ(index.source_type, "gocbcore") << file;
    }
}
