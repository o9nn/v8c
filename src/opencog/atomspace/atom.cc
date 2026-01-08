// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "include/opencog/atom.h"

namespace v8 {
namespace opencog {

uint64_t Atom::next_id_ = 1;

Atom::Atom(AtomType type, const std::string& name)
    : type_(type), name_(name), id_(next_id_++) {}

Node::Node(AtomType type, const std::string& name) : Atom(type, name) {}

Link::Link(AtomType type, const std::string& name,
           const std::vector<std::shared_ptr<Atom>>& outgoing)
    : Atom(type, name), outgoing_(outgoing) {}

}  // namespace opencog
}  // namespace v8
