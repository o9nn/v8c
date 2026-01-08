// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "include/opencog/isolate-mesh.h"

#include "include/v8-initialization.h"
#include "include/v8-local-handle.h"
#include "include/v8-template.h"

namespace v8 {
namespace opencog {

v8::Platform* IsolateMesh::platform_ = nullptr;

// TenantIsolate implementation
TenantIsolate::TenantIsolate(const std::string& tenant_id,
                              v8::Isolate* isolate,
                              const IsolateConfig& config)
    : tenant_id_(tenant_id), isolate_(isolate), config_(config) {
  atomspace_ = AtomSpaceManager::GetInstance()->GetOrCreateAtomSpace(tenant_id);
  SetupContext();
}

TenantIsolate::~TenantIsolate() {
  if (!context_.IsEmpty()) {
    context_.Reset();
  }
}

void TenantIsolate::SetupContext() {
  v8::Isolate::Scope isolate_scope(isolate_);
  v8::HandleScope handle_scope(isolate_);
  
  // Create global object template
  v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate_);
  
  // TODO: Add OpenCog API bindings here
  // This is where we would expose AtomSpace and Agent APIs to JavaScript
  
  v8::Local<v8::Context> context = v8::Context::New(isolate_, nullptr, global);
  context_.Reset(isolate_, context);
}

v8::Local<v8::Context> TenantIsolate::GetContext() {
  return context_.Get(isolate_);
}

// IsolateMesh implementation
IsolateMesh::IsolateMesh() {}

IsolateMesh::~IsolateMesh() {
  std::lock_guard<std::mutex> lock(mutex_);
  
  // Clean up all tenant isolates
  for (auto& pair : tenant_isolates_) {
    pair.second->isolate()->Dispose();
  }
  tenant_isolates_.clear();
}

std::shared_ptr<TenantIsolate> IsolateMesh::CreateTenantIsolate(
    const std::string& tenant_id, const IsolateConfig& config) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  // Check if isolate already exists
  auto it = tenant_isolates_.find(tenant_id);
  if (it != tenant_isolates_.end()) {
    return it->second;
  }
  
  // Create new isolate
  v8::Isolate::CreateParams create_params;
  create_params.array_buffer_allocator =
      v8::ArrayBuffer::Allocator::NewDefaultAllocator();
  
  if (config.heap_size_limit > 0) {
    create_params.constraints.set_max_old_generation_size_in_bytes(
        config.heap_size_limit);
  }
  
  v8::Isolate* isolate = v8::Isolate::New(create_params);
  
  auto tenant_isolate = std::make_shared<TenantIsolate>(
      tenant_id, isolate, config);
  tenant_isolates_[tenant_id] = tenant_isolate;
  
  return tenant_isolate;
}

std::shared_ptr<TenantIsolate> IsolateMesh::GetTenantIsolate(
    const std::string& tenant_id) const {
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = tenant_isolates_.find(tenant_id);
  return (it != tenant_isolates_.end()) ? it->second : nullptr;
}

bool IsolateMesh::RemoveTenantIsolate(const std::string& tenant_id) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  auto it = tenant_isolates_.find(tenant_id);
  if (it == tenant_isolates_.end()) return false;
  
  it->second->isolate()->Dispose();
  tenant_isolates_.erase(it);
  return true;
}

std::vector<std::string> IsolateMesh::GetTenantIds() const {
  std::lock_guard<std::mutex> lock(mutex_);
  std::vector<std::string> ids;
  ids.reserve(tenant_isolates_.size());
  
  for (const auto& pair : tenant_isolates_) {
    ids.push_back(pair.first);
  }
  
  return ids;
}

size_t IsolateMesh::TenantCount() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return tenant_isolates_.size();
}

void IsolateMesh::SetAgentOrchestrator(
    std::shared_ptr<AgentOrchestrator> orchestrator) {
  orchestrator_ = orchestrator;
}

void IsolateMesh::InitializePlatform(v8::Platform* platform) {
  platform_ = platform;
}

v8::Platform* IsolateMesh::GetPlatform() {
  return platform_;
}

}  // namespace opencog
}  // namespace v8
