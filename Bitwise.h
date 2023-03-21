#include <bitset>

#include <cmath>
#include <climits>
#include <functional>
#include <map>
#include <string>

enum class BitwiseOp {
    AND,
    OR,
    XOR,
    NOT,
    SHIFT_LEFT,
    SHIFT_RIGHT,
    ROTATE_LEFT,
    ROTATE_RIGHT,
    FLIP,
    SWAP,
};

const std::map<BitwiseOp, std::string> bitwiseOpLabels = {
    {BitwiseOp::AND, "AND"},
    {BitwiseOp::OR, "OR"},
    {BitwiseOp::XOR, "XOR"},
    {BitwiseOp::NOT, "NOT"},
    {BitwiseOp::SHIFT_LEFT, "SHIFT_LEFT"},
    {BitwiseOp::SHIFT_RIGHT, "SHIFT_RIGHT"},
    {BitwiseOp::ROTATE_LEFT, "ROTATE_LEFT"},
    {BitwiseOp::ROTATE_RIGHT, "ROTATE_RIGHT"},
    {BitwiseOp::FLIP, "FLIP"},
    {BitwiseOp::SWAP, "SWAP"}
};

// Function pointer type for bitwise operations
typedef int32_t (*BitwiseFunc)(int32_t, int32_t);

// Bitwise AND Function
int32_t bitwiseAND(int32_t a, int32_t b);

// Bitwise OR Function
int32_t bitwiseOR(int32_t a, int32_t b);

// Bitwise XOR Function
int32_t bitwiseXOR(int32_t a, int32_t b);

// Bitwise NOT Function
int32_t bitwiseNOT(int32_t a, int32_t b);

// Bitwise Left Shift Function
int32_t bitwiseShiftLeft(int32_t a, int32_t b);

// Bitwise Right Shift Function
int32_t bitwiseShiftRight(int32_t a, int32_t b);

// Bitwise Rotate Left Function
int32_t bitwiseRotateLeft(int32_t a, int32_t b);

// Bitwise Rotate Right Function
int32_t bitwiseRotateRight(int32_t a, int32_t b);

// Bitwise Flip Function
int32_t bitwiseFlip(int32_t a, int32_t b);

// Bitwise Swap Function (returns swapped value without modifying original parameter)
int32_t swapBits(int32_t value, int i, int j);

// Bitwise function dispatcher
int32_t bitwise(int32_t a, int32_t b, BitwiseOp op);


int32_t bitScramble(int32_t value, int32_t seed);

int32_t floatTo24bit(float value);

float intToFloat24bit(int32_t value);


using ByteBeatEquation = std::function<float(int)>;

static const std::map<int, ByteBeatEquation> BYTE_BEAT_EQUATIONS = {
    {0, [](int t) { return char((t % 255 & t) - (t >> 13 & t)); }}, // (t%255&t)-(t>>13&t)
    {1, [](int t) { return char((t & t % 255) - (t >> 13 & (t % (t >> 8 | t >> 16)))); }}, // (t&t%255)-(t>>13&(t%(t>>8|t>>16)))
    {2, [](int t) { return char(t * (t >> 10 & ((t >> 16) + 1))); }}, // t*(t>>10&((t>>16)+1))  //cycles through all t*(t>>10&) melodies, like the 42 melody
    {3, [](int t) { return char((t >> 10) ^ (t >> 14) | (t >> 12) * 42319); }}, // (t>>10)^(t>>14)|(t>>12)*42319 //basic rng w/ slight bias to fliping between high and low
    {4, [](int t) { return char(t >> t); }}, // t>>t //odd thing
    {5, [](int t) { return char((t >> 8 & t) * t); }}, // (t>>8&t)*t //chaotic
    {6, [](int t) { return char((t >> 13 & t) * (t >> 8)); }}, // (t>>13&t)*(t>>8) //Slower version, has interesting properties
    {7, [](int t) { return char((t >> 8 & t) * (t >> 15 & t)); }}, // (t>>8&t)*(t>>15&t) //Ambient
    {8, [](int t) { return char((t % (t >> 8 | t >> 16)) ^ t); }}, // (t%(t>>8|t>>16))^t //mod fractal tree cycles through different rhythms
    {9, [](int t) { return char(t % (t >> 8 | t >> 16)); }}, // t%(t>>8|t>>16)  //Acts like t and can be used for cool effect. Generates interesting and infinite rhythm variations.
    {10, [](int t) { return char(-0.99999999 * t * t); }}, // -0.99999999*t*t //Add more 9s to make slower, remove to make faster.
    {11, [](int t) { return char(t % (t >> 13 & t)); }}, // t%(t>>13&t) //Quiet, do -1 for to make louder and change the rhythm slightly.
    {12, [](int t) { return char(((t % (t >> 16 | t >> 8)) >> 2) & t) - 1; }}, // (((t%(t>>16|t>>8))>>2)&t)-1 //WARNING LOUD! Some kind of glitchcore thing?
    // ... Add the rest of the formulas with their respective comments here
};