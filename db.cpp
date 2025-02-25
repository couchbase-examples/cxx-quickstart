#include "db.h"

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

