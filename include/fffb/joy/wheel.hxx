//
//
//      fffb
//      joy/wheel.hxx
//

#pragma once

#include <fffb/hid/device.hxx>
#include <fffb/joy/protocol.hxx>

#define FFFB_WHEEL_USAGE_PAGE 0x01
#define FFFB_WHEEL_USAGE      0x04


namespace fffb
{


////////////////////////////////////////////////////////////////////////////////

class wheel
{
public:
        static constexpr  constant_force_params default_const_f  { FFFB_FORCE_SLOT_CONSTANT , false, 128, {} } ;
        static constexpr    spring_force_params default_spring_f { FFFB_FORCE_SLOT_SPRING   , false, 127, 128, 3, 3, 0, 0, 0 } ;
        static constexpr    damper_force_params default_damper_f { FFFB_FORCE_SLOT_DAMPER   , false,   0,   0, 0, 0, {} } ;
        static constexpr trapezoid_force_params default_trap_f   { FFFB_FORCE_SLOT_TRAPEZOID, false, 127, 128, 0, 0, 0, 0, {} } ;

        constexpr wheel () noexcept ;

        constexpr ~wheel () noexcept { if( device_ ){ stop_forces() ; enable_autocenter() ; } }

        [[ nodiscard ]] constexpr operator bool () const noexcept { return static_cast< bool >( device_ ) ; }

        constexpr bool calibrate () noexcept ;

        constexpr bool disable_autocenter () const noexcept ;
        constexpr bool  enable_autocenter () const noexcept ;

        bool download_forces () noexcept ;
        constexpr bool  refresh_forces ()       noexcept ;

        bool play_forces () noexcept ;
        bool stop_forces () noexcept ;

        constexpr bool set_led_pattern ( uti::u8_t _pattern_ ) const noexcept ;

        constexpr void q_disable_autocenter () noexcept ;
        constexpr void  q_enable_autocenter () noexcept ;
        void q_set_autocenter(uti::u16_t magnitude) noexcept;

        constexpr void q_download_forces () noexcept ;
        constexpr void  q_refresh_forces () noexcept ;

        constexpr void q_play_forces () noexcept ;
        constexpr void q_stop_forces () noexcept ;

        constexpr void q_set_led_pattern ( uti::u8_t _pattern_ ) noexcept ;

        constexpr bool flush_reports () noexcept ;

        [[ nodiscard ]] constexpr hid_device const & device () const noexcept { return device_ ; }

        [[ nodiscard ]] constexpr constant_force_params       & constant_force ()       noexcept { return constant_ ; }
        [[ nodiscard ]] constexpr constant_force_params const & constant_force () const noexcept { return constant_ ; }

        [[ nodiscard ]] constexpr spring_force_params       & spring_force ()       noexcept { return spring_ ; }
        [[ nodiscard ]] constexpr spring_force_params const & spring_force () const noexcept { return spring_ ; }

        [[ nodiscard ]] constexpr damper_force_params       & damper_force ()       noexcept { return damper_ ; }
        [[ nodiscard ]] constexpr damper_force_params const & damper_force () const noexcept { return damper_ ; }

        [[ nodiscard ]] constexpr trapezoid_force_params       & trapezoid_force ()       noexcept { return trapezoid_ ; }
        [[ nodiscard ]] constexpr trapezoid_force_params const & trapezoid_force () const noexcept { return trapezoid_ ; }
private:
        hid_device     device_ ;
        ffb_protocol protocol_ ;

        constant_force_params   constant_ { default_const_f  } ;
        spring_force_params       spring_ { default_spring_f } ;
        damper_force_params       damper_ { default_damper_f } ;
        trapezoid_force_params trapezoid_ { default_trap_f   } ;

        bool playing_ { false } ;

        vector< report > reports_ {} ;

        constexpr bool _write_report (          report   const & report , char const * scope ) const noexcept ;
        constexpr bool _write_reports ( vector< report > const & reports, char const * scope ) const noexcept ;

