//------------------------------------------------------------------------------------------------
//! ARMAZ Vehicle Scarcity Manager
//! Spawns rare vehicles. No ambient traffic loop.
//------------------------------------------------------------------------------------------------

class ARMAZ_VehicleSpawnRecord
{
	IEntity m_Vehicle;
	vector m_vPosition;
	ARMAZ_VehicleCondition m_eCondition;
	float m_fSpawnTime;

	void ARMAZ_VehicleSpawnRecord(IEntity vehicle, vector position, ARMAZ_VehicleCondition condition)
	{
		m_Vehicle = vehicle;
		m_vPosition = position;
		m_eCondition = condition;
		m_fSpawnTime = GetGame().GetWorld().GetWorldTime() / 1000;
	}
}

class ARMAZ_VehicleScarcityManager
{
	protected ARMAZ_LocationDiscovery m_LocationDiscovery;
	protected ref array<ResourceName> m_aVehiclePrefabs = {};
	protected ref array<ref ARMAZ_VehicleSpawnRecord> m_aSpawnedVehicles = {};

	protected bool m_bEnabled = true;
	protected int m_iMaxVehicles = 18;
	protected float m_fSpawnChance = 0.15;
	protected float m_fUpdateInterval = 300;
	protected float m_fLastUpdateTime = 0;
	protected float m_fCleanupDistance = 2500;

	void ARMAZ_VehicleScarcityManager(ARMAZ_LocationDiscovery locationDiscovery, array<ResourceName> vehiclePrefabs)
	{
		m_LocationDiscovery = locationDiscovery;
		m_aVehiclePrefabs = new array<ResourceName>();
		m_aSpawnedVehicles = new array<ref ARMAZ_VehicleSpawnRecord>();

		if (vehiclePrefabs)
		{
			foreach (ResourceName prefab : vehiclePrefabs)
			{
				if (!prefab.IsEmpty())
					m_aVehiclePrefabs.Insert(prefab);
			}
		}
	}

	void Configure(bool enabled, int maxVehicles, float spawnChance, float updateInterval, float cleanupDistance)
	{
		m_bEnabled = enabled;
		m_iMaxVehicles = maxVehicles;
		m_fSpawnChance = spawnChance;
		m_fUpdateInterval = updateInterval;
		m_fCleanupDistance = cleanupDistance;
	}

	int GetVehicleCount() { CleanupInvalidReferences(); return m_aSpawnedVehicles.Count(); }

	void Update()
	{
		CleanupInvalidReferences();

		if (!m_bEnabled || m_aVehiclePrefabs.Count() == 0 || !m_LocationDiscovery)
			return;

		float now = GetGame().GetWorld().GetWorldTime() / 1000;
		if (now - m_fLastUpdateTime < m_fUpdateInterval)
			return;

		m_fLastUpdateTime = now;

		if (m_aSpawnedVehicles.Count() >= m_iMaxVehicles)
			return;

		if (Math.RandomFloat(0, 1) > m_fSpawnChance)
			return;

		TrySpawnVehicle();
	}

	protected void TrySpawnVehicle()
	{
		array<ref ARMAZ_DiscoveredLocation> locations = {};
		m_LocationDiscovery.GetAllLocations(locations);
		if (locations.Count() == 0)
			return;

		for (int attempt = 0; attempt < 10; attempt++)
		{
			ARMAZ_DiscoveredLocation loc = locations[Math.RandomInt(0, locations.Count())];
			vector pos = GetVehiclePositionNear(loc.GetPosition(), loc.GetRadius());
			if (IsZeroVector(pos))
				continue;

			if (IsTooCloseToExistingVehicle(pos, 200))
				continue;

			SpawnVehicle(pos);
			return;
		}
	}

	protected vector GetVehiclePositionNear(vector center, float radius)
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return "0 0 0";

		for (int i = 0; i < 16; i++)
		{
			float angle = Math.RandomFloat(0, Math.PI2);
			float distance = Math.RandomFloat(20, Math.Max(radius, 80));
			float x = center[0] + Math.Cos(angle) * distance;
			float z = center[2] + Math.Sin(angle) * distance;
			float y = world.GetSurfaceY(x, z);
			vector pos = Vector(x, y, z);

			if (ARMAZ_LocationDiscovery.IsValidGroundPosition(pos))
				return pos;
		}

