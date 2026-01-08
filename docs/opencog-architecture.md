# OpenCog Multi-Tenant Neuro-Symbolic Architecture

## Overview

This implementation provides a comprehensive cognitive architecture framework built on top of the V8 JavaScript engine, integrating:

1. **AtomSpace**: A multi-tenant neuro-symbolic knowledge representation system
2. **Agent-Zero**: A multi-agent autonomous orchestration workbench
3. **Isolate Mesh**: V8 isolate management for modular tenant deployment

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                     Isolate Mesh Manager                         │
│  (Multi-tenant V8 isolate orchestration)                        │
└────────────┬──────────────────────┬──────────────────────┬──────┘
             │                      │                      │
    ┌────────▼────────┐    ┌───────▼────────┐    ┌───────▼────────┐
    │  Tenant 1       │    │  Tenant 2      │    │  Tenant N      │
    ├─────────────────┤    ├────────────────┤    ├────────────────┤
    │ V8 Isolate      │    │ V8 Isolate     │    │ V8 Isolate     │
    │ AtomSpace       │    │ AtomSpace      │    │ AtomSpace      │
    │ Cognitive Agents│    │ Cognitive Agents    │ Cognitive Agents
    └─────────────────┘    └────────────────┘    └────────────────┘
             │                      │                      │
             └──────────────────────┴──────────────────────┘
                                    │
                      ┌─────────────▼──────────────┐
                      │  Agent Orchestrator        │
                      │  (Message routing,         │
                      │   scheduling, coordination)│
                      └────────────────────────────┘
```

## Components

### 1. AtomSpace (Neuro-Symbolic Knowledge Representation)

**Purpose**: Multi-tenant knowledge graph with probabilistic reasoning capabilities.

**Key Features**:
- Node and Link atoms for knowledge representation
- Truth values (strength, confidence) for probabilistic reasoning
- Multi-tenant isolation
- Type-based queries
- Thread-safe operations

**Example Usage**:
```cpp
#include "include/opencog/atomspace.h"

// Get or create AtomSpace for a tenant
auto atomspace = AtomSpaceManager::GetInstance()->GetOrCreateAtomSpace("tenant1");

// Add knowledge nodes
auto concept = atomspace->AddNode(AtomType::CONCEPT_NODE, "Artificial Intelligence");
concept->set_truth_value(TruthValue(0.9, 0.8)); // 90% strength, 80% confidence

// Create relationships
auto node1 = atomspace->AddNode(AtomType::CONCEPT_NODE, "AI");
auto node2 = atomspace->AddNode(AtomType::CONCEPT_NODE, "Machine Learning");
auto link = atomspace->AddLink(
    AtomType::INHERITANCE_LINK, 
    "AI-inherits-ML",
    {node2, node1}
);

// Query knowledge
auto concepts = atomspace->GetAtomsByType(AtomType::CONCEPT_NODE);
auto results = atomspace->Query([](const auto& atom) {
    return atom->truth_value().strength > 0.7;
});
```

### 2. Agent-Zero (Multi-Agent Orchestration)

**Purpose**: Autonomous agent framework with inter-agent communication and coordinated execution.

**Key Features**:
- Agent lifecycle management (initialize, execute, shutdown)
- Inter-agent messaging
- Broadcast capabilities
- Agent scheduling and execution
- Tenant-isolated agent groups

**Example Usage**:
```cpp
#include "include/opencog/agent.h"
#include "include/opencog/agent-orchestrator.h"

// Define a custom cognitive agent
class MyCognitiveAgent : public Agent {
 public:
  MyCognitiveAgent(const std::string& id, const std::string& tenant)
      : Agent(id, tenant) {}

  void Execute() override {
    // Add knowledge to AtomSpace
    auto concept = atomspace_->AddNode(AtomType::CONCEPT_NODE, "ProcessedData");
    
    // Send results to another agent
    SendMessage("analyzer_agent", "data_processed", "Analysis complete");
  }

  void OnMessage(const AgentMessage& message) override {
    // Handle incoming messages
    std::cout << "Received: " << message.payload << std::endl;
  }
};

// Create and orchestrate agents
AgentOrchestrator orchestrator;
auto agent = std::make_shared<MyCognitiveAgent>("agent1", "tenant1");

orchestrator.RegisterAgent(agent);
orchestrator.Start();
orchestrator.ScheduleAgent("agent1");
```

### 3. Isolate Mesh (V8 Integration)

**Purpose**: Manage multiple V8 isolates for tenant-isolated JavaScript execution environments.

**Key Features**:
- Per-tenant V8 isolate allocation
- Configurable heap size limits
- WebAssembly support
- Integration with AtomSpace and Agent framework

**Example Usage**:
```cpp
#include "include/opencog/isolate-mesh.h"

