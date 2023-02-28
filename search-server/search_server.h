#pragma once
#include "document.h"
#include "string_processing.h"
#include "paginator.h"
#include <map>
#include <cmath>

const int MAX_RESULT_DOCUMENT_COUNT = 5;
constexpr auto epsilon = 1e-6;
using namespace std;
 
class SearchServer {
public:
    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words)
        : stop_words_(MakeUniqueNonEmptyStrings(stop_words))  // Extract non-empty stop words
    {
        if (!all_of(stop_words_.begin(), stop_words_.end(), IsValidWord)) {
            throw invalid_argument("Some of stop words are invalid"s);
        }
    }
    
    explicit SearchServer(const string& stop_words_text);
 
    void AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings);
 
    vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus status) const;
 
    vector<Document> FindTopDocuments(const string& raw_query) const;
 
    int GetDocumentCount() const;
  
const std::set<int>::iterator begin() const;

const std::set<int>::iterator end() const;
    
    const std::map<string, double>& GetWordFrequencies(int document_id) const;
   
    void RemoveDocument(int document_id);
    
    const map<set<string>, set<int>>& GetWordsIds();
 
    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query, int document_id) const;
    
    
 template <typename DocumentPredicate>
    vector<Document> FindTopDocuments(const string& raw_query, DocumentPredicate document_predicate) const;
    
private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };
    const set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    map<int, map<string, double>> document_id_word_freqs_;
    map<int, DocumentData> documents_;
    set<int> document_ids_;
    map<string, double> scum;
    map<set<string> , set<int>> words_ids;
    bool IsStopWord(const string& word) const;
 
    static bool IsValidWord(const string& word);
 
    vector<string> SplitIntoWordsNoStop(const string& text) const;
 
    static int ComputeAverageRating(const vector<int>& ratings);
 
    struct QueryWord {
        string data;
        bool is_minus;
        bool is_stop;
    };
 
    QueryWord ParseQueryWord(const string& text) const;
 
    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };
 
    Query ParseQuery(const string& text) const;
    
    template <typename DocumentPredicate>
    vector<Document> FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const;
 
    // Existence required
    double ComputeWordInverseDocumentFreq(const string& word) const;
 
};

void PrintDocument(const Document& document);
 
void PrintMatchDocumentResult(int document_id, const vector<string>& words, DocumentStatus status);
 
void AddDocument(SearchServer& search_server, int document_id, const string& document, DocumentStatus status,
    const vector<int>& ratings);
 
void FindTopDocuments(const SearchServer& search_server, const string& raw_query);


 template <typename DocumentPredicate>
vector<Document> SearchServer::FindTopDocuments(const string& raw_query, DocumentPredicate document_predicate) const {
        const auto query = ParseQuery(raw_query);
 
        auto matched_documents = FindAllDocuments(query, document_predicate);
 
        sort(matched_documents.begin(), matched_documents.end(), [](const Document& lhs, const Document& rhs) {
            if (abs(lhs.relevance - rhs.relevance) < epsilon) {
                return lhs.rating > rhs.rating;
            }
            else {
                return lhs.relevance > rhs.relevance;
            }
        });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
 
        return matched_documents;
    }
    
template <typename DocumentPredicate>
vector<Document> SearchServer::FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const {
        map<int, double> document_to_relevance;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                const auto& document_data = documents_.at(document_id);
                if (document_predicate(document_id, document_data.status, document_data.rating)) {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
                }
            }
        }
 
        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }
 
        vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back({ document_id, relevance, documents_.at(document_id).rating });
        }
        return matched_documents;
    }