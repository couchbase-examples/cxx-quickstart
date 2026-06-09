#include <gtest/gtest.h>
#include "db.h"
#include "operations.h"

class CxxQuickStartTest : public testing::Test {
  protected:
    CxxQuickStartTest()
    {
        std::vector<std::string> keys = {
          "DB_CONN_STR",
          "DB_USERNAME",
          "DB_PASSWORD",
          "BUCKET_NAME",
          "SCOPE_NAME",
          "COL_NAME",
        };
        auto env_values = parseEnvironmentVariables(keys);

        DB_CONN_STRING = env_values[0].empty() ? "couchbase://127.0.0.1" : env_values[0];
        DB_USERNAME = env_values[1].empty() ? "Administrator" : env_values[1];
        DB_PASSWD = env_values[2].empty() ? "password" : env_values[2];
        BUCKET_NAME = env_values[3].empty() ? "travel-sample" : env_values[3];
        SCOPE_NAME = env_values[4].empty() ? "inventory" : env_values[4];
        COL_NAME = env_values[5].empty() ? "airline" : env_values[5];
        INDEX_NAME = "hotel_search_test";
    }

    std::tuple<couchbase::cluster, couchbase::bucket, couchbase::scope, couchbase::collection> connect()
    {
        return connectCluster(DB_CONN_STRING, DB_USERNAME, DB_PASSWD, BUCKET_NAME, SCOPE_NAME, COL_NAME);
    }

    bool ensureSearchIndex(couchbase::scope& scope)
    {
        auto scope_index_manager = scope.search_indexes();
        auto [all_err, all_ind] = scope_index_manager.get_all_indexes().get();
        if (all_err.ec()) {
            return false;
        }
        for (auto x : all_ind) {
            if (x.name == INDEX_NAME) {
                return true;
            }
        }

        tao::json::value v = tao::json::from_file("test_index.json");
        couchbase::management::search::index i;
        i.name = v["name"].get_string();
        i.params_json = tao::json::to_string(v["params"]);
        i.plan_params_json = tao::json::to_string(v["planParams"]);
        i.source_name = v["sourceName"].get_string();
        i.source_type = v["sourceType"].get_string();

        auto err = scope_index_manager.upsert_index(i).get();
        if (err.ec()) {
            return false;
        }

        for (int attempt = 0; attempt < 90; attempt++) {
            auto [count_err, count] = scope_index_manager.get_indexed_documents_count(INDEX_NAME).get();
            if (!count_err.ec() && count >= DOCS_TARGET) {
                return true;
            }
            sleep(2);
        }
        return false;
    }

    std::string DB_CONN_STRING;
    std::string DB_USERNAME;
    std::string DB_PASSWD;
    std::string BUCKET_NAME;
    std::string SCOPE_NAME;
    std::string COL_NAME;
    std::string INDEX_NAME;
    uint64_t DOCS_TARGET = 917; // Number of docs processed for index in default travel-sample
};

TEST(CxxQuickStartConfigTest, CheckParseEnvironmentVariables)
{
    setenv("TEST_VAR1", "Hello", 1);
    setenv("TEST_VAR2", "Hello2", 1);
    setenv("TEST_VAR1", "Hello3", 0);
    std::vector<std::string> expected{ "Hello", "Hello2" };
    auto res = parseEnvironmentVariables({ "TEST_VAR1", "TEST_VAR2" });
    EXPECT_EQ(res.size(), 2);
    for (int i = 0; i < res.size(); i++) {
        EXPECT_EQ(res[i], expected[i]);
    }
}

TEST_F(CxxQuickStartTest, CheckCreateConnection)
{
    auto [cluster, bucket, scope, col] = connect();

    EXPECT_EQ(scope.bucket_name(), BUCKET_NAME);
    EXPECT_EQ(col.bucket_name(), BUCKET_NAME);
    EXPECT_EQ(scope.name(), SCOPE_NAME);
    EXPECT_EQ(col.scope_name(), SCOPE_NAME);
    EXPECT_EQ(col.name(), COL_NAME);
}

