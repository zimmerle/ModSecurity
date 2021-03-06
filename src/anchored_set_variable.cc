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

#include <ctime>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "modsecurity/anchored_set_variable.h"
#include "modsecurity/modsecurity.h"
#include "modsecurity/transaction.h"
#include "src/utils/regex.h"
#include "src/variables/variable.h"

namespace modsecurity {


AnchoredSetVariable::AnchoredSetVariable(Transaction *t,
    const std::string &name)
    : m_transaction(t),
    m_name(name) {
        reserve(10);
    }


void AnchoredSetVariable::unset() {
    clear();
}


// FIXME: It may not be necessary to copy the content of
void AnchoredSetVariable::set(const std::string &key,
    const std::string &value, size_t offset, size_t len) {
    auto var = std::make_shared<VariableValue>(&m_name,
        std::unique_ptr<std::string>(new std::string(key)),
        std::unique_ptr<std::string>(new std::string(value)));

    VariableOrigin origin;
    origin.m_offset = offset;
    origin.m_length = len;

    var->addOrigin(std::move(origin));
    emplace(key, std::move(var));
}


void AnchoredSetVariable::set(const std::string &key,
    const bpstd::string_view &value, size_t offset) {
    auto var = std::make_shared<VariableValue>(&m_name, &key, &value);

    VariableOrigin origin;
    origin.m_offset = offset;
    origin.m_length = value.size();

    var->addOrigin(std::move(origin));
    emplace(key, var);
}


void AnchoredSetVariable::resolve(
    VariableValues *l) const noexcept {
    for (const auto& x : *this) {
        l->insert(l->begin(), x.second);
    }
}


void AnchoredSetVariable::resolve(
    VariableValues *l,
    const variables::KeyExclusions &ke) const noexcept {
    for (const auto& x : *this) {
        if (!ke.toOmit(x.first)) {
            l->insert(l->begin(), x.second);
        } else {
            ms_dbg_a(m_transaction, 7, "Excluding key: " + x.first
                + " from target value.");
        }
    }
}


void AnchoredSetVariable::resolve(const std::string &key,
    VariableValues *l) const noexcept {
    auto range = this->equal_range(key);
    for (auto it = range.first; it != range.second; ++it) {
        l->push_back(it->second);
    }
}


std::unique_ptr<std::string> AnchoredSetVariable::resolveFirst(
    const std::string &key) const noexcept {
    auto range = equal_range(key);
    for (auto it = range.first; it != range.second; ++it) {
        std::unique_ptr<std::string> b(new std::string());
        b->assign(it->second->getValue());
        return b;
    }
    return nullptr;
}


void AnchoredSetVariable::resolveRegularExpression(const Utils::Regex *r,
    VariableValues *l) const noexcept {
    for (const auto& x : *this) {
        int ret = Utils::regex_search(x.first, *r);
        if (ret <= 0) {
            continue;
        }
        l->insert(l->begin(), x.second);
    }
}


void AnchoredSetVariable::resolveRegularExpression(const Utils::Regex *r,
    VariableValues *l,
    const variables::KeyExclusions &ke) const noexcept {
    for (const auto& x : *this) {
        int ret = Utils::regex_search(x.first, *r);
        if (ret <= 0) {
            continue;
        }
        if (!ke.toOmit(x.first)) {
            l->insert(l->begin(), x.second);
        } else {
            ms_dbg_a(m_transaction, 7, "Excluding key: " + x.first
                + " from target value.");
        }
    }
}


}  // namespace modsecurity
