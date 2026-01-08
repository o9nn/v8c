# Getting Started with OpenCog on V8

This guide will help you get started using the OpenCog multi-tenant neuro-symbolic architecture with V8.

## Prerequisites

- V8 development environment set up
- C++17 or later compiler
- Basic understanding of cognitive architectures (optional but helpful)

## Quick Start (5 Minutes)

### 1. Build V8 with OpenCog

```bash
# Clone the repository (if not already done)
git clone https://github.com/o9nn/v8c.git
cd v8c

# Build V8 with OpenCog components
tools/dev/gm.py quiet x64.release

# Build and run the demo
tools/dev/gm.py quiet x64.release opencog_demo
out/x64.release/opencog_demo
```

### 2. Run the Demo

The demo will show:
- Creating tenant isolates
- Setting up AtomSpace knowledge bases
- Registering and orchestrating agents
- Inter-agent communication
- Multi-tenant isolation

Expected output:
```
OpenCog Multi-Tenant Neuro-Symbolic Architecture Demo
======================================================

Creating tenant isolates...
Created 2 tenant isolates

Registering cognitive agents...
Starting agent orchestrator...

Scheduling agent execution...
Agent agent1 executed. AtomSpace size: 1
Agent agent2 executed. AtomSpace size: 2
Agent agent3 executed. AtomSpace size: 1

Sending inter-agent messages...
Agent agent2 received message from agent1: Hello from agent1!
Agent agent1 received message from agent2: Hello back from agent2!

AtomSpace Statistics:
Tenant1 AtomSpace size: 2
Tenant2 AtomSpace size: 1

Isolate Mesh Statistics:
Total tenants: 2
Tenant IDs: tenant1 tenant2 

Shutting down...
Demo completed successfully!
```

## Basic Usage Patterns

### Pattern 1: Single-Tenant Knowledge Base

For applications with one knowledge base:

```cpp
#include "include/opencog.h"

int main() {
  // Get AtomSpace for your application
  auto atomspace = AtomSpaceManager::GetInstance()
      ->GetOrCreateAtomSpace("my_app");
  
  // Add knowledge
  auto ai = atomspace->AddNode(AtomType::CONCEPT_NODE, "AI");
  auto ml = atomspace->AddNode(AtomType::CONCEPT_NODE, "ML");
  
  auto link = atomspace->AddLink(
      AtomType::INHERITANCE_LINK,
      "ML-is-part-of-AI",
      {ml, ai}
  );
  
  // Query knowledge
  auto concepts = atomspace->GetAtomsByType(AtomType::CONCEPT_NODE);
  
  return 0;
}
```

### Pattern 2: Multi-Tenant Application

For SaaS applications with multiple customers:

```cpp
#include "include/opencog.h"

class TenantManager {
 public:
  void CreateTenant(const std::string& customer_id) {
    // Each customer gets isolated AtomSpace
    auto atomspace = AtomSpaceManager::GetInstance()
        ->GetOrCreateAtomSpace(customer_id);
    
    // Initialize with customer-specific knowledge
    auto domain = atomspace->AddNode(
        AtomType::CONCEPT_NODE, 
        "CustomerDomain");
  }
  
  void ProcessCustomerData(const std::string& customer_id, 
                          const std::string& data) {
    auto atomspace = AtomSpaceManager::GetInstance()
        ->GetAtomSpace(customer_id);
    
    // Process data in isolated space
    auto concept = atomspace->AddNode(
        AtomType::CONCEPT_NODE, data);
  }
};
```

### Pattern 3: Autonomous Agent System

For applications with coordinating agents:

```cpp
#include "include/opencog.h"

// Define your agents
class DataCollectorAgent : public Agent {
 public:
  DataCollectorAgent(const std::string& id, const std::string& tenant)
      : Agent(id, tenant) {}
  
  void Execute() override {
    // Collect data, add to atomspace
    auto data = atomspace_->AddNode(
        AtomType::CONCEPT_NODE, "CollectedData");
    
    // Notify analyzer
    SendMessage("analyzer", "data_ready", "New data available");
  }
};

class DataAnalyzerAgent : public Agent {
 public:
  DataAnalyzerAgent(const std::string& id, const std::string& tenant)
      : Agent(id, tenant) {}
  
  void Execute() override {
    // Analyze data from atomspace
    auto concepts = atomspace_->GetAtomsByType(AtomType::CONCEPT_NODE);
    // ... analysis logic ...
  }
  
  void OnMessage(const AgentMessage& msg) override {
    if (msg.type == "data_ready") {
      // Trigger analysis
      Execute();
    }
  }
};

int main() {
  // Set up orchestrator
  AgentOrchestrator orchestrator;
  
  // Create and register agents
  auto collector = std::make_shared<DataCollectorAgent>(
      "collector", "tenant1");
  auto analyzer = std::make_shared<DataAnalyzerAgent>(
      "analyzer", "tenant1");
  
  orchestrator.RegisterAgent(collector);
  orchestrator.RegisterAgent(analyzer);
  
  // Run orchestration
  orchestrator.Start();
  orchestrator.ScheduleAgent("collector");
  
  // ... application runs ...
  
  orchestrator.Stop();
  return 0;
}
```

