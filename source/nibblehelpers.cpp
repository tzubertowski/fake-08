
#include <string>

#include "hostVmShared.h"
#include "nibblehelpers.h"
#include "audioOptimizations.h"



int getCombinedIdx(const int x, const int y){
	//bit shifting might be faster? trying to optimize
    //return y * 64 + (x / 2);
	//return (y << 6) | (x >> 1);
	return COMBINED_IDX(x, y);
}

int isValidSprIdx(int x, int y){
	return IS_VALID_SPR_IDX(x, y);
}

//try look up table to optimize?

void setPixelNibble(const int x, const int y, uint8_t value, uint8_t* targetBuffer) {
	// Simple optimized version without lookup tables  
	int idx = COMBINED_IDX(x, y);
	targetBuffer[idx] = (x & 1)
		? (targetBuffer[idx] & 0x0f) | (value << 4)
		: (targetBuffer[idx] & 0xf0) | (value & 0x0f);
}

uint8_t getPixelNibble(const int x, const int y, const uint8_t* targetBuffer) {
	// Simple optimized version without lookup tables
	uint8_t byte = targetBuffer[COMBINED_IDX(x, y)];
	return (x & 1) ? (byte >> 4) : (byte & 0x0f);
}