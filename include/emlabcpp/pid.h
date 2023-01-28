// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//
//  Copyright © 2022 Jan Veverak Koniarik
//  This file is part of project: emlabcpp
//
#include "emlabcpp/algorithm.h"

#ifdef EMLABCPP_USE_NLOHMANN_JSON
#include <nlohmann/json.hpp>
#endif

#pragma once

namespace emlabcpp
{
// Structure to configure the pid reglator
struct pid_config
{
        /// coeficients
        float p = 0;
        float i = 0;
        float d = 0;

        /// sets this propebly, otherwise the pid won't work for corner cases
        float min = 0;    /// minimal output value
        float max = 100;  /// maximal output value
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

        void set_time( time_type t )
        {
                last_time_ = t;
        }

        void set_config( config conf )
        {
                set_pid( conf.p, conf.i, conf.d );
                set_limits( conf.min, conf.max );
        }

        const config& get_config() const
        {
                return conf_;
        }

        void set_pid( float p, float i, float d )
        {
                conf_.p = p;
                conf_.i = i;
                conf_.d = d;
        }

        void set_limits( float min, float max )
        {
                conf_.min = min;
                conf_.max = max;
                output_   = std::clamp( output_, conf_.min, conf_.max );
                i_term_   = std::clamp( i_term_, conf_.min, conf_.max );
        }

        min_max< float > get_limits() const
        {
                return { conf_.min, conf_.max };
        }

        /// call this reularly, the meaning of time value 'now' is up to you, just be consistent
        ///
        /// Algorithm changes it's internal value output_ to the value that should be set to a
        /// 'thing' that controls measured_ value. It tries to control the 'thing' so the measured
        /// eventually converges to 'desired' value
        float update( time_type now, float measured, float desired )
        {
                auto t_diff = static_cast< float >( now - last_time_ );

                if ( t_diff == 0.f ) {
                        return output_;
                }
                last_desired_ = desired;

                float error = desired - measured;
                i_term_ += conf_.i * ( error * t_diff );
                /// we want to prevent the i_term_ to escallate out of proportion, to prevent it
                /// from going to infinity and beyond
                i_term_ = std::clamp( i_term_, conf_.min, conf_.max );

                float measured_diff = ( measured - last_measured_ ) / t_diff;
                output_             = conf_.p * error + i_term_ - conf_.d * measured_diff;
                output_             = std::clamp( output_, conf_.min, conf_.max );

                last_measured_ = measured;
                last_time_     = now;

                return output_;
        }

        void set_output( float output = 0 )
        {
                output_ = std::clamp( output, conf_.min, conf_.max );
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
struct nlohmann::adl_serializer< emlabcpp::pid_config >
{
        using cfg_type = emlabcpp::pid_config;
        static void to_json( nlohmann::json& j, const cfg_type& cfg )
        {
                j["p"]   = cfg.p;
                j["i"]   = cfg.i;
                j["d"]   = cfg.d;
                j["min"] = cfg.min;
                j["max"] = cfg.max;
        }

        static cfg_type from_json( const nlohmann::json& j )
        {
                cfg_type cfg;
                cfg.p   = j["p"];
                cfg.i   = j["i"];
                cfg.d   = j["d"];
                cfg.min = j["min"];
                cfg.max = j["max"];
                return cfg;
        }
};

#endif
