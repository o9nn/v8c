// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "include/opencog/atomspace.h"

#include <algorithm>

namespace v8 {
namespace opencog {

AtomSpace::AtomSpace(const std::string& tenant_id) : tenant_id_(tenant_id) {}

std::shared_ptr<Node> AtomSpace::AddNode(AtomType type, const std::string& name) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  // Check if atom already exists
  auto it = atoms_by_name_.find(name);
  if (it != atoms_by_name_.end() && it->second->IsNode()) {
    return std::static_pointer_cast<Node>(it->second);
  }
  
  auto node = std::make_shared<Node>(type, name);
  atoms_by_id_[node->id()] = node;
  atoms_by_name_[name] = node;
  atoms_by_type_.insert({type, node});
  
  return node;
}

std::shared_ptr<Link> AtomSpace::AddLink(
    AtomType type, const std::string& name,
    const std::vector<std::shared_ptr<Atom>>& outgoing) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  auto link = std::make_shared<Link>(type, name, outgoing);
  atoms_by_id_[link->id()] = link;
  atoms_by_name_[name] = link;
  atoms_by_type_.insert({type, link});
  
  return link;
}

std::shared_ptr<Atom> AtomSpace::GetAtom(uint64_t id) const {
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = atoms_by_id_.find(id);
  return (it != atoms_by_id_.end()) ? it->second : nullptr;
}

std::shared_ptr<Atom> AtomSpace::GetAtomByName(const std::string& name) const {
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = atoms_by_name_.find(name);
  return (it != atoms_by_name_.end()) ? it->second : nullptr;
}

std::vector<std::shared_ptr<Atom>> AtomSpace::GetAtomsByType(AtomType type) const {
  std::lock_guard<std::mutex> lock(mutex_);
  std::vector<std::shared_ptr<Atom>> result;
  
  auto range = atoms_by_type_.equal_range(type);
  for (auto it = range.first; it != range.second; ++it) {
    result.push_back(it->second);
  }
  
  return result;
}

bool AtomSpace::RemoveAtom(uint64_t id) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  auto it = atoms_by_id_.find(id);
  if (it == atoms_by_id_.end()) return false;
  
  auto atom = it->second;
  atoms_by_name_.erase(atom->name());
  
  // Remove from type index
  auto range = atoms_by_type_.equal_range(atom->type());
  for (auto type_it = range.first; type_it != range.second; ++type_it) {
    if (type_it->second->id() == id) {
      atoms_by_type_.erase(type_it);
      break;
    }
  }
  
  atoms_by_id_.erase(it);
  return true;
}

void AtomSpace::Clear() {
  std::lock_guard<std::mutex> lock(mutex_);
  atoms_by_id_.clear();
  atoms_by_name_.clear();
  atoms_by_type_.clear();
}

size_t AtomSpace::Size() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return atoms_by_id_.size();
}

std::vector<std::shared_ptr<Atom>> AtomSpace::Query(
    const std::function<bool(const std::shared_ptr<Atom>&)>& predicate) const {
  std::lock_guard<std::mutex> lock(mutex_);
  std::vector<std::shared_ptr<Atom>> result;
  
  for (const auto& pair : atoms_by_id_) {
    if (predicate(pair.second)) {
      result.push_back(pair.second);
    }
  }
  
  return result;
}

// AtomSpaceManager implementation
AtomSpaceManager* AtomSpaceManager::GetInstance() {
  static AtomSpaceManager instance;
  return &instance;
}

std::shared_ptr<AtomSpace> AtomSpaceManager::GetOrCreateAtomSpace(
    const std::string& tenant_id) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  auto it = atomspaces_.find(tenant_id);
  if (it != atomspaces_.end()) {
    return it->second;
  }
  
  auto atomspace = std::make_shared<AtomSpace>(tenant_id);
  atomspaces_[tenant_id] = atomspace;
  return atomspace;
}

std::shared_ptr<AtomSpace> AtomSpaceManager::GetAtomSpace(
    const std::string& tenant_id) const {
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = atomspaces_.find(tenant_id);
  return (it != atomspaces_.end()) ? it->second : nullptr;
}

bool AtomSpaceManager::RemoveAtomSpace(const std::string& tenant_id) {
  std::lock_guard<std::mutex> lock(mutex_);
  return atomspaces_.erase(tenant_id) > 0;
}

std::vector<std::string> AtomSpaceManager::GetTenantIds() const {
  std::lock_guard<std::mutex> lock(mutex_);
  std::vector<std::string> ids;
  ids.reserve(atomspaces_.size());
  
  for (const auto& pair : atomspaces_) {
    ids.push_back(pair.first);
  }
  
  return ids;
}

size_t AtomSpaceManager::TenantCount() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return atomspaces_.size();
}

}  // namespace opencog
}  // namespace v8
