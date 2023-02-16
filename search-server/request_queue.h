# pragma once

#include "search_server.h"
#include <deque>

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server);
    
    template <typename DocumentPredicate>
    vector<Document> AddFindRequest(const string& raw_query, DocumentPredicate document_predicate) {
        auto const results = search_server.FindTopDocuments(raw_query, document_predicate);
        int size = results.size();
        requests_.push_back({now_time,size});
        ++now_time;
        if(results.size() == 0){
            ++NoResultRequests;
        }
        
        if(now_time > min_in_day_){
            if(requests_.front().document_result == 0){
                --NoResultRequests;
            }
            requests_.pop_front();
        }
        
        return results;}
    
    vector<Document> AddFindRequest(const string& raw_query, DocumentStatus status);
    
    vector<Document> AddFindRequest(const string& raw_query);
    
    int GetNoResultRequests() const;
    
    int time();
    
private:
    struct QueryResult {
        int time_push;
        int document_result;
        QueryResult(int time, int result) : time_push(time), document_result(result){};
    };
    
    deque<QueryResult> requests_;
    const static int min_in_day_ = 1440;
    int now_time = 0;
    int NoResultRequests = 0;
    const SearchServer& search_server;
};