//------------------------------------------------------------------------------------------------
//! ARMAZ Game Mode Component
//! Standalone zombie/survival objective mode using passive infected spawning.
//! ARMAZ controls spawning, objectives, scarcity, rewards and cleanup.
//! ARMAZ does NOT control zombie combat AI, target selection, behavior trees or waypoints.
//------------------------------------------------------------------------------------------------

[ComponentEditorProps(category: "ARMAZ/GameMode", description: "ARMAZ zombie survival objective game mode")]
class ARMAZ_GameModeComponentClass : SCR_BaseGameModeComponentClass
{
}

class ARMAZ_GameModeComponent : SCR_BaseGameModeComponent
{
	protected static ARMAZ_GameModeComponent s_Instance;

	//------------------------------------------------------------------------------------------------
	// Master enable
	//------------------------------------------------------------------------------------------------
	[Attribute(defvalue: "1", UIWidgets.CheckBox, desc: "Enable ARMAZ", category: "ARMAZ")]
	protected bool m_bEnableARMAZ;

	[Attribute(defvalue: "60", UIWidgets.Slider, desc: "Server system update interval in seconds", params: "15 300 15", category: "ARMAZ")]
	protected float m_fSystemUpdateInterval;

	//------------------------------------------------------------------------------------------------
	// Zombies
	//------------------------------------------------------------------------------------------------
	[Attribute(defvalue: "150", UIWidgets.Slider, desc: "Maximum active zombies", params: "25 500 25", category: "Zombies")]
	protected int m_iMaxZombies;

	[Attribute(defvalue: "850", UIWidgets.Slider, desc: "Zombie spawn radius around players", params: "200 2500 50", category: "Zombies")]
	protected float m_fZombieSpawnRadius;

	[Attribute(defvalue: "150", UIWidgets.Slider, desc: "Minimum safe spawn radius around players", params: "25 500 25", category: "Zombies")]
	protected float m_fZombieSafeRadius;

	[Attribute(defvalue: "1800", UIWidgets.Slider, desc: "Cleanup zombies farther than this from all players", params: "500 4000 100", category: "Zombies")]
	protected float m_fZombieCleanupDistance;

	[Attribute(defvalue: "12", UIWidgets.Slider, desc: "Max zombies spawned per update", params: "1 50 1", category: "Zombies")]
	protected int m_iMaxZombieSpawnPerUpdate;

	[Attribute(defvalue: "", UIWidgets.ResourcePickerThumbnail, desc: "Zombie prefabs. These should contain their own faction/AI behavior.", params: "et", category: "Zombies")]
	protected ref array<ResourceName> m_aZombiePrefabs;

	//------------------------------------------------------------------------------------------------
	// Objectives
	//------------------------------------------------------------------------------------------------
	[Attribute(defvalue: "5", UIWidgets.Slider, desc: "Maximum active objectives", params: "1 20 1", category: "Objectives")]
	protected int m_iMaxActiveObjectives;

	[Attribute(defvalue: "300", UIWidgets.Slider, desc: "Seconds between objective generation", params: "60 1200 30", category: "Objectives")]
	protected float m_fObjectiveInterval;

	[Attribute(defvalue: "600", UIWidgets.Slider, desc: "Minimum distance between objectives", params: "100 2500 100", category: "Objectives")]
	protected float m_fMinObjectiveDistance;

	[Attribute(defvalue: "250", UIWidgets.Slider, desc: "Minimum distance from players for new objectives", params: "0 2000 50", category: "Objectives")]
	protected float m_fMinPlayerDistance;

	[Attribute(defvalue: "1800", UIWidgets.Slider, desc: "Max objective lifetime in seconds. 0 disables timeout.", params: "0 7200 60", category: "Objectives")]
	protected float m_fMaxObjectiveTime;

	[Attribute(defvalue: "180", UIWidgets.Slider, desc: "Completed objective cleanup grace period", params: "30 900 30", category: "Objectives")]
	protected float m_fObjectiveCompletionGracePeriod;

	//------------------------------------------------------------------------------------------------
	// Vehicle scarcity
	//------------------------------------------------------------------------------------------------
	[Attribute(defvalue: "1", UIWidgets.CheckBox, desc: "Enable scarce vehicle spawning", category: "Vehicle Scarcity")]
	protected bool m_bEnableVehicleScarcity;

	[Attribute(defvalue: "18", UIWidgets.Slider, desc: "Maximum spawned vehicles", params: "0 100 1", category: "Vehicle Scarcity")]
	protected int m_iMaxVehicles;

	[Attribute(defvalue: "0.15", UIWidgets.Slider, desc: "Vehicle spawn chance per vehicle update", params: "0.01 1.0 0.01", category: "Vehicle Scarcity")]
	protected float m_fVehicleSpawnChance;

