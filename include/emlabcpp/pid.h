///
/// Copyright (C) 2020 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
/// and associated documentation files (the "Software"), to deal in the Software without
/// restriction, including without limitation the rights to use, copy, modify, merge, publish,
/// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
/// Software is furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all copies or
/// substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
/// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
/// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///

#include "emlabcpp/algorithm.h"

#ifdef EMLABCPP_USE_NLOHMANN_JSON
#include <nlohmann/json.hpp>
#endif

#pragma once

namespace emlabcpp
{

struct pid_coefficients
{
        /// coeficients
        float p = 0;
        float i = 0;
        float d = 0;
};

// Structure to configure the pid reglator
struct pid_config
{
        pid_coefficients coefficients{ .p = 1.f, .i = 0.f, .d = 0.f };

        /// sets this properly, otherwise the pid won't work for corner cases
        min_max< float > limits{ 0.f, 100.f };
};

/// Implementation of PID regulator, the object should be constructed and populated with
/// pid<T>::conf structure with configuration values (p,i,d coeficients, min/max output vals). The
/// object contains all relevantstate data and should be called regularly.
template < typename TimeType >
class pid
{
        /// based on
        /// http://brettbeauregard.com/blog/2011/04/improving-the-beginners-pid-introduction/
public:
        using time_type = TimeType;
        using config    = pid_config;

private:
        config conf_;

        float i_term_        = 0;
        float last_desired_  = 0;
        float last_measured_ = 0;

        time_type last_time_;

        float output_ = 0;

        /// improvements from naive PID:
        /// - we work with derivation of measured, not error (error jumps in case you change desired
        /// value a lot)
        /// - we store i_term_ entirely (i * sum_) instead of just the sum_, makes it easier to
        /// change the scale of i
        ///          (imagine what happens when you store sum_, use i*sum_ in the formula nad
        ///          change i from 1 to 100)

public:
        pid( time_type now, config conf = config{} )
          : last_time_( now )
        {
                set_config( conf );
        }

        void set_last_time( time_type t )
        {
                last_time_ = t;
        }

        void set_last_measured( float v )
        {
                last_measured_ = v;
        }

        void set_config( config conf )
        {
                set_pid( conf.coefficients );
                set_limits( conf.limits );
        }

        [[nodiscard]] const config& get_config() const
        {
                return conf_;
        }

        void set_pid( float p, float i, float d )
        {
                set_pid( { p, i, d } );
        }

        void set_pid( pid_coefficients c )
        {
                conf_.coefficients = c;
        }

        void set_limits( float min, float max )
        {
                set_limits( { min, max } );
        }

        void set_limits( min_max< float > lim )
        {
                conf_.limits = lim;
                output_      = std::clamp( output_, conf_.limits.min, conf_.limits.max );
                i_term_      = std::clamp( i_term_, conf_.limits.min, conf_.limits.max );
        }

        [[nodiscard]] const min_max< float >& get_limits() const
        {
                return conf_.limits;
        }

        /// call this reularly, the meaning of time value 'now' is up to you, just be consistent
        ///
        /// Algorithm changes it's internal value output_ to the value that should be set to a
        /// 'thing' that controls measured_ value. It tries to control the 'thing' so the measured
        /// eventually converges to 'desired' value
        float update( time_type now, float measured, float desired )
        {
                if ( now == last_time_ ) {
                        return output_;
                }

                auto t_diff = static_cast< float >( now - last_time_ );

                last_desired_ = desired;

                const pid_coefficients& coeff = conf_.coefficients;

                float error = desired - measured;
                i_term_ += coeff.i * ( error * t_diff );
                /// we want to prevent the i_term_ to escallate out of proportion, to prevent it
                /// from going to infinity and beyond
                i_term_ = std::clamp( i_term_, conf_.limits.min, conf_.limits.max );

                float measured_diff = ( measured - last_measured_ ) / t_diff;
                output_             = coeff.p * error + i_term_ - coeff.d * measured_diff;
                output_             = std::clamp( output_, conf_.limits.min, conf_.limits.max );

                last_measured_ = measured;
                last_time_     = now;

                return output_;
        }

        void set_output( float output = 0 )
        {
                output_ = std::clamp( output, conf_.limits.min, conf_.limits.max );
        }

        [[nodiscard]] float get_output() const
        {
                return output_;
        }
        [[nodiscard]] float get_measured() const
        {
                return last_measured_;
        }
        [[nodiscard]] float get_desired() const
        {
                return last_desired_;
        }
};

}  // namespace emlabcpp

#ifdef EMLABCPP_USE_NLOHMANN_JSON

template <>
struct nlohmann::adl_serializer< emlabcpp::pid_coefficients >
{
        using cfg_type = emlabcpp::pid_coefficients;
        static void to_json( nlohmann::json& j, const cfg_type& cfg )
        {
                j["p"] = cfg.p;
                j["i"] = cfg.i;
                j["d"] = cfg.d;
        }

        static cfg_type from_json( const nlohmann::json& j )
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
        static void to_json( nlohmann::json& j, const cfg_type& cfg )
        {
                j["coefficients"] = cfg.coefficients;
                j["limits"]       = cfg.limits;
        }

        static cfg_type from_json( const nlohmann::json& j )
        {
                cfg_type cfg;
                cfg.coefficients = j["coefficients"];
                cfg.limits       = j["limits"];
                return cfg;
        }
};

#endif
