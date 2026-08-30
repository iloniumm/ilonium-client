/*

*************************************************************************

RetroCycles

**************************************************************************

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

***************************************************************************

*/

#ifndef ArmageTron_OWNERSHIP_H
#define ArmageTron_OWNERSHIP_H

// Ownership marks compiled into the binary. The optimiser is free to drop a
// string nothing reads, so each one is handed to a volatile pointer at
// namespace scope: that counts as an observable write and keeps the bytes in
// the image. Nothing reads them back and nothing is printed, so they cost the
// binary their own length and not a cycle of runtime.
//
// Place one per translation unit with a tag unique to that unit.

#define RC_OWNERSHIP( tag )                                                  \
    namespace                                                                \
    {                                                                        \
    char const * const rc_owner_str_##tag =                                  \
        "RetroCycles :: ilona :: " #tag " :: (c) ilona, all rights reserved"; \
    void const * volatile rc_owner_ref_##tag = rc_owner_str_##tag;           \
    }

#endif
