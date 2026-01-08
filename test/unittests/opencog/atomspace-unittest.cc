// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "include/opencog/atomspace.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace v8 {
namespace opencog {
namespace {

TEST(AtomSpaceTest, CreateAndRetrieveNode) {
  AtomSpace atomspace("test-tenant");
  
  auto node = atomspace.AddNode(AtomType::CONCEPT_NODE, "TestConcept");
  ASSERT_NE(node, nullptr);
  EXPECT_EQ(node->name(), "TestConcept");
  EXPECT_EQ(node->type(), AtomType::CONCEPT_NODE);
  EXPECT_TRUE(node->IsNode());
  
  // Retrieve by ID
  auto retrieved = atomspace.GetAtom(node->id());
  ASSERT_NE(retrieved, nullptr);
  EXPECT_EQ(retrieved->id(), node->id());
  
  // Retrieve by name
  auto by_name = atomspace.GetAtomByName("TestConcept");
  ASSERT_NE(by_name, nullptr);
  EXPECT_EQ(by_name->name(), "TestConcept");
}

TEST(AtomSpaceTest, CreateLink) {
  AtomSpace atomspace("test-tenant");
  
  auto node1 = atomspace.AddNode(AtomType::CONCEPT_NODE, "Concept1");
  auto node2 = atomspace.AddNode(AtomType::CONCEPT_NODE, "Concept2");
  
  std::vector<std::shared_ptr<Atom>> outgoing = {node1, node2};
  auto link = atomspace.AddLink(AtomType::INHERITANCE_LINK, "InheritanceLink", outgoing);
  
  ASSERT_NE(link, nullptr);
  EXPECT_TRUE(link->IsLink());
  EXPECT_EQ(link->outgoing().size(), 2u);
  EXPECT_EQ(link->outgoing()[0]->name(), "Concept1");
  EXPECT_EQ(link->outgoing()[1]->name(), "Concept2");
}

TEST(AtomSpaceTest, TruthValues) {
  AtomSpace atomspace("test-tenant");
  
  auto node = atomspace.AddNode(AtomType::CONCEPT_NODE, "TestConcept");
  
  TruthValue tv(0.8, 0.9);
  node->set_truth_value(tv);
  
  const auto& retrieved_tv = node->truth_value();
  EXPECT_DOUBLE_EQ(retrieved_tv.strength, 0.8);
  EXPECT_DOUBLE_EQ(retrieved_tv.confidence, 0.9);
}

TEST(AtomSpaceTest, GetAtomsByType) {
  AtomSpace atomspace("test-tenant");
  
  atomspace.AddNode(AtomType::CONCEPT_NODE, "Concept1");
  atomspace.AddNode(AtomType::CONCEPT_NODE, "Concept2");
  atomspace.AddNode(AtomType::PREDICATE_NODE, "Predicate1");
  
  auto concepts = atomspace.GetAtomsByType(AtomType::CONCEPT_NODE);
  EXPECT_EQ(concepts.size(), 2u);
  
  auto predicates = atomspace.GetAtomsByType(AtomType::PREDICATE_NODE);
  EXPECT_EQ(predicates.size(), 1u);
}

TEST(AtomSpaceTest, RemoveAtom) {
  AtomSpace atomspace("test-tenant");
  
  auto node = atomspace.AddNode(AtomType::CONCEPT_NODE, "TestConcept");
  uint64_t id = node->id();
  
  EXPECT_EQ(atomspace.Size(), 1u);
  
  bool removed = atomspace.RemoveAtom(id);
  EXPECT_TRUE(removed);
  EXPECT_EQ(atomspace.Size(), 0u);
  
  auto retrieved = atomspace.GetAtom(id);
  EXPECT_EQ(retrieved, nullptr);
}

TEST(AtomSpaceTest, Query) {
  AtomSpace atomspace("test-tenant");
  
  auto node1 = atomspace.AddNode(AtomType::CONCEPT_NODE, "Concept1");
  auto node2 = atomspace.AddNode(AtomType::CONCEPT_NODE, "Concept2");
  
  node1->set_truth_value(TruthValue(0.9, 0.8));
  node2->set_truth_value(TruthValue(0.5, 0.6));
  
  // Query for atoms with high strength
  auto results = atomspace.Query([](const std::shared_ptr<Atom>& atom) {
    return atom->truth_value().strength > 0.7;
  });
  
  EXPECT_EQ(results.size(), 1u);
  EXPECT_EQ(results[0]->name(), "Concept1");
}

TEST(AtomSpaceManagerTest, MultiTenant) {
  auto manager = AtomSpaceManager::GetInstance();
  
  auto atomspace1 = manager->GetOrCreateAtomSpace("tenant1");
  auto atomspace2 = manager->GetOrCreateAtomSpace("tenant2");
  
  ASSERT_NE(atomspace1, nullptr);
  ASSERT_NE(atomspace2, nullptr);
  EXPECT_NE(atomspace1, atomspace2);
  EXPECT_EQ(atomspace1->tenant_id(), "tenant1");
  EXPECT_EQ(atomspace2->tenant_id(), "tenant2");
  
  atomspace1->AddNode(AtomType::CONCEPT_NODE, "Concept1");
  atomspace2->AddNode(AtomType::CONCEPT_NODE, "Concept2");
  
  EXPECT_EQ(atomspace1->Size(), 1u);
  EXPECT_EQ(atomspace2->Size(), 1u);
  
  // Verify isolation
  auto node1 = atomspace1->GetAtomByName("Concept2");
  EXPECT_EQ(node1, nullptr);
  
  auto node2 = atomspace2->GetAtomByName("Concept1");
  EXPECT_EQ(node2, nullptr);
  
  // Cleanup
  manager->RemoveAtomSpace("tenant1");
  manager->RemoveAtomSpace("tenant2");
}

TEST(AtomSpaceManagerTest, GetTenantIds) {
  auto manager = AtomSpaceManager::GetInstance();
  
  manager->GetOrCreateAtomSpace("tenant_a");
  manager->GetOrCreateAtomSpace("tenant_b");
  manager->GetOrCreateAtomSpace("tenant_c");
  
  auto tenant_ids = manager->GetTenantIds();
  EXPECT_GE(tenant_ids.size(), 3u);
  
  // Cleanup
  manager->RemoveAtomSpace("tenant_a");
  manager->RemoveAtomSpace("tenant_b");
  manager->RemoveAtomSpace("tenant_c");
}

}  // namespace
}  // namespace opencog
}  // namespace v8
