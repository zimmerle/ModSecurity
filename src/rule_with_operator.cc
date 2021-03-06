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


#include <stdio.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <cstring>
#include <list>
#include <utility>
#include <memory>

#include "modsecurity/rules_set.h"
#include "src/operators/operator.h"
#include "modsecurity/actions/action.h"
#include "modsecurity/modsecurity.h"
#include "src/actions/transformations/none.h"
#include "src/actions/tag.h"
#include "src/utils/string.h"
#include "modsecurity/rule_message.h"
#include "src/actions/msg.h"
#include "src/actions/log_data.h"
#include "src/actions/severity.h"
#include "src/actions/capture.h"
#include "src/actions/multi_match.h"
#include "src/actions/set_var.h"
#include "src/actions/block.h"
#include "src/variables/variable.h"
#include "src/variables/rule.h"
#include "src/rule_with_operator.h"
#include "modsecurity/string_view.hpp"



namespace modsecurity {

using operators::Operator;
using actions::Action;
using variables::Variable;
using actions::transformations::None;


RuleWithOperator::RuleWithOperator(Operator *op,
    variables::Variables *_variables,
    Actions *actions,
    Transformations *transformations,
    std::unique_ptr<std::string> fileName,
    int lineNumber)
    : RuleWithActions(actions, transformations, std::move(fileName), lineNumber),
    m_variables(std::unique_ptr<variables::Variables>(_variables)),
    m_operator(std::unique_ptr<Operator>(op)) {
        for (auto &a : *m_variables.get()) {
            variables::RuleVariable *vrule = dynamic_cast<variables::RuleVariable *>(a);
            if (vrule != nullptr) {
                vrule->populate(this);
            }
        }
    }




RuleWithOperator::~RuleWithOperator() {
}


void RuleWithOperator::updateMatchedVars(Transaction *trans,
    std::shared_ptr<const VariableValue> v,
    const bpstd::string_view &value) {
    ms_dbg_a(trans, 9, "Matched vars updated.");
    trans->m_variableMatchedVar.set(value, trans->m_variableOffset);
    trans->m_variableMatchedVarName.set(v);

    trans->m_variableMatchedVars.set(v);
    trans->m_variableMatchedVarsNames.set(v);
}


inline void RuleWithOperator::cleanMatchedVars(Transaction *trans) {
    ms_dbg_a(trans, 9, "Matched vars cleaned.");
    trans->m_variableMatchedVar.unset();
    trans->m_variableMatchedVars.unset();
    trans->m_variableMatchedVarName.unset();
    trans->m_variableMatchedVarsNames.unset();
}


bool RuleWithOperator::executeOperatorAt(Transaction *trans,
    const VariableValue *v,
    const bpstd::string_view &value) const {
#if MSC_EXEC_CLOCK_ENABLED
    clock_t begin = clock();
    clock_t end;
    double elapsed_s = 0;
#endif
    bool ret;

    ms_dbg_a(trans, 9, "Target value: \"" \
        + utils::string::limitTo(80,
            utils::string::toHexIfNeeded(value.to_string())) \
        + "\" (Variable: " + v->getName() + ")");

    ret = m_operator->evaluateInternal(trans, this, value, trans->messageGetLast());

#if MSC_EXEC_CLOCK_ENABLED
    end = clock();
    elapsed_s = static_cast<double>(end - begin) / CLOCKS_PER_SEC;

    ms_dbg_a(trans, 5, "Operator completed in " + \
        std::to_string(elapsed_s) + " seconds");
#endif
    return ret;
}


void RuleWithOperator::getVariablesExceptions(Transaction *t,
    variables::Variables *exclusion, variables::Variables *addition) const {
    for (auto &a : t->m_rules->m_exceptions.m_variable_update_target_by_tag) {
        if (containsTag(*a.first.get(), t) == false) {
            continue;
        }
        Variable *b = a.second.get();
        if (dynamic_cast<variables::VariableModificatorExclusion*>(b)) {
            exclusion->push_back(
                dynamic_cast<variables::VariableModificatorExclusion*>(
                    b)->m_base.get());
        } else {
            addition->push_back(b);
        }
    }

    for (auto &a : t->m_rules->m_exceptions.m_variable_update_target_by_msg) {
        if (containsMsg(*a.first.get(), t) == false) {
            continue;
        }
        Variable *b = a.second.get();
        if (dynamic_cast<variables::VariableModificatorExclusion*>(b)) {
            exclusion->push_back(
                dynamic_cast<variables::VariableModificatorExclusion*>(
                    b)->m_base.get());
        } else {
            addition->push_back(b);
        }
    }

    for (auto &a : t->m_rules->m_exceptions.m_variable_update_target_by_id) {
        if (getId() != a.first) {
            continue;
        }
        Variable *b = a.second.get();
        if (dynamic_cast<variables::VariableModificatorExclusion*>(b)) {
            exclusion->push_back(
                dynamic_cast<variables::VariableModificatorExclusion*>(
                    b)->m_base.get());
        } else {
            addition->push_back(b);
        }
    }
}


inline void RuleWithOperator::getFinalVars(variables::Variables *vars,
    variables::Variables *exclusion, Transaction *trans) const {
    variables::Variables addition;
    getVariablesExceptions(trans, exclusion, &addition);

    for (int i = 0; i < m_variables->size(); i++) {
        Variable *variable = m_variables->at(i);
        if (exclusion->contains(variable)) {
            continue;
        }
        if (std::find_if(trans->m_ruleRemoveTargetById.begin(),
                trans->m_ruleRemoveTargetById.end(),
                [&, variable, this](std::pair<int, std::string> &m) -> bool {
                    return m.first == getId()
                        && m.second == *variable->getVariableKeyWithCollection();
                }) != trans->m_ruleRemoveTargetById.end()) {
            continue;
        }
        if (std::find_if(trans->m_ruleRemoveTargetByIdRange.begin(),
                trans->m_ruleRemoveTargetByIdRange.end(),
                [&, variable, this](std::pair<std::pair<int, int>, std::string> &m) -> bool {
                    return (m.first.first <= getId() && m.first.second >= getId()
                        && m.second == *variable->getVariableKeyWithCollection());
                    }) != trans->m_ruleRemoveTargetByIdRange.end()) {
            continue;
        }
        if (std::find_if(trans->m_ruleRemoveTargetByTag.begin(),
                    trans->m_ruleRemoveTargetByTag.end(),
                    [&, variable, trans, this](
                        std::pair<std::string, std::string> &m) -> bool {
                        return containsTag(m.first, trans)
                            && m.second == *variable->getVariableKeyWithCollection();
                    }) != trans->m_ruleRemoveTargetByTag.end()) {
            continue;
        }
        vars->push_back(variable);
    }

    for (int i = 0; i < addition.size(); i++) {
        Variable *variable = addition.at(i);
        vars->push_back(variable);
    }
}


bool RuleWithOperator::evaluate(Transaction *trans) const {
    bool globalRet = false;
    variables::Variables *variables = m_variables.get();
    bool recursiveGlobalRet;
    std::string eparam;
    variables::Variables vars;
    vars.reserve(4);
    variables::Variables exclusion;

    RuleWithActions::evaluate(trans);

    // FIXME: Make a class runTimeException to handle this cases.
    for (auto &i : trans->m_ruleRemoveById) {
        if (getId() != i) {
            continue;
        }
        ms_dbg_a(trans, 9, "Rule id: " + std::to_string(getId()) +
            " was skipped due to a ruleRemoveById action...");
        return true;
    }
    for (auto &i : trans->m_ruleRemoveByIdRange) {
        if (!(i.first <= getId() && i.second >= getId())) {
            continue;
        }
        ms_dbg_a(trans, 9, "Rule id: " + std::to_string(getId()) +
            " was skipped due to a ruleRemoveById action...");
        return true;
    }

    if (m_operator->m_string) {
        eparam = m_operator->m_string->evaluate(trans);

        if (m_operator->m_string->containsMacro()) {
            eparam = "\"" + eparam + "\" Was: \"" \
                + m_operator->m_string->evaluate(NULL) + "\"";
        } else {
            eparam = "\"" + eparam + "\"";
        }
    ms_dbg_a(trans, 4, "(Rule: " + std::to_string(getId()) \
        + ") Executing operator \"" + getOperatorName() \
        + "\" with param " \
        + eparam \
        + " against " \
        + variables + ".");
    } else {
        ms_dbg_a(trans, 4, "(Rule: " + std::to_string(getId()) \
            + ") Executing operator \"" + getOperatorName() \
            + " against " \
            + variables + ".");
    }


    getFinalVars(&vars, &exclusion, trans);

    VariableValues e;
    for (auto &var : vars) {
        if (!var) {
            continue;
        }
        e.clear();
        var->evaluate(trans, &e);
        for (const auto &vv : e) {
            TransformationsResults transformationsResults;
            const VariableValue *v = vv.get();
            const std::string &value = v->getValue();

            if (exclusion.contains(v) ||
                std::find_if(trans->m_ruleRemoveTargetById.begin(),
                    trans->m_ruleRemoveTargetById.end(),
                    [&, v, this](std::pair<int, std::string> &m) -> bool {
                        return m.first == getId() && m.second == v->getName();
                    }) != trans->m_ruleRemoveTargetById.end()
            ) {
                continue;
            }

            if (exclusion.contains(v) ||
                std::find_if(trans->m_ruleRemoveTargetByIdRange.begin(),
                    trans->m_ruleRemoveTargetByIdRange.end(),
                    [&, v, this](std::pair<std::pair<int, int>, std::string> &m) -> bool {
                        return (m.first.first <= getId() && m.first.second >= getId()
			    && m.second == v->getName());
                    }) != trans->m_ruleRemoveTargetByIdRange.end()
            ) {
                continue;
            }

            if (exclusion.contains(v) ||
                std::find_if(trans->m_ruleRemoveTargetByTag.begin(),
                    trans->m_ruleRemoveTargetByTag.end(),
                    [&, v, trans, this](std::pair<std::string, std::string> &m) -> bool {
                        return containsTag(m.first, trans) && m.second == v->getName();
                    }) != trans->m_ruleRemoveTargetByTag.end()
            ) {
                continue;
            }

            executeTransformations(trans, value, transformationsResults);

            auto iter = transformationsResults.begin();
            if (!processMultiMatch()) {
                iter = transformationsResults.end();
                std::advance(iter, -1);
            }
            while (iter != transformationsResults.end()) {
                bool ret;
                auto &valueTemp = *iter;
                bpstd::string_view view = *valueTemp.getAfter();

                ret = executeOperatorAt(trans, v, view);

                if (ret == true) {
                    trans->messageGetLast()->m_match = m_operator->resolveMatchMessage(trans, v);

                    for (const auto &i : v->getOrigin()) {
                        trans->messageGetLast()->m_reference.append(i.toText());
                    }

                    auto iter2 = transformationsResults.begin();
                    while (iter2 != transformationsResults.end()) {
                        if (iter2->getTransformationName()) {
                            trans->messageGetLast()->m_reference.append(*iter2->getTransformationName());
                        }
                        /*
                        if (iter == iter2) {
                            break;
                        } else if (iter2->getTransformationName()) {
                            trans->messageGetLast()->m_reference.append(",");
                        }
                        */
                        iter2++;
                    }

                    updateMatchedVars(trans, vv, view);
                    executeActionsIndependentOfChainedRuleResult(trans);

                    globalRet = true;
                }

                iter++;
            }
        }
    }

    if (globalRet == false) {
        ms_dbg_a(trans, 4, "Rule returned 0.");
        cleanMatchedVars(trans);
        goto end_clean;
    }
    ms_dbg_a(trans, 4, "Rule returned 1.");

    if (this->hasChainAction() == false) {
        goto end_exec;
    }

    /* FIXME: this check should happens on the parser. */
    if (getChainedNext() == nullptr) {
        ms_dbg_a(trans, 4, "Rule is marked as chained but there " \
            "isn't a subsequent rule.");
        goto end_clean;
    }

    ms_dbg_a(trans, 4, "Executing chained rule.");
    recursiveGlobalRet = getChainedNext()->evaluate(trans);

    if (recursiveGlobalRet == true) {
        goto end_exec;
    }

end_clean:
    return false;

end_exec:
    executeActionsAfterFullMatch(trans);

    /* last rule in the chain. */
    trans->logMatchLastRuleOnTheChain(this);

    if (hasSeverity()) {
        ms_dbg_a(trans, 9, "This rule severity is: " + \
            std::to_string(getSeverity()) + " current transaction is: " + \
            std::to_string(trans->m_highestSeverityAction));

        if (trans->m_highestSeverityAction > getSeverity()) {
            trans->m_highestSeverityAction = getSeverity();
        }
    }

    return true;
}


std::string RuleWithOperator::getOperatorName() const { return m_operator->m_op; }


}  // namespace modsecurity
