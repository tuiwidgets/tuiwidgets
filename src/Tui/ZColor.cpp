#include "ZColor.h"

#include <math.h>

#include <array>

#include <termpaint.h>

TUIWIDGETS_NS_START

ZColor::ZColor(int r, int g, int b)
    : val(TERMPAINT_RGB_COLOR_OFFSET | static_cast<uint8_t>(r) << 16 | static_cast<uint8_t>(g) << 8 | static_cast<uint8_t>(b))
{
}

ZColor::ZColor(Private::GlobalColorRGB globalColor)
    : ZColor(globalColor.r, globalColor.g, globalColor.b)
{
}

ZColor::ZColor(TerminalColor color) : val(ZColor::fromTerminalColor(color).val) {
}

ZColor::ColorType ZColor::colorType() const {
    static_assert ((TERMPAINT_NAMED_COLOR & 0xff000000) == (TERMPAINT_INDEXED_COLOR & 0xff000000),
                   "TERMPAINT_NAMED_COLOR or TERMPAINT_INDEXED_COLOR not as expected");
    switch (val & 0xff000000) {
        case TERMPAINT_RGB_COLOR_OFFSET:
            return ColorType::RGB;
        case TERMPAINT_NAMED_COLOR & 0xff000000:
            if ((val & 0xfff00000) == TERMPAINT_NAMED_COLOR) {
                return ColorType::Terminal;
            } else if ((val & 0xfff00000) == TERMPAINT_INDEXED_COLOR) {
                return ColorType::TerminalIndexed;
            }
            [[fallthrough]];
        default:
            return ColorType::Default;
    }
}

int ZColor::red() const {
    if (colorType() != ColorType::RGB) {
        return 0;
    }
    return val >> 16 & 0xff;
}

void ZColor::setRed(int red) {
    if (colorType() != ColorType::RGB) {
        val = TERMPAINT_RGB_COLOR(red, 0, 0);
    } else {
        val = TERMPAINT_RGB_COLOR_OFFSET | (val & 0x00ffff) | static_cast<uint8_t>(red) << 16;
    }
}

int ZColor::green() const {
    if (colorType() != ColorType::RGB) {
        return 0;
    }
    return val >> 8 & 0xff;
}

void ZColor::setGreen(int green) {
    if (colorType() != ColorType::RGB) {
        val = TERMPAINT_RGB_COLOR(0, green, 0);
    } else {
        val = TERMPAINT_RGB_COLOR_OFFSET | (val & 0xff00ff) | static_cast<uint8_t>(green) << 8;
    }
}

int ZColor::blue() const {
    if (colorType() != ColorType::RGB) {
        return 0;
    }
    return val & 0xff;
}

void ZColor::setBlue(int blue) {
    if (colorType() != ColorType::RGB) {
        val = TERMPAINT_RGB_COLOR(0, 0, blue);
    } else {
        val = TERMPAINT_RGB_COLOR_OFFSET | (val & 0xffff00) | static_cast<uint8_t>(blue);
    }
}

int ZColor::terminalColorIndexed() const {
    if (colorType() == ColorType::TerminalIndexed) {
        return val & 0xff;
    } else {
        return 0;
    }
}

TerminalColor ZColor::terminalColor() const {
    if (colorType() == ColorType::Terminal) {
        return static_cast<TerminalColor>(val & 0xf);
    } else {
        return TerminalColor::black;
    }
}

static const std::array<uint8_t, 16> guessR = {
    0x00, 0xaa, 0x00, 0xaa, 0x00, 0xaa, 0x00, 0xaa,
    0x55, 0xff, 0x55, 0xff, 0x55, 0xff, 0x55, 0xff
};

static const std::array<uint8_t, 16> guessG = {
    0x00, 0x00, 0xaa, 0xaa, 0x00, 0x00, 0xaa, 0xaa,
    0x55, 0x55, 0xff, 0xff, 0x55, 0x55, 0xff, 0xff
};

static const std::array<uint8_t, 16> guessB = {
    0x00, 0x00, 0x00, 0x00, 0xaa, 0xaa, 0xaa, 0xaa,
    0x55, 0x55, 0x55, 0x55, 0xff, 0xff, 0xff, 0xff
};

