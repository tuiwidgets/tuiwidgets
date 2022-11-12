// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_LAYOUT_P_INCLUDED
#define TUIWIDGETS_LAYOUT_P_INCLUDED

#include <Tui/ZLayoutItem.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

namespace Private {

    class SpacerLayoutItem : public ZLayoutItem {
    public:
        explicit SpacerLayoutItem(int width, int height, SizePolicy sizePolicyH, SizePolicy sizePolicyV);
        ~SpacerLayoutItem() override;

    public:
        void setGeometry(QRect r) override;
        QSize sizeHint() const override;
        SizePolicy sizePolicyH() const override;
        SizePolicy sizePolicyV() const override;
        bool isVisible() const override;

        bool isSpacer() const override;

    private:
        SizePolicy _sizePolicyH;
        SizePolicy _sizePolicyV;
        QSize _size;
    };

    void placeWidgetInCell(int x, int y, int width, int height, ZLayoutItem *w, Tui::Alignment align);

    template <typename Place, typename GetHint, typename GetPolicy, typename GetIsSpacer>
    static void boxLayouter(Place &place, GetHint &getHint, GetPolicy &getPolicy, GetIsSpacer &getIsSpacer,
                            int items, int totalSpace, int spacing) {
        int hintSize = 0;
        int numExpanding = 0;
        int numCanGrow = 0;
        int numSpacer = 0;
        int shrinkableSize = 0;
        int spacerSize = 0;

        for (int i = 0; i < items; i++) {
            int hint = std::max(getHint(i), 0);
            hintSize += hint;
            if (getPolicy(i) == SizePolicy::Expanding) {
                numExpanding += 1;
            } else if (getPolicy(i) != SizePolicy::Fixed && getPolicy(i) != SizePolicy::Maximum) {
                numCanGrow += 1;
            }
            if (getPolicy(i) != SizePolicy::Fixed && getPolicy(i) != SizePolicy::Minimum) {
                shrinkableSize += hint;
            }
            if (getIsSpacer(i)) {
                numSpacer += 1;
                spacerSize += hint;
            }
        }

        const int spaceBetween = spacing * (items - 1 - numSpacer);
        const int hintAndSpacing = hintSize + spaceBetween;

        if (hintAndSpacing == totalSpace) {
            int pos = 0;

            for (int i = 0; i < items; i++) {
                int size = std::max(getHint(i), 0);
                place(pos, size, i);
                pos += size;
                if (!getIsSpacer(i)) {
                    pos += spacing;
                }
            }
        } else if (hintAndSpacing > totalSpace) {
            if (hintAndSpacing - spacerSize - spaceBetween <= totalSpace) { // if removing spacing makes this fit
                int remainingSpacingSize = spacerSize + spaceBetween;
                int toDistribute = totalSpace - (hintAndSpacing - spacerSize - spaceBetween);

                int pos = 0;

                for (int i = 0; i < items; i++) {
                    int size = std::max(getHint(i), 0);
                    if (getIsSpacer(i)) {
                        int allocated = remainingSpacingSize ? ((size * toDistribute + remainingSpacingSize / 2) / remainingSpacingSize) : 0;
                        remainingSpacingSize -= size;
                        toDistribute -= allocated;

                        place(pos, allocated, i);
                        pos += allocated;
                    } else {
                        place(pos, size, i);
                        pos += size;

                        int allocated = remainingSpacingSize ? ((spacing * toDistribute + remainingSpacingSize / 2) / remainingSpacingSize) : 0;
                        remainingSpacingSize -= size;
                        toDistribute -= allocated;
                        pos += allocated;
                    }
                }
            } else {
                int contractionNeeded = hintAndSpacing - spacerSize - spaceBetween - totalSpace;
                if (contractionNeeded >= shrinkableSize) {
                    int pos = 0;
                    for (int i = 0; i < items; i++) {
                        if (getIsSpacer(i)) {
                            place(pos, 0, i);
                            continue;
                        }
                        int size = std::max(getHint(i), 0);
                        if (getPolicy(i) == SizePolicy::Fixed || getPolicy(i) == SizePolicy::Minimum) {
                            place(pos, size, i);
                            pos += size;
                        } else {
                            place(pos, 0, i);
                        }
                    }
                } else {
                    int toDistribute = shrinkableSize - contractionNeeded;
                    int remainingShrinkableSize = shrinkableSize;

                    int pos = 0;
                    for (int i = 0; i < items; i++) {
                        if (getIsSpacer(i)) {
                            place(pos, 0, i);
                            continue;
                        }
                        int size = std::max(getHint(i), 0);
                        if (getPolicy(i) == SizePolicy::Fixed || getPolicy(i) == SizePolicy::Minimum) {
                            place(pos, size, i);
                            pos += size;
                        } else {
                            int allocated = remainingShrinkableSize ? ((size * toDistribute + remainingShrinkableSize / 2) / remainingShrinkableSize) : 0;
                            remainingShrinkableSize -= size;
                            toDistribute -= allocated;
                            place(pos, allocated, i);
                            pos += allocated;
                        }
                    }
                }
            }
        } else if (numExpanding) {
            // Distribute space only into expanding items
            int pos = 0;

            const int perItemExpansion = (totalSpace - hintAndSpacing) / numExpanding;
            int remainder = (totalSpace - hintAndSpacing) % numExpanding;

            for (int i = 0; i < items; i++) {
                int size = std::max(getHint(i), 0);
                if (getPolicy(i) == SizePolicy::Expanding) {
                    size += perItemExpansion;
                    if (remainder) {
                        remainder -= 1;
                        size += 1;
                    }
                }

                place(pos, size, i);
                pos += size;
                if (!getIsSpacer(i)) {
                    pos += spacing;
                }
            }
        } else {
            // Distribute space into items that can grow, if none just not allocate remaining space

            int pos = 0;

            const int perItemExpansion = numCanGrow ? (totalSpace - hintAndSpacing) / numCanGrow : 0;
            int remainder = numCanGrow ? (totalSpace - hintAndSpacing) % numCanGrow : 0;

            for (int i = 0; i < items; i++) {
                int size = std::max(getHint(i), 0);
                if (getPolicy(i) != SizePolicy::Fixed
                    && getPolicy(i) != SizePolicy::Maximum) {
                    size += perItemExpansion;
                    if (remainder) {
                        remainder -= 1;
                        size += 1;
                    }
                }

                place(pos, size, i);
                pos += size;
                if (!getIsSpacer(i)) {
                    pos += spacing;
                }
            }
        }
    }

    inline SizePolicy mergePolicy(SizePolicy current, SizePolicy itemPolicy) {
        if (itemPolicy == SizePolicy::Fixed) {
            // Nothing to do
        } else if (itemPolicy == SizePolicy::Minimum) {
        } else if (itemPolicy == SizePolicy::Maximum) {
        } else if (itemPolicy == SizePolicy::Preferred) {
            if (current != SizePolicy::Expanding) {
                current = SizePolicy::Preferred;
            }
        } else if (itemPolicy == SizePolicy::Expanding) {
            current = SizePolicy::Expanding;
        }
        return current;
    }

}

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_LAYOUT_P_INCLUDED
