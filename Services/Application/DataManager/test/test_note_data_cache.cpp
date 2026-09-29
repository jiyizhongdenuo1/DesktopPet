/*
 * @file: test_note_data_cache.cpp
 * @brief: NoteDataCache 单元测试
 * @author: nuo
 * @date: 2026/7/25
 */

#include <gtest/gtest.h>
#include "CNoteDataCache.h"
#include "DDataCache.h"
#include <vector>

using namespace std;

class NoteDataCacheTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_cache = make_shared<CNoteDataCache>();
    }

    shared_ptr<CNoteDataCache> m_cache;
};

// 测试1：初始状态为空
TEST_F(NoteDataCacheTest, InitiallyEmpty) {
    auto snapshot = m_cache->GetCache();

    EXPECT_EQ(snapshot, nullptr);
}

// 测试2：更新缓存
TEST_F(NoteDataCacheTest, UpdateCache) {
    vector<ST_NOTE_DATA> testData(5);

    m_cache->UpdateNoteDataCache(testData);

    auto snapshot = m_cache->GetCache();
    ASSERT_NE(snapshot, nullptr);
    EXPECT_EQ(snapshot->size(), DDataCache::MAX_CACHE_SIZE);
}

// 测试3：失效缓存
TEST_F(NoteDataCacheTest, InvalidateCache) {
    vector<ST_NOTE_DATA> testData(3);
    m_cache->UpdateNoteDataCache(testData);

    // 验证数据存在
    auto before = m_cache->GetCache();
    ASSERT_NE(before, nullptr);

    // 失效缓存
    m_cache->InvalidateCache();

    // 验证已清空
    auto after = m_cache->GetCache();
    EXPECT_NE(after, nullptr);
    EXPECT_EQ((*after)[0].m_s32id, 0);
}

// 测试4：Copy-on-Write 线程安全性
TEST_F(NoteDataCacheTest, CopyOnWriteSafety) {
    vector<ST_NOTE_DATA> originalData(3);
    originalData[0].m_s32id = 10;
    originalData[1].m_s32id = 20;
    originalData[2].m_s32id = 30;
    
    m_cache->UpdateNoteDataCache(originalData);

    // 模拟读者获取快照
    auto readerSnapshot = m_cache->GetCache();
    ASSERT_NE(readerSnapshot, nullptr);

    // 写入者更新数据（不应影响读者的快照）
    vector<ST_NOTE_DATA> newData(2);
    newData[0].m_s32id = 100;
    newData[1].m_s32id = 200;
    
    m_cache->UpdateNoteDataCache(newData);

    // 验证读者仍然持有旧数据
    EXPECT_EQ(readerSnapshot->size(), DDataCache::MAX_CACHE_SIZE);
    EXPECT_EQ((*readerSnapshot)[0].m_s32id, 10);

    // 新的读取者应该看到新数据
    auto newReaderSnapshot = m_cache->GetCache();
    ASSERT_NE(newReaderSnapshot, nullptr);
    EXPECT_EQ(newReaderSnapshot->size(), DDataCache::MAX_CACHE_SIZE);
    EXPECT_EQ((*newReaderSnapshot)[0].m_s32id, 100);
}