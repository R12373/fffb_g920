//
//
//      fffb
//      joy/protocol.hxx
//
#pragma message("USING PROTOCOL.HXX FROM: " __FILE__)
#pragma once


#include <fffb/util/types.hxx>
#include <fffb/hid/report.hxx>
#include <fffb/hid/device.hxx>

#define FFFB_FORCE_MAX_PARAMS 7

#define FFFB_FORCE_SLOT_CONSTANT   0b0001
#define FFFB_FORCE_SLOT_SPRING     0b0011
#define FFFB_FORCE_SLOT_DAMPER     0b0100
#define FFFB_FORCE_SLOT_TRAPEZOID  0b1000
#define FFFB_FORCE_SLOT_AUTOCENTER 0b1111

// HID++ 2.0 Force Feedback feature (0x8123)
// Command IDs match the public Linux hid-logitech-hidpp implementation.




namespace fffb
{

struct hidpp_ctx_t
{
        uti::u8_t dev_index { 0xFF };
        uti::u8_t sw_id     { 0x0E };

        // NEW:
        uti::u8_t report_id { 0x12 };
        std::size_t report_len { 64 };
        bool include_id_in_payload { false };

        uti::u8_t ff_feat_index { 0 };
        bool ff_ready { false };

        // Device-allocated HID++ effect slots (1..N). 0 means “unknown / not allocated yet”.
        uti::u8_t ff_slot_by_force_mask[16] = {0};  // index by (force.params.slot & 0x0F)
        uti::u8_t num_effects_total = 0;            // optional, from GET_INFO
};

inline hidpp_ctx_t & hidpp_ctx() noexcept
{
    static hidpp_ctx_t ctx;
    return ctx;
}


////////////////////////////////////////////////////////////////////////////////

constexpr uti::u32_t Logitech_VendorID         { 0x0000046d } ;
constexpr uti::u32_t Logitech_G923_PS_DeviceID { 0xc266046d } ;
constexpr uti::u32_t Logitech_G29_PS4_DeviceID { 0xc24f046d } ;

constexpr uti::array< uti::u32_t, 2 > known_wheel_device_ids
{
        Logitech_G923_PS_DeviceID,
        Logitech_G29_PS4_DeviceID,
} ;

////////////////////////////////////////////////////////////////////////////////

enum class force_type
{
        CONSTANT  ,
        SPRING    ,
        DAMPER    ,
        TRAPEZOID ,
        COUNT     ,
} ;

struct force_params
{
        uti::u8_t   slot ;
        bool     enabled ;
        uti::u8_t params [ FFFB_FORCE_MAX_PARAMS ] ;
} ;

struct constant_force_params
{
        uti::u8_t      slot { FFFB_FORCE_SLOT_CONSTANT } ;
        bool        enabled { false } ;
        uti::u8_t amplitude ;
        uti::u8_t   padding [ FFFB_FORCE_MAX_PARAMS - 1 ] ;
} ;

struct spring_force_params
{
        uti::u8_t         slot { FFFB_FORCE_SLOT_SPRING } ;
        bool           enabled { false } ;
        uti::u8_t   dead_start ;
        uti::u8_t   dead_end   ;
        uti::u8_t  slope_left  ;
        uti::u8_t  slope_right ;
        uti::u8_t invert_left  ;
        uti::u8_t invert_right ;
        uti::u8_t    amplitude ;
} ;

struct damper_force_params
{
        uti::u8_t         slot { FFFB_FORCE_SLOT_DAMPER } ;
        bool           enabled { false } ;
        uti::u8_t   slope_left ;
        uti::u8_t  slope_right ;
        uti::u8_t  invert_left ;
        uti::u8_t invert_right ;
        uti::u8_t      padding [ FFFB_FORCE_MAX_PARAMS - 4 ] ;
} ;

struct trapezoid_force_params
{

        uti::u8_t          slot { FFFB_FORCE_SLOT_TRAPEZOID } ;
        bool            enabled ;
        uti::u8_t amplitude_max ;
        uti::u8_t amplitude_min ;
        uti::u8_t      t_at_max ;
        uti::u8_t      t_at_min ;
        uti::u8_t  slope_step_x ;
        uti::u8_t  slope_step_y ;
        uti::u8_t       padding [ FFFB_FORCE_MAX_PARAMS - 6 ] ;
} ;

struct force
{
        force_type type ;
        union
        {
                          force_params    params ;
                 constant_force_params  constant ;
                   spring_force_params    spring ;
                   damper_force_params    damper ;
                trapezoid_force_params trapezoid ;
        } ;
} ;

////////////////////////////////////////////////////////////////////////////////

enum class command_type
{
        AUTO_ON       ,
        AUTO_OFF      ,
        AUTO_SET      ,
        LED_SET       ,
        DL_FORCE      ,
        PLAY_FORCE    ,
        REFRESH_FORCE ,
        STOP_FORCE    ,
        COUNT         ,
} ;

enum class ffb_protocol
{
        logitech_classic ,
        logitech_hidpp   ,
        count
} ;

////////////////////////////////////////////////////////////////////////////////

class protocol
{
public:
        static constexpr uti::u16_t HIDPP_FF_BASELINE_AUTOCENTER = 0x0C00; //0x0800;
        static constexpr uti::u8_t HIDPP_FF_GET_INFO          = 0x01;
        static constexpr uti::u8_t HIDPP_FF_RESET_ALL         = 0x11;
        static constexpr uti::u8_t HIDPP_FF_DOWNLOAD_EFFECT   = 0x21;
        static constexpr uti::u8_t HIDPP_FF_SET_EFFECT_STATE  = 0x31;
        static constexpr uti::u8_t HIDPP_FF_DESTROY_EFFECT    = 0x41;
        static constexpr uti::u8_t HIDPP_FF_GET_APERTURE      = 0x51;
        static constexpr uti::u8_t HIDPP_FF_SET_APERTURE      = 0x61;
        static constexpr uti::u8_t HIDPP_FF_GET_GLOBAL_GAINS  = 0x71;
        static constexpr uti::u8_t HIDPP_FF_SET_GLOBAL_GAINS  = 0x81;