### Pattern 4: JavaScript Integration

For exposing cognitive capabilities to JavaScript:

```cpp
#include "include/opencog.h"
#include "include/v8-initialization.h"

class CognitiveJSEngine {
 public:
  void Initialize() {
    // Initialize V8
    v8::V8::InitializeICUDefaultLocation(argv_[0]);
    v8::V8::InitializeExternalStartupData(argv_[0]);
    platform_ = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform_.get());
    v8::V8::Initialize();
    
    // Create isolate mesh
    IsolateMesh::InitializePlatform(platform_.get());
  }
  
  void CreateTenant(const std::string& tenant_id) {
    IsolateConfig config;
    config.heap_size_limit = 256 * 1024 * 1024;
    
    auto tenant = mesh_.CreateTenantIsolate(tenant_id, config);
    
    // Access tenant's atomspace from JavaScript
    // (Binding implementation would go in TenantIsolate::SetupContext)
  }
  
 private:
  IsolateMesh mesh_;
  std::unique_ptr<v8::Platform> platform_;
  char** argv_;
};
```

## Common Tasks

### Adding Knowledge

```cpp
// Simple concept
auto concept = atomspace->AddNode(AtomType::CONCEPT_NODE, "Cat");

// With truth value
concept->set_truth_value(TruthValue(0.95, 0.9));

// Relationship between concepts
auto cat = atomspace->AddNode(AtomType::CONCEPT_NODE, "Cat");
auto animal = atomspace->AddNode(AtomType::CONCEPT_NODE, "Animal");

auto inheritance = atomspace->AddLink(
    AtomType::INHERITANCE_LINK,
    "Cat-inherits-Animal",
    {cat, animal}
);
```

### Querying Knowledge

```cpp
// By name
auto cat = atomspace->GetAtomByName("Cat");

// By type
auto all_concepts = atomspace->GetAtomsByType(AtomType::CONCEPT_NODE);

// Custom predicate
auto high_certainty = atomspace->Query([](const auto& atom) {
    return atom->truth_value().strength > 0.9 &&
           atom->truth_value().confidence > 0.8;
});
```

### Creating Custom Agents

```cpp
class MyAgent : public Agent {
 public:
  MyAgent(const std::string& id, const std::string& tenant)
      : Agent(id, tenant), counter_(0) {}
  
  bool Initialize() override {
    // Setup phase
    std::cout << "Initializing " << agent_id_ << std::endl;
    return Agent::Initialize();
  }
  
  void Execute() override {
    // Main logic
    counter_++;
    
    // Access atomspace
    auto status = atomspace_->AddNode(
        AtomType::CONCEPT_NODE, 
        "ExecutionCount_" + std::to_string(counter_));
    
    // Send results
    if (counter_ % 10 == 0) {
      SendMessage("monitor", "status", 
                  "Completed " + std::to_string(counter_) + " executions");
    }
  }
  
  void OnMessage(const AgentMessage& msg) override {
    // Handle messages
    if (msg.type == "command") {
      Execute();
    }
  }
  
  void Shutdown() override {
    // Cleanup
    std::cout << "Shutting down " << agent_id_ << std::endl;
    Agent::Shutdown();
  }
  
 private:
  int counter_;
};
```

### Managing Multiple Tenants

```cpp
// Create multiple tenant atomspaces
auto manager = AtomSpaceManager::GetInstance();

auto tenant1 = manager->GetOrCreateAtomSpace("customer1");
auto tenant2 = manager->GetOrCreateAtomSpace("customer2");
auto tenant3 = manager->GetOrCreateAtomSpace("customer3");

// Each operates independently
tenant1->AddNode(AtomType::CONCEPT_NODE, "Data1");
tenant2->AddNode(AtomType::CONCEPT_NODE, "Data2");
tenant3->AddNode(AtomType::CONCEPT_NODE, "Data3");

// Get all tenants
auto tenant_ids = manager->GetTenantIds();
for (const auto& id : tenant_ids) {
  auto space = manager->GetAtomSpace(id);
  std::cout << "Tenant " << id << " has " 
            << space->Size() << " atoms" << std::endl;
}

// Remove a tenant
manager->RemoveAtomSpace("customer2");
```

## Integration with Existing Code

### With Existing V8 Application

