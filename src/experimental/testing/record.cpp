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
#include "emlabcpp/experimental/testing/record.h"

#include "experimental/testing/reactor_interface_adapter.h"

using namespace emlabcpp;

std::optional< testing_arg_variant > testing_record::get_arg_variant( const testing_key& key )
{
        comm_.reply< TESTING_ARG >( rid_, key );

        std::optional< testing_arg_variant > res;

        std::optional< testing_controller_reactor_variant > opt_var = comm_.read_variant();
        if ( !opt_var ) {
                comm_.report_failure< TESTING_NO_RESPONSE_E >( TESTING_ARG );
                return {};
        }

        apply_on_match(
            *opt_var,
            [&]( tag< TESTING_ARG >,
                 testing_run_id,
                 const testing_key&,
                 const testing_arg_variant& var ) {
                    res = var;
            },
            [&]( tag< TESTING_ARG_MISSING >, testing_run_id, const testing_key& key ) {
                    comm_.report_failure< TESTING_ARG_MISSING_E >( key );
            },
            [&]< auto ID >( tag< ID >, auto... ) {
                    comm_.report_failure< TESTING_ARG_WRONG_MESSAGE_E >( ID );
            } );

        return res;
}

void testing_record::collect( const testing_key& key, const testing_arg_variant& arg )
{
        comm_.reply< TESTING_COLLECT >( rid_, key, arg );
}

void testing_record::report_wrong_type_error( const testing_key& key )
{
        comm_.report_failure< TESTING_ARG_WRONG_TYPE_E >( key );
}
