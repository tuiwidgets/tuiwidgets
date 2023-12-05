// SPDX-License-Identifier: BSL-1.0

#define CATCH_CONFIG_RUNNER
#include "catchwrapper.h"

#ifdef CATCH3
using ListenerBase = Catch::EventListenerBase;
#else
using ListenerBase = Catch::TestEventListenerBase;
#endif

#include <QString>

struct TestStackListener : ListenerBase {
    using ListenerBase::ListenerBase;

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


struct QtDiagnosticsFallbackListener : ListenerBase {
    using ListenerBase::ListenerBase;

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

#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
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

int main (int argc, char * argv[]) {
    return Catch::Session().run( argc, argv );
}
