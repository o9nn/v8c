// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_OPENCOG_ATOMSPACE_H_
#define V8_OPENCOG_ATOMSPACE_H_

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <unordered_map>

#include "include/opencog/atom.h"

namespace v8 {
namespace opencog {

// Multi-tenant AtomSpace for neuro-symbolic knowledge representation
class AtomSpace {
 public:
  explicit AtomSpace(const std::string& tenant_id);
  ~AtomSpace() = default;

  // Atom management
  std::shared_ptr<Node> AddNode(AtomType type, const std::string& name);
  std::shared_ptr<Link> AddLink(AtomType type, const std::string& name,
                                 const std::vector<std::shared_ptr<Atom>>& outgoing);
  
  std::shared_ptr<Atom> GetAtom(uint64_t id) const;
  std::shared_ptr<Atom> GetAtomByName(const std::string& name) const;
  std::vector<std::shared_ptr<Atom>> GetAtomsByType(AtomType type) const;
  
  bool RemoveAtom(uint64_t id);
  void Clear();
  
  size_t Size() const;
  const std::string& tenant_id() const { return tenant_id_; }

  // Pattern matching and queries
  std::vector<std::shared_ptr<Atom>> Query(
      const std::function<bool(const std::shared_ptr<Atom>&)>& predicate) const;

 private:
  std::string tenant_id_;
  std::unordered_map<uint64_t, std::shared_ptr<Atom>> atoms_by_id_;
  std::unordered_map<std::string, std::shared_ptr<Atom>> atoms_by_name_;
  std::multimap<AtomType, std::shared_ptr<Atom>> atoms_by_type_;
  mutable std::mutex mutex_;
};

// Global multi-tenant AtomSpace manager
class AtomSpaceManager {
 public:
  static AtomSpaceManager* GetInstance();

  std::shared_ptr<AtomSpace> GetOrCreateAtomSpace(const std::string& tenant_id);
  std::shared_ptr<AtomSpace> GetAtomSpace(const std::string& tenant_id) const;
  bool RemoveAtomSpace(const std::string& tenant_id);
  
  std::vector<std::string> GetTenantIds() const;
  size_t TenantCount() const;

 private:
  AtomSpaceManager() = default;
  ~AtomSpaceManager() = default;
  
  mutable std::mutex mutex_;
  std::unordered_map<std::string, std::shared_ptr<AtomSpace>> atomspaces_;
};

}  // namespace opencog
}  // namespace v8

#endif  // V8_OPENCOG_ATOMSPACE_H_
