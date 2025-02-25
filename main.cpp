#include "db.h"
#include "operations.h"

int main(){
    //Important to get all the vars since if it goes out of scope, it won't work.
    auto [cluster, bucket, scope, col] = InitCluster();

    
    
    
    // Reading a document
    std::cout << "Reading doc id:\t" << "airline_10123" << std::endl;
    auto v = Read(col, "airline_10123");
    std::cout << tao::json::to_string(v) << std::endl;
    std::cout << std::endl;
    
    // Upserting a document from string
    std::cout << "Upserting doc id:\t" << "quickstart_test" << std::endl;
    auto upsert_res = Upsert(col, "quickstart_test", "{ \"test\": \"hello\"}", false);
    std::cout << std::endl;
    
    // Creates a search index from the given JSON file
    std::cout << "Creating a search index using the file:\t" << "hotel_search_index.json" << std::endl;
    std::string index_name = CreateSearchIndex(scope, "hotel_search_index.json");
    std::cout << std::endl;
    
    // Performs Search using the given index name with the given name, field and limt.
    std::cout << "Search by name for:\t" << "swanky with limit 50" << std::endl;
    auto search_res = SearchByName(scope, index_name, "swanky", "name", 50);
    std::cout << "Search result contains:\t" << search_res.size() << std::endl;
    std::cout << std::endl;
    
    //Executing a query with positional Parameters
    std::cout << "Executing a Query with positional parameters" << std::endl;
    std::vector<std::string> query_res = Query(scope);
    for (auto& row : query_res) {
        std::cout << row << std::endl;
    }
    std::cout << std::endl;
    
    //Filter
    std::cout << "Filter with conjunction of MatchQuery and TermQuery" << std::endl;
    auto filter_res = Filter(scope, index_name, 50, 1);
    std::cout << "Filter result contains:\t" << filter_res.size() << std::endl;


    return 0;
}