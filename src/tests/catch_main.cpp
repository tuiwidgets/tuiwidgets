// SPDX-License-Identifier: BSL-1.0

#define CATCH_CONFIG_MAIN
#include "catchwrapper.h"

#include <QString>

struct TestStackListener : Catch::TestEventListenerBase {
    using TestEventListenerBase::TestEventListenerBase;

    void sectionStarting(Catch::SectionInfo const &sectionInfo) override {
        names.push_back(sectionInfo.name);
    }

    void sectionEnded(Catch::SectionStats const &sectionStats) override {
        (void)sectionStats;
        names.pop_back();
    }

    static std::vector<std::string> names;
};

std::vector<std::string> TestStackListener::names;

CATCH_REGISTER_LISTENER(TestStackListener)

std::vector<std::string> getCurrentTestNames() {
    return TestStackListener::names;
}


struct QtDiagnosticsFallbackListener : Catch::TestEventListenerBase {
    using TestEventListenerBase::TestEventListenerBase;


    static void qtMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
        (void)type; (void)context;
        FAIL_CHECK("Unexpected qt diagnostic message: " + msg.toStdString());
    }

    void testCaseStarting(const Catch::TestCaseInfo &testInfo) override {
        (void)testInfo;
        oldMessageHandler = qInstallMessageHandler(qtMessageOutput);

    }
    void testCaseEnded(const Catch::TestCaseStats &testCaseStats) override {
        (void)testCaseStats;
        qInstallMessageHandler(oldMessageHandler);
    }

#if defined(__FreeBSD__) || defined(__OpenBSD__)
    // if the first log message is output with a temporary QCoreApplication later it crashes somewhere in
    // QCoreApplication::applicationFilePath. Outputting a log message without a QCoreApplication primes the logging
    // system without stumbling in that crash. This should not be here, but i don't have a good way to debug this right
    // now
    void testRunStarting(const Catch::TestRunInfo&) override {
        qWarning("Starting tests");
    }
#endif

    static QtMessageHandler oldMessageHandler;
};

QtMessageHandler QtDiagnosticsFallbackListener::oldMessageHandler;

CATCH_REGISTER_LISTENER(QtDiagnosticsFallbackListener)
