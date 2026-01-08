// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "include/opencog/agent.h"
#include "include/opencog/agent-orchestrator.h"
#include "testing/gtest/include/gtest/gtest.h"

#include <chrono>
#include <thread>

namespace v8 {
namespace opencog {
namespace {

class TestAgent : public Agent {
 public:
  TestAgent(const std::string& agent_id, const std::string& tenant_id)
      : Agent(agent_id, tenant_id), execute_count_(0) {}

  void Execute() override {
    execute_count_++;
  }

  void OnMessage(const AgentMessage& message) override {
    received_messages_.push_back(message);
  }

  int execute_count() const { return execute_count_; }
  const std::vector<AgentMessage>& received_messages() const {
    return received_messages_;
  }

 private:
  int execute_count_;
  std::vector<AgentMessage> received_messages_;
};

TEST(AgentTest, BasicProperties) {
  TestAgent agent("agent1", "tenant1");
  
  EXPECT_EQ(agent.agent_id(), "agent1");
  EXPECT_EQ(agent.tenant_id(), "tenant1");
  EXPECT_EQ(agent.state(), AgentState::IDLE);
  EXPECT_NE(agent.atomspace(), nullptr);
  EXPECT_EQ(agent.atomspace()->tenant_id(), "tenant1");
}

TEST(AgentTest, Initialization) {
  TestAgent agent("agent1", "tenant1");
  
  EXPECT_TRUE(agent.Initialize());
  EXPECT_EQ(agent.state(), AgentState::IDLE);
}

TEST(AgentOrchestratorTest, RegisterAgent) {
  AgentOrchestrator orchestrator;
  auto agent = std::make_shared<TestAgent>("agent1", "tenant1");
  
  bool registered = orchestrator.RegisterAgent(agent);
  EXPECT_TRUE(registered);
  
  auto retrieved = orchestrator.GetAgent("agent1");
  ASSERT_NE(retrieved, nullptr);
  EXPECT_EQ(retrieved->agent_id(), "agent1");
}

TEST(AgentOrchestratorTest, UnregisterAgent) {
  AgentOrchestrator orchestrator;
  auto agent = std::make_shared<TestAgent>("agent1", "tenant1");
  
  orchestrator.RegisterAgent(agent);
  EXPECT_NE(orchestrator.GetAgent("agent1"), nullptr);
  
  bool unregistered = orchestrator.UnregisterAgent("agent1");
  EXPECT_TRUE(unregistered);
  EXPECT_EQ(orchestrator.GetAgent("agent1"), nullptr);
}

TEST(AgentOrchestratorTest, GetAgentsByTenant) {
  AgentOrchestrator orchestrator;
  
  orchestrator.RegisterAgent(std::make_shared<TestAgent>("agent1", "tenant1"));
  orchestrator.RegisterAgent(std::make_shared<TestAgent>("agent2", "tenant1"));
  orchestrator.RegisterAgent(std::make_shared<TestAgent>("agent3", "tenant2"));
  
  auto tenant1_agents = orchestrator.GetAgentsByTenant("tenant1");
  EXPECT_EQ(tenant1_agents.size(), 2u);
  
  auto tenant2_agents = orchestrator.GetAgentsByTenant("tenant2");
  EXPECT_EQ(tenant2_agents.size(), 1u);
}

TEST(AgentOrchestratorTest, StartStop) {
  AgentOrchestrator orchestrator;
  
  EXPECT_FALSE(orchestrator.IsRunning());
  
  orchestrator.Start();
  EXPECT_TRUE(orchestrator.IsRunning());
  
  orchestrator.Stop();
  EXPECT_FALSE(orchestrator.IsRunning());
}

TEST(AgentOrchestratorTest, ScheduleAndExecute) {
  AgentOrchestrator orchestrator;
  auto agent = std::make_shared<TestAgent>("agent1", "tenant1");
  
  orchestrator.RegisterAgent(agent);
  orchestrator.Start();
  
  EXPECT_EQ(agent->execute_count(), 0);
  
  orchestrator.ScheduleAgent("agent1");
  
  // Wait for execution
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  
  EXPECT_GT(agent->execute_count(), 0);
  
  orchestrator.Stop();
}

TEST(AgentOrchestratorTest, MessageRouting) {
  AgentOrchestrator orchestrator;
  auto agent1 = std::make_shared<TestAgent>("agent1", "tenant1");
  auto agent2 = std::make_shared<TestAgent>("agent2", "tenant1");
  
  orchestrator.RegisterAgent(agent1);
  orchestrator.RegisterAgent(agent2);
  orchestrator.Start();
  
  agent1->SendMessage("agent2", "test", "Hello!");
  
  // Wait for message processing
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  
  const auto& messages = agent2->received_messages();
  EXPECT_EQ(messages.size(), 1u);
  if (!messages.empty()) {
    EXPECT_EQ(messages[0].from_agent_id, "agent1");
    EXPECT_EQ(messages[0].to_agent_id, "agent2");
    EXPECT_EQ(messages[0].type, "test");
    EXPECT_EQ(messages[0].payload, "Hello!");
  }
  
  orchestrator.Stop();
}

TEST(AgentOrchestratorTest, BroadcastMessage) {
  AgentOrchestrator orchestrator;
  auto agent1 = std::make_shared<TestAgent>("agent1", "tenant1");
  auto agent2 = std::make_shared<TestAgent>("agent2", "tenant1");
  auto agent3 = std::make_shared<TestAgent>("agent3", "tenant1");
  
  orchestrator.RegisterAgent(agent1);
  orchestrator.RegisterAgent(agent2);
  orchestrator.RegisterAgent(agent3);
  orchestrator.Start();
  
  orchestrator.BroadcastMessage("agent1", "broadcast", "Hello everyone!");
  
  // Wait for message processing
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  
  // agent1 should not receive its own broadcast
  EXPECT_EQ(agent1->received_messages().size(), 0u);
  
  // agent2 and agent3 should receive the broadcast
  EXPECT_EQ(agent2->received_messages().size(), 1u);
  EXPECT_EQ(agent3->received_messages().size(), 1u);
  
  orchestrator.Stop();
}

TEST(AgentFactoryTest, RegisterAndCreate) {
  auto factory = AgentFactory::GetInstance();
  
  factory->RegisterAgentType("test_agent",
      [](const std::string& id, const std::string& tenant) {
        return std::make_shared<TestAgent>(id, tenant);
      });
  
  auto agent = factory->CreateAgent("test_agent", "agent1", "tenant1");
  ASSERT_NE(agent, nullptr);
  EXPECT_EQ(agent->agent_id(), "agent1");
  EXPECT_EQ(agent->tenant_id(), "tenant1");
}

}  // namespace
}  // namespace opencog
}  // namespace v8
