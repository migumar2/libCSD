/*  BitSequenceBuilder375.h
 * Copyright (C) 2008, Francisco Claude, all rights reserved.
 *
 * Francisco Claude <fclaude@cs.uwaterloo.ca>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef BITSEQUENCEBUILDER375_H
#define BITSEQUENCEBUILDER375_H

#include <libcdsBasics.h>
#include <BitSequence375.h>
#include <BitSequenceBuilder.h>

namespace cds_static
{
    /** BitSequence builder for BitSequence375
     *
     * @author Francisco Claude
     */
    class BitSequenceBuilder375 : public BitSequenceBuilder
    {
        public:
            BitSequenceBuilder375() {};
            virtual ~BitSequenceBuilder375() {};
            virtual BitSequence * build(uint * bitseq, size_t len) const;
            virtual BitSequence * build(const BitString & bs) const;
    };
};
#endif
