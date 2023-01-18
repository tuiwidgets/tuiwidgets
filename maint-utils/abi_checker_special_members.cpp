// SPDX-License-Identifier: BSL-1.0

// This is quite a hack. But better than nothing.
//
// Entry point is the python script with a similar name.
//
// This uses the compiler and the symbols to detect if the compiler generated live cycle and assignment operations are
// defined out of line to ensure ABI stability properties are kept.
//
// It contains some hacks to work around special cases that will need adjusting for future types.
// Assumes itanium c++ name mangling and to be used with an amd64 compiler (other archs might work).

#include <stdio.h>

#include <type_traits>
#include <cxxabi.h>

#include <Tui/ZBasicDefaultWidgetManager.h>
#include <Tui/ZBasicWindowFacet.h>
#include <Tui/ZButton.h>
#include <Tui/ZCheckBox.h>
#include <Tui/ZColor.h>
#include <Tui/ZCommandManager.h>
#include <Tui/ZCommandNotifier.h>
#include <Tui/ZDefaultWidgetManager.h>
#include <Tui/ZDialog.h>
#include <Tui/ZEvent.h>
#include <Tui/ZFormatRange.h>
#include <Tui/ZHBoxLayout.h>
#include <Tui/ZImage.h>
#include <Tui/ZInputBox.h>
#include <Tui/ZLabel.h>
#include <Tui/ZLayout.h>
#include <Tui/ZLayoutItem.h>
#include <Tui/ZListView.h>
#include <Tui/ZMenu.h>
#include <Tui/ZMenuItem.h>
#include <Tui/ZMenubar.h>
#include <Tui/ZPainter.h>
#include <Tui/ZPalette.h>
#include <Tui/ZRadioButton.h>
#include <Tui/ZRoot.h>
#include <Tui/ZShortcut.h>
#include <Tui/ZStyledTextLine.h>
#include <Tui/ZSymbol.h>
#include <Tui/ZTerminal.h>
#include <Tui/ZTextLayout.h>
#include <Tui/ZTextLine.h>
#include <Tui/ZTextMetrics.h>
#include <Tui/ZTextOption.h>
#include <Tui/ZTextStyle.h>
#include <Tui/ZVBoxLayout.h>
#include <Tui/ZWidget.h>
#include <Tui/ZWindow.h>
#include <Tui/ZWindowContainer.h>
#include <Tui/ZWindowFacet.h>
#include <Tui/ZWindowLayout.h>


// detect actual move operations, based on https://stackoverflow.com/questions/51901837/how-to-get-if-a-type-is-truly-move-constructible/51912859#51912859

template<typename P>
struct M
{
    operator P const&();
    operator P&&();
};

template<typename T>
constexpr bool has_move_ctor = std::is_move_constructible_v<T> && !std::is_constructible_v<T, M<T>>;

template<typename T>
constexpr bool has_move_assign = std::is_move_assignable_v<T> && !std::is_assignable_v<T, M<T>>;

// ^^^^^^


enum class Kind {
    Widget = 1,
    Facet = 2,
    Value = 3,
    Event = 4,
    Layout = 5,
    QObject_Intree = 6,
    QObject_Other = 7,
    Misc = 8,
    Inline = 9
};

std::string kindToString(Kind kind) {
    switch (kind) {
        case Kind::Widget: return "Widget";
        case Kind::Facet: return "Facet";
        case Kind::Value: return "Value";
        case Kind::Event: return "Event";
        case Kind::Layout: return "Layout";
        case Kind::QObject_Intree: return "QObject_Intree";
        case Kind::QObject_Other: return "QObject_Other";
        case Kind::Misc: return "Misc";
        case Kind::Inline: return "Inline";
    }
    return "unknown";
}

void expectLinkage(std::string expected, std::string descr) {
    printf("EXPECT _Z%s %s\n", expected.c_str(), descr.c_str());
}

template <typename T>
class DestructorReference : public T {
    using T::T;
};

class SpecialDestructorUsageLayout : public Tui::ZLayout {
    void setGeometry(QRect) override {}
    void removeWidgetRecursively(Tui::ZWidget*) override {}
};

class SpecialDestructorUsageLayoutItem : public Tui::ZLayoutItem {
    void setGeometry(QRect) override {}
    QSize sizeHint() const override { return {}; }
    Tui::SizePolicy sizePolicyH() const override { return {}; }
    Tui::SizePolicy sizePolicyV() const override { return {}; }
    bool isVisible() const override { return {}; }

};

