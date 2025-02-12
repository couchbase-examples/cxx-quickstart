#include "cxx_client.h"

std::vector<std::string> parseEnvironmentVariables(const std::vector<std::string>& keys){
    std::vector<std::string> res(keys.size());
    for(int i=0; i<keys.size(); i++){
        auto x = keys[i];
        if(std::getenv(x.c_str())){
            res[i] = std::getenv(x.c_str());
        }
        else{
            res.push_back(""); 
        }

    }
    return res;
}

bool checkScopeAndColExists(couchbase::bucket& bucket, const std::string& scope_name, const std::string& col_name){
    auto manager = bucket.collections();
    auto [all_err, all_scopes] = manager.get_all_scopes().get();
    if(all_err.ec()){
        std::cout << "Error fetching all scopes\t" << all_err.message() << std::endl;
        return false;
    }
    for(auto x:all_scopes){
        if(x.name == scope_name){
            for(auto y:x.collections){
                if(y.name == col_name){
                    return true;
                }
            }
            break;
        }
    }
    return false;
}

bool checkSearchEnabled(couchbase::cluster& cluster, int min_nodes = 1){
    auto [p_err, p_res] = cluster.ping().get();
    if(p_err.ec()){
        std::cout << "Error contacting the Search Service, make sure the nodes have it enabled." << std::endl;
        return false;
    }
    else{
        auto ends = p_res.endpoints()[couchbase::service_type::search];
        int num_found = 0;
        for(auto x:ends){
            if(x.state() == couchbase::ping_state::ok){
                num_found++;
            }
        }
        return (num_found >= min_nodes);
    }
}

std::tuple<couchbase::cluster, couchbase::bucket, couchbase::scope, couchbase::collection> connectCluster(const std::string& DB_CONN_STRING,
    const std::string& DB_USERNAME,
    const std::string& DB_PASSWD,
    const std::string& BUCKET_NAME,
    const std::string& SCOPE_NAME,
    const std::string& COL_NAME)
    {
    auto options = couchbase::cluster_options(DB_USERNAME, DB_PASSWD);
    auto [connect_err, cluster] = couchbase::cluster::connect(DB_CONN_STRING, options).get();
    if(connect_err){
        std::cout << "Problem in connecting to cluster " << connect_err.message() << std::endl;
    }


    auto bucket = cluster.bucket(BUCKET_NAME);
    if(!checkScopeAndColExists(bucket, SCOPE_NAME, COL_NAME)){
        std::cout << "Invalid Scope or Collection name" << std::endl;
    }

    auto scope = bucket.scope(SCOPE_NAME);
    auto col = scope.collection(COL_NAME);

    auto [ex_err, ex_res] = col.exists("airline_10123").get();
    if(ex_err.ec()){
        std::cout << "Problem Checking existence\t" << ex_err.message() << std::endl; 
    }

    return {cluster, bucket, scope, col};
}

std::tuple<couchbase::cluster, couchbase::bucket, couchbase::scope, couchbase::collection>  InitCluster()
{   
    std::vector<std::string> keys = {
        "DB_CONN_STR",
        "DB_USERNAME",
        "DB_PASSWORD",
        "BUCKET_NAME",
        "SCOPE_NAME",
        "COL_NAME"
    };

    std::vector<std::string> res = parseEnvironmentVariables(keys);

    auto DB_CONN_STRING = res[0], DB_USERNAME = res[1], DB_PASSWD = res[2], BUCKET_NAME = res[3], SCOPE_NAME = res[4], COL_NAME = res[5];
    return connectCluster(
        DB_CONN_STRING,
        DB_USERNAME,
        DB_PASSWD,
        BUCKET_NAME,
        SCOPE_NAME,
        COL_NAME
    );
    
}

int UpsertFromFile(couchbase::collection& col, const std::string& doc_id, const std::string& file_path){
    tao::json::value v = tao::json::from_file(file_path);
    auto [up_error, up_res] = col.upsert(doc_id, v).get();
    if(up_error.ec()){
        std::cout << "Problem Upserting doc_id:\t" << doc_id << std::endl;
        return 0;
    }
    else return 1;
}

int UpsertFromString(couchbase::collection& col, const std::string& doc_id, const std::string& json_doc){
    tao::json::value v = tao::json::from_string(json_doc);
    auto [up_error, up_res] = col.upsert(doc_id, v).get();
    if(up_error.ec()){
        std::cout << "Problem Upserting doc_id:\t" << doc_id << "\t" << up_error.message() << std::endl;
        return 0;
    }
    else return 1;
}

