////////////////////////////////////////////////////////////////////////////////
/// /file
/// \brief     Settings file for testing `csvlog' module
/// \author    Sergey Shershakov
/// \version   0.1.0
/// \date      23.02.2016
/// \copyright (c) xidv.ru 2014�2016.
///            This source is for internal use only � Restricted Distribution.
///            All rights reserved.
///
/// HOW TO SET A TEST ENVIRONMENT:
/// one need to define CSVLOG1_TEST_LOGS_BASE_DIR directive to set it to a
/// machine/user specific value
///
////////////////////////////////////////////////////////////////////////////////

#ifndef XI_LDOPA_LOG_CSVLOG_TEST_SETTINGS_H_
#define XI_LDOPA_LOG_CSVLOG_TEST_SETTINGS_H_

// if no CSVLOG1_TEST_LOGS_BASE_DIR is set, use default value
#ifndef CSVLOG1_TEST_LOGS_BASE_DIR
#define CSVLOG1_TEST_LOGS_BASE_DIR "C:/Users/maxga/CLionProjects/190506143700_ldopa-0.1.2/tests/gtest/work_files/"
#endif

// if no TS_TEST_MODELS_BASE_DIR is set, use default value
#ifndef TS_TEST_MODELS_BASE_DIR
#define TS_TEST_MODELS_BASE_DIR "C:/Users/maxga/CLionProjects/190506143700_ldopa-0.1.2/tests/gtest/work_files/"
#endif

#endif // XI_LDOPA_LOG_CSVLOG_TEST_SETTINGS_H_