// Initialize V8 platform
std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
v8::V8::InitializePlatform(platform.get());
v8::V8::Initialize();

// Create isolate mesh
IsolateMesh::InitializePlatform(platform.get());
IsolateMesh mesh;

// Configure and create tenant isolate
IsolateConfig config;
config.heap_size_limit = 256 * 1024 * 1024; // 256 MB
config.enable_wasm = true;

auto tenant = mesh.CreateTenantIsolate("tenant1", config);

// Access tenant's AtomSpace
auto atomspace = tenant->atomspace();

// Get V8 context for JavaScript execution
v8::Isolate::Scope isolate_scope(tenant->isolate());
v8::HandleScope handle_scope(tenant->isolate());
auto context = tenant->GetContext();
```

## Complete Integration Example

```cpp
#include "include/opencog.h"
#include "include/libplatform/libplatform.h"
#include "include/v8-initialization.h"

int main(int argc, char* argv[]) {
  // 1. Initialize V8
  v8::V8::InitializeICUDefaultLocation(argv[0]);
  v8::V8::InitializeExternalStartupData(argv[0]);
  std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
  v8::V8::InitializePlatform(platform.get());
  v8::V8::Initialize();

  // 2. Create isolate mesh
  IsolateMesh::InitializePlatform(platform.get());
  IsolateMesh mesh;

  // 3. Create agent orchestrator
  auto orchestrator = std::make_shared<AgentOrchestrator>();
  mesh.SetAgentOrchestrator(orchestrator);

  // 4. Create tenant isolates
  IsolateConfig config;
  config.heap_size_limit = 256 * 1024 * 1024;
  auto tenant = mesh.CreateTenantIsolate("tenant1", config);

  // 5. Create and register agents
  auto agent = std::make_shared<MyCognitiveAgent>("agent1", "tenant1");
  orchestrator->RegisterAgent(agent);

  // 6. Start orchestration
  orchestrator->Start();
  orchestrator->ScheduleAgent("agent1");

  // 7. Work with AtomSpace
  auto atomspace = tenant->atomspace();
  auto concept = atomspace->AddNode(AtomType::CONCEPT_NODE, "Knowledge");

  // 8. Cleanup
  orchestrator->Stop();
  v8::V8::Dispose();
  v8::V8::DisposePlatform();

  return 0;
}
```

## Atom Types

The following atom types are supported for knowledge representation:

- `NODE`: Basic node atom
- `LINK`: Link connecting atoms
- `CONCEPT_NODE`: Represents a concept
- `PREDICATE_NODE`: Represents a predicate
- `VARIABLE_NODE`: Represents a variable
- `EVALUATION_LINK`: Evaluation of a predicate
- `INHERITANCE_LINK`: Inheritance relationship
- `SIMILARITY_LINK`: Similarity relationship
- `EXECUTION_LINK`: Execution context

## Agent States

Agents can be in the following states:

- `IDLE`: Agent is idle and ready for execution
- `RUNNING`: Agent is currently executing
- `PAUSED`: Agent execution is paused
- `COMPLETED`: Agent has completed its task
- `FAILED`: Agent execution failed

## Building

The OpenCog components are integrated into the V8 build system:

```bash
# Build V8 with OpenCog support
tools/dev/gm.py quiet x64.release

# Run the demo
out/x64.release/opencog-demo
```

## Testing

Unit tests are provided for all components:

```bash
# Run OpenCog tests
tools/run-tests.py --outdir=out/x64.release unittests/opencog/*
```

## Thread Safety

All components are designed to be thread-safe:
- AtomSpace uses mutex locks for concurrent access
- AgentOrchestrator uses separate mutexes for agents and messages
- IsolateMesh uses mutex locks for tenant management

## Multi-Tenancy

The architecture provides strong isolation between tenants:
- Each tenant has its own V8 isolate with separate heap
- Each tenant has its own AtomSpace with isolated knowledge
- Agents are tenant-scoped with controlled inter-tenant communication
- Resource limits can be configured per tenant

## Performance Considerations

- **AtomSpace**: O(1) lookup by ID or name, O(n) for type-based queries
- **Agent Orchestration**: Asynchronous message processing with queue
- **Isolate Mesh**: Minimal overhead for tenant isolation
- **Memory**: Configurable heap limits per tenant isolate

## Future Enhancements

Potential areas for extension:
- Pattern matching engine for AtomSpace queries
- Attention allocation mechanism for cognitive focus
- JavaScript bindings for AtomSpace and Agent APIs
- Distributed agent coordination across multiple processes
- Persistence layer for AtomSpace knowledge
- Advanced reasoning algorithms (PLN, ECAN)

## References

- OpenCog: https://opencog.org/
- V8 Isolates: https://v8.dev/docs/embed
- Neuro-Symbolic AI: Combining neural and symbolic approaches
