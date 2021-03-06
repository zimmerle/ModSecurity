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


#include <string>

#include "modsecurity/modsecurity.h"
#include "modsecurity/actions/action.h"

#include "src/actions/transformations/transformation.h"


#ifndef SRC_ACTIONS_TRANSFORMATIONS_REMOVE_COMMENTS_CHAR_H_
#define SRC_ACTIONS_TRANSFORMATIONS_REMOVE_COMMENTS_CHAR_H_


namespace modsecurity {
namespace actions {
namespace transformations {


class RemoveCommentsChar : public Transformation {
 public:
    RemoveCommentsChar()
        : Action("t:removeCommentsChar")
    { }

    void execute(const Transaction *t,
        const ModSecString &in,
        ModSecString &out) noexcept override;
};


}  // namespace transformations
}  // namespace actions
}  // namespace modsecurity


#endif  // SRC_ACTIONS_TRANSFORMATIONS_REMOVE_COMMENTS_CHAR_H_