        static constexpr uti::u8_t HIDPP_FF_EFFECT_SPRING     = 0x06;
        static constexpr uti::u8_t HIDPP_FF_EFFECT_AUTOSTART  = 0x80;
        static constexpr uti::u8_t HIDPP_FF_EFFECT_CONSTANT   = 0x00;
        static constexpr uti::u8_t HIDPP_FF_EFFECT_DAMPER     = 0x07;
        static constexpr uti::u8_t HIDPP_FF_EFFECT_RAMP       = 0x0A;

        static constexpr uti::u8_t HIDPP_FF_EFFECT_STATE_STOP = 0x01;
        static constexpr uti::u8_t HIDPP_FF_EFFECT_STATE_PLAY = 0x02;

        static report hidpp_ff_set_autocenter(uti::u16_t magnitude) noexcept;
        static constexpr report build_report ( ffb_protocol const protocol, command_type const cmd_type, uti::u8_t slots ) noexcept ;
        static constexpr report build_report ( ffb_protocol const protocol, command_type const cmd_type, force const & f ) noexcept ;

        static constexpr report set_led_pattern ( ffb_protocol const protocol, uti::u8_t pattern ) noexcept ;

        static report disable_autocenter ( ffb_protocol const protocol, uti::u8_t slots ) noexcept ;
        // static constexpr report  enable_autocenter ( ffb_protocol const protocol, uti::u8_t slots ) noexcept ;
        static report enable_autocenter ( ffb_protocol const protocol, uti::u8_t slots ) noexcept ;

        static constexpr report set_autocenter ( ffb_protocol const protocol, force const & force ) noexcept ;
        static report download_force ( ffb_protocol const protocol, force const & force ) noexcept ;

        static report play_force ( ffb_protocol const protocol, uti::u8_t slots ) noexcept ;
        static constexpr report refresh_force ( ffb_protocol const protocol, force const & f ) noexcept ;
        static report    stop_force ( ffb_protocol const protocol, uti::u8_t slots ) noexcept ;

        static constexpr vector< report > init_sequence ( ffb_protocol const protocol, uti::u32_t device_id ) noexcept ;

        static bool hidpp_download_force_sync(hid_device& dev, force const& f) noexcept;
        static bool hidpp_set_effect_state_sync(hid_device& dev,
                                        uti::u8_t effect_slot,
                                        uti::u8_t state) noexcept;

        static bool hidpp_destroy_effect_sync(hid_device& dev,
                                        uti::u8_t effect_slot) noexcept;
        static bool hidpp_ping( hid_device & dev,
                        uti::u8_t & out_major,
                        uti::u8_t & out_minor,
                        uti::u8_t & out_device_index,
                        int timeout_ms = 250 ) noexcept;
        static bool hidpp_init( hid_device & dev, uti::u8_t dev_index ) noexcept;
        static bool hidpp_root_get_feature(
        hid_device & dev,
        uti::u8_t dev_index,
        uti::u16_t feature_id,
        uti::u8_t & out_feature_index,
        uti::u8_t & out_feature_type
        ) noexcept;
        static report hidpp_ff_reset_all() noexcept;




private:
        static constexpr report _make_classic_report() noexcept
        {
                report rep{};
                rep.report_type = kIOHIDReportTypeOutput;
                rep.report_id   = 0;
                rep.len         = 8;
                return rep;
        }

        static constexpr report _classic_1b( uti::u8_t b0 ) noexcept
        {
                auto rep = _make_classic_report();
                rep.data[0] = b0;
                return rep;
        }
        static constexpr report _classic_2b( uti::u8_t b0, uti::u8_t b1 ) noexcept
        {
                auto rep = _make_classic_report();
                rep.data[0] = b0;
                rep.data[1] = b1;
                return rep;
        }
        static constexpr report _classic_3b( uti::u8_t b0, uti::u8_t b1, uti::u8_t b2 ) noexcept
        {
                auto rep = _make_classic_report();
                rep.data[0] = b0;
                rep.data[1] = b1;
                rep.data[2] = b2;
                return rep;
        }
        static constexpr report _classic_4b( uti::u8_t b0, uti::u8_t b1, uti::u8_t b2, uti::u8_t b3) noexcept
        {
                auto rep = _make_classic_report();
                rep.data[0] = b0;
                rep.data[1] = b1;
                rep.data[2] = b2;
                rep.data[3] = b3;
                return rep;
        }
        static constexpr report _classic_5b( uti::u8_t b0, uti::u8_t b1, uti::u8_t b2, uti::u8_t b3, uti::u8_t b4) noexcept
        {
                auto rep = _make_classic_report();
                rep.data[0] = b0;
                rep.data[1] = b1;
                rep.data[2] = b2;
                rep.data[3] = b3;
                rep.data[4] = b4;
                return rep;
        }
        static constexpr report _classic_6b( uti::u8_t b0, uti::u8_t b1, uti::u8_t b2, uti::u8_t b3, uti::u8_t b4, uti::u8_t b5) noexcept
        {
                auto rep = _make_classic_report();
                rep.data[0] = b0;
                rep.data[1] = b1;
                rep.data[2] = b2;
                rep.data[3] = b3;
                rep.data[4] = b4;
                rep.data[5] = b5;
                return rep;
        }

