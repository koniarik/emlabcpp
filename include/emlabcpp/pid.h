#include "emlabcpp/algorithm.h"

#pragma once

namespace emlabcpp
{

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

        float i_term_     = 0;
        float last_input_ = 0;

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
                float t_diff = static_cast< float >( now - last_time_ );

                if ( t_diff == 0.f ) {
                        return output_;
                }

                float error = desired - input;
                i_term_ += conf_.i * ( error * t_diff );
                // we want to prevent the i_term_ to escallate out of proportion, to prevent it from
                // going to infinity and beyond
                i_term_ = std::clamp( i_term_, conf_.min, conf_.max );

                float input_diff = ( input - last_input_ ) / t_diff;
                output_          = conf_.p * error + i_term_ - conf_.d * input_diff;
                output_          = std::clamp( output_, conf_.min, conf_.max );

                last_input_ = input;
                last_time_  = now;

                return output_;
        }

        float get_output()
        {
                return output_;
        }
};

}  // namespace emlabcpp
