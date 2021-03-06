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


#include "src/actions/rule_id.h"

#include <string>


namespace modsecurity {
namespace actions {


bool RuleId::init(std::string *error) {
    std::string a = m_parserPayload;

    std::istringstream iss(a);
    iss >> m_ruleId;
    if (iss.fail()) {
        m_ruleId = 0;
        error->assign("The input \"" + a + "\" does not " \
            "seems to be a valid rule id.");
        return false;
    }

    if (a != std::to_string(m_ruleId) || m_ruleId < 0) {
        error->assign("The input \"" + a + "\" does not seems " \
            "to be a valid rule id.");
        return false;
    }
    return true;
}


}  // namespace actions
}  // namespace modsecurity
