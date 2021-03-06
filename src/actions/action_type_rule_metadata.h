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


#include "modsecurity/actions/action.h"
#include "src/rule_with_actions.h"

#ifndef SRC_ACTIONS_ACTION_TYPE_CONFIGURE_H_
#define SRC_ACTIONS_ACTION_TYPE_CONFIGURE_H_


namespace modsecurity {
namespace actions {


class ActionTypeRuleMetaData : public virtual Action {
 public:
     /**
     *
     * Action that are executed while loading the configuration. For instance
     * the rule ID or the rule phase.
     *
     */
    ActionTypeRuleMetaData()
        : Action()
    { };

    virtual void configure(RuleWithActions *rule) = 0;
};


}  // namespace actions
}  // namespace modsecurity

#endif  // SRC_ACTIONS_ACTION_TYPE_CONFIGURE_H_
