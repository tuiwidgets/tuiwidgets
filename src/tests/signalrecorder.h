// SPDX-License-Identifier: BSL-1.0

#ifndef SIGNALRECORDER_H
#define SIGNALRECORDER_H

#include <any>
#include <unordered_map>

#include <QObject>

#include "catchwrapper.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"

#define RECORDER_SIGNAL(signal) signal, std::string(#signal).substr(1)

class SignalRecorder : public QObject {
public:
    using Pointer = void (QObject::*)();

    template<typename SIGNAL>
    void watchSignal(const typename QtPrivate::FunctionPointer<SIGNAL>::Object *sender, SIGNAL signal, std::string name) {
        signalNames[reinterpret_cast<Pointer>(signal)] = name;
        QObject::connect(sender, signal, this, [this, signal](auto... arguments) {
            std::vector<std::any> args;
            (args.emplace_back(arguments), ...);
            records.push_back(Record{ reinterpret_cast<Pointer>(signal), std::move(args)});
        });
    }

    template<typename SIGNAL, typename... ARGS>
    [[nodiscard]]
    bool consumeFirst(SIGNAL signal, ARGS... args) {
        if (!records.size()) {
            UNSCOPED_INFO("No more signals recorded");
            return false;
        }
        Pointer actualSignal = records[0].pointer;
        auto actualArgs = records[0].args;
        records.erase(records.begin());
        if (reinterpret_cast<Pointer>(signal) != actualSignal) {
            std::string expectedName = signalNames[reinterpret_cast<Pointer>(signal)];
            if (expectedName.empty()) {
                expectedName = "<unwatched signal>";
            }
            UNSCOPED_INFO("Signal does not match. Called was " << signalNames[actualSignal] << " expected was " << expectedName);
            return false;
        }
        return checkArgs(0, actualArgs, args...);
    }

    [[nodiscard]]
    bool noMoreSignal() {
        return records.empty();
    }

protected:
    bool checkArgs(size_t idx, std::vector<std::any> actual) {
        (void)idx; (void)actual;
        // end of recursion
        return true;
    }

    template<typename T, typename... ARGS>
    bool checkArgs(size_t idx, const std::vector<std::any> &actualArgs, T expected, ARGS... expectedRest) {
        if (idx >= actualArgs.size()) {
            UNSCOPED_INFO("More arguments specified than available");
            return false;
        }
        bool ok = true;
        if (actualArgs[idx].type() == typeid(expected)) {
            if (std::any_cast<T>(actualArgs[idx]) != expected) {
                ok = false;
                UNSCOPED_INFO("argument " << idx + 1 << " does not match expected value");
                auto actual = std::any_cast<T>(actualArgs[idx]);
                CHECK(actual == expected);
            }
        } else {
            UNSCOPED_INFO("Argument type mismatch on argument " << idx + 1);
            ok = false;
        }
        return ok && checkArgs(idx + 1, actualArgs, expectedRest...);
    }

    struct Record {
        Pointer pointer;
        std::vector<std::any> args;
    };

    std::vector<Record> records;

    struct DegenerateHash
    {
        std::size_t operator()(Pointer const& s) const noexcept {
            (void)s;
            return 0;
        }
    };

    std::unordered_map<Pointer, std::string, DegenerateHash> signalNames;
};

#pragma GCC diagnostic pop

#endif // SIGNALRECORDER_H
