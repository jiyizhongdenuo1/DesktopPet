# C++ 编码命名规范

本文档定义 DesktopPet 项目的 C++ 代码命名规则，所有新代码必须遵循此规范。

---

## 1. 类名

- **风格**：前缀 + PascalCase
- **实现类**：`C` + PascalCase
  - 示例：`CDataSave`、`CDataRWMgr`、`CThreadHandler`
- **接口类**：`I` + PascalCase
  - 示例：`INoteDataBuffer`、`IFactory`、`IThreadFactory`
- **私有实现类**：PascalCase + `Private` 后缀
  - 示例：`CDataRWMgrPrivate`、`CNoteDataCachePrivate`

---

## 2. 结构体

- **风格**：`st_` + PascalCase，另有大写 typedef 别名
- **示例**：
  - `st_NoteData` → `ST_NOTE_DATA`
  - `st_FileHeaderBase` → `FILE_HEADER_BASE`
  - `st_NoteFileHeader` → `NOTE_FILE_HEADER`

---

## 3. 成员变量

- **风格**：`m_` + 类型前缀 + PascalCase
- **类型前缀对照**：

| 前缀 | 类型 | 示例 |
|------|------|------|
| `m_s32` | INT32 | `m_s32id`、`m_s32FileSize` |
| `m_s64` | INT64 | `m_s64FileStoreLimit`、`m_s64HeaderSize` |
| `m_b` | BOOL | `m_bCompleted`、`m_bIsExit` |
| `m_e` | 枚举 | `m_eNoteLevel`、`m_eEventType` |
| `m_c` | char数组 | `m_cEvent[20]`、`m_cReserved[20]` |
| `m_sz` | char定长数组 | `m_szContent[4096]` |
| `m_str` | std::string | `m_strNoteFileName` |
| `m_p` | 裸指针/智能指针 | `m_pThread`、`m_pFileHeader` |
| `m_mutex` | mutex | `m_mutexNote` |
| `m_que` | queue | `m_queSaveEvent` |
| `m_map` | map | `m_mapFunc` |

---

## 4. 函数参数与临时变量（核心区分规则）

### 4.1 函数参数（无下划线）

- **风格**：类型前缀 + PascalCase
- **指针/智能指针参数须加 `p` 前缀**
- **示例**：`s32Size`、`pBuffer`、`strFileName`、`bIsTrunateLast`、`pFileHeader`、`pDataSaver`

### 4.2 函数内临时变量（有下划线）

- **风格**：类型前缀 + `_` + PascalCase（中间加下划线与参数做区分）
- **示例**：
  ```cpp
  INT32 s32_ReadDataByte = 0;    // 临时变量，有下划线
  char *p_FileHeader = m_pFileHeader.get();  // 临时变量，有下划线
  bool b_Ret = ReadFileHeader(strFileName);  // 临时变量，有下划线
  ```
- **类型前缀对照**：

| 前缀 | 类型 | 示例 |
|------|------|------|
| `s32` | INT32 | `s32_ReadDataByte`、`s32_KeepDataLen` |
| `s64` | INT64 | `s64_HeaderSize`、`s64_StoreCount` |
| `p` | 指针 | `p_Buffer`、`p_tempBuffer`、`p_FileHeader` |
| `b` | bool/BOOL | `b_Ret`、`b_Success` |
| `str` | std::string | `str_TempFileName` |
| `fs` | filesystem::path | `fs_Path` |

### 4.3 C 风格临时变量（保持原样）

- 下列类型的临时变量保持 C 风格命名，**不加下划线、不加类型前缀**：
  - **文件流**：`ifstream`、`ofstream`、`fstream` → 统一用 `file`
  - **filesystem::path**：`path`（非 `fs_Path`，`fs_Path` 仅在同时有其他 path 变量时使用）
- **示例**：
  ```cpp
  ifstream file(strFileName, std::ios::binary);   // C 风格，保持 file
  filesystem::path path_Dir = path.parent_path(); // C 风格，path + 后缀
  ```

---

## 5. 枚举

- **风格**：`E_` + snake_case
- **示例**：`E_NOTE_EVENT_TYPE`、`E_NOTE_EVENT_WAKEUP_LEVEL`、`E_THREAD_ID`
- **枚举值**：同样 snake_case
  - 示例：`E_NOTE_EVENT_NONE`、`E_THREAD_DYNC_DATA`

---

## 6. 常量 / constexpr / 宏

- **常量、constexpr（命名空间/类内）**：**UPPER_SNAKE_CASE**
  - 示例：`SINGLE_SAVE_NOTE_DATA_COUNT`、`NOTE_FILE_NAME`、`TRUNCATE_KEEP_RATIO_PERCENT`、`MAX_FILE_STORE_LIMIT`、`STORE_PATH`、`CONTENT_LENGTH_MAX`、`RESERVED_COUNT`
- **宏**：UPPER_SNAKE_CASE
  - 示例：`RELEASEIF`、`ARRAYSIZE`、`MAX_PATH`、`PATH_MAX`

---

## 7. 命名空间

- **风格**：PascalCase
- **示例**：`CPath`、`CLimit`、`DSaveDefine`、`DataSaveFucName`、`NoteSpace`、`FixedValueSpace`

---

## 8. 全局单例宏

- **风格**：`g_` + 类名
- **示例**：`g_CCommonConfig`、`g_ServiceLocator`

---

## 9. 函数 / 方法

- **风格**：PascalCase
- **示例**：`SaveNoteData`、`ReadFileData`、`Write2FileData`、`GetHeaderSize`、`SetStoreCount`

---

## 10. D-Pointer 固定模式

- **风格**：`d_ptr`，指向 `XxxPrivate` 实现的 `unique_ptr` 成员
- **示例**：`std::unique_ptr<CDataRWMgrPrivate> d_ptr`

---

## 11. 规则检查清单

| 检查项 | 规则 |
|--------|------|
| 类名 | 以 `C`/`I` 开头，PascalCase |
| 成员变量 | `m_` + 类型前缀 + PascalCase |
| 函数参数 | 类型前缀 + PascalCase（**无下划线**）；指针须 `p` 前缀 |
| 临时变量 | 类型前缀 + `_` + PascalCase（**有下划线**） |
| C 风格变量 | `file`、`path` 等保持原样 |
| 枚举 | `E_` + snake_case |
| 常量/constexpr | UPPER_SNAKE_CASE |
| 宏 | UPPER_SNAKE_CASE |
| 命名空间 | PascalCase |
| 单例宏 | `g_` + 类名 |

---

## 12. 典型示例

```cpp
// 类名：C + PascalCase
class CDataSave
{
public:
    // 成员变量：m_ + 类型前缀 + PascalCase
    std::mutex m_mutexNote;
    std::unique_ptr<st_FileHeaderBase> m_pFileHeader;

    // 函数参数：类型前缀 + PascalCase（无下划线）
    // 指针参数须 p 前缀
    void ReadFileData(const std::string &strFileName, char *pBuffer, INT32 &s32BufferSize, INT32 s32ReadStartPos);
    bool UpdateNoteFileHeader(const std::string &strFileName, st_FileHeaderBase *pFileHeader);
};

void CDataSave::ReadFileData(const std::string &strFileName, char *pBuffer, INT32 &s32BufferSize, INT32 s32ReadStartPos)
{
    // 临时变量：类型前缀 + _ + PascalCase（有下划线）
    INT32 s32_ReadDataByte = 0;
    bool b_IsOpen = false;

    // C 风格临时变量：保持 file 命名
    ifstream file(strFileName, std::ios::binary);
    b_IsOpen = file.is_open();
}
```