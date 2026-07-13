/*
* @file: CTestDataRW.h
 * @brief: 数据读写模块测试
 * @author: nuo
 * @date: 2026/7/7
 * @Detail: 测试 CNoteDataRW 类的文件读写功能
 */

#pragma once

#include <QObject>
#include <QtTest/QtTest>
#include <vector>
#include <string>

class CNoteDataRW;

class CTestDataRW : public QObject
{
    Q_OBJECT

private slots:
    // 测试初始化和清理
    void initTestCase();
    void cleanupTestCase();

    // 测试用例
    void testSaveNoteData();
    void testReadNoteData();
    void testSaveAndReadConsistency();
    void testFileAutoCreate();
    void testDataIntegrity();
    void testMultipleNotesSaveRead();

private:
    CNoteDataRW* m_pNoteDataRW;
    std::string m_testFilePath;
};