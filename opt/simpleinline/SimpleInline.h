/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <map>
#include <set>

#include "DexClass.h"
#include "IRCode.h"
#include "Inliner.h"
#include "Pass.h"
#include "Resolver.h"

class SimpleInlinePass : public Pass {
 public:
  SimpleInlinePass() : Pass("SimpleInlinePass") {}

  virtual void configure_pass(const JsonWrapper& jw) override {
    jw.get("virtual", true, m_virtual_inline);
    jw.get("throws", false, m_inliner_config.throws_inline);
    jw.get("enforce_method_size_limit",
           true,
           m_inliner_config.enforce_method_size_limit);
    jw.get("no_inline_annos", {}, m_no_inline_annos);
    jw.get("force_inline_annos", {}, m_force_inline_annos);
    jw.get("multiple_callers", false, m_multiple_callers);

    std::vector<std::string> black_list;
    jw.get("black_list", {}, black_list);
    for (const auto& type_s : black_list) {
      m_inliner_config.black_list.emplace(DexType::make_type(type_s.c_str()));
    }

    std::vector<std::string> caller_black_list;
    jw.get("caller_black_list", {}, caller_black_list);
    for (const auto& type_s : caller_black_list) {
      m_inliner_config.caller_black_list.emplace(
          DexType::make_type(type_s.c_str()));
    }
  }

  virtual void run_pass(DexStoresVector&, ConfigFiles&, PassManager&) override;

 private:
  std::unordered_set<DexMethod*> gather_non_virtual_methods(
      Scope& scope,
      const std::unordered_set<DexType*>& no_inline,
      const std::unordered_set<DexType*>& force_inline);

 private:
  // count of instructions that define a method as inlinable always
  static const size_t SMALL_CODE_SIZE = 3;

  // inline virtual methods
  bool m_virtual_inline;
  // inline methods with multiple callers
  bool m_multiple_callers;

  MultiMethodInliner::Config m_inliner_config;

  // annotations indicating not to inline a function
  std::vector<std::string> m_no_inline_annos;

  // annotations indicating to always inline a function
  std::vector<std::string> m_force_inline_annos;

  // set of inlinable methods
  std::unordered_set<DexMethod*> inlinable;

  // keep a map from refs to defs or nullptr if no method was found
  MethodRefCache resolved_refs;
};
