// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>
#include <memory>

#include "include/libplatform/libplatform.h"
#include "include/v8-initialization.h"
#include "include/opencog/atomspace.h"
#include "include/opencog/agent.h"
#include "include/opencog/agent-orchestrator.h"
#include "include/opencog/isolate-mesh.h"

using namespace v8::opencog;

// Example cognitive agent
class CognitiveAgent : public Agent {
 public:
  CognitiveAgent(const std::string& agent_id, const std::string& tenant_id)
      : Agent(agent_id, tenant_id) {}

  void Execute() override {
    // Add some knowledge to the AtomSpace
    auto concept = atomspace_->AddNode(AtomType::CONCEPT_NODE, "TestConcept");
    concept->set_truth_value(TruthValue(0.9, 0.8));
    
    std::cout << "Agent " << agent_id_ << " executed. AtomSpace size: "
              << atomspace_->Size() << std::endl;
  }

  void OnMessage(const AgentMessage& message) override {
    std::cout << "Agent " << agent_id_ << " received message from "
              << message.from_agent_id << ": " << message.payload << std::endl;
  }
};

int main(int argc, char* argv[]) {
  // Initialize V8
  v8::V8::InitializeICUDefaultLocation(argv[0]);
  v8::V8::InitializeExternalStartupData(argv[0]);
  std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
  v8::V8::InitializePlatform(platform.get());
  v8::V8::Initialize();

  std::cout << "OpenCog Multi-Tenant Neuro-Symbolic Architecture Demo" << std::endl;
  std::cout << "======================================================" << std::endl;

  // Initialize IsolateMesh
  IsolateMesh::InitializePlatform(platform.get());
  auto mesh = std::make_shared<IsolateMesh>();

  // Create agent orchestrator
  auto orchestrator = std::make_shared<AgentOrchestrator>();
  mesh->SetAgentOrchestrator(orchestrator);

  // Create tenant isolates
  IsolateConfig config;
  config.heap_size_limit = 256 * 1024 * 1024; // 256 MB
  
  std::cout << "\nCreating tenant isolates..." << std::endl;
  auto tenant1 = mesh->CreateTenantIsolate("tenant1", config);
  auto tenant2 = mesh->CreateTenantIsolate("tenant2", config);
  
  std::cout << "Created " << mesh->TenantCount() << " tenant isolates" << std::endl;

  // Create and register agents
  std::cout << "\nRegistering cognitive agents..." << std::endl;
  auto agent1 = std::make_shared<CognitiveAgent>("agent1", "tenant1");
  auto agent2 = std::make_shared<CognitiveAgent>("agent2", "tenant1");
  auto agent3 = std::make_shared<CognitiveAgent>("agent3", "tenant2");

  orchestrator->RegisterAgent(agent1);
  orchestrator->RegisterAgent(agent2);
  orchestrator->RegisterAgent(agent3);

  // Start orchestrator
  std::cout << "Starting agent orchestrator..." << std::endl;
  orchestrator->Start();

  // Schedule agent execution
  std::cout << "\nScheduling agent execution..." << std::endl;
  orchestrator->ScheduleAgent("agent1");
  orchestrator->ScheduleAgent("agent2");
  orchestrator->ScheduleAgent("agent3");

  // Send inter-agent messages
  std::cout << "\nSending inter-agent messages..." << std::endl;
  agent1->SendMessage("agent2", "greeting", "Hello from agent1!");
  agent2->SendMessage("agent1", "response", "Hello back from agent2!");

  // Wait a bit for execution
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  // Show AtomSpace statistics
  std::cout << "\nAtomSpace Statistics:" << std::endl;
  auto atomspace1 = AtomSpaceManager::GetInstance()->GetAtomSpace("tenant1");
  auto atomspace2 = AtomSpaceManager::GetInstance()->GetAtomSpace("tenant2");
  
  if (atomspace1) {
    std::cout << "Tenant1 AtomSpace size: " << atomspace1->Size() << std::endl;
  }
  if (atomspace2) {
    std::cout << "Tenant2 AtomSpace size: " << atomspace2->Size() << std::endl;
  }

  // Show mesh statistics
  std::cout << "\nIsolate Mesh Statistics:" << std::endl;
  std::cout << "Total tenants: " << mesh->TenantCount() << std::endl;
  auto tenant_ids = mesh->GetTenantIds();
  std::cout << "Tenant IDs: ";
  for (const auto& id : tenant_ids) {
    std::cout << id << " ";
  }
  std::cout << std::endl;

  // Cleanup
  std::cout << "\nShutting down..." << std::endl;
  orchestrator->Stop();

  // Dispose V8
  v8::V8::Dispose();
  v8::V8::DisposePlatform();

  std::cout << "Demo completed successfully!" << std::endl;
  return 0;
}
