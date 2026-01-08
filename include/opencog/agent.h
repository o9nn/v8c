// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_OPENCOG_AGENT_H_
#define V8_OPENCOG_AGENT_H_

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "include/opencog/atomspace.h"

namespace v8 {
namespace opencog {

// Forward declarations
class AgentOrchestrator;

// Agent states
enum class AgentState {
  IDLE,
  RUNNING,
  PAUSED,
  COMPLETED,
  FAILED
};

// Message for inter-agent communication
struct AgentMessage {
  std::string from_agent_id;
  std::string to_agent_id;
  std::string type;
  std::string payload;
  uint64_t timestamp;
};

// Base Agent class for autonomous behavior
class Agent {
 public:
  Agent(const std::string& agent_id, const std::string& tenant_id);
  virtual ~Agent() = default;

  // Agent lifecycle
  virtual bool Initialize();
  virtual void Execute() = 0;
  virtual void Shutdown();

  // Message handling
  virtual void OnMessage(const AgentMessage& message);
  void SendMessage(const std::string& to_agent_id, const std::string& type,
                   const std::string& payload);

  // State management
  AgentState state() const { return state_; }
  void set_state(AgentState state) { state_ = state; }

  // Agent properties
  const std::string& agent_id() const { return agent_id_; }
  const std::string& tenant_id() const { return tenant_id_; }
  std::shared_ptr<AtomSpace> atomspace() const { return atomspace_; }

  void set_orchestrator(AgentOrchestrator* orchestrator) {
    orchestrator_ = orchestrator;
  }

 protected:
  std::string agent_id_;
  std::string tenant_id_;
  AgentState state_;
  std::shared_ptr<AtomSpace> atomspace_;
  AgentOrchestrator* orchestrator_;
};

// Agent factory for creating different agent types
class AgentFactory {
 public:
  using AgentCreator = std::function<std::shared_ptr<Agent>(
      const std::string& agent_id, const std::string& tenant_id)>;

  static AgentFactory* GetInstance();

  void RegisterAgentType(const std::string& type, AgentCreator creator);
  std::shared_ptr<Agent> CreateAgent(const std::string& type,
                                      const std::string& agent_id,
                                      const std::string& tenant_id);

 private:
  AgentFactory() = default;
  std::map<std::string, AgentCreator> creators_;
};

}  // namespace opencog
}  // namespace v8

#endif  // V8_OPENCOG_AGENT_H_
