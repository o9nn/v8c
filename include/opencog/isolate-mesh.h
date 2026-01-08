// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_OPENCOG_ISOLATE_MESH_H_
#define V8_OPENCOG_ISOLATE_MESH_H_

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "include/v8-isolate.h"
#include "include/v8-context.h"
#include "include/v8-platform.h"
#include "include/opencog/atomspace.h"
#include "include/opencog/agent-orchestrator.h"

namespace v8 {
namespace opencog {

// Configuration for isolate creation
struct IsolateConfig {
  size_t heap_size_limit;
  bool enable_wasm;
  bool enable_inspector;
  
  IsolateConfig() 
      : heap_size_limit(0),
        enable_wasm(true),
        enable_inspector(false) {}
};

// Tenant isolate context
class TenantIsolate {
 public:
  TenantIsolate(const std::string& tenant_id,
                v8::Isolate* isolate,
                const IsolateConfig& config);
  ~TenantIsolate();

  v8::Isolate* isolate() const { return isolate_; }
  const std::string& tenant_id() const { return tenant_id_; }
  std::shared_ptr<AtomSpace> atomspace() const { return atomspace_; }
  
  void SetupContext();
  v8::Local<v8::Context> GetContext();

 private:
  std::string tenant_id_;
  v8::Isolate* isolate_;
  IsolateConfig config_;
  v8::Global<v8::Context> context_;
  std::shared_ptr<AtomSpace> atomspace_;
};

// Isolate mesh for managing multiple V8 isolates
class IsolateMesh {
 public:
  IsolateMesh();
  ~IsolateMesh();

  // Isolate lifecycle management
  std::shared_ptr<TenantIsolate> CreateTenantIsolate(
      const std::string& tenant_id, const IsolateConfig& config);
  std::shared_ptr<TenantIsolate> GetTenantIsolate(
      const std::string& tenant_id) const;
  bool RemoveTenantIsolate(const std::string& tenant_id);

  // Mesh operations
  std::vector<std::string> GetTenantIds() const;
  size_t TenantCount() const;

  // Agent orchestration integration
  void SetAgentOrchestrator(std::shared_ptr<AgentOrchestrator> orchestrator);
  std::shared_ptr<AgentOrchestrator> agent_orchestrator() const {
    return orchestrator_;
  }

  // Platform management
  static void InitializePlatform(v8::Platform* platform);
  static v8::Platform* GetPlatform();

 private:
  mutable std::mutex mutex_;
  std::map<std::string, std::shared_ptr<TenantIsolate>> tenant_isolates_;
  std::shared_ptr<AgentOrchestrator> orchestrator_;
  static v8::Platform* platform_;
};

}  // namespace opencog
}  // namespace v8

#endif  // V8_OPENCOG_ISOLATE_MESH_H_