        static constexpr report  _constant_force ( ffb_protocol const protocol, force const & force ) noexcept ;
        static constexpr report    _spring_force ( ffb_protocol const protocol, force const & force ) noexcept ;
        static constexpr report    _damper_force ( ffb_protocol const protocol, force const & force ) noexcept ;
        static constexpr report _trapezoid_force ( ffb_protocol const protocol, force const & force ) noexcept ;
} ;



inline bool protocol::hidpp_ping( hid_device & dev,
                                  uti::u8_t & out_major,
                                  uti::u8_t & out_minor,
                                  uti::u8_t & out_device_index,
                                  int timeout_ms ) noexcept
{
        // HID++2.0 draft "GetProtocolVersion/Ping":
        // Request:  0x10 DevIndex 0x00 0x1n 0x00 0x00 0xUU
        // Response: 0x10 DevIndex 0x00 0x1n 0xXX 0xYY 0xUU
        // where n is SwID (1..15) and UU is arbitrary ping byte. :contentReference[oaicite:3]{index=3}

        constexpr uti::u8_t kReportId = 0x10;     // short message
        constexpr uti::u8_t kFeature  = 0x00;     // special "protocol version" probe
        constexpr uti::u8_t kSwId     = 0x0E;     // 1..15 (choose any non-zero)
        constexpr uti::u8_t kFnSw     = (uti::u8_t)(0x10 | kSwId);  // 0x1n
        constexpr uti::u8_t kPingByte = 0xAA;     // any value you like

        // Device index is ambiguous across Logitech implementations.
        // Try a small set that commonly works (direct device vs receiver routing).
        constexpr uti::u8_t candidates[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0xFF, 0x00 };

        if( !dev.open() )
                return false;

        // Make sure input callbacks are active (your device.hxx should provide this).
        
        dev.enable_input_reports();

        auto dump = [&](uti::u8_t const* msg, std::size_t n) {
        // print first up to 20 bytes
        char buf[256];
        std::size_t m = (n > 20) ? 20 : n;
        std::size_t off = 0;
        for (std::size_t i=0; i<m && off+4 < sizeof(buf); i++) {
                off += (std::size_t)snprintf(buf+off, sizeof(buf)-off, "%02X ", (unsigned)msg[i]);
        }
        FFFB_F_INFO_S("hidpp_ping","rx (%zu bytes): %s", (size_t)n, buf);
        };
 
        auto try_ping = [&](uti::u8_t dev_index, bool include_id_in_payload,
                        uti::u8_t & maj, uti::u8_t & min, uti::u8_t & replied_index) -> bool
        {
                constexpr uti::u8_t kFeature  = 0x00;
                constexpr uti::u8_t kSwId     = 0x0E;                    // 1..15
                constexpr uti::u8_t kFnSw     = (uti::u8_t)(0x10 | kSwId);// 0x1n
                constexpr uti::u8_t kPingByte = 0xAA;

                auto dump = [&](uti::u8_t const * msg, std::size_t n) noexcept
                {
                        char buf[256];
                        std::size_t m = (n > 20) ? 20 : n;
                        std::size_t off = 0;

                        for( std::size_t i = 0; i < m && off + 4 < sizeof(buf); ++i )
                                off += (std::size_t)std::snprintf(buf + off, sizeof(buf) - off, "%02X ", (unsigned)msg[i]);

                        FFFB_F_INFO_S("hidpp_ping","rx (%zu): %s", (size_t)n, buf);
                };

                auto normalize = [&](report const & in, uti::u8_t (&msg)[FFFB_REPORT_MAX_LEN], std::size_t & msg_len) noexcept
                {
                        const bool id_in_payload =
                                (in.len >= 1) &&
                                (in.data[0] == in.report_id) &&
                                (in.report_id == 0x10 || in.report_id == 0x11 || in.report_id == 0x12);

                        if( id_in_payload )
                        {
                                msg_len = in.len;
                                std::memcpy(msg, in.data, msg_len);
                        }
                        else
                        {
                                msg[0] = in.report_id;
                                msg_len = in.len + 1;
                                if( in.len > 0 )
                                        std::memcpy(msg + 1, in.data, (std::size_t)in.len);
                        }
                };

                // We will try both report IDs for the request: short (0x10) and long (0x11)
                // Fix B: some devices only reply properly when you use the long format.
                const uti::u8_t report_ids_to_try[] = { 0x10, 0x11 };

                for( uti::u8_t req_report_id : report_ids_to_try )
                {
                        report req{};
                        req.report_type = kIOHIDReportTypeOutput;
                        req.report_id   = req_report_id;

                        if( req_report_id == 0x10 )
                        {
                                // SHORT request: 7 bytes if ID included, otherwise 6 bytes
                                if( include_id_in_payload )
                                {
                                        req.len     = 7;
                                        req.data[0] = 0x10;
                                        req.data[1] = dev_index;
                                        req.data[2] = kFeature;
                                        req.data[3] = kFnSw;
                                        req.data[4] = 0x00;
                                        req.data[5] = 0x00;
                                        req.data[6] = kPingByte;
                                }
                                else
                                {
                                        req.len     = 6;
                                        req.data[0] = dev_index;
                                        req.data[1] = kFeature;
                                        req.data[2] = kFnSw;
                                        req.data[3] = 0x00;
                                        req.data[4] = 0x00;
                                        req.data[5] = kPingByte;
                                }
                        }
                        else // req_report_id == 0x11 (LONG)
                        {
                                // LONG request: 20 bytes if ID included, otherwise 19 bytes
                                if( include_id_in_payload )
                                {
                                        req.len     = 20;
                                        req.data[0] = 0x11;
                                        req.data[1] = dev_index;
                                        req.data[2] = kFeature;
                                        req.data[3] = kFnSw;
                                        req.data[4] = 0x00;
                                        req.data[5] = 0x00;
                                        req.data[6] = kPingByte;
                                        // rest padded with zeros automatically
                                }
                                else
                                {
                                        req.len     = 19;
                                        req.data[0] = dev_index;
                                        req.data[1] = kFeature;
                                        req.data[2] = kFnSw;
                                        req.data[3] = 0x00;
                                        req.data[4] = 0x00;
                                        req.data[5] = kPingByte;
                                        // rest padded with zeros automatically
                                }
                        }

                        if( !dev.write(req) )
                                continue;

                        const int slice_ms = 10;
                        int waited = 0;

                        while( waited < timeout_ms )
                        {
                                report in{};
                                if( !dev.read_input(in, slice_ms) )
                                {
                                        waited += slice_ms;
                                        continue;
                                }

                                uti::u8_t msg[FFFB_REPORT_MAX_LEN]{};
                                std::size_t msg_len = 0;
                                normalize(in, msg, msg_len);

                                // Dump EVERYTHING we see while debugging (you can remove later)
                                if( msg_len > 0 )
                                        dump(msg, msg_len);

                                if( msg_len < 7 )
                                        continue;

                                // Accept reply report id 0x10 or 0x11
                                // if( msg[0] != 0x10 && msg[0] != 0x11 )
                                //         continue;
                                if( msg[0] != 0x10 && msg[0] != 0x11 && msg[0] != 0x12 ) continue;
                                // Expected: [rid] [dev_index] [0x00] [0x1n] [maj] [min] [ping]
                                // Fix A: allow replied dev index to be 0xFF or 0x00 even if we addressed something else
                                if( msg[1] != dev_index && msg[1] != 0xFF && msg[1] != 0x00 )
                                        continue;

                                if( msg[2] != 0x00 )
                                        continue;

                                if( msg[3] != kFnSw )
                                        continue;

                                if( msg[6] != kPingByte )
                                        continue;


                                auto & ctx = hidpp_ctx();
                                ctx.report_id = in.report_id;     // e.g. 0x12
                                ctx.report_len = (std::size_t)in.len;   // e.g. 64

                                // if the payload includes the report id as the first byte
                                ctx.include_id_in_payload = (in.len >= 1 && in.data[0] == in.report_id);
                                maj = msg[4];
                                min = msg[5];
                                replied_index = msg[1];  // capture actual replied index
                                return true;
                        }
                }

                return false;
        };

        for( uti::u8_t dev_index : candidates )
        {

                uti::u8_t maj=0, min=0, ridx=0;

                // Try variant 1 (ID in payload)
                if( try_ping(dev_index, true, maj, min, ridx) ) 
                {
                        out_major = maj;
                        out_minor = min;
                        out_device_index = ridx;
                        return true;
                }

                // Try variant 2 (ID NOT in payload)
                if( try_ping(dev_index, false, maj, min, ridx) ) 
                {
                        out_major = maj;
                        out_minor = min;
                        out_device_index = ridx;
                        return true;
                }
        }
        return false;
}

inline bool protocol::hidpp_root_get_feature(
    hid_device & dev,
    uti::u8_t dev_index,
    uti::u16_t feature_id,
    uti::u8_t & out_feature_index,
    uti::u8_t & out_feature_type
) noexcept
{
    constexpr int timeout_ms = 250;   // keep small; you retry anyway
//     constexpr uti::u8_t kReportId = 0x10;

    const uti::u8_t kReportId = hidpp_ctx().report_id ? hidpp_ctx().report_id : 0x12;
    constexpr uti::u8_t kRootFeatureIndex = 0x00; // root page
    const uti::u8_t swid = hidpp_ctx().sw_id;
    const uti::u8_t fn = (uti::u8_t)((0x00u << 4) | (swid & 0x0F)); // == swid, usually 0x0E
        //     const uti::u8_t fn   = (uti::u8_t)(0x10 | (swid & 0x0F)); // your working pattern
//     constexpr uti::u8_t kTag  = 0xAA;

    auto try_once = [&](bool include_id_in_payload) -> bool
    {
        report req{};
        req.report_type = kIOHIDReportTypeOutput;
        req.report_id   = kReportId;

        const uti::u8_t hi = (uti::u8_t)(feature_id >> 8);
        const uti::u8_t lo = (uti::u8_t)(feature_id & 0xFF);

        if (include_id_in_payload)
        {
                req.len     = 7;
                req.data[0] = kReportId;
                req.data[1] = dev_index;
                req.data[2] = kRootFeatureIndex; // 0x00
                req.data[3] = fn;
                req.data[4] = hi;
                req.data[5] = lo;
                req.data[6] = 0x00;              // NOT 0xAA
        }
        else
        {
                req.len     = 6;
                req.data[0] = dev_index;
                req.data[1] = kRootFeatureIndex; // 0x00
                req.data[2] = fn;
                req.data[3] = hi;
                req.data[4] = lo;
                req.data[5] = 0x00;              // NOT 0xAA
        }

        if (!dev.write(req))
            return false;

        const int slice_ms = 10;
        int waited = 0;

        while (waited < timeout_ms)
        {
            report in{};
            if (!dev.read_input(in, slice_ms))
            {
                waited += slice_ms;
                continue;
            }

            // normalize so msg[0] is report_id
            uti::u8_t msg[FFFB_REPORT_MAX_LEN]{};
            std::size_t msg_len = 0;

            const bool id_in_payload =
                (in.len >= 1) &&
                (in.data[0] == in.report_id) &&
                (in.report_id == 0x10 || in.report_id == 0x11 || in.report_id == 0x12);

            if (id_in_payload)
            {
                msg_len = in.len;
                std::memcpy(msg, in.data, msg_len);
            }
            else
            {
                msg[0] = in.report_id;
                msg_len = (std::size_t)in.len + 1;
                if (in.len > 0)
                    std::memcpy(msg + 1, in.data, (std::size_t)in.len);
            }

            // Expected: [rid] [dev] [feature=0] [fn] [feat_index] [feat_type] [tag]
            if (msg_len < 7) continue;
            if (msg[1] != dev_index) continue;
            if (msg[2] != kRootFeatureIndex) continue;
            if (msg[3] != fn) continue;
            if( msg[0] != 0x10 && msg[0] != 0x11 && msg[0] != 0x12 ) continue;

        //     if (msg[6] != kTag) continue;

            out_feature_index = msg[4];
            if( out_feature_index == 0x00 || out_feature_index == 0xFF ) continue; // treat as not found / invalid
            out_feature_type  = msg[5];
            return true;
        }

        return false;
    };

    // try both macOS buffer formats
    if (try_once(true))  return true;
    if (try_once(false)) return true;
    return false;
}

inline bool protocol::hidpp_init(hid_device & dev, uti::u8_t dev_index) noexcept
{
    auto & ctx = hidpp_ctx();
    ctx.dev_index = dev_index;

    uti::u8_t ff_index = 0, ff_type = 0;

    // 0x8123 = “Force Feedback” feature in Logitech HID++ (used by G920/G29 class)
    if (!protocol::hidpp_root_get_feature(dev, dev_index, 0x8123, ff_index, ff_type))
        return false;

    ctx.ff_feat_index = ff_index;
    ctx.ff_ready = true;
    return true;
}


static inline std::size_t _hidpp_report_len_for_id(uti::u8_t report_id) noexcept
{
        switch( report_id )
        {
                case 0x10: return 7;   // short
                case 0x11: return 20;  // long
                case 0x12: return 64;  // very long
                default:   return 64;
        }
}

static inline report _hidpp_ff_cmd(uti::u8_t command,
                                  uti::u8_t const * params,
                                  std::size_t params_len) noexcept
{
        auto const & ctx = hidpp_ctx();

        report rep{};
        rep.report_type = kIOHIDReportTypeOutput;

        const bool id_in_payload = ctx.include_id_in_payload;

        auto can_fit = [&](uti::u8_t rid, std::size_t full_len) noexcept -> bool
        {
                std::size_t need = (id_in_payload ? 1 : 0) + 3 + params_len;
                return need <= full_len && full_len <= rep.capacity();
        };

        // Prefer the format learned during ping, but upgrade to 0x12 if needed.
        uti::u8_t rid = ctx.report_id ? ctx.report_id : (uti::u8_t)0x12;

        std::size_t full_len = (rid == ctx.report_id && ctx.report_len)
                ? ctx.report_len
                : _hidpp_report_len_for_id(rid);

        if( !can_fit(rid, full_len) )
        {
                rid = 0x12;
                full_len = _hidpp_report_len_for_id(rid);
        }

        rep.report_id = rid;
        rep.len       = full_len;  // IMPORTANT: send full padded report

        std::size_t off = 0;
        if( id_in_payload )
                rep.data[off++] = rid;

        rep.data[off++] = ctx.dev_index;
        rep.data[off++] = ctx.ff_feat_index;
        rep.data[off++] = command;

        for( std::size_t i = 0; i < params_len && off < rep.len; ++i )
                rep.data[off++] = params[i];

        return rep;
}

static inline bool _hidpp_ff_cmd_sync(
    hid_device & dev,
    uti::u8_t command,
    uti::u8_t const * params,
    std::size_t params_len,
    report & out_resp,
    int timeout_ms = 50
) noexcept
{
    auto const & ctx = hidpp_ctx();
    report out = _hidpp_ff_cmd(command, params, params_len);

    if (!dev.write(out))
        return false;

    // Filter until we see a reply that looks like our FF feature.
    for (;;)
    {
        report r{};
        if (!dev.read_input(r, timeout_ms))
            return false;

        std::size_t off = ctx.include_id_in_payload ? 1 : 0;
        if (r.len < off + 3) continue;

        uti::u8_t dev_index = r.data[off + 0];
        uti::u8_t feat      = r.data[off + 1];
        uti::u8_t cmd       = r.data[off + 2];

        // Accept if it’s the FF feature reply and command matches (allow high-bit variations).
        if (dev_index == ctx.dev_index && feat == ctx.ff_feat_index && ((cmd & 0x7F) == command))
        {
            out_resp = r;
            return true;
        }
    }
}

inline bool protocol::hidpp_download_force_sync(hid_device & dev, force const & f) noexcept
{
    auto & ctx = hidpp_ctx();
    if (!ctx.ff_ready) return false;

    report out = protocol::download_force(ffb_protocol::logitech_hidpp, f);
    if (out.len == 0) return false;

    report resp{};
    // Extract command + params pointer from out so we can resend via cmd_sync:
    // Easiest: rebuild params here per force type for now.
    // (Since we’re only doing constant first, handle just constant.)
    if (f.type != force_type::CONSTANT) return false;

    // Build params exactly like the HID++ case above, but as raw params[] so cmd_sync can get reply:
    uti::u8_t amplitude = f.constant.amplitude;
    uti::u8_t & slot_ref = ctx.ff_slot_by_force_mask[f.params.slot & 0x0F];

    int delta = (int)amplitude - 128;
    int level = (delta >= 0) ? (delta * 0x7fff) / 127 : (delta * 0x8000) / 128;
    if (level >  0x7fff) level =  0x7fff;
    if (level < -0x8000) level = -0x8000;

    uti::u8_t params[14] = {0};
    params[0] = slot_ref; // 0 => allocate
    params[1] = (uti::u8_t)(protocol::HIDPP_FF_EFFECT_CONSTANT | protocol::HIDPP_FF_EFFECT_AUTOSTART);
    params[6] = (uti::u8_t)((level >> 8) & 0xFF);
    params[7] = (uti::u8_t)(level & 0xFF);

    if (!_hidpp_ff_cmd_sync(dev, protocol::HIDPP_FF_DOWNLOAD_EFFECT, params, sizeof(params), resp, 50))
        return false;

    // Response: slot is params[0]
    std::size_t off = ctx.include_id_in_payload ? 1 : 0;
    if (resp.len < off + 4) return false;

    uti::u8_t returned_slot = resp.data[off + 3 + 0];
    if (returned_slot != 0)
        slot_ref = returned_slot; // store it forever

    return true;
}

inline bool protocol::hidpp_set_effect_state_sync(hid_device & dev, uti::u8_t slot, uti::u8_t state) noexcept
{
    if (slot == 0) return true; // nothing to do
    report resp{};
    uti::u8_t params[2] = { slot, state };
    return _hidpp_ff_cmd_sync(dev, protocol::HIDPP_FF_SET_EFFECT_STATE, params, sizeof(params), resp, 50);
}

inline bool protocol::hidpp_destroy_effect_sync(hid_device & dev, uti::u8_t slot) noexcept
{
    if (slot == 0) return true;
    report resp{};
    uti::u8_t params[1] = { slot };
    return _hidpp_ff_cmd_sync(dev, protocol::HIDPP_FF_DESTROY_EFFECT, params, sizeof(params), resp, 50);
}


inline report hidpp_build_output(
    uti::u8_t dev_index,
    uti::u8_t feature_index,
    uti::u8_t fn,                    // FULL cmd byte: (group<<4) | sw_id
    uti::u8_t const* params,
    std::size_t params_len
) noexcept
{
    auto const & ctx = hidpp_ctx();

    report r{};
    r.report_type = kIOHIDReportTypeOutput;
    r.report_id   = ctx.report_id;

    std::size_t off = 0;

    if (ctx.include_id_in_payload)
        r.data[off++] = r.report_id;

    r.data[off++] = dev_index;
    r.data[off++] = feature_index;
    r.data[off++] = fn;

    for (std::size_t i = 0; i < params_len; ++i)
        r.data[off++] = params[i];

    r.len = off;
    return r;
}


constexpr ffb_protocol get_supported_protocol ( hid_device const & device ) noexcept ;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

constexpr report protocol::build_report ( ffb_protocol const protocol, command_type const cmd_type, uti::u8_t slots ) noexcept
{
        switch( cmd_type )
        {
                case command_type::AUTO_OFF      : return disable_autocenter( protocol, slots ) ;
                case command_type::AUTO_ON       : return  enable_autocenter( protocol, slots ) ;
                case command_type::PLAY_FORCE    : return         play_force( protocol, slots ) ;
                case command_type::STOP_FORCE    : return         stop_force( protocol, slots ) ;
                case command_type::AUTO_SET      : [[ fallthrough ]] ;
                case command_type::DL_FORCE      : [[ fallthrough ]] ;
                case command_type::REFRESH_FORCE :
                        FFFB_F_ERR_S( "protocol::build_report", "selected command requires parameters" ) ;
                        return {} ;
                default :
                        FFFB_F_ERR_S( "protocol::build_report", "unknown command" ) ;
                        return {} ;
        }
}

constexpr report protocol::build_report ( ffb_protocol const protocol, command_type const cmd_type, force const & f ) noexcept
{
        switch( cmd_type )
        {
                case command_type::AUTO_SET      : return     set_autocenter( protocol, f ) ;
                case command_type::DL_FORCE      : return     download_force( protocol, f ) ;
                case command_type::REFRESH_FORCE : return      refresh_force( protocol, f ) ;
                case command_type::AUTO_ON       : [[ fallthrough ]] ;
                case command_type::AUTO_OFF      : [[ fallthrough ]] ;
                case command_type::PLAY_FORCE    : [[ fallthrough ]] ;
                case command_type::STOP_FORCE    :
                        FFFB_F_ERR_S( "protocol::build_report", "excess parameters for selected command" ) ;
                        return {} ;
                default :
                        FFFB_F_ERR_S( "protocol::build_report", "unknown command" ) ;
                        return {} ;
        }
}

constexpr report protocol::set_led_pattern ( ffb_protocol const protocol, uti::u8_t pattern ) noexcept
{
        pattern = pattern & 0b00011111 ;

        switch( protocol )
        {
                case ffb_protocol::logitech_classic : return _classic_4b( 0xF8, 0x12, pattern, 0x00) ;
                case ffb_protocol::logitech_hidpp   : return {} ;
                        FFFB_F_ERR_S( "protocol::set_led_pattern", "protocol not implemented" ) ;
                        return {} ;
                default :
                        FFFB_F_ERR_S( "protocol::set_led_pattern", "protocol not supported" ) ;
                        return {} ;
        }
}


inline report protocol::disable_autocenter(ffb_protocol const protocol, uti::u8_t slots) noexcept
{
    uti::u8_t command = (slots << 4) | 0x05;


    switch (protocol)
    {
        case ffb_protocol::logitech_classic:
            return _classic_1b(command);

        case ffb_protocol::logitech_hidpp:
        (void)slots;
        return hidpp_ff_set_autocenter(0);

        default:
            return {};
    }
}


inline report protocol::enable_autocenter ( ffb_protocol const protocol, uti::u8_t slots ) noexcept
{
        uti::u8_t command = ( slots << 4 ) | 0x04 ;

        switch( protocol )
        {
                case ffb_protocol::logitech_classic : return _classic_1b( command ); ;
                case ffb_protocol::logitech_hidpp: (void)slots; return hidpp_ff_set_autocenter(protocol::HIDPP_FF_BASELINE_AUTOCENTER);
                        FFFB_F_ERR_S( "protocol::enable_autocenter", "protocol not implemented" ) ;
                        return {} ;
                default :
                        FFFB_F_ERR_S( "protocol::enable_autocenter", "protocol not supported" ) ;
                        return {} ;
        }
}

constexpr report protocol::set_autocenter ( ffb_protocol const protocol, force const & f ) noexcept
{
        uti::u8_t command = ( f.params.slot << 4 ) | 0x0e ;

        uti::u8_t   slope_l = f.spring.slope_left  | 0b0111 ;
        uti::u8_t   slope_r = f.spring.slope_right | 0b0111 ;
        uti::u8_t amplitude = f.spring.amplitude            ;

        switch( protocol )
        {
                case ffb_protocol::logitech_classic : return _classic_6b(command, 0x00, slope_l, slope_r, amplitude, 0x00) ;
                case ffb_protocol::logitech_hidpp   :
                        FFFB_F_ERR_S( "protocol::set_autocenter", "protocol not implemented" ) ;
                        return {} ;
                default :
                        FFFB_F_ERR_S( "protocol::set_autocenter", "protocol not supported" ) ;
                        return {} ;
        }
}

inline report protocol::download_force ( ffb_protocol const protocol, force const & f ) noexcept
{
        switch( f.type )
        {
                case force_type:: CONSTANT : return  _constant_force( protocol, f ) ;
                case force_type::   SPRING : return    _spring_force( protocol, f ) ;
                case force_type::   DAMPER : return    _damper_force( protocol, f ) ;
                case force_type::TRAPEZOID : return _trapezoid_force( protocol, f ) ;
                default:
                        FFFB_F_ERR_S( "protocol::download_force", "force type not supported" ) ;
                        return {} ;
        }
}



inline report protocol::play_force(ffb_protocol const protocol, uti::u8_t slots) noexcept
{
        FFFB_F_ERR_S("protocol::play_force", "NEW_PLAY_FORCE_IS_RUNNING");
        uti::u8_t command = ( slots << 4 ) | 0x02;

        switch( protocol )
        {
                case ffb_protocol::logitech_classic:
                        return _classic_2b(command, 0x00);

                case ffb_protocol::logitech_hidpp:
                {
                        auto const & ctx = hidpp_ctx();

                        if( !ctx.ff_ready )
                        {
                                FFFB_F_ERR_S("protocol::play_force", "hidpp not initialized (no ff feature index)");
                                return {};
                        }

                        // Minimal safe behavior: a “GET_INFO”-style no-op (function 0x00)
                        // This avoids returning len=0 while we implement real effect start/stop next.
                        uti::u8_t const cmd_get_info = (uti::u8_t)((0x00u << 4) | (ctx.sw_id & 0x0F));

                        uti::u8_t params[1] = { slots }; // include something deterministic
                        return _hidpp_ff_cmd(cmd_get_info, params, 1);
                }

                default:
                        FFFB_F_ERR_S("protocol::play_force", "protocol not supported");
                        return {};
        }
}

constexpr report protocol::refresh_force ( ffb_protocol const protocol, force const & f ) noexcept
{
        report rep = download_force( protocol, f ) ;

        rep.data[ 0 ] &= 0xF0 ;
        rep.data[ 0 ] |= 0x0C ;

        return rep ;
}

// inline report protocol::hidpp_ff_reset_all() noexcept
// {
//     auto const& ctx = hidpp_ctx();
//     if (!ctx.ff_ready)
//         return {};

// //     // HID++ 2.0 0x8123 "Force Feedback" feature: RESET_ALL (0x11)
// //     return _hidpp_ff_cmd(HIDPP_FF_RESET_ALL, nullptr, 0);
//     // 0x8123 ForceFeedback: RESET_ALL (0x11), no params.
//     return _hidpp_ff_cmd(0x11, nullptr, 0);
// }

inline report protocol::hidpp_ff_reset_all() noexcept
{
    auto const& ctx = hidpp_ctx();
    if (!ctx.ff_ready) return {};
    return _hidpp_ff_cmd(protocol::HIDPP_FF_RESET_ALL, nullptr, 0);
}

inline report protocol::hidpp_ff_set_autocenter(uti::u16_t magnitude) noexcept
{
    auto const& ctx = hidpp_ctx();

    if (!ctx.ff_ready)
        return {};

    // 0x8123 ForceFeedback: DOWNLOAD_EFFECT (0x21), 18 params.
    // This mirrors the Linux hid-logitech-hidpp set_autocenter layout.
    uti::u8_t params[18] = {0};

    params[0] = 0x00;                 // slot (0 = allocate/reserved)
    params[1] = (uti::u8_t)(0x06u | 0x80u); // SPRING | AUTOSTART
    // params[2..5] = duration+delay = 0

    // magnitude -> coefficients/saturation (Linux driver mapping)
    params[8]  = params[14] = (uti::u8_t)(magnitude >> 11);
    params[9]  = params[15] = (uti::u8_t)((magnitude >> 3) & 0xFF);
    params[6]  = params[16] = (uti::u8_t)(magnitude >> 9);
    params[7]  = params[17] = (uti::u8_t)((magnitude >> 1) & 0xFF);

    return _hidpp_ff_cmd(0x21, params, sizeof(params));
}


inline report protocol::stop_force(ffb_protocol const protocol, uti::u8_t slots) noexcept
{
    uti::u8_t command = (slots << 4) | 0x03;

    switch (protocol)
    {
        case ffb_protocol::logitech_classic:
            return _classic_2b(command, 0x00);

        case ffb_protocol::logitech_hidpp:
        {
            auto const & ctx = hidpp_ctx();
            if (!ctx.ff_ready)
                return {};

        //     // function nibble = 0x03, low nibble = sw_id
        //     uti::u8_t fn = (uti::u8_t)((0x03u << 4) | (ctx.sw_id & 0x0F));

        //     uti::u8_t params[1] = { slots };
        //     return hidpp_build_output(ctx.dev_index, ctx.ff_feat_index, fn, params, 1);
            (void)slots;
            return hidpp_ff_reset_all();
        }

        default:
            return {};
    }
}

constexpr vector< report > protocol::init_sequence ( ffb_protocol const protocol, uti::u32_t device_id ) noexcept
{
        vector< report > reports ;

        if( protocol == ffb_protocol::logitech_classic )
        {
                switch( device_id )
                {
                        case Logitech_G923_PS_DeviceID:
                                reports.push_back( _classic_5b(0x30, 0xf8, 0x09, 0x05, 0x01) ) ;
                                break ;
                        case Logitech_G29_PS4_DeviceID:
                                reports.push_back( _classic_5b(0x30, 0xf8, 0x09, 0x05, 0x01) ) ;
                                break ;
                        default:
                                FFFB_F_ERR_S( "protocol::init_sequence", "unknown device id" ) ;
                }
        }
        return reports ;
}

constexpr report protocol::_constant_force ( ffb_protocol const protocol, force const & f ) noexcept
{
        uti::u8_t command   = f.params.slot << 4 ;
        uti::u8_t amplitude = f.constant.amplitude ;

        switch( protocol )
        {
                case ffb_protocol::logitech_classic:
                {
                        auto rep = _make_classic_report(); // sets type/id/len and zeros data[]

                        rep.data[0] = command;
                        rep.data[1] = 0x00;
                        rep.data[2] = amplitude;
                        rep.data[3] = amplitude;
                        rep.data[4] = amplitude;
                        rep.data[5] = amplitude;
                        rep.data[6] = 0x00;
                        // rep.data[7]

                        return rep;
                }
                // case ffb_protocol::logitech_hidpp:
                // {
                // auto const & ctx = hidpp_ctx();
                // if (!ctx.ff_ready)
                // {
                //         FFFB_F_ERR_S("protocol::_constant_force", "hidpp not initialized (no ff feature index)");
                //         return {};
                // }

                // // Your amplitude is 0..255, with 128 treated as "neutral" (0 force).
                // // Convert to signed 16-bit like the Linux driver expects in params[6..7]. :contentReference[oaicite:3]{index=3}
                // const int force_s16 = ((int)amplitude - 128) << 8; // -32768..~32512

                // // Linux upload layout: size = 14 for FF_CONSTANT. :contentReference[oaicite:4]{index=4}
                // uti::u8_t params[14] = {0};

                // // Slot handling note:
                // // Linux sets params[0] (slot) in its work handler; for new effects it ends up as 0 (allocate). :contentReference[oaicite:5]{index=5}
                // // Your code uses fixed "slots", so we try using your slot number directly.
                // // If this turns out to be incompatible, change params[0] to 0 to "allocate a slot".
                // // params[0] = f.params.slot;
                // params[0] = 0;
                // params[1] = (uti::u8_t)(HIDPP_FF_EFFECT_CONSTANT | HIDPP_FF_EFFECT_AUTOSTART);

                // // replay.length and replay.delay (ms). You don’t have these in your struct yet, so keep 0 = continuous/no delay.
                // params[2] = params[3] = 0;
                // params[4] = params[5] = 0;

                // // Force, big-endian (matches Linux: params[6]=force>>8, params[7]=force&255). :contentReference[oaicite:6]{index=6}
                // params[6] = (uti::u8_t)((force_s16 >> 8) & 0xFF);
                // params[7] = (uti::u8_t)(force_s16 & 0xFF);

                // // Envelope [8..13] left as 0 (no attack/fade).

                // return _hidpp_ff_cmd(HIDPP_FF_DOWNLOAD_EFFECT, params, sizeof(params));
                // }
                case ffb_protocol::logitech_hidpp:
                {
                auto & ctx = hidpp_ctx();
                if (!ctx.ff_ready) return {};

                uti::u8_t & slot_ref = ctx.ff_slot_by_force_mask[f.params.slot & 0x0F];
                uti::u8_t slot = slot_ref;          // 0 if unknown -> allocate

                // Map amplitude (0..255, 128 neutral) -> s16 level
                int delta = (int)amplitude - 128;
                int level;
                if (delta >= 0) level = (delta * 0x7fff) / 127;
                else            level = (delta * 0x8000) / 128;
                if (level >  0x7fff) level =  0x7fff;
                if (level < -0x8000) level = -0x8000;

                uti::u8_t params[14] = {0};
                params[0] = slot;  // 0 => allocate slot (device returns slot in reply) :contentReference[oaicite:8]{index=8}
                params[1] = (uti::u8_t)(protocol::HIDPP_FF_EFFECT_CONSTANT | protocol::HIDPP_FF_EFFECT_AUTOSTART);

                // length/delay = 0 (continuous)
                params[2]=params[3]=params[4]=params[5]=0;

                params[6] = (uti::u8_t)((level >> 8) & 0xFF);
                params[7] = (uti::u8_t)(level & 0xFF);

                // envelope [8..13] left as 0
                return _hidpp_ff_cmd(protocol::HIDPP_FF_DOWNLOAD_EFFECT, params, sizeof(params));
                }

                default :
                        FFFB_F_ERR_S( "protocol::_constant_force", "protocol not supported" ) ;
                        return {} ;
        }
}

constexpr report protocol::_spring_force ( ffb_protocol const protocol, force const & f ) noexcept
{
        uti::u8_t command = f.params.slot << 4 ;

        uti::u8_t dead_start   = f.spring.dead_start   ;
        uti::u8_t dead_end     = f.spring.dead_end     ;
        uti::u8_t slope_left   = f.spring.slope_left   & 0b0111 ;
        uti::u8_t slope_right  = f.spring.slope_right  & 0b0111 ;
        uti::u8_t invert_left  = f.spring.invert_left  & 0b0001 ;
        uti::u8_t invert_right = f.spring.invert_right & 0b0001 ;
        uti::u8_t amplitude    = f.spring.amplitude    ;

        switch( protocol )
        {
                case ffb_protocol::logitech_classic:
                {
                        auto rep = _make_classic_report(); // sets type/id/len and zeros data[]

                        rep.data[0] = command;
                        rep.data[1] = 0x01;
                        rep.data[2] = dead_start;
                        rep.data[3] = dead_end;
                        rep.data[4] = uti::u8_t( (  slope_right << 4 ) |  slope_left );
                        rep.data[5] = uti::u8_t( ( invert_right << 4 ) | invert_left );
                        rep.data[6] = amplitude ;
                        // rep.data[7]

                        return rep;
                }
                // case ffb_protocol::logitech_classic : return { command, 0x01,
                //                                                dead_start, dead_end,
                //                                                uti::u8_t( (  slope_right << 4 ) |  slope_left ),
                //                                                uti::u8_t( ( invert_right << 4 ) | invert_left ),
                //                                                amplitude } ;
                case ffb_protocol::logitech_hidpp :
                        FFFB_F_ERR_S( "protocol::_spring_force", "protocol not implemented" ) ;
                        return {} ;
                default :
                        FFFB_F_ERR_S( "protocol::_spring_force", "protocol not supported" ) ;
                        return {} ;
        }

}

constexpr report protocol::_damper_force ( ffb_protocol const protocol, force const & f ) noexcept
{
        uti::u8_t command = f.params.slot << 4 ;

        uti::u8_t slope_left   = f.damper.slope_left   & 0b0111 ;
        uti::u8_t slope_right  = f.damper.slope_right  & 0b0111 ;
        uti::u8_t invert_left  = f.damper.invert_left  & 0b0001 ;
        uti::u8_t invert_right = f.damper.invert_right & 0b0001 ;

        switch( protocol )
        {       
                case ffb_protocol::logitech_classic:
                {
                        auto rep = _make_classic_report(); // sets type/id/len and zeros data[]

                        rep.data[0] = command;
                        rep.data[1] = 0x02;
                        rep.data[2] = slope_left;
                        rep.data[3] = invert_left;
                        rep.data[4] = slope_right;
                        rep.data[5] = invert_right;
                        rep.data[6] = 0x00;
                        // rep.data[7] stays 0 (already zeroed)

                        return rep;
                }
                // case ffb_protocol::logitech_classic : return { command, 0x02, slope_left, invert_left, slope_right, invert_right, 0x00 } ;
                case ffb_protocol::logitech_hidpp :
                        FFFB_F_ERR_S( "protocol::_damper_force", "protocol not implemented" ) ;
                        return {} ;
                default :
                        FFFB_F_ERR_S( "protocol::_damper_force", "protocol not supported" ) ;
                        return {} ;
        }

}

constexpr report protocol::_trapezoid_force ( ffb_protocol const protocol, force const & f ) noexcept
{
        uti::u8_t command = f.params.slot << 4 ;

        uti::u8_t max_amp =   f.trapezoid.amplitude_max ;
        uti::u8_t min_amp =   f.trapezoid.amplitude_min ;
        uti::u8_t   t_max =   f.trapezoid.t_at_max ;
        uti::u8_t   t_min =   f.trapezoid.t_at_min ;
        uti::u8_t    dxdy = ( f.trapezoid.slope_step_x << 4 )
                            | f.trapezoid.slope_step_y ;

        switch( protocol )
        {
                case ffb_protocol::logitech_classic:
                {
                        auto rep = _make_classic_report(); // sets type/id/len and zeros data[]

                        rep.data[0] = command;
                        rep.data[1] = 0x06;
                        rep.data[2] = max_amp;
                        rep.data[3] = min_amp;
                        rep.data[4] = t_max;
                        rep.data[5] = t_min;
                        rep.data[6] = dxdy;
                        // rep.data[7]

                        return rep;
                }
                // case ffb_protocol::logitech_classic : return { command, 0x06, max_amp, min_amp, t_max, t_min, dxdy } ;
                case ffb_protocol::logitech_hidpp :
                        FFFB_F_ERR_S( "protocol::_trapezoid_force", "protocol not implemented" ) ;
                        return {} ;
                default :
                        FFFB_F_ERR_S( "protocol::_trapezoid_force", "protocol not supported" ) ;
                        return {} ;
        }
}



constexpr ffb_protocol get_supported_protocol ( hid_device const & device ) noexcept
{
        if( device.vendor_id() != Logitech_VendorID )
                return ffb_protocol::count;

        switch( device.product_id() )
        {
                // G920 (often c261 first, then c262)
                case 0xC261:
                case 0xC262:
                        return ffb_protocol::logitech_hidpp;

                default:
                        return ffb_protocol::logitech_classic;
        }
}
////////////////////////////////////////////////////////////////////////////////


} // namespace fffb
