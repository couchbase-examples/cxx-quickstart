/* Generated file, do not edit */

#ifndef CXXTEST_RUNNING
#define CXXTEST_RUNNING
#endif

#define _CXXTEST_HAVE_STD
#include <cxxtest/TestListener.h>
#include <cxxtest/TestTracker.h>
#include <cxxtest/TestRunner.h>
#include <cxxtest/RealDescriptions.h>
#include <cxxtest/TestMain.h>
#include <cxxtest/ErrorPrinter.h>

int main( int argc, char *argv[] ) {
 int status;
    CxxTest::ErrorPrinter tmp;
    CxxTest::RealWorldDescription::_worldName = "cxxtest";
    status = CxxTest::Main< CxxTest::ErrorPrinter >( tmp, argc, argv );
    return status;
}
bool suite_TestQuickStart_init = false;
#include "tests.h"

static TestQuickStart suite_TestQuickStart;

static CxxTest::List Tests_TestQuickStart = { 0, 0 };
CxxTest::StaticSuiteDescription suiteDescription_TestQuickStart( "tests.h", 3, "TestQuickStart", suite_TestQuickStart, Tests_TestQuickStart );

static class TestDescription_suite_TestQuickStart_TestBasic : public CxxTest::RealTestDescription {
public:
 TestDescription_suite_TestQuickStart_TestBasic() : CxxTest::RealTestDescription( Tests_TestQuickStart, suiteDescription_TestQuickStart, 6, "TestBasic" ) {}
 void runTest() { suite_TestQuickStart.TestBasic(); }
} testDescription_suite_TestQuickStart_TestBasic;

#include <cxxtest/Root.cpp>
const char* CxxTest::RealWorldDescription::_worldName = "cxxtest";
