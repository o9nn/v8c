// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "include/opencog/agent-orchestrator.h"

#include <algorithm>
#include <chrono>
#include <thread>

namespace v8 {
namespace opencog {

AgentOrchestrator::AgentOrchestrator() : running_(false) {}

AgentOrchestrator::~AgentOrchestrator() {
  Stop();
}

bool AgentOrchestrator::RegisterAgent(std::shared_ptr<Agent> agent) {
  if (!agent) return false;

  std::lock_guard<std::mutex> lock(agents_mutex_);
  
  if (agents_.find(agent->agent_id()) != agents_.end()) {
    return false; // Agent already registered
  }

  agent->set_orchestrator(this);
  agents_[agent->agent_id()] = agent;
  return agent->Initialize();
}

bool AgentOrchestrator::UnregisterAgent(const std::string& agent_id) {
  std::lock_guard<std::mutex> lock(agents_mutex_);
  
  auto it = agents_.find(agent_id);
  if (it == agents_.end()) return false;

  it->second->Shutdown();
  agents_.erase(it);
  return true;
}

std::shared_ptr<Agent> AgentOrchestrator::GetAgent(
    const std::string& agent_id) const {
  std::lock_guard<std::mutex> lock(agents_mutex_);
  auto it = agents_.find(agent_id);
  return (it != agents_.end()) ? it->second : nullptr;
}

std::vector<std::shared_ptr<Agent>> AgentOrchestrator::GetAgentsByTenant(
    const std::string& tenant_id) const {
  std::lock_guard<std::mutex> lock(agents_mutex_);
  std::vector<std::shared_ptr<Agent>> result;
  
  for (const auto& pair : agents_) {
    if (pair.second->tenant_id() == tenant_id) {
      result.push_back(pair.second);
    }
  }
  
  return result;
}

void AgentOrchestrator::Start() {
  if (running_) return;
  
  running_ = true;
  orchestrator_thread_ = std::thread(&AgentOrchestrator::OrchestratorLoop, this);
}

void AgentOrchestrator::Stop() {
  if (!running_) return;
  
  running_ = false;
  if (orchestrator_thread_.joinable()) {
    orchestrator_thread_.join();
  }
}

void AgentOrchestrator::RouteMessage(const AgentMessage& message) {
  std::lock_guard<std::mutex> lock(messages_mutex_);
  message_queue_.push(message);
}

void AgentOrchestrator::BroadcastMessage(const std::string& from_agent_id,
                                          const std::string& type,
                                          const std::string& payload) {
  std::lock_guard<std::mutex> lock(agents_mutex_);
  
  for (const auto& pair : agents_) {
    if (pair.first != from_agent_id) {
      AgentMessage message;
      message.from_agent_id = from_agent_id;
      message.to_agent_id = pair.first;
      message.type = type;
      message.payload = payload;
      message.timestamp = 
          std::chrono::system_clock::now().time_since_epoch().count();
      RouteMessage(message);
    }
  }
}

void AgentOrchestrator::ScheduleAgent(const std::string& agent_id) {
  std::lock_guard<std::mutex> lock(messages_mutex_);
  scheduled_agents_.push(agent_id);
}

void AgentOrchestrator::OrchestratorLoop() {
  while (running_) {
    ProcessMessageQueue();
    
    // Execute scheduled agents
    std::string agent_id;
    {
      std::lock_guard<std::mutex> lock(messages_mutex_);
      if (!scheduled_agents_.empty()) {
        agent_id = scheduled_agents_.front();
        scheduled_agents_.pop();
      }
    }
    
    if (!agent_id.empty()) {
      auto agent = GetAgent(agent_id);
      if (agent && agent->state() == AgentState::IDLE) {
        agent->set_state(AgentState::RUNNING);
        try {
          agent->Execute();
          agent->set_state(AgentState::IDLE);
        } catch (...) {
          agent->set_state(AgentState::FAILED);
        }
      }
    }
    
    // Small delay to prevent busy-waiting
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

void AgentOrchestrator::ProcessMessageQueue() {
  std::vector<AgentMessage> messages;
  
  {
    std::lock_guard<std::mutex> lock(messages_mutex_);
    while (!message_queue_.empty()) {
      messages.push_back(message_queue_.front());
      message_queue_.pop();
    }
  }
  
  for (const auto& message : messages) {
    auto agent = GetAgent(message.to_agent_id);
    if (agent) {
      agent->OnMessage(message);
    }
  }
}

}  // namespace opencog
}  // namespace v8
