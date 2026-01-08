// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_OPENCOG_H_
#define V8_OPENCOG_H_

/**
 * OpenCog Multi-Tenant Neuro-Symbolic Architecture with Agent-Zero Orchestration
 * 
 * This module provides a comprehensive cognitive architecture framework built on
 * top of V8 JavaScript engine, combining:
 * 
 * 1. AtomSpace: A neuro-symbolic knowledge representation system with multi-tenant
 *    support for isolated cognitive workspaces.
 * 
 * 2. Agent-Zero: A multi-agent autonomous orchestration workbench enabling
 *    coordinated execution of cognitive agents across tenant boundaries.
 * 
 * 3. Isolate Mesh: A V8 isolate management system providing modular deployment
 *    of JavaScript execution environments for each tenant.
 * 
 * Architecture Overview:
 * ----------------------
 * 
 * [Tenant 1]                    [Tenant 2]                    [Tenant N]
 *    |                              |                              |
 *    ├─ V8 Isolate                  ├─ V8 Isolate                  ├─ V8 Isolate
 *    ├─ AtomSpace (Knowledge)       ├─ AtomSpace (Knowledge)       ├─ AtomSpace
 *    └─ Agents (Autonomous)         └─ Agents (Autonomous)         └─ Agents
 *         |                              |                              |
 *         └──────────────────────────────┴──────────────────────────────┘
 *                                        |
 *                              Agent Orchestrator
 *                            (Message Routing & Scheduling)
 * 
 * Key Features:
 * -------------
 * - Multi-tenant isolation with separate V8 isolates per tenant
 * - Neuro-symbolic knowledge representation via AtomSpace
 * - Autonomous agent execution with inter-agent communication
 * - Thread-safe concurrent operations
 * - Modular and extensible architecture
 * 
 * Usage Example:
 * --------------
 * ```cpp
 * // Initialize V8 and create isolate mesh
 * IsolateMesh mesh;
 * auto tenant = mesh.CreateTenantIsolate("tenant1", config);
 * 
 * // Access AtomSpace for knowledge management
 * auto atomspace = tenant->atomspace();
 * auto concept = atomspace->AddNode(AtomType::CONCEPT_NODE, "AI");
 * 
 * // Create and orchestrate agents
 * AgentOrchestrator orchestrator;
 * auto agent = std::make_shared<MyAgent>("agent1", "tenant1");
 * orchestrator.RegisterAgent(agent);
 * orchestrator.Start();
 * ```
 */

#include "include/opencog/atom.h"
#include "include/opencog/atomspace.h"
#include "include/opencog/agent.h"
#include "include/opencog/agent-orchestrator.h"
#include "include/opencog/isolate-mesh.h"

#endif  // V8_OPENCOG_H_
