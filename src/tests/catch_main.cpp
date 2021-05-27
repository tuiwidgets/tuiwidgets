#define CATCH_CONFIG_MAIN
#include "../third-party/catch.hpp"

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
