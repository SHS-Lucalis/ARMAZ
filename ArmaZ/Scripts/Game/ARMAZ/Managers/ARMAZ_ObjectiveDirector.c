//------------------------------------------------------------------------------------------------
//! ARMAZ Objective Director
//! Creates survival objectives and lets zombie spawning remain passive/natural.
//------------------------------------------------------------------------------------------------

class ARMAZ_ObjectiveDirector
{
	protected ARMAZ_LocationDiscovery m_LocationDiscovery;
	protected ARMAZ_LootManager m_LootManager;
	protected ref array<ref ARMAZ_ObjectiveBase> m_aActiveObjectives = {};
	protected int m_iMaxActiveObjectives = 5;
	protected float m_fObjectiveInterval = 300;
	protected float m_fLastObjectiveSpawnTime = 0;
	protected float m_fMinObjectiveDistance = 600;
	protected float m_fMinPlayerDistance = 250;
	protected float m_fMaxObjectiveTime = 1800;
	protected float m_fCompletionGracePeriod = 180;
	protected int m_iNextObjectiveId = 1;
	protected bool m_bEnabled = true;

	void ARMAZ_ObjectiveDirector(ARMAZ_LocationDiscovery locationDiscovery, ARMAZ_LootManager lootManager)
	{
		m_LocationDiscovery = locationDiscovery;
		m_LootManager = lootManager;
		m_aActiveObjectives = new array<ref ARMAZ_ObjectiveBase>();
	}

	void Configure(int maxActive, float interval, float minObjectiveDistance, float minPlayerDistance, float maxObjectiveTime, float completionGracePeriod)
	{
		m_iMaxActiveObjectives = maxActive;
		m_fObjectiveInterval = interval;
		m_fMinObjectiveDistance = minObjectiveDistance;
		m_fMinPlayerDistance = minPlayerDistance;
		m_fMaxObjectiveTime = maxObjectiveTime;
		m_fCompletionGracePeriod = completionGracePeriod;
	}

	void SetEnabled(bool enabled) { m_bEnabled = enabled; }
	int GetActiveObjectiveCount() { return m_aActiveObjectives.Count(); }
	void GetActiveObjectives(notnull array<ref ARMAZ_ObjectiveBase> outObjectives)
	{
		foreach (ARMAZ_ObjectiveBase obj : m_aActiveObjectives)
			outObjectives.Insert(obj);
	}

	void Update()
	{
		UpdateObjectives();
		CleanupFinishedObjectives();

		if (!m_bEnabled)
			return;

		float now = GetGame().GetWorld().GetWorldTime() / 1000;
		if (m_aActiveObjectives.Count() >= m_iMaxActiveObjectives)
			return;

		if (m_aActiveObjectives.Count() > 0 && now - m_fLastObjectiveSpawnTime < m_fObjectiveInterval)
			return;

		SpawnObjective();
	}

	protected void UpdateObjectives()
	{
		foreach (ARMAZ_ObjectiveBase obj : m_aActiveObjectives)
		{
			if (obj)
				obj.Update(m_LootManager);
		}
	}

	protected void CleanupFinishedObjectives()
	{
		for (int i = m_aActiveObjectives.Count() - 1; i >= 0; i--)
		{
			ARMAZ_ObjectiveBase obj = m_aActiveObjectives[i];
			if (!obj)
			{
				m_aActiveObjectives.Remove(i);
				continue;
			}

			if (!obj.ShouldCleanup())
				continue;

			if (m_LocationDiscovery)
				m_LocationDiscovery.ReleaseLocation(obj.GetLocation());

			obj.Cleanup(false);
			m_aActiveObjectives.Remove(i);
		}
	}

	protected bool SpawnObjective()
	{
		if (!m_LocationDiscovery)
			return false;

		array<vector> existingPositions = {};
		foreach (ARMAZ_ObjectiveBase active : m_aActiveObjectives)
		{
			if (active)
				existingPositions.Insert(active.GetPosition());
		}

		array<vector> playerPositions = {};
		ARMAZ_ZombieSpawnManager.GetPlayerPositions(playerPositions);

		ARMAZ_DiscoveredLocation loc = m_LocationDiscovery.GetRandomAvailableLocation(existingPositions, playerPositions, m_fMinObjectiveDistance, m_fMinPlayerDistance);
		if (!loc)
			return false;

		ARMAZ_ObjectiveType type = GetRandomObjectiveType(loc);
		ARMAZ_ObjectiveBase objective = CreateObjectiveForType(type, m_iNextObjectiveId, loc);
		if (!objective || !objective.Initialize(type, m_fMaxObjectiveTime, m_fCompletionGracePeriod))
		{
			m_LocationDiscovery.ReleaseLocation(loc);
			return false;
		}

		m_iNextObjectiveId++;
		m_fLastObjectiveSpawnTime = GetGame().GetWorld().GetWorldTime() / 1000;
		m_aActiveObjectives.Insert(objective);
		return true;
	}

	protected ARMAZ_ObjectiveType GetRandomObjectiveType(ARMAZ_DiscoveredLocation loc)
	{
		if (loc && loc.IsMilitary())
		{
			int militaryRoll = Math.RandomInt(0, 100);
			if (militaryRoll < 45)
				return ARMAZ_ObjectiveType.MILITARY_STASH;
			if (militaryRoll < 70)
				return ARMAZ_ObjectiveType.VEHICLE_PARTS;
		}

		int roll = Math.RandomInt(0, 100);
		if (roll < 20) return ARMAZ_ObjectiveType.LOOT_CACHE;
		if (roll < 35) return ARMAZ_ObjectiveType.MEDICAL_SUPPLY;
		if (roll < 50) return ARMAZ_ObjectiveType.FUEL_DEPOT;
		if (roll < 63) return ARMAZ_ObjectiveType.RADIO_TOWER;
		if (roll < 75) return ARMAZ_ObjectiveType.RESTORE_POWER;
		if (roll < 88) return ARMAZ_ObjectiveType.VEHICLE_PARTS;
		if (roll < 95) return ARMAZ_ObjectiveType.EXTRACTION_INTEL;
		return ARMAZ_ObjectiveType.MILITARY_STASH;
	}

	protected ARMAZ_ObjectiveBase CreateObjectiveForType(ARMAZ_ObjectiveType type, int id, ARMAZ_DiscoveredLocation loc)
	{
		// All first-pass objectives use the same base behavior.
		// The type drives reward tier, notification text, and future extension points.
		return new ARMAZ_ObjectiveBase(id, loc);
	}
}