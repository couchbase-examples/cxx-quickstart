#include <gtest/gtest.h>
#include "db.h"
#include "operations.h"

class CxxQuickStartTest : public testing::Test{
    protected:
        CxxQuickStartTest(){
            DB_CONN_STRING="couchbase://127.0.0.1";
            DB_USERNAME="Administrator";
            DB_PASSWD="password";
            BUCKET_NAME="travel-sample";
            SCOPE_NAME="inventory";
            COL_NAME="airline";

            auto [cluster, bucket, scope, col] = connectCluster(DB_CONN_STRING, 
            DB_USERNAME, 
            DB_PASSWD, 
            BUCKET_NAME, 
            SCOPE_NAME, 
            COL_NAME);
            INDEX_NAME = "hotel_search_test";
            auto scope_index_manager = scope.search_indexes();
            auto [all_err, all_ind] = scope_index_manager.get_all_indexes().get();
            bool found = false;
            for(auto x:all_ind){
                if(x.name == INDEX_NAME) found = true;
            }
            if(!found){
                tao::json::value v = tao::json::from_file("test_index.json");
                auto scope_index_manager = scope.search_indexes();
                couchbase::management::search::index i;
                i.name = v["name"].get_string();
                i.params_json = tao::json::to_string(v["params"]);
                i.plan_params_json = tao::json::to_string(v["planParams"]);
                i.source_name = v["sourceName"].get_string();
                i.source_type = v["sourceType"].get_string();
                auto err = scope_index_manager.upsert_index(i).get();

                // Blocked waiting till index ingests docs
                while(scope_index_manager.get_indexed_documents_count(INDEX_NAME).get().second < DOCS_TARGET){
                    sleep(2);
                }
            }
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

TEST_F(CxxQuickStartTest, CheckParseEnvironmentVariables){
    setenv("TEST_VAR1", "Hello", 1);
    setenv("TEST_VAR2", "Hello2", 1);
    setenv("TEST_VAR1", "Hello3", 0);
    std::vector<std::string> expected{"Hello", "Hello2"};
    auto res = parseEnvironmentVariables({"TEST_VAR1", "TEST_VAR2"});
    EXPECT_EQ(res.size(), 2);
    for(int i=0; i<res.size(); i++){
        EXPECT_EQ(res[i], expected[i]);
    }
}

TEST_F(CxxQuickStartTest, CheckCreateConnection){
    auto [cluster, bucket, scope, col] = connectCluster(DB_CONN_STRING, 
                                                        DB_USERNAME, 
                                                        DB_PASSWD, 
                                                        BUCKET_NAME, 
                                                        SCOPE_NAME, 
                                                        COL_NAME);

    EXPECT_EQ(scope.bucket_name(), BUCKET_NAME);
    EXPECT_EQ(col.bucket_name(), BUCKET_NAME);
    EXPECT_EQ(scope.name(), SCOPE_NAME);
    EXPECT_EQ(col.scope_name(), SCOPE_NAME);
    EXPECT_EQ(col.name(), COL_NAME);
}

TEST_F(CxxQuickStartTest, CheckCheckScopeColExists){
    auto [cluster, bucket, scope, col] = connectCluster(DB_CONN_STRING, 
                                                        DB_USERNAME, 
                                                        DB_PASSWD, 
                                                        BUCKET_NAME, 
                                                        SCOPE_NAME, 
                                                        COL_NAME);
    //Both Correct
    auto res = checkScopeAndColExists(bucket, "tenant_agent_00", "bookings");
    ASSERT_TRUE(res);

    //Both Wrong
    res = checkScopeAndColExists(bucket, "tenant-agent_0", "booking");
    ASSERT_FALSE(res);

    //Collection is wrong
    res = checkScopeAndColExists(bucket, "tenant_agent_00", "booking");
    ASSERT_FALSE(res);

    //Scope is wrong
    res = checkScopeAndColExists(bucket, "tenant-agent_0", "bookings");
    ASSERT_FALSE(res);

}

TEST_F(CxxQuickStartTest, CheckRead){
    auto [cluster, bucket, scope, col] = connectCluster(DB_CONN_STRING, 
                                                        DB_USERNAME, 
                                                        DB_PASSWD, 
                                                        BUCKET_NAME, 
                                                        SCOPE_NAME, 
                                                        COL_NAME);

    // Insert this string.
    std::string test_val{ "{\"test_val\": \"cxx_quickstart_test\"}" };
    tao::json::value v = tao::json::from_string(test_val);
    std::string doc_id{"cxx_test_1"};
    auto [up_error, up_res] = col.upsert(doc_id, v).get();
    if(up_error.ec()){
        FAIL();
    }

    auto read_val = Read(col, doc_id);
    EXPECT_EQ(v, read_val);
    auto [del_error, del_res] = col.remove(doc_id).get();
    if(del_error.ec()){
        FAIL();
    }
}

TEST_F(CxxQuickStartTest, CheckUpsertFromString){
    auto [cluster, bucket, scope, col] = connectCluster(DB_CONN_STRING, 
                                                        DB_USERNAME, 
                                                        DB_PASSWD, 
                                                        BUCKET_NAME, 
                                                        SCOPE_NAME, 
                                                        COL_NAME);

    // Insert this string.
    std::string test_val{ "{\"test_val\": \"cxx_quickstart_test\"}" };
    tao::json::value v = tao::json::from_string(test_val); //For checking get result
    std::string doc_id{"cxx_test_1"};
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

TEST_F(CxxQuickStartTest, CheckUpsertFromFile){
    auto [cluster, bucket, scope, col] = connectCluster(DB_CONN_STRING, 
                                                        DB_USERNAME, 
                                                        DB_PASSWD, 
                                                        BUCKET_NAME, 
                                                        SCOPE_NAME, 
                                                        COL_NAME);

    std::string doc_id{"cxx_test_1"};
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

TEST_F(CxxQuickStartTest, CheckInsertFromString){
    auto [cluster, bucket, scope, col] = connectCluster(DB_CONN_STRING, 
                                                        DB_USERNAME, 
                                                        DB_PASSWD, 
                                                        BUCKET_NAME, 
                                                        SCOPE_NAME, 
                                                        COL_NAME);

    // Insert this string.
    std::string test_val{ "{\"test_val\": \"cxx_quickstart_test\"}" };
    tao::json::value v = tao::json::from_string(test_val); //For checking get result
    std::string doc_id{"cxx_test_1"};
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

TEST_F(CxxQuickStartTest, CheckInsertFromFile){
    auto [cluster, bucket, scope, col] = connectCluster(DB_CONN_STRING, 
                                                        DB_USERNAME, 
                                                        DB_PASSWD, 
                                                        BUCKET_NAME, 
                                                        SCOPE_NAME, 
                                                        COL_NAME);

    std::string doc_id{"cxx_test_1"};
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

TEST_F(CxxQuickStartTest, CheckDelete){
    auto [cluster, bucket, scope, col] = connectCluster(DB_CONN_STRING, 
                                                        DB_USERNAME, 
                                                        DB_PASSWD, 
                                                        BUCKET_NAME, 
                                                        SCOPE_NAME, 
                                                        COL_NAME);

    std::string test_val{ "{\"test_val\": \"cxx_quickstart_test\"}" };
    tao::json::value v = tao::json::from_string(test_val);
    std::string doc_id{"cxx_test_1"};
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

TEST_F(CxxQuickStartTest, CheckQuery){
    auto [cluster, bucket, scope, col] = connectCluster(DB_CONN_STRING, 
                                                        DB_USERNAME, 
                                                        DB_PASSWD, 
                                                        BUCKET_NAME, 
                                                        SCOPE_NAME, 
                                                        COL_NAME);

    std::string expected_json{R"({
        "airline": {
            "id": 18241,
            "type": "airline",
            "name": "Royal Airways",
            "iata": "KG",
            "icao": "RAW",
            "callsign": "RAW",
            "country": "United States"
        }
    })"};

    auto v = tao::json::from_string(expected_json);
    std::string query = {"SELECT * FROM `travel-sample`.inventory.airline WHERE name='Royal Airways' LIMIT 10"};
    auto q_res = Query(scope, query);
    auto rows = q_res.rows_as<couchbase::codec::tao_json_serializer>();
    EXPECT_EQ(rows.size(), 1);
    EXPECT_EQ(rows[0], v);

}

TEST_F(CxxQuickStartTest, CheckSearchIndexExists){
    auto [cluster, bucket, scope, col] = connectCluster(DB_CONN_STRING, 
                                                        DB_USERNAME, 
                                                        DB_PASSWD, 
                                                        BUCKET_NAME, 
                                                        SCOPE_NAME, 
                                                        COL_NAME);

    
    auto scope_index_manager = scope.search_indexes();
    auto res = searchIndexExists(scope_index_manager, INDEX_NAME);
    EXPECT_TRUE(res);

    res = searchIndexExists(scope_index_manager, INDEX_NAME + "TEST");
    EXPECT_FALSE(res);

}


TEST_F(CxxQuickStartTest, CheckCreateSearchIndex){
    auto [cluster, bucket, scope, col] = connectCluster(DB_CONN_STRING, 
                                                        DB_USERNAME, 
                                                        DB_PASSWD, 
                                                        BUCKET_NAME, 
                                                        SCOPE_NAME, 
                                                        COL_NAME);
    auto scope_index_manager = scope.search_indexes();

    auto [all_err, all_ind] = scope_index_manager.get_all_indexes().get();
    bool found = false;
    for(auto x:all_ind){
        if(x.name == "hotel_search_test2") found = true;
    }
    EXPECT_FALSE(found);

    CreateSearchIndex(scope, "test_index2.json");
    found = false;
    auto [all_err1, all_ind1] = scope_index_manager.get_all_indexes().get();
    for(auto x:all_ind1){
        if(x.name == "hotel_search_test2") found = true;
    }
    EXPECT_TRUE(found);
    
    auto err = scope_index_manager.drop_index("hotel_search_test2").get();
    EXPECT_FALSE(err.ec());
    auto [all_err2, all_ind2] = scope_index_manager.get_all_indexes().get();
    found = false;
    for(auto x:all_ind2){
        if(x.name == "hotel_search_test2") found = true;
    }
    EXPECT_FALSE(found);
}

TEST_F(CxxQuickStartTest, CheckSearchByName){
    auto [cluster, bucket, scope, col] = connectCluster(DB_CONN_STRING, 
                                                        DB_USERNAME, 
                                                        DB_PASSWD, 
                                                        BUCKET_NAME, 
                                                        SCOPE_NAME, 
                                                        COL_NAME);


    auto res = SearchByName(scope, INDEX_NAME, "swanky", "name", 50);
    EXPECT_EQ(res.size(), 9);
}

TEST_F(CxxQuickStartTest, CheckFilter){
    auto [cluster, bucket, scope, col] = connectCluster(DB_CONN_STRING, 
                                                        DB_USERNAME, 
                                                        DB_PASSWD, 
                                                        BUCKET_NAME, 
                                                        SCOPE_NAME, 
                                                        COL_NAME);

    auto res = Filter(scope, INDEX_NAME, 13, 0);
    EXPECT_EQ(res.size(), 13);

    res = Filter(scope, INDEX_NAME);
    EXPECT_EQ(res.size(), 10); 
    
    res = Filter(scope, INDEX_NAME, 50, 1);
    EXPECT_EQ(res.size(), 47);
    res = Filter(scope, INDEX_NAME, 50);
    EXPECT_EQ(res.size(), 48); 
}