static const std::array<uint8_t, 6> guessCube = {
    0, 95, 135, 175, 215, 255
};

static const std::array<uint8_t, 24> guessGrey = {
    8,   18,  28,  38,  48,  58,  68,  78,  88,  98,  108, 118,
    128, 138, 148, 158, 168, 178, 188, 198, 208, 218, 228, 238
};

int ZColor::redOrGuess() const {
    switch (val & 0xff000000) {
        case TERMPAINT_RGB_COLOR_OFFSET:
            return val >> 16 & 0xff;
        case TERMPAINT_NAMED_COLOR & 0xff000000:
            if ((val & 0xfff00000) == TERMPAINT_NAMED_COLOR) {
                return guessR[val & 0xf];
            } else if ((val & 0xfff00000) == TERMPAINT_INDEXED_COLOR) {
                int idx = val & 0xff;
                if (idx < 16) {
                    return guessR[idx];
                } else if (idx >= 232) {
                    return guessGrey[idx - 232];
                } else {
                    return guessCube[(idx-16) / 36];
                }
            }
            [[fallthrough]];
        default:
            // nothing much we can do here
            return 0x80;
    }
}

int ZColor::greenOrGuess() const {
    switch (val & 0xff000000) {
        case TERMPAINT_RGB_COLOR_OFFSET:
            return val >> 8 & 0xff;
        case TERMPAINT_NAMED_COLOR & 0xff000000:
            if ((val & 0xfff00000) == TERMPAINT_NAMED_COLOR) {
                return guessG[val & 0xf];
            } else if ((val & 0xfff00000) == TERMPAINT_INDEXED_COLOR) {
                int idx = val & 0xff;
                if (idx < 16) {
                    return guessG[idx];
                } else if (idx >= 232) {
                    return guessGrey[idx - 232];
                } else {
                    return guessCube[((idx-16) / 6) % 6];
                }
            }
            [[fallthrough]];
        default:
            // nothing much we can do here
            return 0x80;
    }
}

int ZColor::blueOrGuess() const {
    switch (val & 0xff000000) {
        case TERMPAINT_RGB_COLOR_OFFSET:
            return val & 0xff;
        case TERMPAINT_NAMED_COLOR & 0xff000000:
            if ((val & 0xfff00000) == TERMPAINT_NAMED_COLOR) {
                return guessB[val & 0xf];
            } else if ((val & 0xfff00000) == TERMPAINT_INDEXED_COLOR) {
                int idx = val & 0xff;
                if (idx < 16) {
                    return guessB[idx];
                } else if (idx >= 232) {
                    return guessGrey[idx - 232];
                } else {
                    return guessCube[(idx-16) % 6];
                }
            }
            [[fallthrough]];
        default:
            // nothing much we can do here
            return 0x80;
    }
}