        bool _init_protocol () noexcept ;
} ;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

constexpr wheel::wheel () noexcept
{
        vector< hid_device > devices = list_hid_devices() ;

        for (auto & device : devices)
        {
                if (device.vendor_id() != Logitech_VendorID)
                        continue;

                if( device.usage_page() != 0x01 ) continue;
                if( device.usage() != 0x04 ) continue;  // <- critical: only joystick interface
                if( device.product_id() != 0xC261 && device.product_id() != 0xC262 ) continue;


                // TEMP: don’t skip by usage for Logitech while we locate HID++ interface
                // (keep logging usage_page/usage)

                uti::u8_t maj=0, min=0, idx=0;

                device.open();
                bool ok = protocol::hidpp_ping(device, maj, min, idx);
                device.close();

                FFFB_F_INFO_S("probe",
                        "dev=%08x vid=%04x pid=%04x usage_page=%x usage=%x hidpp_ping=%d ver=%u.%u idx=%02x",
                        device.device_id(), device.vendor_id(), device.product_id(),
                        device.usage_page(), device.usage(),
                        ok ? 1 : 0, (unsigned)maj, (unsigned)min, (unsigned)idx);

                if (ok)
                {
                        device_ = UTI_MOVE(device);
                        protocol_ = ffb_protocol::logitech_hidpp;
                        break;
                }
                }
                if( device_ )
                {
                        _init_protocol() ;
                }
                else
                {
                        FFFB_F_ERR_S( "wheel::ctor", "no known wheels found!" ) ;
                }
        }

////////////////////////////////////////////////////////////////////////////////

constexpr bool wheel::calibrate () noexcept
{
        if( !device_ )
        {
                return false ;
        }
        if( !disable_autocenter() )
        {
                return false ;
        }
        if( !stop_forces() )
        {
                return false ;
        }

        // turn right
        {
                constant_.  enabled = true ;
                constant_.amplitude =   96 ;

                download_forces() ;
                play_forces() ;
                FFFB_DBG_S( "wheel::calibrate", "turning right..." ) ;
                usleep( 750 * 1000 ) ;
        }
        // turn left
        {
                constant_.amplitude = 160 ;

                refresh_forces() ;
                FFFB_DBG_S( "wheel::calibrate", "turning left..." ) ;
                usleep( 1000 * 1000 ) ;
        }
        // shake it
        {
                stop_forces() ;
                constant_.enabled = false ;

                trapezoid_.      enabled = true ;
                trapezoid_.amplitude_max =   96 ;
                trapezoid_.amplitude_min =  160 ;
                trapezoid_.     t_at_max =   32 ;
                trapezoid_.     t_at_min =   32 ;
                trapezoid_. slope_step_x =    6 ;
                trapezoid_. slope_step_y =    6 ;

                download_forces() ;
                play_forces() ;
                FFFB_DBG_S( "wheel::calibrate", "shaking it..." ) ;
                usleep( 1000 * 1000 ) ;
                stop_forces() ;
                trapezoid_.enabled = false ;
        }
        // return to center
        {
                stop_forces() ;
                enable_autocenter() ;
                FFFB_DBG_S( "wheel::calibrate", "recentering..." ) ;
                usleep( 750 * 1000 ) ;
                disable_autocenter() ;
        }
        if( !stop_forces() )
        {
                return false ;
        }
        return true ;
}

////////////////////////////////////////////////////////////////////////////////

constexpr bool wheel::disable_autocenter () const noexcept
{       auto rep = protocol::disable_autocenter(protocol_, 0x0F);
        if (rep.len == 0) return true;   // treat “not implemented” as no-op
        return _write_report( rep, "wheel::disable_autocenter" ) ;
}

inline void wheel::q_set_autocenter(uti::u16_t magnitude) noexcept
{
    if (protocol_ == ffb_protocol::logitech_hidpp)
    {
        reports_.emplace_back(protocol::hidpp_ff_set_autocenter(magnitude));
        return;
    }

    // Classic fallback: keep existing behavior
    if (magnitude == 0)
        q_disable_autocenter();
    else
        q_enable_autocenter();
}


constexpr bool wheel::enable_autocenter () const noexcept
{       auto rep = protocol::enable_autocenter(protocol_, 0x0F);
        if (rep.len == 0) return true;   // treat “not implemented” as no-op
        return _write_report(rep, "wheel::enable_autocenter" ) ;
}

constexpr void wheel::q_disable_autocenter () noexcept
{
        reports_.emplace_back( protocol::disable_autocenter( protocol_, 0x0F ) ) ;
}

constexpr void wheel::q_enable_autocenter () noexcept
{
        reports_.emplace_back( protocol::enable_autocenter( protocol_, 0x0F ) ) ;
}

////////////////////////////////////////////////////////////////////////////////

// constexpr bool wheel::download_forces () const noexcept
// {
//         force f_const  { force_type::CONSTANT , {} } ;
//         force f_spring { force_type::SPRING   , {} } ;
//         force f_damper { force_type::DAMPER   , {} } ;
//         force f_trap   { force_type::TRAPEZOID, {} } ;

//         f_const .constant  = constant_   ;
//         f_spring.spring    = spring_     ;
//         f_damper.damper    = damper_     ;
//         f_trap  .trapezoid = trapezoid_  ;

//         vector< report > reports( 5 ) ;

//         if( f_const.params.enabled )
//         {
//                 reports.emplace_back( protocol::download_force( protocol_, f_const ) ) ;
//         }
//         if( f_spring.params.enabled )
//         {
//                 reports.emplace_back( protocol::download_force( protocol_, f_spring ) ) ;
//         }
//         if( f_damper.params.enabled )
//         {
//                 reports.emplace_back( protocol::download_force( protocol_, f_damper ) ) ;
//         }
//         if( f_trap.params.enabled )
//         {
//                 reports.emplace_back( protocol::download_force( protocol_, f_trap ) ) ;
//         }
//         return _write_reports( reports, "wheel::download_forces" ) ;
// }

inline bool wheel::download_forces() noexcept
{
    force f_const  { force_type::CONSTANT , {} };
    force f_spring { force_type::SPRING   , {} };
    force f_damper { force_type::DAMPER   , {} };
    force f_trap   { force_type::TRAPEZOID, {} };

    f_const .constant  = constant_;
    f_spring.spring    = spring_;
    f_damper.damper    = damper_;
    f_trap  .trapezoid = trapezoid_;

    // --- HID++ path: do “send + read reply” per effect (no report batching) ---
    if (protocol_ == ffb_protocol::logitech_hidpp)
    {
        if (!device_.open())
            return false;

        if (!device_.enable_input_reports())
        {
            device_.close();
            return false;
        }

        bool ok = true;

        if (f_const.params.enabled)
            ok = ok && protocol::hidpp_download_force_sync(device_, f_const);

        if (f_spring.params.enabled)
            ok = ok && protocol::hidpp_download_force_sync(device_, f_spring);

        if (f_damper.params.enabled)
            ok = ok && protocol::hidpp_download_force_sync(device_, f_damper);

        if (f_trap.params.enabled)
            ok = ok && protocol::hidpp_download_force_sync(device_, f_trap);

        device_.close();
        return ok;
    }

    // --- Classic path: batch output reports like before ---
    vector<report> reports;
    reports.reserve(4);  // IMPORTANT: don't pre-fill with empty reports

    if (f_const.params.enabled)  reports.emplace_back(protocol::download_force(protocol_, f_const));
    if (f_spring.params.enabled) reports.emplace_back(protocol::download_force(protocol_, f_spring));
    if (f_damper.params.enabled) reports.emplace_back(protocol::download_force(protocol_, f_damper));
    if (f_trap.params.enabled)   reports.emplace_back(protocol::download_force(protocol_, f_trap));

    return _write_reports(reports, "wheel::download_forces");
}

constexpr void wheel::q_download_forces () noexcept
{
        force f_const  { force_type::CONSTANT , {} } ;
        force f_spring { force_type::SPRING   , {} } ;
        force f_damper { force_type::DAMPER   , {} } ;
        force f_trap   { force_type::TRAPEZOID, {} } ;

        f_const .constant  = constant_   ;
        f_spring.spring    = spring_     ;
        f_damper.damper    = damper_     ;
        f_trap  .trapezoid = trapezoid_  ;

        if( f_const.params.enabled )
        {
                reports_.emplace_back( protocol::download_force( protocol_, f_const ) ) ;
        }
        if( f_spring.params.enabled )
        {
                reports_.emplace_back( protocol::download_force( protocol_, f_spring ) ) ;
        }
        if( f_damper.params.enabled )
        {
                reports_.emplace_back( protocol::download_force( protocol_, f_damper ) ) ;
        }
        if( f_trap.params.enabled )
        {
                reports_.emplace_back( protocol::download_force( protocol_, f_trap ) ) ;
        }
}

////////////////////////////////////////////////////////////////////////////////

inline bool wheel::play_forces () noexcept
{
        playing_ = true ;

        uti::u8_t slots { 0 } ;

        if( constant_ .enabled ) slots |= constant_ .slot ;
        if( spring_   .enabled ) slots |= spring_   .slot ;
        if( damper_   .enabled ) slots |= damper_   .slot ;
        if( trapezoid_.enabled ) slots |= trapezoid_.slot ;
        auto rep = protocol::play_force(protocol_, slots);
        if (rep.len == 0) return true;   // treat “not implemented” as no-op
        return _write_report(rep,  "wheel::play_forces" ) ;
}

constexpr void wheel::q_play_forces () noexcept
{
        playing_ = true ;

        uti::u8_t slots { 0 } ;

        if( constant_ .enabled ) slots |= constant_ .slot ;
        if( spring_   .enabled ) slots |= spring_   .slot ;
        if( damper_   .enabled ) slots |= damper_   .slot ;
        if( trapezoid_.enabled ) slots |= trapezoid_.slot ;

        reports_.emplace_back( protocol::play_force( protocol_, slots ) ) ;
}

////////////////////////////////////////////////////////////////////////////////

// constexpr bool wheel::stop_forces () noexcept
// {
//         playing_ = false ;
//         auto rep = protocol::play_force(protocol_, 0x0F);
//         if (rep.len == 0) return true;   // treat “not implemented” as no-op
//         return _write_report(rep, "wheel::stop_forces" ) ;
//             playing_ = false;

//         if (protocol_ == ffb_protocol::logitech_hidpp)
//         {
//                 // Kill any AUTOSTART effects (constant from calibrate, etc.)
//                 return _write_report(protocol::hidpp_ff_reset_all(), "wheel::hidpp_ff_reset_all(stop_forces)");
//         }

//         auto rep = protocol::play_force(protocol_, 0x0F);
//         if (rep.len == 0) return true;
//         return _write_report(rep, "wheel::stop_forces");
// }

// constexpr bool wheel::stop_forces () noexcept
// {
//     playing_ = false;

//     if (protocol_ == ffb_protocol::logitech_hidpp)
//     {
//         if (!device_.open()) return false;
//         device_.enable_input_reports();

//         auto & ctx = hidpp_ctx();
//         // Stop all tracked slots (constant/spring/damper/trap/autocenter if present)
//         for (int i = 0; i < 16; ++i)
//         {
//             uti::u8_t slot = ctx.ff_slot_by_force_mask[i];
//             if (slot) protocol::hidpp_set_effect_state_sync(device_, slot, protocol::HIDPP_FF_EFFECT_STATE_STOP);
//         }

//         device_.close();
//         return true;
//     }

//     // classic path (leave as-is or switch to stop_force() if you want)
//     auto rep = protocol::play_force(protocol_, 0x0F);
//     if (rep.len == 0) return true;
//     return _write_report(rep, "wheel::stop_forces");
// }

inline bool wheel::stop_forces() noexcept
{
    playing_ = false;

    // For HID++: RESET_ALL clears everything, including your baseline spring.
    // Re-apply baseline autocenter immediately so the wheel doesn't go back to "default stiff".
    if (protocol_ == ffb_protocol::logitech_hidpp)
    {
        bool ok = true;

        // stop everything
        ok = ok && _write_report(protocol::hidpp_ff_reset_all(), "wheel::hidpp_ff_reset_all(stop)");

        // choose one of these:
        ok = ok && _write_report(
            protocol::hidpp_ff_set_autocenter(protocol::HIDPP_FF_BASELINE_AUTOCENTER),
            "wheel::hidpp_ff_set_autocenter(BASELINE-after-stop)"
        );

        // If you want it totally limp after stop instead, use:
        // ok = ok && _write_report(protocol::hidpp_ff_set_autocenter(0), "autocenter_off_after_stop");

        return ok;
    }

    // Classic path unchanged
    auto rep = protocol::stop_force(protocol_, 0x0F);
    if (rep.len == 0) return true;
    return _write_report(rep, "wheel::stop_forces");
}

constexpr void wheel::q_stop_forces () noexcept
{
        playing_ = false ;

        reports_.emplace_back( protocol::stop_force( protocol_, 0x0F ) ) ;
}

////////////////////////////////////////////////////////////////////////////////

constexpr bool wheel::refresh_forces () noexcept
{
        if( !playing_ ) return play_forces() ;

        force f_const  { force_type::CONSTANT , {} } ;
        force f_spring { force_type::SPRING   , {} } ;
        force f_damper { force_type::DAMPER   , {} } ;
        force f_trap   { force_type::TRAPEZOID, {} } ;

        f_const.constant =  constant_ ;
        f_spring. spring =    spring_ ;
        f_damper. damper =    damper_ ;
        f_trap.trapezoid = trapezoid_ ;

        vector< report > reports( 4 ) ;

        if( f_const.params.enabled )
        {
                reports.emplace_back( protocol::refresh_force( protocol_, f_const ) ) ;
        }
        if( f_spring.params.enabled )
        {
                reports.emplace_back( protocol::refresh_force( protocol_, f_spring ) ) ;
        }
        if( f_damper.params.enabled )
        {
                reports.emplace_back( protocol::refresh_force( protocol_, f_damper ) ) ;
        }
        if( f_trap.params.enabled )
        {
                reports.emplace_back( protocol::refresh_force( protocol_, f_trap ) ) ;
        }
        return _write_reports( reports, "wheel::refresh_forces" ) ;
}

constexpr void wheel::q_refresh_forces () noexcept
{
        if( !playing_ ) q_play_forces() ;

        force f_const  { force_type::CONSTANT , {} } ;
        force f_spring { force_type::SPRING   , {} } ;
        force f_damper { force_type::DAMPER   , {} } ;
        force f_trap   { force_type::TRAPEZOID, {} } ;

        f_const.constant =  constant_ ;
        f_spring. spring =    spring_ ;
        f_damper. damper =    damper_ ;
        f_trap.trapezoid = trapezoid_ ;

        if( f_const.params.enabled )
        {
                reports_.emplace_back( protocol::refresh_force( protocol_, f_const ) ) ;
        }
        if( f_spring.params.enabled )
        {
                reports_.emplace_back( protocol::refresh_force( protocol_, f_spring ) ) ;
        }
        if( f_damper.params.enabled )
        {
                reports_.emplace_back( protocol::refresh_force( protocol_, f_damper ) ) ;
        }
        if( f_trap.params.enabled )
        {
                reports_.emplace_back( protocol::refresh_force( protocol_, f_trap ) ) ;
        }
}

////////////////////////////////////////////////////////////////////////////////

constexpr bool wheel::set_led_pattern ( uti::u8_t pattern ) const noexcept
{
        auto rep = protocol::set_led_pattern(protocol_, pattern);
        if (rep.len == 0) return true;   // treat “not implemented” as no-op
        return _write_report( rep, "wheel::set_led_pattern" ) ;
}

constexpr void wheel::q_set_led_pattern ( uti::u8_t pattern ) noexcept
{
        reports_.emplace_back( protocol::set_led_pattern( protocol_, pattern ) ) ;
}

////////////////////////////////////////////////////////////////////////////////

constexpr bool wheel::flush_reports () noexcept
{
        auto res = _write_reports( reports_, "wheel::flush" ) ;
        reports_.clear() ;

        return res ;
}

////////////////////////////////////////////////////////////////////////////////

constexpr bool wheel::_write_report ( report const & report, [[ maybe_unused ]] char const * scope ) const noexcept
{
        if( !device_.open() )
        {
                FFFB_F_ERR_S( scope, "failed opening device %x", device_.device_id() ) ;
                return false ;
        }
        if( !device_.write( report ) )
        {
                FFFB_F_ERR_S( scope, "failed sending report to device %x", device_.device_id() ) ;
                return false ;
        }
        if( !device_.close() )
        {
                FFFB_F_ERR_S( scope, "failed closing device %x", device_.device_id() ) ;
                return false ;
        }
        return true ;
}

////////////////////////////////////////////////////////////////////////////////

constexpr bool wheel::_write_reports ( vector< report > const & reports, [[ maybe_unused ]] char const * scope ) const noexcept
{
        if( !device_.open() )
        {
                FFFB_F_ERR_S( scope, "failed opening device %x", device_.device_id() ) ;
                return false ;
        }
        for( auto const & rep : reports )
        {
                if( !device_.write( rep ) )
                {
                        FFFB_F_ERR_S( scope, "failed sending report to device %x", device_.device_id() ) ;
                        return false ;
                }
        }
        if( !device_.close() )
        {
                FFFB_F_ERR_S( scope, "failed closing device %x", device_.device_id() ) ;
                return false ;
        }
        return true ;
}

////////////////////////////////////////////////////////////////////////////////


bool wheel::_init_protocol() noexcept
{
    if( protocol_ == ffb_protocol::logitech_hidpp )
    {
        uti::u8_t maj=0, min=0, idx=0;

        if( !device_.open() )
        {
            FFFB_F_ERR_S("wheel::init_protocol", "failed opening device %x", device_.device_id());
            return false;
        }

        if (!device_.enable_input_reports()) {
        FFFB_F_ERR_S("wheel::init_protocol", "enable_input_reports failed");
        device_.close();
        return false;
        }

        bool ok_ping = protocol::hidpp_ping(device_, maj, min, idx);
        if( !ok_ping )
        {
            FFFB_F_ERR_S("wheel::init_protocol", "HID++ ping FAILED");
            device_.close();
            return false;
        }

        FFFB_F_INFO_S("wheel::init_protocol",
                      "HID++ ping OK: version %u.%u (dev_index=0x%02x)",
                      (unsigned)maj, (unsigned)min, (unsigned)idx);

        if( !protocol::hidpp_init(device_, idx) )
        {
            FFFB_F_ERR_S("wheel::init_protocol", "HID++ init FAILED (no FF feature)");
            device_.close();
            return false;
        }
        hidpp_ctx().dev_index = idx;

        // device_ is already open here:
        if (!device_.write(protocol::hidpp_ff_reset_all()))
        FFFB_F_ERR_S("wheel::init_protocol", "hidpp_ff_reset_all write failed");

        if (!device_.write(protocol::hidpp_ff_set_autocenter(protocol::HIDPP_FF_BASELINE_AUTOCENTER)))
        FFFB_F_ERR_S("wheel::init_protocol", "hidpp_ff_set_autocenter write failed");

        device_.close();
        return true;
    }

    auto init_sequence = protocol::init_sequence(protocol_, device_.device_id());
    if( init_sequence.empty() ) return true;
    return _write_reports(init_sequence, "wheel::init_sequence");
}

////////////////////////////////////////////////////////////////////////////


} // namespace fffb
