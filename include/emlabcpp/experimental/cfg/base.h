#include <cstdint>

#pragma once

namespace emlabcpp::cfg
{
///
/// one set of configuration stored in memory is called `record`, it is composed of following
/// sections (stored in memory as displayed):
///
/// | signature | header | field | field | field ...
///
/// The procedure of storing the record should be as follows:
///  1) store header
///  2) store fields
///  3) write signature
///
/// Reading is as follows:
///  1) go over potential spots for records, find spot that is not yet discarded and has highest id
///  2) try to read header and check it's checksum (if fails: discard and goto 1)
///  3) try to check checksum in signature (if fails: discard and goto 1)
///  4) read fields

using checksum  = uint32_t;
using record_id = uint32_t;

struct signature
{
        // id of record stored in memory, each new record should have higher than the previous one
        record_id id;
        // checksm of header
        checksum header_chck;
        // checksm of fields
        checksum fields_chck;
        // checksum of id, header_chck, fields_chck
        checksum sig_chck;
};

template < typename Payload >
struct header
{
        // extra payload for specific use case
        Payload pl;
        // number of fields stored after the header
        uint32_t field_count;
};

}  // namespace emlabcpp::cfg
