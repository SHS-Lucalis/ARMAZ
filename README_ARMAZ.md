# ARMAZ

**ARMAZ** is a standalone Arma Reforger zombie-survival scenario/game-mode framework built around scarcity, exploration, dynamic objectives, and naturally reacting infected AI.

This first-pass implementation was created from the architectural ideas of the SHS scenario framework, but it intentionally avoids importing the SHS military AI-controller systems. ARMAZ controls the world state: where zombies spawn, where objectives appear, where loot is rewarded, how rare vehicles are, and when far-away entities are cleaned up. Zombie behavior itself is expected to come from the configured zombie prefab, faction setup, perception, and vanilla/engine AI behavior.

## Design Goal

ARMAZ is not meant to be a standard military PvE objective generator with zombies added on top. It is intended to become a survival mode where players must search towns, bases, and wilderness locations for equipment while managing risk, low supplies, rare vehicles, and unpredictable infected populations.

The core gameplay loop is:

1. Players spawn into a hostile survival environment.
2. The system discovers useful world locations such as towns, villages, bases, and wilderness areas.
3. Random zombie populations spawn around players and objectives.
4. Objectives appear at selected locations and provide equipment, fuel, medical items, vehicle parts, or progression intel.
5. Vehicles are scarce, often damaged, out of fuel, or barely usable.
6. Players push farther into the map to obtain better gear and survive larger infected threats.

## Core Rule

ARMAZ controls spawning, scarcity, objectives, rewards, and cleanup.

ARMAZ does **not** control zombie combat AI.

That means the zombie system should not force targets, spam waypoints, teleport infected, override AI brains, or constantly micromanage movement. Zombies are spawned into valid locations and allowed to react through their configured AI/faction behavior.

## Included Systems

### `ARMAZ_GameModeComponent`

Main server-side controller for ARMAZ. It owns and initializes the managers, exposes World Editor attributes, and runs the update loop.

Primary responsibilities:

- Start ARMAZ systems on the server.
- Initialize location discovery.
- Initialize zombie spawning.
- Initialize objective generation.
- Initialize vehicle scarcity.
- Initialize loot handling.
- Initialize performance scaling.

### `ARMAZ_LocationDiscovery`

Finds usable world locations for objectives, zombie populations, and vehicle scarcity.

Supported location concepts:

- Towns
- Villages
- Military sites
- Named map locations
- Generated wilderness points

This system is meant to replace hardcoded objective placement with reusable map-aware placement.

### `ARMAZ_ZombieSpawnManager`

Handles passive infected spawning and cleanup.

This manager only:

- Chooses valid spawn positions.
- Enforces zombie caps.
- Spawns configured zombie prefabs.
- Tracks active zombies.
- Cleans up dead or far-away zombies.

This manager does **not**:

- Assign AI waypoints.
- Force zombie targets.
- Override AI behavior trees.
- Run a custom AI commander.
- Teleport zombies to players.
- Replace faction/perception logic.

### `ARMAZ_ObjectiveDirector`

Creates survival-focused objectives at discovered locations.

Current objective types:

- `LOOT_CACHE`
- `MEDICAL_SUPPLY`
- `FUEL_DEPOT`
- `RADIO_TOWER`
- `RESTORE_POWER`
- `VEHICLE_PARTS`
- `EXTRACTION_INTEL`
- `MILITARY_STASH`

The first-pass objective completion condition is simple: a player entering the objective radius can complete the objective. This keeps Milestone 1 stable. Later passes can replace that with interaction-based repair, power restoration, radio activation, container unlocking, or extraction-chain logic.

### `ARMAZ_LootManager`

Spawns rewards for completed objectives.

Loot categories are separated into tiers:

- Common loot
- Uncommon loot
- Rare loot
- Military loot
- Medical loot
- Fuel loot
- Vehicle part loot

The actual loot prefabs should be configured in World Editor through the game mode component attributes.

### `ARMAZ_VehicleScarcityManager`

Controls rare vehicle availability.

ARMAZ should not feel like a normal vehicle-heavy military scenario. Vehicles should be valuable and unreliable.

Vehicle conditions are rolled as:

- Wrecked or unusable
- Damaged with no fuel
- Damaged with low fuel
- Working with low fuel
- Rare working vehicle

The current first-pass implementation scaffolds the condition system and spawn logic. Exact fuel/damage application depends on the vehicle components used by the selected prefabs and should be wired once final ARMAZ vehicle prefabs are chosen.

### `ARMAZ_PerformanceManager`

