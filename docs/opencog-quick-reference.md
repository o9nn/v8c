# OpenCog Quick Reference Guide

## AtomSpace API

### Create/Get AtomSpace
```cpp
// Get singleton manager
auto manager = AtomSpaceManager::GetInstance();

// Get or create atomspace for tenant
auto atomspace = manager->GetOrCreateAtomSpace("tenant_id");

// Get existing atomspace
auto atomspace = manager->GetAtomSpace("tenant_id");
```

### Add Atoms
```cpp
// Add a concept node
auto concept = atomspace->AddNode(AtomType::CONCEPT_NODE, "MyConceptName");

// Add a predicate node
auto predicate = atomspace->AddNode(AtomType::PREDICATE_NODE, "IsA");

// Add a link between atoms
std::vector<std::shared_ptr<Atom>> outgoing = {concept1, concept2};
auto link = atomspace->AddLink(AtomType::INHERITANCE_LINK, "LinkName", outgoing);
```

### Set Truth Values
```cpp
// Create truth value (strength, confidence)
TruthValue tv(0.9, 0.8);  // 90% strength, 80% confidence

// Set on atom
atom->set_truth_value(tv);

// Get truth value
const auto& tv = atom->truth_value();
double strength = tv.strength;
double confidence = tv.confidence;
```

### Query Atoms
```cpp
// Get atom by ID
auto atom = atomspace->GetAtom(atom_id);

// Get atom by name
auto atom = atomspace->GetAtomByName("MyConceptName");

// Get all atoms of a type
auto concepts = atomspace->GetAtomsByType(AtomType::CONCEPT_NODE);

// Custom query with predicate
auto results = atomspace->Query([](const auto& atom) {
    return atom->truth_value().strength > 0.7;
});
```

### Manage AtomSpace
```cpp
// Get size
size_t count = atomspace->Size();

// Remove atom
bool removed = atomspace->RemoveAtom(atom_id);

// Clear all atoms
atomspace->Clear();
```

## Agent API

### Define Custom Agent
```cpp
class MyAgent : public Agent {
 public:
  MyAgent(const std::string& id, const std::string& tenant)
      : Agent(id, tenant) {}

  void Execute() override {
    // Add knowledge to atomspace
    auto concept = atomspace_->AddNode(AtomType::CONCEPT_NODE, "Data");
    
    // Send message to another agent
    SendMessage("other_agent_id", "data_ready", "payload");
  }

  void OnMessage(const AgentMessage& message) override {
    // Handle incoming messages
    std::cout << "Received: " << message.payload << std::endl;
  }
};
```

### Agent Orchestration
```cpp
// Create orchestrator
AgentOrchestrator orchestrator;

// Create and register agents
auto agent1 = std::make_shared<MyAgent>("agent1", "tenant1");
auto agent2 = std::make_shared<MyAgent>("agent2", "tenant1");

orchestrator.RegisterAgent(agent1);
orchestrator.RegisterAgent(agent2);

// Start orchestration
orchestrator.Start();

// Schedule agent execution
orchestrator.ScheduleAgent("agent1");

// Send messages
agent1->SendMessage("agent2", "greeting", "Hello!");

// Broadcast to all agents
orchestrator.BroadcastMessage("agent1", "announcement", "Important!");

// Stop orchestration
orchestrator.Stop();
```

### Agent Factory
```cpp
// Register agent type
AgentFactory::GetInstance()->RegisterAgentType("my_agent",
    [](const std::string& id, const std::string& tenant) {
        return std::make_shared<MyAgent>(id, tenant);
    });

// Create agent from factory
auto agent = AgentFactory::GetInstance()->CreateAgent(
    "my_agent", "agent1", "tenant1");
```

## Isolate Mesh API

### Initialize Platform
```cpp
// Initialize V8
v8::V8::InitializeICUDefaultLocation(argv[0]);
v8::V8::InitializeExternalStartupData(argv[0]);
auto platform = v8::platform::NewDefaultPlatform();
v8::V8::InitializePlatform(platform.get());
v8::V8::Initialize();

// Initialize isolate mesh
IsolateMesh::InitializePlatform(platform.get());
```

