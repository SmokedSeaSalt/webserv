#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../../incl/doctest.h"
#include "InputArgs.hpp"
#include <iostream>

////////////////////////////////////////////////////////////////////////////////
// Basic                                                                      //
////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Basic test")
{
    SUBCASE("Default")
    {
        const int argc   = 2;
        char      arg0[] = "Name";
        char      arg1[] = "configpath.conf";
        char*     argv[] = {arg0, arg1};

        CHECK(InputArgs::parseArguments(argc, argv) == true);
        CHECK(InputArgs::args.configFile == "configpath.conf");
    }

    SUBCASE("No config file")
    {
        const int argc   = 1;
        char      arg0[] = "Name";
        char*     argv[] = {arg0};

        CHECK(InputArgs::parseArguments(argc, argv) == false);
    }

    SUBCASE("Too many arguments")
    {
        const int argc   = 3;
        char      arg0[] = "Name";
        char      arg1[] = "configpath.conf";
        char      arg2[] = "anotherone";
        char*     argv[] = {arg0, arg1, arg2};

        CHECK(InputArgs::parseArguments(argc, argv) == false);
    }
}

TEST_CASE("Basic flags")
{
    SUBCASE("Default order")
    {
        const int argc   = 8;
        char      arg0[] = "Name";
        char      arg1[] = "-p";
        char      arg2[] = "/user/home/test";
        char      arg3[] = "-l";
        char      arg4[] = "logfile.log";
        char      arg5[] = "-d";
        char      arg6[] = "3";
        char      arg7[] = "configpath.conf";
        char*     argv[] = {arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7};

        CHECK(InputArgs::parseArguments(argc, argv) == true);
        CHECK(InputArgs::args.configFile == "configpath.conf");
        CHECK(InputArgs::args.relativePath == "/user/home/test");
        CHECK(InputArgs::args.logFile == "logfile.log");
        CHECK(InputArgs::args.logLevel == LogLevel::kErrors);
    }

    SUBCASE("Random order")
    {
        const int argc   = 8;
        char      arg0[] = "Name";
        char      arg1[] = "-d";
        char      arg2[] = "3";
        char      arg3[] = "configpath.conf";
        char      arg4[] = "-p";
        char      arg5[] = "/user/home/test";
        char      arg6[] = "-l";
        char      arg7[] = "logfile.log";
        char*     argv[] = {arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7};

        CHECK(InputArgs::parseArguments(argc, argv) == true);
        CHECK(InputArgs::args.configFile == "configpath.conf");
        CHECK(InputArgs::args.relativePath == "/user/home/test");
        CHECK(InputArgs::args.logFile == "logfile.log");
        CHECK(InputArgs::args.logLevel == LogLevel::kErrors);
    }

    SUBCASE("No config file")
    {
        const int argc   = 7;
        char      arg0[] = "Name";
        char      arg1[] = "-p";
        char      arg2[] = "/user/home/test";
        char      arg3[] = "-l";
        char      arg4[] = "logfile.log";
        char      arg5[] = "-d";
        char      arg6[] = "3";
        char*     argv[] = {arg0, arg1, arg2, arg3, arg4, arg5, arg6};

        CHECK(InputArgs::parseArguments(argc, argv) == false);
    }

    SUBCASE("Too many arguments")
    {
        const int argc   = 9;
        char      arg0[] = "Name";
        char      arg1[] = "-d";
        char      arg2[] = "3";
        char      arg3[] = "configpath.conf";
        char      arg4[] = "-p";
        char      arg5[] = "/user/home/test";
        char      arg6[] = "-l";
        char      arg7[] = "logfile.log";
        char      arg8[] = "extra_arg";
        char*     argv[] = {arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8};

        CHECK(InputArgs::parseArguments(argc, argv) == false);
    }
}