#if !defined(TUIWIDGETS_NO_GAMMA_LUT)
/*
The following lookup table was generated with this code:
import math
for i in range(256): print(math.pow(i/255, 2.2), end=', ')
*/
static const std::array<double, 256> gamma22ToLinearLUT = {
    0.0, 5.077051900661759e-06, 2.3328004666098932e-05, 5.69217657121931e-05, 0.00010718736234124402,
    0.0001751239775030267, 0.00026154375454849144, 0.0003671362698159426, 0.0004925037871914326, 0.0006381828421670219,
    0.0008046584995130583, 0.0009923743040743253, 0.0012017395224384016, 0.001433134589671864, 0.0016869153167892836,
    0.0019634162133964697, 0.002262953160706434, 0.002585825596234168, 0.0029323183239383624, 0.003302703032003638,
    0.0036972395789001307, 0.004116177093282753, 0.00455975492252602, 0.005028203456855535, 0.0055217448502396585,
    0.006040593654849813, 0.006584957382581685, 0.007155037004573032, 0.00775102739766061, 0.008373117745148581,
    0.009021491898012131, 0.009696328701658229, 0.010397802292555288, 0.011126082368383245, 0.011881334434813665,
    0.012663720031582098, 0.013473396940142641, 0.014310519374884064, 0.015175238159625197, 0.016067700890886875,
    0.016988052089250045, 0.017936433339950233, 0.018912983423721504, 0.01991783843878572, 0.020951131914781092,
    0.02201299491933654, 0.023103556157921437, 0.02422294206753424, 0.025371276904734584, 0.026548682828472916,
    0.027755279978126032, 0.028991186547107816, 0.030256518852388652, 0.03155139140022645, 0.03287591694838383,
    0.03423020656508195, 0.035614369684918774, 0.0370285141619602, 0.03847274632019464, 0.03994717100152558,
    0.04145189161146246, 0.042987010162657116, 0.04455262731642138, 0.04614884242235095, 0.04777575355617064,
    0.04943345755590796, 0.051122050056493396, 0.05284162552287903, 0.05459227728176034, 0.05637409755197975,
    0.05818717747368544, 0.06003160713631323, 0.06190747560545576, 0.06381487094867724, 0.06575388026033009,
    0.06772458968542432, 0.0697270844425988, 0.07176144884623908, 0.07382776632778461, 0.07592611945626479,
    0.07805658995810189, 0.08021925873621505, 0.08241420588845923, 0.08464151072542946, 0.08690125178766034,
    0.08919350686224782, 0.09151835299891949, 0.09387586652557776, 0.09626612306333969, 0.09868919754109445,
    0.10114516420959989, 0.10363409665513738, 0.1061560678127439, 0.10871114997903854, 0.11129941482466024,
    0.11392093340633272, 0.11657577617857154, 0.11926401300504741, 0.12198571316961948, 0.1247409453870513,
    0.12752977781342206, 0.13035227805624436, 0.1332085131842997, 0.13609854973720245, 0.1390224537347025,
    0.14198029068573553, 0.14497212559723088, 0.14799802298268516, 0.15105804687051058, 0.15415226081216518,
    0.1572807278900734, 0.16044351072534352, 0.16364067148528988, 0.1668722718907655, 0.1701383732233124,
    0.173439036332135, 0.17677432164090326, 0.18014428915439032, 0.18354899846495082, 0.18698850875884424,
    0.1904628788224093, 0.19397216704809314, 0.19751643144034023, 0.20109572962134564, 0.20471011883667684,
    0.20835965596076741, 0.2120443975022877, 0.215764399609395, 0.2195197180748679, 0.22331040834112742,
    0.2271365255051489, 0.23099812432326744, 0.23489525921588023, 0.2388279842720483, 0.24279635325400195,
    0.24680041960155044, 0.25084023643640047, 0.2549158565663851, 0.25902733248960613, 0.2631747163984916,
    0.267358060183772, 0.2715774154383751, 0.27583283346124515, 0.2801243652610849, 0.28445206156002445,
    0.2888159727972186, 0.29321614913237454, 0.2976526404492112, 0.30212549635885283, 0.3066347662031576,
    0.31118049905798434, 0.3157627437363971, 0.3203815487918104, 0.3250369625210763, 0.3297290329675149,
    0.33445780792388924, 0.3392233349353267, 0.34402566130218676, 0.348864834082879, 0.35374090009662945,
    0.3586539059261989, 0.36360389792055325, 0.36859092219748707, 0.37361502464620194, 0.37867625092984036,
    0.3837746464879752, 0.38891025653905886, 0.39408312608282897, 0.39929329990267437, 0.4045408225679618,
    0.40982573843632336, 0.41514809165590655, 0.42050792616758714, 0.42590528570714575, 0.4313402138074096,
    0.4368127538003594, 0.4423229488192018, 0.4478708418004099, 0.4534564754857306, 0.4590798924241601,
    0.4647411349738896, 0.4704402453042184, 0.4761772653974402, 0.4819522370506978, 0.48776520187781053,
    0.49361620131107364, 0.4995052766030301, 0.505432468828216, 0.5113978188848795, 0.5174013674966733,
    0.5234431552143247, 0.5295232224172772, 0.5356416093153108, 0.5417983559501369, 0.5479935021969718,
    0.5542270877660852, 0.5604991522043282, 0.5668097348966382, 0.5731588750675233, 0.5795466117825252,
    0.5859729839496614, 0.5924380303208466, 0.598941789493296, 0.6054842999109072, 0.6120655998656237,
    0.6186857274987796, 0.6253447208024265, 0.6320426176206414, 0.6387794556508168, 0.6455552724449345,
    0.6523701054108211, 0.6592239918133873, 0.6661169687758508, 0.6730490732809419, 0.6800203421720954,
    0.6870308121546249, 0.6940805197968822, 0.7011695015314021, 0.7082977936560323, 0.7154654323350483,
    0.7226724536002546, 0.7299188933520705, 0.7372047873606051, 0.744530171266715, 0.7518950805830509,
    0.7592995506950911, 0.7667436168621613, 0.7742273142184416, 0.7817506777739623, 0.7893137424155858,
    0.7969165429079781, 0.8045591138945669, 0.8122414898984895, 0.8199637053235279, 0.8277257944550337,
    0.8355277914608409, 0.8433697303921693, 0.8512516451845149, 0.8591735696585323, 0.8671355375209048,
    0.8751375823652049, 0.8831797376727453, 0.8912620368134188, 0.8993845130465294, 0.907547199521614,
    0.9157501292792527, 0.9239933352518732, 0.9322768502645428, 0.940600707035753, 0.9489649381781952,
    0.9573695761995268, 0.9658146535031301, 0.9743002023888613, 0.9828262550537913, 0.9913928435929399, 1.0
};
#endif

