// Copyleft 2023 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      26jan23	initial version

*/

// An interval set is a mixed-radix number, and its radices (or places) are
// defined by a hexadecimal value called a set code. Each hexadecimal digit
// specifies the range of one of the set's places. A place's value can vary
// from zero to its range minus one, and the leftmost digit corresponds to
// the least significant place. For example, the code 246 defines a set in
// which the least significant place ranges from 0 to 1, the middle place
// ranges from 0 to 3, and the most significant place ranges from 0 to 5.
// This set has a total range of 12 (2 + 4 + 6), and the number of unique
// states (or permutations) it can have is 48 (2 x 4 x 6).
//
// A set must have at least two places, the minimum place range is two, and
// the sum of a set's place ranges can't exceed twelve. Each set is given in
// its prime form, which excludes phase shifts and reversals. For example,
// the sets 246, 462, 624, 642, 426, and 264 are considered equivalent, and
// their prime form is 246. Given the preceding constraints, the list below
// is exhaustive. The intended use is atonal music theory, and specifically
// permutational chord progressions. 
//
// The set codes are listed in order by a three-level sort:
//	1. number of places
//	2. sum of place ranges
//  3. set code value

INTERVAL_SET(22)
INTERVAL_SET(23)
INTERVAL_SET(24)
INTERVAL_SET(33)
INTERVAL_SET(25)
INTERVAL_SET(34)
INTERVAL_SET(26)
INTERVAL_SET(35)
INTERVAL_SET(44)
INTERVAL_SET(27)
INTERVAL_SET(36)
INTERVAL_SET(45)
INTERVAL_SET(28)
INTERVAL_SET(37)
INTERVAL_SET(46)
INTERVAL_SET(55)
INTERVAL_SET(29)
INTERVAL_SET(38)
INTERVAL_SET(47)
INTERVAL_SET(56)
INTERVAL_SET(2A)
INTERVAL_SET(39)
INTERVAL_SET(48)
INTERVAL_SET(57)
INTERVAL_SET(66)
INTERVAL_SET(222)
INTERVAL_SET(223)
INTERVAL_SET(224)
INTERVAL_SET(233)
INTERVAL_SET(225)
INTERVAL_SET(234)
INTERVAL_SET(333)
INTERVAL_SET(226)
INTERVAL_SET(235)
INTERVAL_SET(244)
INTERVAL_SET(334)
INTERVAL_SET(227)
INTERVAL_SET(236)
INTERVAL_SET(245)
INTERVAL_SET(335)
INTERVAL_SET(344)
INTERVAL_SET(228)
INTERVAL_SET(237)
INTERVAL_SET(246)
INTERVAL_SET(255)
INTERVAL_SET(336)
INTERVAL_SET(345)
INTERVAL_SET(444)
INTERVAL_SET(2222)
INTERVAL_SET(2223)
INTERVAL_SET(2224)
INTERVAL_SET(2233)
INTERVAL_SET(2225)
INTERVAL_SET(2234)
INTERVAL_SET(2333)
INTERVAL_SET(2226)
INTERVAL_SET(2235)
INTERVAL_SET(2244)
INTERVAL_SET(2334)
INTERVAL_SET(3333)
INTERVAL_SET(22222)
INTERVAL_SET(22223)
INTERVAL_SET(22224)
INTERVAL_SET(22233)
INTERVAL_SET(222222)

#undef INTERVAL_SET
