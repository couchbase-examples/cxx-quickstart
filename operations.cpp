#include "operations.h"

int Upsert(couchbase::collection& col, const std::string& doc_id, const std::string& value, bool file_flag){
    tao::json::value v;
    if(file_flag){
        v = tao::json::from_file(value);
    } 
    else{
        v = tao::json::from_string(value);
    }
    auto [up_error, up_res] = col.upsert(doc_id, v).get();
    if(up_error.ec()){
        std::cout << "Problem Upserting doc_id:\t" << doc_id << std::endl;
        return 0;
    }
    else return 1;
}

int Insert(couchbase::collection& col, const std::string& doc_id, const std::string& value, bool file_flag){
    tao::json::value v;
    if(file_flag){
        v = tao::json::from_file(value);
    } 
    else{
        v = tao::json::from_string(value);
    }
    auto [in_error, in_res] = col.insert(doc_id, v).get();
    if(in_error.ec()){
        std::cout << "Problem Insert doc_id:\t" << doc_id << std::endl;
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
