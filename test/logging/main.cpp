#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../incl/doctest.h"
#include "logging.hpp"
#include <cstdio>
#include <string>
#include <unistd.h>

static std::string capture_stderr(auto fn)
{
    int fds[2];
    pipe(fds);
    int saved = dup(STDERR_FILENO);
    dup2(fds[1], STDERR_FILENO);
    close(fds[1]);

    fn();
    fflush(stderr);

    dup2(saved, STDERR_FILENO);
    close(saved);

    std::string out;
    char        buf[256];
    ssize_t     n;
    while ((n = read(fds[0], buf, sizeof(buf))) > 0)
        out.append(buf, n);
    close(fds[0]);
    return out;
}

TEST_CASE("logs to stderr")
{
    Logging::g_logger.level = LogLevel::kInfo;
    auto out                = capture_stderr([]
                              { LOG(LogLevel::kInfo, "hello {}", 42); });
    CHECK(out.find("[INFO]") != std::string::npos);
    CHECK(out.find("hello 42") != std::string::npos);
}

TEST_CASE("logs to stderr with to low loglevel")
{
    Logging::g_logger.level = LogLevel::kInfo;
    auto out                = capture_stderr([]
                              { LOG(LogLevel::kDebug, "hello {}", 42); });
    CHECK(out.find("[INFO]") == std::string::npos);
    CHECK(out.find("hello 42") == std::string::npos);
}

TEST_CASE("silent suppresses output")
{
    Logging::g_logger.level = LogLevel::kSilent;
    auto out                = capture_stderr([]
                              { LOG(LogLevel::kInfo, "should not appear"); });
    CHECK(out.empty());
}

TEST_CASE("logs to file")
{
    Logging::g_logger.level = LogLevel::kInfo;

    char tmp[]             = "/tmp/log_test_XXXXXX";
    int  fd                = mkstemp(tmp);
    Logging::g_logger.file = fdopen(fd, "w+");

    capture_stderr([]
                   { LOG(LogLevel::kInfo, "file test"); });

    rewind(Logging::g_logger.file);
    std::string out;
    char        buf[256];
    size_t      n;
    while ((n = fread(buf, 1, sizeof(buf), Logging::g_logger.file)) > 0)
        out.append(buf, n);

    fclose(Logging::g_logger.file);
    Logging::g_logger.file = nullptr;
    unlink(tmp);

    CHECK(out.find("file test") != std::string::npos);
}

TEST_CASE("actual terminal output")
{
    Logging::g_logger.level = LogLevel::kInfo;
    LOG(LogLevel::kInfo, "This is a test if it acutally prints to the terminal.");
    LOG(LogLevel::kDebug, "This should not print to the terminal.");
}