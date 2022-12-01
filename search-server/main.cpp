
void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_HINT(server.FindTopDocuments("in"s).empty(),
            "Stop words must be excluded from documents"s);
    }
}
void TestAddDocument() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

}

void TestMinusWords() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };
    const int doc_id_2 = 43;
    const string content_2 = "in the city"s;
    const vector<int> ratings_2 = { 1, 2, 3 };
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id_2, content_2, DocumentStatus::ACTUAL, ratings_2);
        ASSERT_EQUAL(server.FindTopDocuments("in -cat").size(), 1);
    }
}

void TestMatching() {

    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };


    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        auto [word, _] = server.MatchDocument("in the", doc_id);
        ASSERT_EQUAL(word.size(), 2);
    }
}

void TestSort() {
    const int doc_id = 41;
    const string content = "cat fly on "s;
    const vector<int> ratings = { 1, 2, 3 };

    const int doc_id_2 = 43;
    const string content_2 = " in the city"s;
    const vector<int> ratings_2 = { 1, 1, 1 };

    const int doc_id_3 = 42;
    const string content_3 = "Cj from LOS Santos"s;
    const vector<int> ratings_3 = { 1, 2, 3 };
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id_2, content_2, DocumentStatus::ACTUAL, ratings_2);
        server.AddDocument(doc_id_3, content_3, DocumentStatus::ACTUAL, ratings_3);
        vector<Document> documents = server.FindTopDocuments("Cj in", DocumentStatus::ACTUAL);
        ASSERT(documents[0].relevance > documents[1].relevance);
    }


}

void TestRating() {
    const int doc_id = 41;
    const string content = "cat fly on "s;
    const vector<int> ratings = { 1, 2, 3 };
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        vector<Document> documents = server.FindTopDocuments("cat", DocumentStatus::ACTUAL);
        int k = 0;
        for (int s : ratings) {
            k += s;
        }
        ASSERT_EQUAL(documents.size(), 1);
        ASSERT_EQUAL(k / ratings.size(), documents[0].rating);

    }

}
void TestSearchServer() {
    TestExcludeStopWordsFromAddedDocumentContent();
    TestAddDocument();
    TestMinusWords();
    TestSort();
    TestRating();
    TestMatching();
}