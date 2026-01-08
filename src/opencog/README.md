# OpenCog Integration for V8

This directory contains the OpenCog multi-tenant neuro-symbolic architecture integrated with V8 JavaScript engine.

## Overview

OpenCog provides a cognitive architecture framework with three main components:

1. **AtomSpace** - Multi-tenant neuro-symbolic knowledge representation
2. **Agent-Zero** - Multi-agent autonomous orchestration 
3. **Isolate Mesh** - V8 isolate management for modular tenant deployment

## Directory Structure

```
src/opencog/
├── atomspace/          # AtomSpace knowledge representation
│   ├── atom.cc
│   └── atomspace.cc
├── agents/             # Agent-Zero orchestration
│   ├── agent.cc
│   └── agent-orchestrator.cc
├── isolate-mesh/       # V8 isolate management
│   └── isolate-mesh.cc
└── BUILD.gn           # Build configuration
```

## Building

OpenCog is built as part of the standard V8 build:

```bash
# Build V8 with OpenCog
tools/dev/gm.py quiet x64.release

# Build and run the demo
tools/dev/gm.py quiet x64.release opencog_demo
out/x64.release/opencog_demo
```

## Testing

Run OpenCog unit tests:

```bash
tools/run-tests.py --outdir=out/x64.release unittests
```

Specific OpenCog tests:

```bash
# AtomSpace tests
tools/run-tests.py --outdir=out/x64.release opencog/atomspace-unittest

# Agent tests
tools/run-tests.py --outdir=out/x64.release opencog/agent-unittest
```

## API Usage

See [docs/opencog-architecture.md](../../docs/opencog-architecture.md) for detailed API documentation and examples.

Quick example:

```cpp
#include "include/opencog.h"

// Create AtomSpace for a tenant
auto atomspace = AtomSpaceManager::GetInstance()->GetOrCreateAtomSpace("tenant1");

// Add knowledge
auto concept = atomspace->AddNode(AtomType::CONCEPT_NODE, "AI");
concept->set_truth_value(TruthValue(0.9, 0.8));

// Create and orchestrate agents
AgentOrchestrator orchestrator;
auto agent = std::make_shared<MyAgent>("agent1", "tenant1");
orchestrator.RegisterAgent(agent);
orchestrator.Start();
```

## Public Headers

Public API headers are in `include/opencog/`:

- `atom.h` - Atom types and truth values
- `atomspace.h` - AtomSpace container and manager
- `agent.h` - Agent base class and factory
- `agent-orchestrator.h` - Agent orchestration
- `isolate-mesh.h` - V8 isolate management

The main convenience header is `include/opencog.h` which includes all components.

## License

Copyright 2026 the V8 project authors. All rights reserved.
Use of this source code is governed by a BSD-style license.