class SpecialDestructorUsageDefaultWidgetManager : public Tui::ZDefaultWidgetManager {
    void setDefaultWidget(Tui::ZWidget *) override {};
    Tui::ZWidget *defaultWidget() const override { return {}; };
    bool isDefaultWidgetActive() const override { return {}; };
};

class SpecialDestructorUsageTerminalConnectionDelegate : public Tui::ZTerminal::TerminalConnectionDelegate {
    void write(const char*, int) override {};
    void flush() override {};
    void restoreSequenceUpdated(const char*, int) override {};
    void deinit(bool) override {};
};

class Tui::v0::ZEventPrivate {};
Tui::v0::ZEvent::ZEvent(Type type, std::unique_ptr<ZEventPrivate> pimpl)
    : QEvent(type), tuiwidgets_pimpl_ptr(move(pimpl))
{
}

class SpecialDestructorUsageEvent : public Tui::ZEvent {
    SpecialDestructorUsageEvent() : Tui::ZEvent(QEvent::Type::None, nullptr) {}
};

void specialDestructorUsage() {
    SpecialDestructorUsageLayout x1;
    SpecialDestructorUsageLayoutItem x2;
    SpecialDestructorUsageDefaultWidgetManager x3;
    SpecialDestructorUsageEvent x4;
    SpecialDestructorUsageTerminalConnectionDelegate x5;
}

