#include "request_queue.h"

RequestQueue::RequestQueue(const SearchServer& search_server) : search_server(search_server){}

vector<Document> RequestQueue::AddFindRequest(const string& raw_query, DocumentStatus status) {
        ++now_time;
        auto const results = search_server.FindTopDocuments(raw_query, status);
        int size = results.size();
        requests_.push_back({now_time,size});
        if(results.size() == 0){
            ++NoResultRequests;
        }
        
        if(now_time > min_in_day_){
            if(requests_.front().document_result == 0){
                --NoResultRequests;
            }
            requests_.pop_front();
        }
        
        return results;
    }

vector<Document> RequestQueue::AddFindRequest(const string& raw_query) {
        auto const results = search_server.FindTopDocuments(raw_query);
        
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
        
        return results;
    }

int RequestQueue::GetNoResultRequests() const {
        return NoResultRequests;
    }


    int RequestQueue::time(){
        return now_time;
    }