/*
 * ModSecurity, http://www.modsecurity.org/
 * Copyright (c) 2015 Trustwave Holdings, Inc. (http://www.trustwave.com/)
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

#ifdef __cplusplus
#include <stack>
#include <vector>
#include <string>
#include <list>
#include <memory>
#include <utility>
#endif

#ifndef SRC_RULE_WITH_OPERATOR_H_
#define SRC_RULE_WITH_OPERATOR_H_

#include "modsecurity/transaction.h"
#include "modsecurity/modsecurity.h"
#include "modsecurity/variable_value.h"
#include "modsecurity/rule.h"
#include "src/rule_with_actions.h"
#include "src/variables/variable.h"
#include "src/operators/operator.h"
#include "modsecurity/string_view.hpp"

#ifdef __cplusplus

namespace modsecurity {


class RuleWithOperator : public RuleWithActions {
 public:
    RuleWithOperator(operators::Operator *op,
        variables::Variables *variables,
        Actions *actions,
        Transformations *transformations,
        std::unique_ptr<std::string> fileName,
        int lineNumber);

    RuleWithOperator(const RuleWithOperator &op)
        : RuleWithActions(op),
        m_variables(op.m_variables),
        m_operator(op.m_operator)
    {
        for (auto &a : *m_variables.get()) {
            variables::RuleVariable *vrule = dynamic_cast<variables::RuleVariable *>(a);
            if (vrule != nullptr) {
                vrule->populate(this);
            }
        }
    };

    RuleWithOperator &operator=(const RuleWithOperator& r) {
        RuleWithActions::operator = (r);
        m_variables = r.m_variables;
        m_operator = r.m_operator;
        return *this;
    }

    virtual ~RuleWithOperator();

    bool evaluate(Transaction *transaction) const override;

    void getVariablesExceptions(Transaction *t,
        variables::Variables *exclusion, variables::Variables *addition) const;
    inline void getFinalVars(variables::Variables *vars,
        variables::Variables *eclusion, Transaction *trans) const;

    bool executeOperatorAt(Transaction *transaction,
        const VariableValue *v,
        const bpstd::string_view &value) const;

    static void updateMatchedVars(Transaction *transaction,
        std::shared_ptr<const VariableValue> v,
        const bpstd::string_view &value);

    static void cleanMatchedVars(Transaction *trasn);

    std::string getOperatorName() const;

    virtual std::string getReference() const override {
        return std::to_string(getId());
    }

    virtual void dump(std::stringstream &out) const override {
        Rule::dump(out);
        out << "# RuleWithOperator" << std::endl;
        out << "SecRule ";
        out << m_variables->getVariableNames() << " ";
        out << "\"" << "@" << m_operator->m_op << " " << m_operator->m_param << "\"";
        out << std::endl;
    }

 private:
    std::shared_ptr<modsecurity::variables::Variables> m_variables;
    std::shared_ptr<operators::Operator> m_operator;
};


}  // namespace modsecurity
#endif


#endif  // SRC_RULE_WITH_OPERATOR_H_

