/* SPDX-License-Identifier: Apache-2.0 */

#ifndef MATRIX_FONT_H_
#define MATRIX_FONT_H_

#include <stdint.h>
#include <zephyr/sys/util.h>

/* Each byte is a row; bit 0 is the leftmost pixel. */
#define MATRIX_PIXEL_BIT(idx, val) ((val) ? BIT(idx) : 0)
#define MATRIX_PIXEL_MASK(...) (FOR_EACH_IDX(MATRIX_PIXEL_BIT, (|), __VA_ARGS__))

/* Uppercase A-Z, indexed by character - 'A'. */
static const uint8_t matrix_letter_patterns[26][5] = {
	/* A */
	{
		MATRIX_PIXEL_MASK(0, 1, 1, 1, 0),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
		MATRIX_PIXEL_MASK(1, 1, 1, 1, 1),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
	},
	/* B */
	{
		MATRIX_PIXEL_MASK(1, 1, 1, 1, 0),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
		MATRIX_PIXEL_MASK(1, 1, 1, 1, 0),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
		MATRIX_PIXEL_MASK(1, 1, 1, 1, 0),
	},
	/* C */
	{
		MATRIX_PIXEL_MASK(0, 1, 1, 1, 1),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 0),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 0),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 0),
		MATRIX_PIXEL_MASK(0, 1, 1, 1, 1),
	},
	/* D */
	{
		MATRIX_PIXEL_MASK(1, 1, 1, 1, 0),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
		MATRIX_PIXEL_MASK(1, 1, 1, 1, 0),
	},
	/* E */
	{
		MATRIX_PIXEL_MASK(1, 1, 1, 1, 1),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 0),
		MATRIX_PIXEL_MASK(1, 1, 1, 1, 0),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 0),
		MATRIX_PIXEL_MASK(1, 1, 1, 1, 1),
	},
	/* F */
	{
		MATRIX_PIXEL_MASK(1, 1, 1, 1, 1),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 0),
		MATRIX_PIXEL_MASK(1, 1, 1, 1, 0),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 0),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 0),
	},
	/* G */
	{
		MATRIX_PIXEL_MASK(0, 1, 1, 1, 1),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 0),
		MATRIX_PIXEL_MASK(1, 0, 1, 1, 1),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
		MATRIX_PIXEL_MASK(0, 1, 1, 1, 1),
	},
	/* H */
	{
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
		MATRIX_PIXEL_MASK(1, 1, 1, 1, 1),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
	},
	/* I */
	{
		MATRIX_PIXEL_MASK(1, 1, 1, 1, 1),
		MATRIX_PIXEL_MASK(0, 0, 1, 0, 0),
		MATRIX_PIXEL_MASK(0, 0, 1, 0, 0),
		MATRIX_PIXEL_MASK(0, 0, 1, 0, 0),
		MATRIX_PIXEL_MASK(1, 1, 1, 1, 1),
	},
	/* J */
	{
		MATRIX_PIXEL_MASK(0, 0, 1, 1, 1),
		MATRIX_PIXEL_MASK(0, 0, 0, 1, 0),
		MATRIX_PIXEL_MASK(0, 0, 0, 1, 0),
		MATRIX_PIXEL_MASK(1, 0, 0, 1, 0),
		MATRIX_PIXEL_MASK(0, 1, 1, 0, 0),
	},
	/* K */
	{
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
		MATRIX_PIXEL_MASK(1, 0, 0, 1, 0),
		MATRIX_PIXEL_MASK(1, 1, 1, 0, 0),
		MATRIX_PIXEL_MASK(1, 0, 0, 1, 0),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
	},
	/* L */
	{
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 0),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 0),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 0),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 0),
		MATRIX_PIXEL_MASK(1, 1, 1, 1, 1),
	},
	/* M */
	{
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
		MATRIX_PIXEL_MASK(1, 1, 0, 1, 1),
		MATRIX_PIXEL_MASK(1, 0, 1, 0, 1),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
	},
	/* N */
	{
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
		MATRIX_PIXEL_MASK(1, 1, 0, 0, 1),
		MATRIX_PIXEL_MASK(1, 0, 1, 0, 1),
		MATRIX_PIXEL_MASK(1, 0, 0, 1, 1),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
	},
	/* O */
	{
		MATRIX_PIXEL_MASK(0, 1, 1, 1, 0),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
		MATRIX_PIXEL_MASK(0, 1, 1, 1, 0),
	},
	/* P */
	{
		MATRIX_PIXEL_MASK(1, 1, 1, 1, 0),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
		MATRIX_PIXEL_MASK(1, 1, 1, 1, 0),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 0),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 0),
	},
	/* Q */
	{
		MATRIX_PIXEL_MASK(0, 1, 1, 1, 0),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
		MATRIX_PIXEL_MASK(1, 0, 1, 0, 1),
		MATRIX_PIXEL_MASK(1, 0, 0, 1, 0),
		MATRIX_PIXEL_MASK(0, 1, 1, 0, 1),
	},
	/* R */
	{
		MATRIX_PIXEL_MASK(1, 1, 1, 1, 0),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
		MATRIX_PIXEL_MASK(1, 1, 1, 1, 0),
		MATRIX_PIXEL_MASK(1, 0, 0, 1, 0),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
	},
	/* S */
	{
		MATRIX_PIXEL_MASK(0, 1, 1, 1, 1),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 0),
		MATRIX_PIXEL_MASK(0, 1, 1, 1, 0),
		MATRIX_PIXEL_MASK(0, 0, 0, 0, 1),
		MATRIX_PIXEL_MASK(1, 1, 1, 1, 0),
	},
	/* T */
	{
		MATRIX_PIXEL_MASK(1, 1, 1, 1, 1),
		MATRIX_PIXEL_MASK(0, 0, 1, 0, 0),
		MATRIX_PIXEL_MASK(0, 0, 1, 0, 0),
		MATRIX_PIXEL_MASK(0, 0, 1, 0, 0),
		MATRIX_PIXEL_MASK(0, 0, 1, 0, 0),
	},
	/* U */
	{
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
		MATRIX_PIXEL_MASK(0, 1, 1, 1, 0),
	},
	/* V */
	{
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
		MATRIX_PIXEL_MASK(0, 1, 0, 1, 0),
		MATRIX_PIXEL_MASK(0, 0, 1, 0, 0),
	},
	/* W */
	{
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
		MATRIX_PIXEL_MASK(1, 0, 1, 0, 1),
		MATRIX_PIXEL_MASK(1, 1, 0, 1, 1),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
	},
	/* X */
	{
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
		MATRIX_PIXEL_MASK(0, 1, 0, 1, 0),
		MATRIX_PIXEL_MASK(0, 0, 1, 0, 0),
		MATRIX_PIXEL_MASK(0, 1, 0, 1, 0),
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
	},
	/* Y */
	{
		MATRIX_PIXEL_MASK(1, 0, 0, 0, 1),
		MATRIX_PIXEL_MASK(0, 1, 0, 1, 0),
		MATRIX_PIXEL_MASK(0, 0, 1, 0, 0),
		MATRIX_PIXEL_MASK(0, 0, 1, 0, 0),
		MATRIX_PIXEL_MASK(0, 0, 1, 0, 0),
	},
	/* Z */
	{
		MATRIX_PIXEL_MASK(1, 1, 1, 1, 1),
		MATRIX_PIXEL_MASK(0, 0, 0, 1, 0),
		MATRIX_PIXEL_MASK(0, 0, 1, 0, 0),
		MATRIX_PIXEL_MASK(0, 1, 0, 0, 0),
		MATRIX_PIXEL_MASK(1, 1, 1, 1, 1),
	},
};

static const uint8_t heart_pattern[5] = {
	MATRIX_PIXEL_MASK(0, 1, 0, 1, 0),
	MATRIX_PIXEL_MASK(1, 1, 1, 1, 1),
	MATRIX_PIXEL_MASK(1, 1, 1, 1, 1),
	MATRIX_PIXEL_MASK(0, 1, 1, 1, 0),
	MATRIX_PIXEL_MASK(0, 0, 1, 0, 0),
};

static const uint8_t blank_pattern[5] = {0};

#undef MATRIX_PIXEL_MASK
#undef MATRIX_PIXEL_BIT

#endif /* MATRIX_FONT_H_ */
