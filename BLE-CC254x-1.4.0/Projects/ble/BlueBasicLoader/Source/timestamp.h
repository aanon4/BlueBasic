/*
 * timestamp.h
 *
 * Timestamp for loader to force and update.
 */

#if TARGET_CC2540
#define BUILD_TIMESTAMP "CC2540/00000000000000"
#elif TARGET_CC2541
#define BUILD_TIMESTAMP "CC2541/00000000000000"
#elif TARGET_PETRA
#define BUILD_TIMESTAMP "Petra/00000000000000"
#else
#error "Unknown loader target"
#endif