int InsertFromFile(couchbase::collection& col, const std::string& doc_id, const std::string& file_path){
    tao::json::value v = tao::json::from_file(file_path);
    auto [in_error, in_res] = col.insert(doc_id, v).get();
    if(in_error.ec()){
        std::cout << "Problem Insert doc_id:\t" << doc_id << std::endl;
        return 0;
    }
    else return 1;
}

int InsertFromString(couchbase::collection& col, const std::string& doc_id, const std::string& json_doc){
    tao::json::value v = tao::json::from_string(json_doc);
    auto [in_error, in_res] = col.insert(doc_id, v).get();
    if(in_error.ec()){
        std::cout << "Problem Insert doc_id:\t" << doc_id << "\t" << in_error.message() << std::endl;
        return 0;
    }
    else return 1;
}

tao::json::value Read(couchbase::collection& col, const std::string& doc_id){
    auto [ex_err, ex_res] = col.exists(doc_id).get();
    if(ex_err.ec()){
        std::cout << "Problem Checking existence\t" << ex_err.message() << std::endl; 
        return tao::json::value({});
    }
    std::cout << ex_res.exists() << std::endl;
    auto [get_err, get_res] = col.get(doc_id).get();
    if(get_err.ec()){
        std::cout << "Problem Reading doc_id:\t" << doc_id << "\t" << get_err.ec() << std::endl;
        return tao::json::value({});
    }
    auto doc = get_res.content_as<tao::json::value>();
    return doc;
}

int Delete(couchbase::collection& col, const std::string& doc_id){
    auto [delete_err, delete_res] = col.remove(doc_id).get();
    if(delete_err.ec()){
        std::cout << "Problem Deleting doc_id:\t" << doc_id << std::endl;
        return 0;
    }
    return 1;
}

couchbase::query_result Query(couchbase::scope& scope, std::string& query, const couchbase::query_options& opts){
    auto [q_err, q_res] = scope.query(query, opts).get();
    if(q_err.ec()){
        std::cout << "Error executing query: " << q_err.message() << std::endl;
        return couchbase::query_result{};
    }

    return q_res; 

}

bool searchIndexExists(couchbase::scope_search_index_manager& sc_manager, const std::string& index_name){
    auto [all_err, all_ind] = sc_manager.get_all_indexes().get();
    for(auto x:all_ind){
        if(x.name == index_name) return true;
    }
    return false;
}

std::string CreateSearchIndex(couchbase::scope& scope, const std::string& index_file){
    auto scope_index_manager = scope.search_indexes();
    tao::json::value v = tao::json::from_file(index_file);

    if(searchIndexExists(scope_index_manager, v["name"].get_string())){
        std::cout << "Index with same name already exists." << std::endl;
        return v["name"].get_string();
    }

    // Need to build up members manually
    couchbase::management::search::index i;
    i.name = v["name"].get_string();
    i.params_json = tao::json::to_string(v["params"]);
    i.plan_params_json = tao::json::to_string(v["planParams"]);
    i.source_name = v["sourceName"].get_string();
    i.source_type = v["sourceType"].get_string();

    auto err = scope_index_manager.upsert_index(i).get();
    if(err.ec()){
        std::cout << "Error Upserting Index\t" << err.message() << std::endl;
        return "";
    }
    return i.name;
}

std::vector<std::string> SearchByName(couchbase::scope& scope, const std::string& index, const std::string& name, const std::string& field, int limit){
    couchbase::search_request searchQ(couchbase::match_query(name).field(field));
    couchbase::search_options opts = couchbase::search_options().limit(limit);

    auto [s_err, s_res] = scope.search(index, searchQ, opts).get();

    if(s_err.ec()){
        std::cout << "Error during search\t" << s_err.message() << std::endl;
        return std::vector<std::string>{}; 
    }
    std::vector<std::string> rows_res{};
    // Reference is important since the copy constructor is deleted
    for(auto &row:s_res.rows()){
        rows_res.push_back(row.id());
    }
    return rows_res;
}

std::vector<std::string> Filter(couchbase::scope& scope, const std::string& index_name, int limit, int offset){
    auto query = couchbase::conjunction_query{
        couchbase::match_query("United States").field("country"),
        couchbase::term_query("San Diego").field("city")
    };

    auto opts = couchbase::search_options().fields({"*"});
    if(offset != -1) opts = opts.skip(offset);
    if(limit != -1) opts = opts.limit(limit);
    auto [err,res] = scope.search(index_name, couchbase::search_request(query), opts).get();

    if(err.ec()){
        std::cout << "Problem in performing search:\t" << err.message() << std::endl;
    }
    
    std::vector<std::string> rows_res{};
    // Reference is important since the copy constructor is deleted
    for(auto &row:res.rows()){
        rows_res.push_back(row.id());
    }
    return rows_res;
    
}