### Create Tenant Isolates
```cpp
// Create isolate mesh
IsolateMesh mesh;

// Configure isolate
IsolateConfig config;
config.heap_size_limit = 256 * 1024 * 1024;  // 256 MB
config.enable_wasm = true;
config.enable_inspector = false;

// Create tenant isolate
auto tenant = mesh.CreateTenantIsolate("tenant1", config);

// Access tenant's atomspace
auto atomspace = tenant->atomspace();

// Get V8 isolate
v8::Isolate* isolate = tenant->isolate();

// Get context
auto context = tenant->GetContext();
```

### Manage Isolate Mesh
```cpp
// Get tenant isolate
auto tenant = mesh.GetTenantIsolate("tenant1");

// Remove tenant isolate
bool removed = mesh.RemoveTenantIsolate("tenant1");

// Get all tenant IDs
auto tenant_ids = mesh.GetTenantIds();

// Get tenant count
size_t count = mesh.TenantCount();
```

### Integrate with Agent Orchestrator
```cpp
// Set orchestrator for mesh
auto orchestrator = std::make_shared<AgentOrchestrator>();
mesh.SetAgentOrchestrator(orchestrator);

// Get orchestrator from mesh
auto orch = mesh.agent_orchestrator();
```

## Atom Types

- `AtomType::NODE` - Basic node
- `AtomType::LINK` - Basic link
- `AtomType::CONCEPT_NODE` - Concept
- `AtomType::PREDICATE_NODE` - Predicate
- `AtomType::VARIABLE_NODE` - Variable
- `AtomType::EVALUATION_LINK` - Evaluation
- `AtomType::INHERITANCE_LINK` - Inheritance
- `AtomType::SIMILARITY_LINK` - Similarity
- `AtomType::EXECUTION_LINK` - Execution

## Agent States

- `AgentState::IDLE` - Ready for execution
- `AgentState::RUNNING` - Currently executing
- `AgentState::PAUSED` - Execution paused
- `AgentState::COMPLETED` - Task completed
- `AgentState::FAILED` - Execution failed

## Message Structure

```cpp
struct AgentMessage {
  std::string from_agent_id;
  std::string to_agent_id;
  std::string type;
  std::string payload;
  uint64_t timestamp;
};
```

## Complete Example

```cpp
#include "include/opencog.h"
#include "include/libplatform/libplatform.h"
#include "include/v8-initialization.h"

int main(int argc, char* argv[]) {
  // 1. Initialize V8
  v8::V8::InitializeICUDefaultLocation(argv[0]);
  v8::V8::InitializeExternalStartupData(argv[0]);
  auto platform = v8::platform::NewDefaultPlatform();
  v8::V8::InitializePlatform(platform.get());
  v8::V8::Initialize();

  // 2. Create isolate mesh
  IsolateMesh::InitializePlatform(platform.get());
  IsolateMesh mesh;

  // 3. Create orchestrator
  auto orchestrator = std::make_shared<AgentOrchestrator>();
  mesh.SetAgentOrchestrator(orchestrator);

  // 4. Create tenant
  IsolateConfig config;
  config.heap_size_limit = 256 * 1024 * 1024;
  auto tenant = mesh.CreateTenantIsolate("tenant1", config);

  // 5. Add knowledge
  auto atomspace = tenant->atomspace();
  auto concept = atomspace->AddNode(AtomType::CONCEPT_NODE, "AI");
  concept->set_truth_value(TruthValue(0.9, 0.8));

  // 6. Create agents
  auto agent = std::make_shared<MyAgent>("agent1", "tenant1");
  orchestrator->RegisterAgent(agent);

  // 7. Run orchestration
  orchestrator->Start();
  orchestrator->ScheduleAgent("agent1");

  // Wait for execution
  std::this_thread::sleep_for(std::chrono::seconds(1));

  // 8. Cleanup
  orchestrator->Stop();
  v8::V8::Dispose();
  v8::V8::DisposePlatform();

  return 0;
}
```
