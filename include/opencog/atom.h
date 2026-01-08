// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_OPENCOG_ATOM_H_
#define V8_OPENCOG_ATOM_H_

#include <memory>
#include <string>
#include <vector>
#include <cstdint>

namespace v8 {
namespace opencog {

// Forward declarations
class AtomSpace;

// Atom types for neuro-symbolic representation
enum class AtomType {
  NODE,           // Basic node atom
  LINK,           // Link connecting atoms
  CONCEPT_NODE,   // Represents a concept
  PREDICATE_NODE, // Represents a predicate
  VARIABLE_NODE,  // Represents a variable
  EVALUATION_LINK,// Evaluation of a predicate
  INHERITANCE_LINK,// Inheritance relationship
  SIMILARITY_LINK, // Similarity relationship
  EXECUTION_LINK   // Execution context
};

// Truth value for probabilistic reasoning
struct TruthValue {
  double strength;    // Probability [0, 1]
  double confidence;  // Confidence [0, 1]
  
  TruthValue() : strength(1.0), confidence(1.0) {}
  TruthValue(double s, double c) : strength(s), confidence(c) {}
};

// Base Atom class
class Atom {
 public:
  Atom(AtomType type, const std::string& name);
  virtual ~Atom() = default;

  AtomType type() const { return type_; }
  const std::string& name() const { return name_; }
  uint64_t id() const { return id_; }
  
  const TruthValue& truth_value() const { return truth_value_; }
  void set_truth_value(const TruthValue& tv) { truth_value_ = tv; }

  virtual bool IsNode() const { return false; }
  virtual bool IsLink() const { return false; }

 protected:
  AtomType type_;
  std::string name_;
  uint64_t id_;
  TruthValue truth_value_;

 private:
  static uint64_t next_id_;
};

// Node atoms represent concepts, predicates, or variables
class Node : public Atom {
 public:
  Node(AtomType type, const std::string& name);
  ~Node() override = default;

  bool IsNode() const override { return true; }
};

// Link atoms connect other atoms
class Link : public Atom {
 public:
  Link(AtomType type, const std::string& name, 
       const std::vector<std::shared_ptr<Atom>>& outgoing);
  ~Link() override = default;

  bool IsLink() const override { return true; }
  const std::vector<std::shared_ptr<Atom>>& outgoing() const { 
    return outgoing_; 
  }

 private:
  std::vector<std::shared_ptr<Atom>> outgoing_;
};

}  // namespace opencog
}  // namespace v8

#endif  // V8_OPENCOG_ATOM_H_