	[Attribute(defvalue: "300", UIWidgets.Slider, desc: "Vehicle scarcity update interval", params: "60 1800 60", category: "Vehicle Scarcity")]
	protected float m_fVehicleUpdateInterval;

	[Attribute(defvalue: "", UIWidgets.ResourcePickerThumbnail, desc: "Scarce vehicle prefabs", params: "et", category: "Vehicle Scarcity")]
	protected ref array<ResourceName> m_aVehiclePrefabs;

	//------------------------------------------------------------------------------------------------
	// Location filters
	//------------------------------------------------------------------------------------------------
	[Attribute(defvalue: "1", UIWidgets.CheckBox, desc: "Include cities", category: "Locations")]
	protected bool m_bIncludeCities;

	[Attribute(defvalue: "1", UIWidgets.CheckBox, desc: "Include towns", category: "Locations")]
	protected bool m_bIncludeTowns;

	[Attribute(defvalue: "1", UIWidgets.CheckBox, desc: "Include villages", category: "Locations")]
	protected bool m_bIncludeVillages;

	[Attribute(defvalue: "1", UIWidgets.CheckBox, desc: "Include military bases", category: "Locations")]
	protected bool m_bIncludeMilitaryBases;

	[Attribute(defvalue: "1", UIWidgets.CheckBox, desc: "Include named local map locations", category: "Locations")]
	protected bool m_bIncludeNamedLocations;

	[Attribute(defvalue: "1", UIWidgets.CheckBox, desc: "Generate wilderness locations", category: "Locations")]
	protected bool m_bGenerateRandomLocations;

	[Attribute(defvalue: "75", UIWidgets.Slider, desc: "Number of generated wilderness locations", params: "0 300 10", category: "Locations")]
	protected int m_iRandomLocationCount;

	//------------------------------------------------------------------------------------------------
	// Loot tables
	//------------------------------------------------------------------------------------------------
	[Attribute(defvalue: "", UIWidgets.ResourcePickerThumbnail, desc: "Common loot prefabs", params: "et", category: "Loot")]
	protected ref array<ResourceName> m_aCommonLootPrefabs;

	[Attribute(defvalue: "", UIWidgets.ResourcePickerThumbnail, desc: "Uncommon loot prefabs", params: "et", category: "Loot")]
	protected ref array<ResourceName> m_aUncommonLootPrefabs;

	[Attribute(defvalue: "", UIWidgets.ResourcePickerThumbnail, desc: "Rare loot prefabs", params: "et", category: "Loot")]
	protected ref array<ResourceName> m_aRareLootPrefabs;

	[Attribute(defvalue: "", UIWidgets.ResourcePickerThumbnail, desc: "Military loot prefabs", params: "et", category: "Loot")]
	protected ref array<ResourceName> m_aMilitaryLootPrefabs;

	//------------------------------------------------------------------------------------------------
	// Performance
	//------------------------------------------------------------------------------------------------
	[Attribute(defvalue: "1", UIWidgets.CheckBox, desc: "Enable performance scaling", category: "Performance")]
	protected bool m_bEnablePerformanceScaling;

	[Attribute(defvalue: "30", UIWidgets.Slider, desc: "Target server FPS", params: "15 60 1", category: "Performance")]
	protected float m_fTargetFPS;

	[Attribute(defvalue: "25", UIWidgets.Slider, desc: "Warning server FPS", params: "10 60 1", category: "Performance")]
	protected float m_fWarningFPS;

	[Attribute(defvalue: "20", UIWidgets.Slider, desc: "Critical server FPS", params: "5 60 1", category: "Performance")]
	protected float m_fCriticalFPS;

	[Attribute(defvalue: "35", UIWidgets.Slider, desc: "Minimum zombie cap under performance pressure", params: "10 300 5", category: "Performance")]
	protected int m_iMinimumZombieCap;

	[Attribute(defvalue: "15", UIWidgets.Slider, desc: "Zombie cap adjustment step", params: "5 100 5", category: "Performance")]
	protected int m_iZombieAdjustStep;

	//------------------------------------------------------------------------------------------------
	// Runtime managers
	//------------------------------------------------------------------------------------------------
	protected ref ARMAZ_LocationDiscovery m_LocationDiscovery;
	protected ref ARMAZ_ZombieSpawnManager m_ZombieSpawnManager;
	protected ref ARMAZ_ObjectiveDirector m_ObjectiveDirector;
	protected ref ARMAZ_VehicleScarcityManager m_VehicleScarcityManager;
	protected ref ARMAZ_LootManager m_LootManager;
	protected ref ARMAZ_PerformanceManager m_PerformanceManager;

	protected float m_fLastSystemUpdateTime = 0;
	protected bool m_bInitialized = false;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		s_Instance = this;

		if (!GetGame().InPlayMode())
			return;

