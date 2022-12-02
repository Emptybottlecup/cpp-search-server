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
        ASSERT_EQUAL(server.GetDocumentCount(), 0);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_EQUAL(server.GetDocumentCount(), 1);
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
        server.SetStopWords("in");
        auto [words_1, status_1] = server.MatchDocument("in", doc_id);
        ASSERT_EQUAL(words_1.size(), 0);
        auto [words, status] = server.MatchDocument("in the", doc_id);
        ASSERT_EQUAL(words.size(), 1);
        ASSERT(DocumentStatus::ACTUAL == status);
        vector<string> raw_query = SplitIntoWords("in the");
        int value = 0;
        for (auto word : raw_query) {
            value += count(words.begin(), words.end(), word);
        }
        ASSERT(value >= 1);



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
        ASSERT(documents.size() >= 2);
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
        int all_score = 0;
        for (int score : ratings) {
            all_score += score;
        }
        ASSERT_EQUAL(documents.size(), 1);
        ASSERT_EQUAL(all_score / ratings.size(), documents[0].rating);

    }

}
void TestCalcRating() {
    SearchServer server;
    server.AddDocument(1, "Amg mercedes 5 "s, DocumentStatus::ACTUAL, { 1, 2, 3 });
    server.AddDocument(2, "Bmw m5 cs"s, DocumentStatus::ACTUAL, { 0, 0, 0 });
    server.AddDocument(3, "Kebab Turkey"s, DocumentStatus::ACTUAL, { 10, 20, 30 });
    const auto documents = server.FindTopDocuments("Kebab cs 5");
    ASSERT(documents[0].rating == 20);
    ASSERT(documents[1].rating == 2);
    ASSERT(documents[2].rating == 0);
}

void TestPredicate() {
    SearchServer server;
    server.AddDocument(1, "Amg mercedes 5 "s, DocumentStatus::ACTUAL, { 1, 2, 3 });
    server.AddDocument(2, "Bmw m5 cs"s, DocumentStatus::ACTUAL, { 0, 0, 0 });
    server.AddDocument(3, "Kebab Turkey"s, DocumentStatus::ACTUAL, { 10, 20, 30 });
    const auto documents = server.FindTopDocuments("Kebab cs 5", [](int document_id, [[maybe_unused]] DocumentStatus status, [[maybe_unused]] int rating) { return document_id % 2 == 0; });
    ASSERT(documents.size() == 1);
    ASSERT(documents[0].id == 2);
}
void TestStatusFilter() {

    SearchServer server;
    server.AddDocument(1, "Amg mercedes 5 "s, DocumentStatus::ACTUAL, { 1, 2, 3 });
    server.AddDocument(2, "Bmw m5 cs"s, DocumentStatus::BANNED, { 0, 0, 0 });
    server.AddDocument(3, "Kebab Turkey"s, DocumentStatus::REMOVED, { 10, 20, 30 });
    server.AddDocument(4, "Kebab swarma"s, DocumentStatus::REMOVED, { 10, 20, 30 });
    const auto documents = server.FindTopDocuments("Kebab cs 5", DocumentStatus::ACTUAL);
    ASSERT(documents.size() == 1);
    ASSERT(documents[0].id == 1);
    const auto documents_2 = server.FindTopDocuments("Kebab cs 5", DocumentStatus::BANNED);
    ASSERT(documents_2.size() == 1);
    ASSERT(documents_2[0].id == 2);
    const auto  documents_3 = server.FindTopDocuments("Kebab cs 5", DocumentStatus::REMOVED);
    ASSERT(documents_3.size() == 2);
    ASSERT(documents_3[0].id == 3);
    ASSERT(documents_3[1].id == 4);
}





void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestAddDocument);
    RUN_TEST(TestMinusWords);
    RUN_TEST(TestSort);
    RUN_TEST(TestRating);
    RUN_TEST(TestMatching);
    RUN_TEST(TestCalcRating);
    RUN_TEST(TestPredicate);
    RUN_TEST(TestStatusFilter);
}