TEST_F(CxxQuickStartTest, CheckCheckScopeColExists)
{
    auto [cluster, bucket, scope, col] = connect();
    auto res = checkScopeAndColExists(bucket, "tenant_agent_00", "bookings");
    ASSERT_TRUE(res);

    res = checkScopeAndColExists(bucket, "tenant-agent_0", "booking");
    ASSERT_FALSE(res);

    res = checkScopeAndColExists(bucket, "tenant_agent_00", "booking");
    ASSERT_FALSE(res);

    res = checkScopeAndColExists(bucket, "tenant-agent_0", "bookings");
    ASSERT_FALSE(res);
}

TEST_F(CxxQuickStartTest, CheckRead)
{
    auto [cluster, bucket, scope, col] = connect();

    std::string test_val{ "{\"test_val\": \"cxx_quickstart_test\"}" };
    tao::json::value v = tao::json::from_string(test_val);
    std::string doc_id{ "cxx_test_1" };
    auto [up_error, up_res] = col.upsert(doc_id, v).get();
    if (up_error.ec()) {
        FAIL();
    }

    auto read_val = Read(col, doc_id);
    EXPECT_EQ(v, read_val);
    auto [del_error, del_res] = col.remove(doc_id).get();
    if (del_error.ec()) {
        FAIL();
    }
}

TEST_F(CxxQuickStartTest, CheckUpsertFromString)
{
    auto [cluster, bucket, scope, col] = connect();

    std::string test_val{ "{\"test_val\": \"cxx_quickstart_test\"}" };
    tao::json::value v = tao::json::from_string(test_val);
    std::string doc_id{ "cxx_test_1" };
    auto upsert_res = Upsert(col, doc_id, test_val, false);
    EXPECT_TRUE(upsert_res);

    auto [ex_err, ex_res] = col.exists(doc_id).get();
    EXPECT_FALSE(ex_err.ec());
    auto [get_err, get_res] = col.get(doc_id).get();
    EXPECT_FALSE(get_err.ec());
    auto doc = get_res.content_as<tao::json::value>();
    EXPECT_EQ(doc, v);
    auto [del_error, del_res] = col.remove(doc_id).get();
    EXPECT_FALSE(del_error.ec());
}

TEST_F(CxxQuickStartTest, CheckUpsertFromFile)
{
    auto [cluster, bucket, scope, col] = connect();

    std::string doc_id{ "cxx_test_1" };
    auto upsert_res = Upsert(col, doc_id, "test_doc.json", true);
    EXPECT_TRUE(upsert_res);

    auto v = tao::json::from_file("test_doc.json");
    auto [ex_err, ex_res] = col.exists(doc_id).get();
    EXPECT_FALSE(ex_err.ec());
    auto [get_err, get_res] = col.get(doc_id).get();
    EXPECT_FALSE(get_err.ec());
    auto doc = get_res.content_as<tao::json::value>();
    EXPECT_EQ(doc, v);
    auto [del_error, del_res] = col.remove(doc_id).get();
    EXPECT_FALSE(del_error.ec());
}

TEST_F(CxxQuickStartTest, CheckInsertFromString)
{
    auto [cluster, bucket, scope, col] = connect();

    std::string test_val{ "{\"test_val\": \"cxx_quickstart_test\"}" };
    tao::json::value v = tao::json::from_string(test_val);
    std::string doc_id{ "cxx_test_1" };
    auto upsert_res = Insert(col, doc_id, test_val, false);
    EXPECT_TRUE(upsert_res);

    auto [ex_err, ex_res] = col.exists(doc_id).get();
    EXPECT_FALSE(ex_err.ec());
    auto [get_err, get_res] = col.get(doc_id).get();
    EXPECT_FALSE(get_err.ec());
    auto doc = get_res.content_as<tao::json::value>();
    EXPECT_EQ(doc, v);
    auto [del_error, del_res] = col.remove(doc_id).get();
    EXPECT_FALSE(del_error.ec());
}

TEST_F(CxxQuickStartTest, CheckInsertFromFile)
{
    auto [cluster, bucket, scope, col] = connect();

    std::string doc_id{ "cxx_test_1" };
    auto upsert_res = Insert(col, doc_id, "test_doc.json", true);
    EXPECT_TRUE(upsert_res);

    auto v = tao::json::from_file("test_doc.json");
    auto [ex_err, ex_res] = col.exists(doc_id).get();
    EXPECT_FALSE(ex_err.ec());
    auto [get_err, get_res] = col.get(doc_id).get();
    EXPECT_FALSE(get_err.ec());
    auto doc = get_res.content_as<tao::json::value>();
    EXPECT_EQ(doc, v);
    auto [del_error, del_res] = col.remove(doc_id).get();
    EXPECT_FALSE(del_error.ec());
}

