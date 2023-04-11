#include "process_queries.h"
#include <algorithm>
#include <iterator>
#include <execution>

std::vector<std::vector<Document>> ProcessQueries(
    const SearchServer& search_server,
    const std::vector<std::string>& queries) {
    std::vector<std::vector<Document>> vector_return (queries.size());
    std::transform(std::execution::par, queries.begin(),queries.cend(),vector_return.begin(), [&search_server](std::string words) {
        return search_server.FindTopDocuments(words); 
    });
    return vector_return;
}

std::list<Document> ProcessQueriesJoined(const SearchServer& search_server, const std::vector<std::string>& queries) {
    std::list<Document> list_return;
    for(auto doc : ProcessQueries(search_server, queries)) {
        for(auto one_doc : doc) {
            list_return.push_back(one_doc);
        }
    }
    return list_return;   
}