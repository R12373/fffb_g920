//
//      fffb
//      hid/report.hxx
//

#pragma once

#include <fffb/util/log.hxx>
#include <fffb/util/types.hxx>

#include <IOKit/hid/IOHIDLib.h>
#include <cstddef>   // size_t
#include <cstdint>   // uint8_t

// 8 was fine for “classic”.
// HID++ commonly needs 20 (long) and sometimes larger.
// 64 keeps us flexible without changing everything again soon.
#define FFFB_REPORT_MAX_LEN 64

namespace fffb
{

struct report
{
        // HID report id (0 if none). HID++ commonly uses 0x10/0x11/0x12 depending on format.
        uti::u8_t report_id { 0 };

        // What kind of HID report this is (output/feature/input).
        IOHIDReportType report_type { kIOHIDReportTypeOutput };

        // How many bytes in `data` are valid for send/read.
        std::size_t len { 0 };

        uti::u8_t data[ FFFB_REPORT_MAX_LEN ] { 0 };

        uti::u8_t       & operator[] ( std::size_t const index )       noexcept { return data[ index ]; }
        uti::u8_t const & operator[] ( std::size_t const index ) const noexcept { return data[ index ]; }

        [[nodiscard]] constexpr std::size_t capacity() const noexcept { return FFFB_REPORT_MAX_LEN; }
};

// Send exactly `report.len` bytes using `report.report_id` and `report.report_type`.
[[nodiscard]] inline bool write_report( apple::hid_device * device, report const & rep ) noexcept
{
        // Defensive: don’t send empty / oversized packets.
        if( rep.len == 0 || rep.len > rep.capacity() )
        {
                FFFB_F_ERR_S( "write_report", "invalid report len=%zu", (size_t)rep.len );
                return false;
        }

        return apple::_try(
                IOHIDDeviceSetReport(
                        device,
                        rep.report_type,
                        rep.report_id,
                        rep.data,
                        (CFIndex)rep.len
                ),
                "write_report"
        );
}

// Basic synchronous get-report helper.
// Note: Many devices deliver replies as INPUT reports via callbacks, not GetReport.
// This is still useful for FEATURE reads and gives us a primitive to build on.
[[nodiscard]] inline bool read_report( apple::hid_device * device, report & rep ) noexcept
{
        CFIndex n = (CFIndex)rep.capacity();

        auto ok = apple::_try(
                IOHIDDeviceGetReport(
                        device,
                        rep.report_type,
                        rep.report_id,
                        rep.data,
                        &n
                ),
                "read_report"
        );

        if( !ok ) return false;

        if( n < 0 ) n = 0;
        rep.len = (std::size_t)n;
        return true;
}

} // namespace fffb

