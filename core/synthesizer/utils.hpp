/*******************************<GINKGO LICENSE>******************************
Copyright 2017-2018

Karlsruhe Institute of Technology
Universitat Jaume I
University of Tennessee

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************<GINKGO LICENSE>*******************************/

#ifndef GKO_CORE_SYNTHESIZER_UTILS_
#define GKO_CORE_SYNTHESIZER_UTILS_


#include "core/base/types.hpp"


namespace gko {
namespace syn {


template <typename T, T... Values>
struct value_sequence {
    static constexpr size_type size = sizeof...(Values);
    using type = T;

private:
    template <size_type K, T First, T... Rest>
    struct get_impl {
        static constexpr T value = get_impl<K - 1, Rest...>::value;
    };

    template <T First, T... Rest>
    struct get_impl<0, First, Rest...> {
        static constexpr T value = First;
    };

public:
    template <size_type K>
    struct get {
        static_assert(K < size, "Out of bounds error");
        static constexpr T value = get_impl<K, Values...>::value;
    };
};


template <typename T, T... Values>
constexpr size_type value_sequence<T, Values...>::size;
template <typename T, T... Values>
template <size_type K>
constexpr T value_sequence<T, Values...>::get<K>::value;


/*
namespace detail {


template <typename ValueSequence, typename ValueSequence::type Value>
struct prepend_impl;

template <typename T, T... Values, T Value>
struct prepend_impl<value_sequence<T, Values...>, Value> {
    using type = value_sequence<T, Value, Values...>;
};

}  // namespace detail


template <typename ValueSequence, typename ValueSequence::type Value>
using prepend = typename detail::prepend_impl<Type, TypeSequence>::type;
*/


}  // namespace syn
}  // namespace gko


#endif  // GKO_CORE_SYNTHESIZER_UTILS_