Maintains performance limits for zombie-heavy gameplay.

Responsibilities:

- Track performance state.
- Adjust effective zombie caps.
- Reduce spawning pressure when performance drops.
- Prioritize cleanup of far-away spawned entities.

Zombie modes can become expensive quickly, so this system should remain part of the core ARMAZ architecture.

### `ARMAZ_MapMarkerManager`

Provides the call sites for objective and event map markers.

Marker creation is currently placeholder/scaffolded because exact marker APIs and ownership behavior can vary depending on the scenario setup. Notifications and logging are implemented, and marker functionality can be expanded during the next pass.

### `ARMAZ_Notification`

Simple helper for ARMAZ event messaging.

Expected uses:

- Objective discovered
- Objective completed
- Horde nearby
- Vehicle found
- Extraction intel found
- Loot obtained

## Systems Intentionally Excluded

The following SHS military framework systems are intentionally not part of ARMAZ first pass:

- `SHS_AIManager`
- `SHS_CounterAttackManager`
- `SHS_ConvoyManager`
- `SHS_AmbientTrafficManager`
- `SHS_CivilianManager`
- `SHS_RoamingPatrolManager`
- `SHS_MineManager`
- `SHS_MortarStrikeManager`

These systems are built around military objective gameplay. ARMAZ needs survival pressure, infected presence, scarcity, exploration, and loot progression instead.

## Recommended Starting Settings

For early testing, use conservative values:

| Setting | Recommended Test Value |
|---|---:|
| Max Zombies | 50-75 |
| Max Active Objectives | 2-3 |
| Max Vehicles | 5-10 |
| System Update Interval | 30-60 seconds |
| Objective Interval | 180-300 seconds |
| Zombie Spawn Radius | 500-900 meters |
| Zombie Safe Radius | 100-200 meters |

Raise caps only after confirming server stability.

## Workbench Setup

1. Create or open the ARMAZ mod project.
2. Copy `Scripts/Game/ARMAZ` into the mod.
3. Add `ARMAZ_GameModeComponent` to the game mode entity or game mode prefab.
4. Fill the Zombie Prefabs array with zombie/infected prefabs that already have working AI/faction behavior.
5. Fill loot prefab arrays for common, uncommon, rare, military, medical, fuel, and vehicle-part rewards.
6. Fill vehicle prefabs if vehicle scarcity is enabled.
7. Start with low caps and validate server boot before increasing density.
8. Test objective spawning, zombie cleanup, and loot reward spawning before adding advanced objectives.

## First-Pass Limitations

This is a Milestone 1 implementation, not the final game mode.

Known limitations:

- Objective completion is radius-based for now.
- Vehicle fuel and damage application is scaffolded and needs final prefab-specific component calls.
- Map marker creation is scaffolded and needs final scenario-specific marker implementation.
- Zombie behavior depends entirely on the configured zombie prefab, faction, and AI setup.
- No persistence layer is included yet.
- No extraction endgame is implemented yet beyond the objective type placeholder.
- No horde escalation director is included yet.

## Recommended Next Milestones

### Milestone 2: Survival Gameplay Depth

- Add interaction-based objective completion.
- Add loot containers that require searching/opening.
- Add fuel, vehicle parts, and repair progression.
- Add medical supply objectives.
- Add better map marker support.

### Milestone 3: Horde and Pressure Systems

- Add horde director.
- Add sound-attraction events.
- Add objective-based zombie escalation.
- Add night-time threat scaling.
- Add safe-zone or extraction-zone logic.

### Milestone 4: Progression and Persistence

- Add extraction intel chain.
- Add server-side progression state.
- Add persistent discovered locations/objectives if desired.
- Add optional player stash or base-building hooks.

## Development Notes

Keep ARMAZ modular. Each manager should have a narrow job:

- The game mode coordinates systems.
- The location manager finds valid places.
- The objective director decides what players should do.
- The zombie manager controls population only.
- The loot manager controls rewards.
- The vehicle manager controls scarcity.
- The performance manager controls caps and cleanup.

Avoid reintroducing direct AI command logic unless ARMAZ later adds a separate human NPC/bandit faction. Even then, keep infected AI passive and prefab-driven.

## Project Identity

ARMAZ should feel like its own game mode:

- Zombies are unpredictable.
- Equipment is earned through objectives.
- Vehicles are rare and valuable.
- Movement across the map has risk.
- Players survive through planning, scavenging, and teamwork.

The goal is not to overwhelm the engine with controlled AI. The goal is to let the world feel dangerous while keeping the backend stable.
