/*
 * @file: CTestDataRW.cpp
 * @brief: 数据读写模块测试实现
 * @author: nuo
 * @date: 2026/7/7
 * @Detail: 测试 CNoteDataRW 类的文件读写功能
 */

#include "CTestDataRW.h"
#include "CNoteDataRW.h"
#include "DServiceBase.h"
#include <QDateTime>
#include <QDir>
#include <cstring>

void CTestDataRW::initTestCase()
{
    // 初始化被测对象
    m_pNoteDataRW = new CNoteDataRW();
    QVERIFY2(m_pNoteDataRW != nullptr, "Failed to create CNoteDataRW instance");

    // 设置测试文件路径
    m_testFilePath = QDir::tempPath().toStdString() + "/test_notes.dat";
}

void CTestDataRW::cleanupTestCase()
{
    // 清理测试文件
    QFile::remove(QString::fromStdString(m_testFilePath));

    // 清理资源
    delete m_pNoteDataRW;
    m_pNoteDataRW = nullptr;
}

void CTestDataRW::testSaveNoteData()
{
    // 创建测试数据
    ST_NOTE_DATA testData;
    testData.m_s32id = 1;
    std::strcpy(testData.m_szContent, "Test note content for saving");
    testData.m_eNoteLevel = E_NOTE_EVENT_WAKEUP_LEVEL_NORMAL;
    testData.m_eEventType = E_NOTE_EVENT_NONE;
    testData.m_s64NoteTime = QDateTime::currentMSecsSinceEpoch() / 1000;
    testData.m_bCompleted = FALSE;
    testData.m_bDeleted = FALSE;

    // 执行保存
    m_pNoteDataRW->SaveNoteData(m_testFilePath, testData);

    // 验证文件是否创建
    QFile file(QString::fromStdString(m_testFilePath));
    QVERIFY2(file.exists(), "File should be created after saving");
}

void CTestDataRW::testReadNoteData()
{
    // 先确保有测试数据
    ST_NOTE_DATA testData;
    testData.m_s32id = 2;
    std::strcpy(testData.m_szContent, "Test note for reading");
    testData.m_s64NoteTime = QDateTime::currentMSecsSinceEpoch() / 1000;
    m_pNoteDataRW->SaveNoteData(m_testFilePath, testData);

    // 执行读取
    std::vector<ST_NOTE_DATA> readData;
    m_pNoteDataRW->ReadNoteData(m_testFilePath, readData);

    // 验证读取结果
    QVERIFY2(readData.size() > 0, "Should read at least one note");
}

void CTestDataRW::testSaveAndReadConsistency()
{
    // 创建测试数据
    ST_NOTE_DATA originalData;
    originalData.m_s32id = 3;
    const char* testContent = "Consistency test note content";
    std::strcpy(originalData.m_szContent, testContent);
    originalData.m_eNoteLevel = E_NOTE_EVENT_WAKEUP_LEVEL_IMPORTANT;
    originalData.m_eEventType = E_NOTE_EVENT_WORK;
    originalData.m_s64NoteTime = 1609459200; // 2021-01-01 00:00:00
    originalData.m_s64RemindTime = 1609545600; // 2021-01-02 00:00:00
    originalData.m_bCompleted = FALSE;

    // 保存数据
    m_pNoteDataRW->SaveNoteData(m_testFilePath, originalData);

    // 读取数据
    std::vector<ST_NOTE_DATA> readData;
    m_pNoteDataRW->ReadNoteData(m_testFilePath, readData);

    // 验证数据一致性
    QVERIFY2(readData.size() >= 1, "Should have at least one note");
    ST_NOTE_DATA& readNote = readData[0];

    // 验证 ID
    QCOMPARE(readNote.m_s32id, originalData.m_s32id);

    // 验证内容
    QCOMPARE(std::string(readNote.m_szContent), std::string(originalData.m_szContent));

    // 验证等级
    QCOMPARE(readNote.m_eNoteLevel, originalData.m_eNoteLevel);

    // 验证类型
    QCOMPARE(readNote.m_eEventType, originalData.m_eEventType);

    // 验证时间
    QCOMPARE(readNote.m_s64NoteTime, originalData.m_s64NoteTime);
}

void CTestDataRW::testFileAutoCreate()
{
    // 删除可能存在的测试文件
    QFile::remove(QString::fromStdString(m_testFilePath));

    // 创建新数据
    ST_NOTE_DATA testData;
    testData.m_s32id = 4;
    std::strcpy(testData.m_szContent, "Auto create test");
    testData.m_s64NoteTime = QDateTime::currentMSecsSinceEpoch() / 1000;

    // 保存到不存在的文件
    m_pNoteDataRW->SaveNoteData(m_testFilePath, testData);

    // 验证文件自动创建
    QFile file(QString::fromStdString(m_testFilePath));
    QVERIFY2(file.exists(), "File should be automatically created");
}

void CTestDataRW::testDataIntegrity()
{
    // 创建包含特殊字符的测试数据
    ST_NOTE_DATA testData;
    testData.m_s32id = 5;
    std::strcpy(testData.m_szContent, "Note with special chars: 你好世界! @#$%^&*() 测试内容");
    testData.m_eNoteLevel = E_NOTE_EVENT_WAKEUP_LEVEL_URGENT;
    testData.m_bCompleted = TRUE;
    testData.m_bDeleted = FALSE;

    // 保存数据
    m_pNoteDataRW->SaveNoteData(m_testFilePath, testData);

    // 读取数据
    std::vector<ST_NOTE_DATA> readData;
    m_pNoteDataRW->ReadNoteData(m_testFilePath, readData);

    // 验证完整性
    QVERIFY2(readData.size() >= 1, "Should have data");
    ST_NOTE_DATA& readNote = readData[0];

    // 验证特殊字符内容
    QCOMPARE(std::string(readNote.m_szContent), std::string(testData.m_szContent));

    // 验证状态标记
    QCOMPARE(readNote.m_bCompleted, testData.m_bCompleted);
    QCOMPARE(readNote.m_bDeleted, testData.m_bDeleted);
}

void CTestDataRW::testMultipleNotesSaveRead()
{
    // 清除旧文件
    QFile::remove(QString::fromStdString(m_testFilePath));

    // 保存多条笔记
    const int noteCount = 3;
    for (int i = 0; i < noteCount; ++i) {
        ST_NOTE_DATA testData;
        testData.m_s32id = i + 1;
        char content[100];
        std::sprintf(content, "Note %d: Test content", i + 1);
        std::strcpy(testData.m_szContent, content);
        testData.m_s64NoteTime = QDateTime::currentMSecsSinceEpoch() / 1000 + i;

        // 追加到文件末尾
        m_pNoteDataRW->SaveNoteData(m_testFilePath, testData, i);
    }

    // 读取所有笔记
    std::vector<ST_NOTE_DATA> readData;
    m_pNoteDataRW->ReadNoteData(m_testFilePath, readData);

    // 验证数量
    QVERIFY2(readData.size() >= noteCount, "Should have at least 3 notes");

    // 验证内容
    for (int i = 0; i < noteCount && i < static_cast<int>(readData.size()); ++i) {
        char expected[100];
        std::sprintf(expected, "Note %d: Test content", i + 1);
        QCOMPARE(std::string(readData[i].m_szContent), std::string(expected));
    }
}

// 注册测试
QTEST_APPLESS_MAIN(CTestDataRW)
#include "CTestDataRW.moc"
