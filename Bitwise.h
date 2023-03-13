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