template <typename T>
void testInner(Kind kind, bool run, T *a, T *b) {

    auto classMangled = std::string(typeid(T).name());
    auto classMangledWithoutE = classMangled.substr(0, classMangled.size() - 1);
    
    int status;    
    char *ret = abi::__cxa_demangle(classMangled.c_str(), 0, 0, &status);
    auto className = std::string(ret);
    
    std::string selfref = "1";
    
    if (className == "Tui::v0::ZTerminal::OffScreen") {
        selfref = "2";
    }
    
    printf("CLASS %s kind=%s\n", className.c_str(), kindToString(kind).c_str());
    
    if (std::is_base_of_v<Tui::ZWidget, T>) {
        if (kind != Kind::Widget) {
            printf("ERROR %s is a widget but has kind %s\n", className.c_str(), kindToString(kind).c_str());
        }
    } else {
        if (kind == Kind::Widget) {
            printf("ERROR %s is not a widget\n", className.c_str());
        }
    }
    
    if (std::is_base_of_v<QObject, T>) {
        if (kind != Kind::Widget && kind != Kind::Facet && kind != Kind::Layout
             && kind != Kind::QObject_Intree && kind != Kind::QObject_Other) {
            printf("ERROR %s is a QObject but has kind %s\n", className.c_str(), kindToString(kind).c_str());
        }
    } else {
        if (kind == Kind::Facet || kind == Kind::Layout || kind == Kind::Facet
            || kind == Kind::QObject_Intree || kind == Kind::QObject_Other) {
            printf("ERROR %s is not a %s\n", className.c_str(), kindToString(kind).c_str());
        }
    }
    
    if (std::is_base_of_v<Tui::ZEvent, T>) {
        if (kind != Kind::Event) {
            printf("ERROR %s is a ZEvent but has kind %s\n", className.c_str(), kindToString(kind).c_str());
        }
    } else {
        if (kind == Kind::Event) {
            printf("ERROR %s is not a ZEvent\n", className.c_str());
        }
    }
    
    std::string ops;

    if constexpr (std::is_default_constructible_v<T>) {
        if (run) {
            [[maybe_unused]] T c;
        }
        expectLinkage(classMangledWithoutE + "C1Ev", className + "#default ctor");
        ops += "D";
    } else {
        ops += "-";
    }

    if constexpr (std::is_copy_constructible_v<T>) {
        if (run) {
            T c = *b;
        }
        expectLinkage(classMangledWithoutE + "C1ERKS" + selfref + "_", className + "#copy ctor");
        ops += "C";
    } else {
        ops += "-";
    }

    // Not using std::is_move_constructible because that also considers a copy constructor as valid, but for ABI we need to ignore that.
    if constexpr (has_move_ctor<T>) {
        if (run) {
            T c = std::move(*b);
        }
        expectLinkage(classMangledWithoutE + "C1EOS" + selfref +"_", className + "#copy ctor&&");
        ops += "M";
    } else {
        ops += "-";
    }

    if constexpr (std::is_copy_assignable_v<T>) {
        if (run) {
            *a = *b;
        }
        expectLinkage(classMangledWithoutE + "aSERKS" + selfref +"_", className + "#operator=");
        ops += "c";
    } else {
        ops += "-";
    }
    
    // Not using std::is_move_assignable because that also considers a copy assign as valid, but for ABI we need to ignore that.
    if constexpr (has_move_assign<T>) {
        if (run) {
            *a = std::move(*b);
        }
        expectLinkage(classMangledWithoutE + "aSEOS" + selfref +"_", className + "#operator=&&");
        ops += "m";
    } else {
        ops += "-";
    }

    //  has_virtual_destructor
    if constexpr (std::is_destructible_v<T>) {
        if (run) {
            delete a;
        }
        expectLinkage(classMangledWithoutE + "D1Ev", className + "#dtor");
        if constexpr (std::has_virtual_destructor_v<T>) {
            ops += "v";
            
            // This again is a huge hack to try to generate a reference to the destructor symbol.
            // For virtual destructors this happens when generating the constructor which implies generating the vtable...
            if constexpr (std::is_same_v<T, Tui::ZWindowFacet> || std::is_same_v<T, Tui::ZWindowContainer>) {
                // nullptr overload exists but is troublesome
                if (run) {
                    DestructorReference<T> x;
                }
            } else if constexpr (std::is_same_v<T, Tui::ZPainter>) {
                // can only be created using existing ZPainter
            } else if constexpr (std::is_constructible_v<T, nullptr_t> && !std::is_same_v<T, Tui::ZTextMetrics>) {
                if (run) {
                    DestructorReference<T> x(nullptr);
                }
            } else if constexpr (std::is_constructible_v<T, QStringList>) {
                if (run) {
                    DestructorReference<T> x(QStringList{});
                }
            } else if constexpr (std::is_constructible_v<T, Tui::ZFocusEvent::FocusIn>) {
                if (run) {
                    DestructorReference<T> x(Tui::ZFocusEvent::focusIn);
                }
            } else if constexpr (std::is_constructible_v<T, Tui::ZKeySequence, nullptr_t>) {
                if (run) {
                    DestructorReference<T> x({}, {});
                }
            } else if constexpr (std::is_constructible_v<T, int, Qt::KeyboardModifiers, QString>) {
                if (run) {
                    DestructorReference<T> x({}, {}, {});
                }
            } else if constexpr (std::is_constructible_v<T, QPoint, QPoint>) {
                if (run) {
                    DestructorReference<T> x({}, {});
                }
            } else if constexpr (std::is_constructible_v<T, QSize, QSize>) {
                if (run) {
                    DestructorReference<T> x({}, {});
                }
            } else if constexpr (std::is_constructible_v<T, QSet<Tui::ZSymbol>>) {
                if (run) {
                    DestructorReference<T> x(QSet<Tui::ZSymbol>{});
                }
            } else if constexpr (std::is_constructible_v<T, Tui::ZSymbol>) {
                if (run) {
                    DestructorReference<T> x(Tui::ZSymbol{});
                }
            } else if constexpr (std::is_default_constructible_v<T>) {
                if (run) {
                    DestructorReference<T> x;
                }
            }
        } else {
            ops += "d";
        }
    } else {
        ops += "-";
    }
    
    if (ops == "DCMcmd") {
        ops += " movable value                             ";
    } else if (ops == "D----v") {
        ops += " id with default ctor                      ";
    } else if (ops == "-----v") {
        ops += " id                                        ";
    } else if (ops == "-C-c-d") {
        ops += " value without default                     ";
    } else if (ops == "DC-c-d") {
        ops += " value with default                        ";
    } else if (ops == "-CMcmd") {
        ops += " movable value without default             ";
    } else if (ops == "-CMcmv") {
        ops += " movable value without default virtual dtor";
    } else if (ops == "DCMcmv") {
        ops += " movable value with default, virtual dtor  ";
    } else if (ops == "-C---v") {
        ops += " copy only virtual dtor                    ";
    } else if (ops == "D----d") {
        ops += " default + dtor                            ";
    } else {
        ops += " ?????";
    }
    
    printf("CLASS-SIG %s %s\n", ops.c_str(), className.c_str());
}

void *a_buff = malloc(1024);
void *b_buff = malloc(1024);

template <typename T>
void test(Kind kind, bool run) {
    // dereferencing the result of the reinterpret_cast·s of course is undefined, but we don't intent to do that anyway.
    // Hopefully the compiler won't do something stupid with this. The pointers are only supposed to be used in code
    // that never runs for generating references to elf symbols to be detectet later.
    testInner(kind, run, reinterpret_cast<T*>(a_buff), reinterpret_cast<T*>(b_buff));
}