```cpp
// Your existing V8 code
v8::Isolate* your_isolate = v8::Isolate::GetCurrent();

// Add OpenCog capabilities
auto atomspace = AtomSpaceManager::GetInstance()
    ->GetOrCreateAtomSpace("your_app");

// Use atomspace alongside your V8 code
auto concept = atomspace->AddNode(AtomType::CONCEPT_NODE, "Feature");
```

### With Existing Agent Systems

```cpp
// Wrap your existing agent in OpenCog Agent
class LegacyAgentWrapper : public Agent {
 public:
  LegacyAgentWrapper(LegacyAgent* legacy, 
                     const std::string& id, 
                     const std::string& tenant)
      : Agent(id, tenant), legacy_(legacy) {}
  
  void Execute() override {
    // Call legacy agent
    legacy_->Process();
    
    // Store results in atomspace
    auto result = atomspace_->AddNode(
        AtomType::CONCEPT_NODE, 
        legacy_->GetResult());
  }
  
 private:
  LegacyAgent* legacy_;
};
```

## Debugging Tips

### Enable Logging

```cpp
// In your agent
void Execute() override {
  std::cout << "[" << agent_id_ << "] Executing..." << std::endl;
  
  // ... logic ...
  
  std::cout << "[" << agent_id_ << "] AtomSpace size: " 
            << atomspace_->Size() << std::endl;
}
```

### Inspect AtomSpace

```cpp
void DumpAtomSpace(std::shared_ptr<AtomSpace> atomspace) {
  auto atoms = atomspace->Query([](const auto&) { return true; });
  
  std::cout << "AtomSpace for tenant: " << atomspace->tenant_id() << std::endl;
  std::cout << "Total atoms: " << atoms.size() << std::endl;
  
  for (const auto& atom : atoms) {
    std::cout << "  - " << atom->name() 
              << " (type: " << static_cast<int>(atom->type()) << ")"
              << " [" << atom->truth_value().strength << ","
              << atom->truth_value().confidence << "]" << std::endl;
  }
}
```

### Monitor Agent Activity

```cpp
class DebugAgent : public Agent {
 public:
  DebugAgent(const std::string& id, const std::string& tenant)
      : Agent(id, tenant) {}
  
  void Execute() override {
    // This agent just monitors others
  }
  
  void OnMessage(const AgentMessage& msg) override {
    std::cout << "[Monitor] " << msg.from_agent_id 
              << " -> " << msg.to_agent_id
              << " (" << msg.type << "): " << msg.payload << std::endl;
  }
};

// Register as monitor
auto monitor = std::make_shared<DebugAgent>("monitor", "tenant1");
orchestrator.RegisterAgent(monitor);

// All messages will be logged by setting up broadcast forwarding
```

## Performance Tips

1. **Reuse AtomSpace References**
   ```cpp
   // Good: Cache the reference
   auto atomspace = manager->GetAtomSpace("tenant1");
   for (int i = 0; i < 1000; i++) {
     atomspace->AddNode(...);
   }
   
   // Avoid: Repeated lookups
   for (int i = 0; i < 1000; i++) {
     manager->GetAtomSpace("tenant1")->AddNode(...);
   }
   ```

2. **Batch Operations**
   ```cpp
   // Add multiple atoms at once
   std::vector<std::shared_ptr<Atom>> atoms;
   for (const auto& name : names) {
     atoms.push_back(atomspace->AddNode(AtomType::CONCEPT_NODE, name));
   }
   ```

3. **Use Type Queries Over Full Scans**
   ```cpp
   // Good: Type-specific query
   auto concepts = atomspace->GetAtomsByType(AtomType::CONCEPT_NODE);
   
   // Avoid: Full scan with filter
   auto concepts = atomspace->Query([](const auto& atom) {
     return atom->type() == AtomType::CONCEPT_NODE;
   });
   ```

## Next Steps

1. **Read the Documentation**
   - [Architecture Overview](opencog-architecture.md)
   - [API Reference](opencog-quick-reference.md)
   - [Implementation Details](opencog-implementation-summary.md)

2. **Explore the Code**
   - Check out `samples/opencog-demo.cc` for a complete example
   - Review the test files for usage patterns
   - Examine the headers in `include/opencog/`

3. **Build Your Application**
   - Start with a simple single-tenant atomspace
   - Add agents as needed
   - Scale to multi-tenant when required

4. **Join the Community**
   - Report issues on GitHub
   - Contribute improvements
   - Share your use cases

## Support

- **Source Code**: https://github.com/o9nn/v8c
- **Documentation**: See `docs/` directory
- **Examples**: See `samples/` directory
- **Tests**: See `test/unittests/opencog/`

Happy cognitive computing! 🧠🤖