TEST_F(CxxQuickStartTest, CheckDelete)
{
    auto [cluster, bucket, scope, col] = connect();

    std::string test_val{ "{\"test_val\": \"cxx_quickstart_test\"}" };
    tao::json::value v = tao::json::from_string(test_val);
    std::string doc_id{ "cxx_test_1" };
    auto [up_error, up_res] = col.upsert(doc_id, v).get();
    EXPECT_FALSE(up_error.ec());

    auto [ex_err, ex_res] = col.exists(doc_id).get();
    EXPECT_FALSE(ex_err.ec());
    EXPECT_TRUE(ex_res.exists());

    auto res = Delete(col, doc_id);
    EXPECT_TRUE(res);

    auto [ex_err1, ex_res1] = col.exists(doc_id).get();
    EXPECT_FALSE(ex_err1.ec());
    EXPECT_FALSE(ex_res1.exists());
}

TEST_F(CxxQuickStartTest, CheckQuery)
{
    auto [cluster, bucket, scope, col] = connect();

    std::string expected_val("hotel_26169 United States\n4.750000 San Francisco/Twin Peaks-Lake Merced");

    auto q_res = Query(scope);
    EXPECT_EQ(q_res.size(), 5);
    EXPECT_EQ(q_res[0], expected_val);
}

TEST_F(CxxQuickStartTest, CheckSearchIndexExists)
{
    auto [cluster, bucket, scope, col] = connect();
    ASSERT_TRUE(ensureSearchIndex(scope));

    auto scope_index_manager = scope.search_indexes();
    auto res = searchIndexExists(scope_index_manager, INDEX_NAME);
    EXPECT_TRUE(res);

    res = searchIndexExists(scope_index_manager, INDEX_NAME + "TEST");
    EXPECT_FALSE(res);
}

TEST_F(CxxQuickStartTest, CheckCreateSearchIndex)
{
    auto [cluster, bucket, scope, col] = connect();
    ASSERT_TRUE(ensureSearchIndex(scope));
    auto scope_index_manager = scope.search_indexes();

    auto [all_err, all_ind] = scope_index_manager.get_all_indexes().get();
    bool found = false;
    for (auto x : all_ind) {
        if (x.name == "hotel_search_test2") found = true;
    }
    EXPECT_FALSE(found);

    CreateSearchIndex(scope, "test_index2.json");
    found = false;
    auto [all_err1, all_ind1] = scope_index_manager.get_all_indexes().get();
    for (auto x : all_ind1) {
        if (x.name == "hotel_search_test2") found = true;
    }
    EXPECT_TRUE(found);

    auto err = scope_index_manager.drop_index("hotel_search_test2").get();
    EXPECT_FALSE(err.ec());
    auto [all_err2, all_ind2] = scope_index_manager.get_all_indexes().get();
    found = false;
    for (auto x : all_ind2) {
        if (x.name == "hotel_search_test2") found = true;
    }
    EXPECT_FALSE(found);
}

TEST_F(CxxQuickStartTest, CheckSearchByName)
{
    auto [cluster, bucket, scope, col] = connect();
    ASSERT_TRUE(ensureSearchIndex(scope));

    auto res = SearchByName(scope, INDEX_NAME, "swanky", "name", 50);
    EXPECT_EQ(res.size(), 9);
}

TEST_F(CxxQuickStartTest, CheckFilter)
{
    auto [cluster, bucket, scope, col] = connect();
    ASSERT_TRUE(ensureSearchIndex(scope));

    auto res = Filter(scope, INDEX_NAME, 13, 0);
    EXPECT_EQ(res.size(), 13);

    res = Filter(scope, INDEX_NAME);
    EXPECT_EQ(res.size(), 10);

    res = Filter(scope, INDEX_NAME, 50, 1);
    EXPECT_EQ(res.size(), 47);
    res = Filter(scope, INDEX_NAME, 50);
    EXPECT_EQ(res.size(), 48);
}
