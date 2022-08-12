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

#pragma once

namespace emlabcpp
{

/// Implementation of PID regulator, the object should be constructed and populated with
/// pid<T>::conf structure with configuration values (p,i,d coeficients, min/max output vals). The
/// object contains all relevantstate data and should be called regularly.
template < typename TimeType >
class pid
{
        /// based on
        /// http://brettbeauregard.com/blog/2011/04/improving-the-beginners-pid-introduction/
public:
        /// use this structure to configure the pid reglator
        struct config
        {
                /// coeficients
                float p = 0;
                float i = 0;
                float d = 0;

                /// sets this propebly, otherwise the pid won't work for corner cases
                float min = 0;    /// minimal output value
                float max = 100;  /// maximal output value
        };

        using time_type = TimeType;

private:
        config conf_;

        float i_term_       = 0;
        float last_desired_ = 0;
        float last_input_   = 0;

        time_type last_time_;

        float output_ = 0;

        /// improvements from naive PID:
        /// - we work with derivation of input, not error (error jumps in case you change desired
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

        void set_output( float output = 0 )
        {
                output_ = output;
        }

        /// To correctly reset the pid, tell it the actuall output_ value
        /// That is required for cases when you set it up manually without pids knowledge
        void reset( float output = 0 )
        {
                output_     = output;
                last_input_ = 0;
                i_term_     = std::clamp( output_, conf_.min, conf_.max );
        }

        void set_config( config conf )
        {
                conf_   = conf;
                output_ = std::clamp( output_, conf_.min, conf_.max );
                i_term_ = std::clamp( i_term_, conf_.min, conf_.max );
        }

        /// call this reularly, the meaning of time value 'now' is up to you, just be consistent
        ///
        /// Algorithm changes it's internal value output_ to the value that should be set to a
        /// 'thing' that controls input_ value. It tries to control the 'thing' so the input
        /// eventually converges to 'desired' value
        float update( time_type now, float input, float desired )
        {
                auto t_diff = static_cast< float >( now - last_time_ );

                if ( t_diff == 0.f ) {
                        return output_;
                }
                last_desired_ = desired;

                float error = desired - input;
                i_term_ += conf_.i * ( error * t_diff );
                /// we want to prevent the i_term_ to escallate out of proportion, to prevent it
                /// from going to infinity and beyond
                i_term_ = std::clamp( i_term_, conf_.min, conf_.max );

                float input_diff = ( input - last_input_ ) / t_diff;
                output_          = conf_.p * error + i_term_ - conf_.d * input_diff;
                output_          = std::clamp( output_, conf_.min, conf_.max );

                last_input_ = input;
                last_time_  = now;

                return output_;
        }

        [[nodiscard]] float get_output() const
        {
                return output_;
        }
        [[nodiscard]] float get_input() const
        {
                return last_input_;
        }
        [[nodiscard]] float get_desired() const
        {
                return last_desired_;
        }
};

}  // namespace emlabcpp
