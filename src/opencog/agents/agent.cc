// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "include/opencog/agent.h"
#include "include/opencog/agent-orchestrator.h"

#include <chrono>

namespace v8 {
namespace opencog {

Agent::Agent(const std::string& agent_id, const std::string& tenant_id)
    : agent_id_(agent_id),
      tenant_id_(tenant_id),
      state_(AgentState::IDLE),
      orchestrator_(nullptr) {
  atomspace_ = AtomSpaceManager::GetInstance()->GetOrCreateAtomSpace(tenant_id);
}

bool Agent::Initialize() {
  state_ = AgentState::IDLE;
  return true;
}

void Agent::Shutdown() {
  state_ = AgentState::COMPLETED;
}

void Agent::OnMessage(const AgentMessage& message) {
  // Default implementation - subclasses should override
}

void Agent::SendMessage(const std::string& to_agent_id,
                        const std::string& type,
                        const std::string& payload) {
  if (orchestrator_) {
    AgentMessage message;
    message.from_agent_id = agent_id_;
    message.to_agent_id = to_agent_id;
    message.type = type;
    message.payload = payload;
    message.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    orchestrator_->RouteMessage(message);
  }
}

// AgentFactory implementation
AgentFactory* AgentFactory::GetInstance() {
  static AgentFactory instance;
  return &instance;
}

void AgentFactory::RegisterAgentType(const std::string& type,
                                      AgentCreator creator) {
  creators_[type] = creator;
}

std::shared_ptr<Agent> AgentFactory::CreateAgent(const std::string& type,
                                                  const std::string& agent_id,
                                                  const std::string& tenant_id) {
  auto it = creators_.find(type);
  if (it != creators_.end()) {
    return it->second(agent_id, tenant_id);
  }
  return nullptr;
}

}  // namespace opencog
}  // namespace v8