int main(int argc, char* argv[]) {
    (void)argv;
    bool run = false;
    if (argc > 100) {
        // ensure optimizer has no way to tell run is always false
        run = true;
    }
    
    test<Tui::ZBasicDefaultWidgetManager>(Kind::Facet, run);
    test<Tui::ZBasicWindowFacet>(Kind::Facet, run);
    test<Tui::ZButton>(Kind::Widget, run);
    test<Tui::ZCheckBox>(Kind::Widget, run);
    test<Tui::ZCloseEvent>(Kind::Event, run);
    test<Tui::ZColor>(Kind::Value, run);
    test<Tui::ZColorHSV>(Kind::Value, run);
    test<Tui::Private::GlobalColorRGB>(Kind::Inline, run); // needs to be usable for constexpr globals
    test<Tui::ZCommandManager>(Kind::Facet, run);
    test<Tui::ZCommandNotifier>(Kind::QObject_Intree, run);
    test<Tui::ZDefaultWidgetManager>(Kind::Facet, run);
    test<Tui::ZDialog>(Kind::Widget, run);
    test<Tui::ZEvent>(Kind::Event, run);
    test<Tui::ZFocusEvent>(Kind::Event, run);
    test<Tui::ZFormatRange>(Kind::Value, run);
    test<Tui::ZHBoxLayout>(Kind::Layout, run);
    test<Tui::ZImage>(Kind::Value, run);
    test<Tui::ZInputBox>(Kind::Widget, run);
    test<Tui::ZKeyEvent>(Kind::Event, run);
    test<Tui::ZKeySequence>(Kind::Value, run);
    test<Tui::ZLabel>(Kind::Widget, run);
    test<Tui::ZLayout>(Kind::Layout, run);
    test<Tui::ZLayoutItem>(Kind::Misc, run);
    test<Tui::ZListView>(Kind::Widget, run);
    test<Tui::ZMenu>(Kind::Widget, run);
    test<Tui::ZMenuItem>(Kind::Value, run);
    test<Tui::ZMenubar>(Kind::Widget, run);
    test<Tui::ZMoveEvent>(Kind::Event, run);
    test<Tui::ZOtherChangeEvent>(Kind::Event, run);
    test<Tui::ZPaintEvent>(Kind::Event, run);
    test<Tui::ZPainter>(Kind::Value, run);
    test<Tui::ZPalette>(Kind::Value, run);
    test<Tui::ZPalette::ColorDef>(Kind::Inline, run);
    test<Tui::ZPalette::AliasDef>(Kind::Inline, run);
    test<Tui::ZPalette::RuleCmd>(Kind::Inline, run);
    test<Tui::ZPalette::RuleDef>(Kind::Inline, run);
    test<Tui::ZPasteEvent>(Kind::Event, run);
    test<Tui::ZPendingKeySequenceCallbacks>(Kind::Value, run);
    test<Tui::ZRadioButton>(Kind::Widget, run);
    test<Tui::ZRawSequenceEvent>(Kind::Event, run);
    test<Tui::ZResizeEvent>(Kind::Event, run);
    test<Tui::ZRoot>(Kind::Widget, run);
    test<Tui::ZShortcut>(Kind::QObject_Intree, run);
    test<Tui::ZStyledTextLine>(Kind::Value, run);
    test<Tui::ZSymbol>(Kind::Inline, run);
    test<Tui::ZImplicitSymbol>(Kind::Inline, run);    
    test<Tui::ZTerminal>(Kind::QObject_Other, run);
    test<Tui::ZTerminal::FileDescriptor>(Kind::Inline, run); 
    test<Tui::ZTerminal::OffScreen>(Kind::Value, run);
    test<Tui::ZTerminal::TerminalConnectionDelegate>(Kind::Misc, run);
    test<Tui::ZTerminal::TerminalConnection>(Kind::Misc, run);
    test<Tui::ZTerminalNativeEvent>(Kind::Event, run);
    test<Tui::ZTextLayout>(Kind::Misc, run);
    test<Tui::ZTextLine>(Kind::Widget, run);
    test<Tui::ZTextLineRef>(Kind::Value, run);
    test<Tui::ZTextMetrics>(Kind::Value, run);
    test<Tui::ZTextMetrics::ClusterSize>(Kind::Inline, run);
    test<Tui::ZTextOption>(Kind::Value, run);
    test<Tui::ZTextOption::Tab>(Kind::Inline, run);
    test<Tui::ZTextStyle>(Kind::Value, run);
    test<Tui::ZVBoxLayout>(Kind::Layout, run);
    test<Tui::ZWidget>(Kind::Widget, run);
    test<Tui::ZWindow>(Kind::Widget, run);
    test<Tui::ZWindowContainer>(Kind::QObject_Other, run);
    test<Tui::ZWindowFacet>(Kind::Facet, run);
    test<Tui::ZWindowLayout>(Kind::Layout, run);
}
