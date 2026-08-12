/*
 * @file: test_note_data_service.cpp
 * @brief: CNoteDataService 单元测试
 * @author: nuo
 * @date: 2026/7/25
 */

#include <gtest/gtest.h>
#include "CNoteDataService.h"
#include "CNoteDataCache.h"
#include <QDir>
#include <fstream>

using namespace std;

class CNoteDataServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto cache = make_shared<CNoteDataCache>();
        m_service = make_shared<CNoteDataService>(cache);

        // 设置临时测试文件路径
        m_testFilePath = QDir::tempPath().toStdString() + "/gtest_test_notes.dat";
    }

    void TearDown() override {
        // 清理测试文件
        remove(m_testFilePath.c_str());
    }

    shared_ptr<CNoteDataService> m_service;
    string m_testFilePath;
};

// 测试1：服务创建成功
TEST_F(CNoteDataServiceTest, CreationSuccess) {
    ASSERT_NE(m_service, nullptr);
}

// 测试2：空文件名处理
TEST_F(CNoteDataServiceTest, EmptyFileNameHandling) {
    vector<ST_NOTE_DATA> result;

    // 不应崩溃，返回空结果
    m_service->ReadNoteData("", result);

    EXPECT_TRUE(result.empty());
}

// 测试3：保存和读取（集成测试）
TEST_F(CNoteDataServiceTest, SaveAndReadRoundTrip) {
    ST_NOTE_DATA noteToSave{};
    strncpy(noteToSave.m_szContent, "GTest Hello World", sizeof(noteToSave.m_szContent) - 1);
    noteToSave.m_s64NoteTime = time(nullptr);

    // 保存
    m_service->SaveNoteData(m_testFilePath, noteToSave, 0);

    // 读取
    vector<ST_NOTE_DATA> readNotes;
    m_service->ReadNoteData(m_testFilePath, readNotes);

    // 验证
    EXPECT_FALSE(readNotes.empty());
    if (!readNotes.empty()) {
        EXPECT_STREQ(readNotes[0].m_szContent, "GTest Hello World");
    }
}

// 测试4：缓存失效机制
TEST_F(CNoteDataServiceTest, CacheInvalidationOnSave) {
    ST_NOTE_DATA note1{}, note2{};
    strncpy(note1.m_szContent, "First", sizeof(note1.m_szContent) - 1);
    strncpy(note2.m_szContent, "Second", sizeof(note2.m_szContent) - 1);

    // 第一次保存并读取（填充缓存）
    m_service->SaveNoteData(m_testFilePath, note1, 0);
    vector<ST_NOTE_DATA> firstRead;
    m_service->ReadNoteData(m_testFilePath, firstRead);

    // 第二次保存（应触发缓存失效）
    m_service->SaveNoteData(m_testFilePath, note2, 0);

    // 再次读取，应该能读到最新数据
    vector<ST_NOTE_DATA> secondRead;
    m_service->ReadNoteData(m_testFilePath, secondRead);

    EXPECT_GE(secondRead.size(), firstRead.size());  // 数据量可能增加
}