static double gamma22ToLinear(uint8_t val) {
#if !defined(TUIWIDGETS_NO_GAMMA_LUT)
    // Using lookup tables in both directions speeds up ZColor->ZColorHSV->ZColor by almost 6x, so trade memory
    // for performance
    return gamma22ToLinearLUT[val];
#else
    return pow(val / 255., 2.2);
#endif
}

#if !defined(TUIWIDGETS_NO_GAMMA_LUT)
/*
The following lookup table was generated with this code:
import math, pprint
lut = {}
N = 350
for i in range(1000):
    d = i/1000
    lut[int(math.sqrt(math.pow(d, 2.2)) * N + 0.5)] = int(d*255)
for expected, val in [(i, math.pow(i/255, 2.2)) for i in range(256)]:
    lut[int(math.sqrt(val) * N + 0.5)] = expected

pprint.pprint(lut)
for expected, val in [(i, math.pow(i/255, 2.2)) for i in range(256)]:
    res = lut[int(math.sqrt(val) * N + 0.5)]
    if expected - res != 0:
        print('failed')
        break

matplotlib.pyplot.plot([i/10000 for i in range(10001)], [int(pow(i/10000, 1 / 2.2)*255 + 0.5) for i in range(10001)])
matplotlib.pyplot.plot([i/10000 for i in range(10001)], [lut[int(math.sqrt(i/10000) * N + 0.5)] for i in range(10001)])
*/
static const std::array<uint16_t, 351> linearToGamma22LUT = {
     0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 14, 15, 16, 17, 18, 19, 20, 21, 21, 22, 23, 24, 25, 26, 26, 27,
     28, 29, 30, 31, 31, 32, 33, 34, 35, 35, 36, 37, 38, 39, 39, 40, 41, 42, 43, 43, 44, 45, 46, 47, 47, 48, 49, 50,
     50, 51, 52, 53, 54, 54, 55, 56, 57, 57, 58, 59, 60, 60, 61, 62, 63, 63, 64, 65, 66, 67, 67, 68, 69, 70, 70, 71,
     72, 73, 73, 74, 75, 76, 76, 77, 78, 79, 79, 80, 81, 82, 82, 83, 84, 84, 85, 86, 87, 87, 88, 89, 90, 90, 91, 92,
     93, 93, 94, 95, 95, 96, 97, 98, 98, 99, 100, 101, 101, 102, 103, 103, 104, 105, 106, 106, 107, 108, 109, 109, 110,
     111, 111, 112, 113, 114, 114, 115, 116, 116, 117, 118, 119, 119, 120, 121, 121, 122, 123, 124, 124, 125, 126, 126,
     127, 128, 129, 129, 130, 131, 131, 132, 133, 134, 134, 135, 136, 136, 137, 138, 138, 139, 140, 141, 141, 142, 143,
     143, 144, 145, 145, 146, 147, 148, 148, 149, 150, 150, 151, 152, 152, 153, 154, 155, 155, 156, 157, 157, 158, 159,
     159, 160, 161, 162, 162, 163, 164, 164, 165, 166, 166, 167, 168, 168, 169, 170, 170, 171, 172, 173, 173, 174, 175,
     175, 176, 177, 177, 178, 179, 179, 180, 181, 181, 182, 183, 184, 184, 185, 186, 186, 187, 188, 188, 189, 190, 190,
     191, 192, 192, 193, 194, 194, 195, 196, 196, 197, 198, 199, 199, 200, 201, 201, 202, 203, 203, 204, 205, 205, 206,
     207, 207, 208, 209, 209, 210, 211, 211, 212, 213, 213, 214, 215, 215, 216, 217, 217, 218, 219, 219, 220, 221, 221,
     222, 223, 224, 224, 225, 226, 226, 227, 228, 228, 229, 230, 230, 231, 232, 232, 233, 234, 234, 235, 236, 236, 237,
     238, 238, 239, 240, 240, 241, 242, 242, 243, 244, 244, 245, 246, 246, 247, 248, 248, 249, 250, 250, 251, 252, 252,
     253, 254, 254, 255
};
#endif

