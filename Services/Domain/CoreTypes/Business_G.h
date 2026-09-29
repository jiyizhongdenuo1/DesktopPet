/*
 * @file: Business_G.h
 * @brief: 
 * @author: nuo
 * @date: 2026/6/2
 * @Detail:
 */
#include "Export.h"
#if defined(MODULEBASE_LIBRARY)
#  define BUSINESS_EXPORT DLL_EXPORT
#else
#  define BUSINESS_EXPORT DLL_IMPORT
#endif