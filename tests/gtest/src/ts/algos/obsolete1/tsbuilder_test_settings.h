///////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Testing BidiGraph with fixture 1
///
/// HOW TO SET A TEST ENVIRONMENT:
/// one need to define CSVLOG1_TEST_LOGS_BASE_DIR directive to set it to a
/// machine/user specific value
///
///////////////////////////////////////////////////////////////////////////////

#ifndef XI_LDOPA_TRSS_TSBUILDER_TEST_SETTINGS_H_
#define XI_LDOPA_TRSS_TSBUILDER_TEST_SETTINGS_H_

// if no CSVLOG1_TEST_LOGS_BASE_DIR is set, use default value
#ifndef CSVLOG1_TEST_LOGS_BASE_DIR
#define CSVLOG1_TEST_LOGS_BASE_DIR "C:/Users/maxga/CLionProjects/190506143700_ldopa-0.1.2/tests/gtest/work_files/"
#endif

#ifndef EVENTLOG_DOTWRITER_TEST_OUTFILES_BASE_DIR
#define EVENTLOG_DOTWRITER_TEST_OUTFILES_BASE_DIR    CSVLOG1_TEST_LOGS_BASE_DIR "GrViz/"                                    
#endif


#endif // XI_LDOPA_TRSS_TSBUILDER_TEST_SETTINGS_H_
