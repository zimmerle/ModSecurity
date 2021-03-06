/*
 * ModSecurity, http://www.modsecurity.org/
 * Copyright (c) 2015 - 2020 Trustwave Holdings, Inc. (http://www.trustwave.com/)
 *
 * You may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * If any of the files related to licensing are missing or if you have any
 * other questions related to licensing please contact Trustwave Holdings, Inc.
 * directly using the email address security@modsecurity.org.
 *
 */


#include "src/actions/transformations/upper_case.h"

#include <string>

#include "modsecurity/modsecurity.h"
#include "modsecurity/transaction.h"


namespace modsecurity {
namespace actions {
namespace transformations {


void UpperCase::execute(const Transaction *t,
    const ModSecString &in,
    ModSecString &out) noexcept {
    std::locale loc;
    out.reserve(in.size());
    for (std::string::size_type i=0; i < in.size(); ++i) {
        out += std::toupper(in[i], loc);
    }
}


}  // namespace transformations
}  // namespace actions
}  // namespace modsecurity