uint8_t linearToGamma22(double val) {
#if !defined(TUIWIDGETS_NO_GAMMA_LUT)
    // Using lookup tables in both directions speeds up ZColor->ZColorHSV->ZColor by almost 6x, so trade memory
    // for performance
    unsigned int idx = sqrt(val) * 350 + 0.5;
    return linearToGamma22LUT[idx <= 350 ? idx : 350];
#else
    return pow(val, 1 / 2.2) * 255 + 0.5;
#endif
}

ZColorHSV ZColor::toHsv() const {
    double r;
    double g;
    double b;

    if ((val & 0xff000000) == TERMPAINT_RGB_COLOR_OFFSET) {
        r = gamma22ToLinear(val >> 16 & 0xff);
        g = gamma22ToLinear(val >> 8 & 0xff);
        b = gamma22ToLinear(val & 0xff);
    } else {
        r = gamma22ToLinear(redOrGuess());
        g = gamma22ToLinear(greenOrGuess());
        b = gamma22ToLinear(blueOrGuess());
    }

    ZColorHSV ret;

    // this is the calculation from wikipedia with two changes:
    // using a small offset in the divisor to avoid division by zero
    // replacing min and max by an cascade of if·s that establish the order of the 3 color components and
    //      then use that to pick the right color component for min/max. This avoids repeating conditional evaluation.

    if (g >= b) {
        if (r >= g) {
            // r >= g >= b
            double delta = r - b;
            ret.setSaturation(delta / (r + 1e-20f));
            ret.setValue(r);
            ret.setHue(60 * (g - b) / (delta + 1e-20f));
        } else {
            // g >= b, g > r
            if (b >= r) {
                // g >= b >= r
                double delta = g - r;
                ret.setSaturation(delta / (g + 1e-20f));
                ret.setValue(g);
                ret.setHue(120 + 60 * (b - r) / (delta + 1e-20f));
            } else {
                // g >= b, g > r, r > b
                //   -> g > r > b
                double delta = g - b;
                ret.setSaturation(delta / (g));
                ret.setValue(g);
                ret.setHue(120 + 60 * (b - r) / (delta));
            }
        }
    } else {
        // b > g
        if (r >= g) {
            // b > g, r >= g
            if (b >= r) {
                // r >= g, b >= r
                //   -> b >= r >= g
                double delta = b - g;
                ret.setSaturation(delta / (b + 1e-20f));
                ret.setValue(b);
                ret.setHue(240 + 60 * (r - g) / (delta + 1e-20f));
            } else {
                // b > g, r > b
                //   -> r > b > g
                double delta = r - g;
                ret.setSaturation(delta / (r));
                ret.setValue(r);
                ret.setHue(360 + 60 * (g - b) / (delta));
            }
        } else {
            // b > g, g > r
            //   -> b > g > r
            double delta = b - r;
            ret.setSaturation(delta / (b));
            ret.setValue(b);
            ret.setHue(240 + 60 * (r - g) / (delta));
        }
    }

    return ret;
}

uint32_t ZColor::nativeValue() const {
    return val;
}

ZColor ZColor::fromRgb(int r, int g, int b) {
    return ZColor(r, g, b);
}

