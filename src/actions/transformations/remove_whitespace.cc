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


#include "src/actions/transformations/remove_whitespace.h"

#include <string>

#include "modsecurity/modsecurity.h"
#include "modsecurity/transaction.h"

#include "src/utils/string.h"


namespace modsecurity {
namespace actions {
namespace transformations {


void RemoveWhitespace::execute(const Transaction *t,
    const ModSecString &in,
    ModSecString &out) noexcept {
    out = in;
    int64_t i = 0;

    // loop through all the chars
    while (i < out.size()) {
        // remove whitespaces and non breaking spaces (NBSP)
        if (isspace(out[i]) || (out[i] == NBSP)) {
            out.erase(i, 1);
        } else {
          /* if the space is not a whitespace char, increment counter
           counter should not be incremented if a character is erased because
           the index erased will be replaced by the following character */
          i++;
        }
    }
}


}  // namespace transformations
}  // namespace actions
}  // namespace modsecurity