		GetGame().GetCallqueue().CallLater(InitializeARMAZ, 1000, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void InitializeARMAZ()
	{
		if (!m_bEnableARMAZ || m_bInitialized)
			return;

		if (!Replication.IsServer())
		{
			Print("[ARMAZ] Client component present. Server owns ARMAZ runtime managers.", LogLevel.NORMAL);
			return;
		}

		m_LocationDiscovery = new ARMAZ_LocationDiscovery();
		m_LocationDiscovery.SetFilters(m_bIncludeCities, m_bIncludeTowns, m_bIncludeVillages, m_bIncludeMilitaryBases, m_bIncludeNamedLocations);
		m_LocationDiscovery.SetRandomLocationGeneration(m_bGenerateRandomLocations, m_iRandomLocationCount);
		m_LocationDiscovery.DiscoverLocations();

		m_LootManager = new ARMAZ_LootManager(m_aCommonLootPrefabs, m_aUncommonLootPrefabs, m_aRareLootPrefabs, m_aMilitaryLootPrefabs);

		m_ZombieSpawnManager = new ARMAZ_ZombieSpawnManager(m_aZombiePrefabs, m_iMaxZombies, m_fZombieSpawnRadius, m_fZombieSafeRadius);
		m_ZombieSpawnManager.Configure(m_iMaxZombies, m_fZombieSpawnRadius, m_fZombieSafeRadius, m_fZombieCleanupDistance, m_iMaxZombieSpawnPerUpdate);

		m_ObjectiveDirector = new ARMAZ_ObjectiveDirector(m_LocationDiscovery, m_LootManager);
		m_ObjectiveDirector.Configure(m_iMaxActiveObjectives, m_fObjectiveInterval, m_fMinObjectiveDistance, m_fMinPlayerDistance, m_fMaxObjectiveTime, m_fObjectiveCompletionGracePeriod);

		m_VehicleScarcityManager = new ARMAZ_VehicleScarcityManager(m_LocationDiscovery, m_aVehiclePrefabs);
		m_VehicleScarcityManager.Configure(m_bEnableVehicleScarcity, m_iMaxVehicles, m_fVehicleSpawnChance, m_fVehicleUpdateInterval, 2500);

		m_PerformanceManager = new ARMAZ_PerformanceManager(m_ZombieSpawnManager, m_iMaxZombies);
		m_PerformanceManager.Configure(m_fTargetFPS, m_fWarningFPS, m_fCriticalFPS, m_iMinimumZombieCap, m_iZombieAdjustStep, 45);

		m_bInitialized = true;
		m_fLastSystemUpdateTime = 0;

		Print(string.Format("[ARMAZ] Initialized. Locations=%1 MaxZombies=%2 MaxObjectives=%3", m_LocationDiscovery.GetLocationCount(), m_iMaxZombies, m_iMaxActiveObjectives), LogLevel.NORMAL);

		int intervalMs = m_fSystemUpdateInterval * 1000;
		if (intervalMs < 1000)
			intervalMs = 1000;
		GetGame().GetCallqueue().CallLater(UpdateARMAZSystems, intervalMs, true);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateARMAZSystems()
	{
		if (!m_bInitialized || !Replication.IsServer())
			return;

		if (m_bEnablePerformanceScaling && m_PerformanceManager)
			m_PerformanceManager.Update(m_fSystemUpdateInterval);
		if (m_ObjectiveDirector)
			m_ObjectiveDirector.Update();

		array<ref ARMAZ_ObjectiveBase> activeObjectives = {};
		if (m_ObjectiveDirector)
			m_ObjectiveDirector.GetActiveObjectives(activeObjectives);

		bool pauseSpawning = m_PerformanceManager && m_PerformanceManager.ShouldPauseSpawning();
		if (m_ZombieSpawnManager)
		{
			m_ZombieSpawnManager.SetEnabled(!pauseSpawning);
			m_ZombieSpawnManager.Update(activeObjectives);
		}

		if (m_VehicleScarcityManager)
			m_VehicleScarcityManager.Update();
	}

	//------------------------------------------------------------------------------------------------
	static ARMAZ_GameModeComponent GetInstance()
	{
		return s_Instance;
	}

	ARMAZ_LocationDiscovery GetLocationDiscovery() { return m_LocationDiscovery; }
	ARMAZ_ZombieSpawnManager GetZombieSpawnManager() { return m_ZombieSpawnManager; }
	ARMAZ_ObjectiveDirector GetObjectiveDirector() { return m_ObjectiveDirector; }
	ARMAZ_VehicleScarcityManager GetVehicleScarcityManager() { return m_VehicleScarcityManager; }
	ARMAZ_LootManager GetLootManager() { return m_LootManager; }
	ARMAZ_PerformanceManager GetPerformanceManager() { return m_PerformanceManager; }
}