		return "0 0 0";
	}

	protected IEntity SpawnVehicle(vector position)
	{
		ResourceName prefab = m_aVehiclePrefabs[Math.RandomInt(0, m_aVehiclePrefabs.Count())];
		Resource res = Resource.Load(prefab);
		if (!res)
			return null;

		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = position;
		Math3D.AnglesToMatrix(Vector(Math.RandomFloat(0, 360), 0, 0), params.Transform);
		params.Transform[3] = position;

		IEntity vehicle = GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), params);
		if (!vehicle)
			return null;

		ARMAZ_VehicleCondition condition = RollVehicleCondition();
		ApplyVehicleCondition(vehicle, condition);
		m_aSpawnedVehicles.Insert(new ARMAZ_VehicleSpawnRecord(vehicle, position, condition));

		if (condition == ARMAZ_VehicleCondition.WORKING_LOW_FUEL || condition == ARMAZ_VehicleCondition.RARE_WORKING)
			ARMAZ_Notification.ShowVehicleFound(position);

		return vehicle;
	}

	protected ARMAZ_VehicleCondition RollVehicleCondition()
	{
		int roll = Math.RandomInt(0, 100);
		if (roll < 50) return ARMAZ_VehicleCondition.WRECK;
		if (roll < 75) return ARMAZ_VehicleCondition.DAMAGED_NO_FUEL;
		if (roll < 90) return ARMAZ_VehicleCondition.DAMAGED_LOW_FUEL;
		if (roll < 98) return ARMAZ_VehicleCondition.WORKING_LOW_FUEL;
		return ARMAZ_VehicleCondition.RARE_WORKING;
	}

	protected string GetVehicleConditionName(ARMAZ_VehicleCondition condition)
	{
		switch (condition)
		{
			case ARMAZ_VehicleCondition.WRECK: return "WRECK";
			case ARMAZ_VehicleCondition.DAMAGED_NO_FUEL: return "DAMAGED_NO_FUEL";
			case ARMAZ_VehicleCondition.DAMAGED_LOW_FUEL: return "DAMAGED_LOW_FUEL";
			case ARMAZ_VehicleCondition.WORKING_LOW_FUEL: return "WORKING_LOW_FUEL";
			case ARMAZ_VehicleCondition.RARE_WORKING: return "RARE_WORKING";
		}

		return "UNKNOWN";
	}

	protected void ApplyVehicleCondition(IEntity vehicle, ARMAZ_VehicleCondition condition)
	{
		// Hook point: fuel/damage APIs vary by vehicle prefab and Reforger version.
		// This first pass marks scarcity state through spawn probability and logs the intended condition.
		Print(string.Format("[ARMAZ-Vehicles] Spawned vehicle condition=%1 at %2", GetVehicleConditionName(condition), vehicle.GetOrigin().ToString()), LogLevel.NORMAL);
	}

	protected bool IsTooCloseToExistingVehicle(vector pos, float distance)
	{
		foreach (ARMAZ_VehicleSpawnRecord record : m_aSpawnedVehicles)
		{
			if (record && vector.Distance(record.m_vPosition, pos) < distance)
				return true;
		}

		return false;
	}

	protected void CleanupInvalidReferences()
	{
		for (int i = m_aSpawnedVehicles.Count() - 1; i >= 0; i--)
		{
			ARMAZ_VehicleSpawnRecord record = m_aSpawnedVehicles[i];
			if (!record || !record.m_Vehicle)
			{
				m_aSpawnedVehicles.Remove(i);
				continue;
			}

			// Keep player-found vehicles around. Only remove distant wrecks/damaged spawns when no player is nearby.
			if (IsTooFarFromPlayers(record.m_Vehicle.GetOrigin()) && record.m_eCondition == ARMAZ_VehicleCondition.WRECK)
			{
				SCR_EntityHelper.DeleteEntityAndChildren(record.m_Vehicle);
				m_aSpawnedVehicles.Remove(i);
			}
		}
	}

	protected bool IsTooFarFromPlayers(vector pos)
	{
		array<vector> playerPositions = {};
		ARMAZ_ZombieSpawnManager.GetPlayerPositions(playerPositions);

		if (playerPositions.Count() == 0)
			return false;

		foreach (vector playerPos : playerPositions)
		{
			if (vector.Distance(pos, playerPos) < m_fCleanupDistance)
				return false;
		}

		return true;
	}
	protected bool IsZeroVector(vector pos)
	{
		return pos[0] == 0 && pos[1] == 0 && pos[2] == 0;
	}

}
