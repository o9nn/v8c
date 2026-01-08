// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_OPENCOG_AGENT_ORCHESTRATOR_H_
#define V8_OPENCOG_AGENT_ORCHESTRATOR_H_

#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include "include/opencog/agent.h"

namespace v8 {
namespace opencog {

// Multi-agent orchestration workbench
class AgentOrchestrator {
 public:
  AgentOrchestrator();
  ~AgentOrchestrator();

  // Agent management
  bool RegisterAgent(std::shared_ptr<Agent> agent);
  bool UnregisterAgent(const std::string& agent_id);
  std::shared_ptr<Agent> GetAgent(const std::string& agent_id) const;
  std::vector<std::shared_ptr<Agent>> GetAgentsByTenant(
      const std::string& tenant_id) const;

  // Orchestration control
  void Start();
  void Stop();
  bool IsRunning() const { return running_; }

  // Message routing
  void RouteMessage(const AgentMessage& message);
  void BroadcastMessage(const std::string& from_agent_id,
                        const std::string& type,
                        const std::string& payload);

  // Agent execution
  void ScheduleAgent(const std::string& agent_id);

 private:
  void OrchestratorLoop();
  void ProcessMessageQueue();

  bool running_;
  std::thread orchestrator_thread_;
  mutable std::mutex agents_mutex_;
  mutable std::mutex messages_mutex_;
  
  std::map<std::string, std::shared_ptr<Agent>> agents_;
  std::queue<AgentMessage> message_queue_;
  std::queue<std::string> scheduled_agents_;
};

}  // namespace opencog
}  // namespace v8

#endif  // V8_OPENCOG_AGENT_ORCHESTRATOR_H_