ZColor ZColor::defaultColor() {
    ZColor col;
    col.val = TERMPAINT_DEFAULT_COLOR;
    return col;
}

ZColor ZColor::fromTerminalColor(TerminalColor color) {
    return fromTerminalColor(static_cast<int>(color));
}

ZColor ZColor::fromTerminalColor(int color) {
    ZColor col;
    col.val = TERMPAINT_NAMED_COLOR | (color & 0xf);
    return col;
}

ZColor ZColor::fromTerminalColorIndexed(int color) {
    ZColor col;
    col.val = TERMPAINT_INDEXED_COLOR | (color & 0xff);
    return col;
}

ZColor ZColor::fromHsvStrict(double hue, double saturation, double value) {
    // hue between 0 (inclusive) and 360 (exclusive)
    // saturation between 0 (inclusive) and 1 (inclusive)
    // value between 0 (inclusive) and 1 (inclusive)
    auto retfRGB = [](double r, double g, double b) {
        return ZColor::fromRgb(linearToGamma22(r), linearToGamma22(g), linearToGamma22(b));
    };

    // This is the calculation from wikipedia, but moving all parts of the calculation into the conditional
    // blocks where this allows using information from the conditionals to simplify the calculation

    double chroma = value * saturation;
    double m = value - chroma;
    if (hue < 60.) { // hue < 0 is invalid
        double x = chroma * (hue / 60.);
        return retfRGB(chroma + m, x + m, m);
    } else if (hue < 60.*2) { // 1 < H' < 2
        double x = chroma * (1 - ((hue - 60.) / 60.));
        return retfRGB(x + m, chroma + m, m);
    } else if (hue < 60.*3) { // 2 < H' < 3
        double x = chroma * ((hue - 60. * 2) / 60.);
        return retfRGB(m, chroma + m, x + m);
    } else if (hue < 60.*4) { // 3 < H' < 4
        double x = chroma * (1 - ((hue - 60.*3) / 60.));
        return retfRGB(m, x + m, chroma + m);
    } else if (hue < 60.*5) { // 4 < H' < 5
        double x = chroma * ((hue - 60. * 4) / 60.);
        return retfRGB(x + m, m, chroma + m);
    } else { // 5 < H' < 6
        double x = chroma * (1 - ((hue - 60.*5) / 60.));
        return retfRGB(chroma + m, m, x + m);
    }
}

ZColor ZColor::fromHsvStrict(const ZColorHSV &hsv) {
    return fromHsvStrict(hsv.hue(), hsv.saturation(), hsv.value());
}

ZColor ZColor::fromHsv(double hue, double saturation, double value) {
    if (value > 1.0) {
        value = 1.0;
    }
    if (value < 0) {
        value = 0;
    }
    if (saturation > 1.0) {
        saturation = 1.0;
    }
    if (saturation < 0) {
        saturation = 0;
    }
    while (hue < 0) {
        hue += 360;
    }
    while (hue > 360) {
        hue -= 360;
    }

    return fromHsvStrict(hue, saturation, value);
}

ZColor ZColor::fromHsv(const ZColorHSV &hsv) {
    return fromHsv(hsv.hue(), hsv.saturation(), hsv.value());
}

bool ZColor::operator==(const ZColor &other) const {
    return val == other.val;
}

bool ZColor::operator!=(const ZColor &other) const {
    return val != other.val;
}

ZColorHSV::ZColorHSV() {
}

ZColorHSV::ZColorHSV(double hue, double saturation, double value) : _hue(hue), _saturation(saturation), _value(value) {
}

double ZColorHSV::hue() const {
    return _hue;
}

void ZColorHSV::setHue(double hue) {
    _hue = hue;
}

double ZColorHSV::saturation() const {
    return _saturation;
}

void ZColorHSV::setSaturation(double saturation) {
    _saturation = saturation;
}

double ZColorHSV::value() const {
    return _value;
}

void ZColorHSV::setValue(double value) {
    _value = value;
}

bool ZColorHSV::operator==(const ZColorHSV &other) const {
    return hue() == other.hue() && saturation() == other.saturation() && value() == other.value();
}

bool ZColorHSV::operator!=(const ZColorHSV &other) const {
    return !(*this == other);
}

TUIWIDGETS_NS_END
