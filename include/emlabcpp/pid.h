/// MIT License
///
/// Copyright (c) 2025 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
///

#pragma once

#include "./min_max.h"

#ifdef EMLABCPP_USE_NLOHMANN_JSON
#include <nlohmann/json.hpp>
#endif

namespace emlabcpp
{

struct pid_coefficients
{
        /// coeficients
        float p = 0;
        float i = 0;
        float d = 0;
};

/// Structure to configure the pid regulator
struct pid_config
{
        pid_coefficients coefficients{ .p = 1.f, .i = 0.f, .d = 0.f };

        /// limits the output of the pid regulator and internal anti-windup mechanism
        min_max< float > limits{ 0.f, 100.f };
};

/// Implementation of PID regulator, the object should be constructed and populated with
/// pid<T>::conf structure with configuration values (p,i,d coeficients, min/max output vals). The
/// object contains all relevantstate data and should be called regularly.
///
/// based on
/// http://brettbeauregard.com/blog/2011/04/improving-the-beginners-pid-introduction/
//
template < typename TimeType >
struct pid
{
        using time_type = TimeType;
        using config    = pid_config;

        config    cfg;
        float     i_sum         = 0;
        float     last_measured = 0;
        time_type last_time;
        float     output;

        pid( time_type now, config const& conf = config{} )
          : cfg( conf )
          , last_time( now )
          , output( clamp( 0.f, cfg.limits ) )
        {
        }
};

template < typename TimeType >
void update_limits( pid< TimeType >& pid, min_max< float > lim )
{
        pid.cfg.limits = lim;
        pid.output     = clamp( pid.output, pid.cfg.limits );
        pid.i_sum      = clamp( pid.i_sum, pid.cfg.limits );
}

template < typename TimeType >
void update_output( pid< TimeType >& pid, float output )
{
        pid.output = clamp( output, pid.cfg.limits );
}

/// Call this reularly, the meaning of time value 'now' is up to you, just be consistent.
///
/// Algorithm changes it's internal value output to the value that should be set to a
/// 'thing' that controls measured_ value. It tries to control the 'thing' so the measured
/// eventually converges to 'desired' value
///
/// improvements from naive PID:
/// - we work with derivation of measured, not error (error jumps in case you change desired
/// value a lot)
/// - we store i_sum entirely (i * sum_) instead of just the sum_, makes it easier to
/// change the scale of i
///          (imagine what happens when you store sum_, use i*sum_ in the formula nad
///          change i from 1 to 100)
template < typename TimeType >
float update( pid< TimeType >& pid, TimeType now, float measured, float desired )
{
        if ( now == pid.last_time )
                return pid.output;

        auto t_diff = static_cast< float >( now - pid.last_time );

        pid_coefficients const& coeff = pid.cfg.coefficients;

        float const error = desired - measured;
        pid.i_sum += coeff.i * ( error * t_diff );
        pid.i_sum = clamp( pid.i_sum, pid.cfg.limits );

        float const measured_diff = ( measured - pid.last_measured ) / t_diff;
        pid.output                = coeff.p * error + pid.i_sum - coeff.d * measured_diff;
        pid.output                = clamp( pid.output, pid.cfg.limits );

        pid.last_measured = measured;
        pid.last_time     = now;

        return pid.output;
}

template < typename TimeType >
void reset( pid< TimeType >& pid, TimeType now, float last_measured )
{
        pid.last_time     = now;
        pid.last_measured = last_measured;
}

}  // namespace emlabcpp

#ifdef EMLABCPP_USE_NLOHMANN_JSON

template <>
struct nlohmann::adl_serializer< emlabcpp::pid_coefficients >
{
        using cfg_type = emlabcpp::pid_coefficients;

        static void to_json( nlohmann::json& j, cfg_type const& cfg )
        {
                j["p"] = cfg.p;
                j["i"] = cfg.i;
                j["d"] = cfg.d;
        }

        static cfg_type from_json( nlohmann::json const& j )
        {
                cfg_type cfg;
                cfg.p = j["p"];
                cfg.i = j["i"];
                cfg.d = j["d"];
                return cfg;
        }
};

template <>
struct nlohmann::adl_serializer< emlabcpp::pid_config >
{
        using cfg_type = emlabcpp::pid_config;

        static void to_json( nlohmann::json& j, cfg_type const& cfg )
        {
                j["coefficients"] = cfg.coefficients;
                j["limits"]       = cfg.limits;
        }

        static cfg_type from_json( nlohmann::json const& j )
        {
                cfg_type cfg;
                cfg.coefficients = j["coefficients"];
                cfg.limits       = j["limits"];
                return cfg;
        }
};

#endif
