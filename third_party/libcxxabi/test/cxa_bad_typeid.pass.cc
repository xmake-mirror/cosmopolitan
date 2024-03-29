//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===------------------------------------------------------------------------===//

// UNSUPPORTED: c++03

#include "third_party/libcxxabi/include/cxxabi.h"
#include "third_party/libcxx/cassert"
#include "libc/isystem/stdlib.h"
#include "third_party/libcxx/exception"
#include "third_party/libcxx/typeinfo"
#include "third_party/libcxx/string"

#include "third_party/libcxxabi/libcxx/test/support/test_macros.hh"

class Base {
  virtual void foo() {};
};

class Derived : public Base {};

std::string test_bad_typeid(Derived *p) {
    return typeid(*p).name();
}

void my_terminate() { exit(0); }

int main ()
{
    // swap-out the terminate handler
    void (*default_handler)() = std::get_terminate();
    std::set_terminate(my_terminate);

#ifndef TEST_HAS_NO_EXCEPTIONS
    try {
#endif
        test_bad_typeid(nullptr);
        assert(false);
#ifndef TEST_HAS_NO_EXCEPTIONS
    } catch (std::bad_typeid const&) {
        // success
        return 0;
    } catch (...) {
        assert(false);
    }
#endif

    // failure, restore the default terminate handler and fire
    std::set_terminate(default_handler);
    std::terminate();
}
