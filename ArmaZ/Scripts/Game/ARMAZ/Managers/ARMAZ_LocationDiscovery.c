//------------------------------------------------------------------------------------------------
//! ARMAZ Location Discovery
//! Discovers map locations and optional generated wilderness points for zombie/objective placement.
//------------------------------------------------------------------------------------------------

class ARMAZ_DiscoveredLocation
{
	protected string m_sName;
	protected vector m_vPosition;
	protected EMapDescriptorType m_eType;
	protected float m_fRadius;
	protected bool m_bInUse;
	protected IEntity m_LocationEntity;
	protected bool m_bGenerated;

	void ARMAZ_DiscoveredLocation(string name, vector position, EMapDescriptorType type, float radius, IEntity entity = null, bool generated = false)
	{
		m_sName = name;
		m_vPosition = position;
		m_eType = type;
		m_fRadius = radius;
		m_LocationEntity = entity;
		m_bGenerated = generated;
		m_bInUse = false;
	}

	string GetName() { return m_sName; }
	vector GetPosition() { return m_vPosition; }
	EMapDescriptorType GetType() { return m_eType; }
	float GetRadius() { return m_fRadius; }
	bool IsInUse() { return m_bInUse; }
	bool IsGenerated() { return m_bGenerated; }
	IEntity GetEntity() { return m_LocationEntity; }
	void SetInUse(bool inUse) { m_bInUse = inUse; }

	bool IsMilitary()
	{
		switch (m_eType)
		{
			case EMapDescriptorType.MDT_AIRPORT:
			case EMapDescriptorType.MDT_BASE:
			case EMapDescriptorType.MDT_PORT:
				return true;
		}

		return false;
	}

	string GetTypeName()
	{
		if (m_bGenerated)
			return "Wilderness";

		switch (m_eType)
		{
			case EMapDescriptorType.MDT_NAME_CITY: return "City";
			case EMapDescriptorType.MDT_NAME_TOWN: return "Town";
			case EMapDescriptorType.MDT_NAME_VILLAGE: return "Village";
			case EMapDescriptorType.MDT_NAME_SETTLEMENT: return "Settlement";
			case EMapDescriptorType.MDT_NAME_LOCAL: return "Location";
			case EMapDescriptorType.MDT_AIRPORT: return "Airport";
			case EMapDescriptorType.MDT_BASE: return "Base";
			case EMapDescriptorType.MDT_PORT: return "Port";
		}

		return "Area";
	}
}

class ARMAZ_LocationDiscovery
{
	protected ref array<ref ARMAZ_DiscoveredLocation> m_aLocations = {};
	protected ref array<ref ARMAZ_DiscoveredLocation> m_aGeneratedLocations = {};
	protected ref array<EMapDescriptorType> m_aAllowedTypes = {};

	protected bool m_bIncludeCities = true;
	protected bool m_bIncludeTowns = true;
	protected bool m_bIncludeVillages = true;
	protected bool m_bIncludeMilitaryBases = true;
	protected bool m_bIncludeNamedLocations = true;
	protected bool m_bGenerateRandomLocations = true;
	protected int m_iRandomLocationCount = 75;

	protected ref array<string> m_aGeneratedNames = {
		"Abandoned Farm", "Overgrown Roadblock", "Dead Radio Site", "Forest Camp", "Silent Checkpoint",
		"Collapsed Supply Point", "Overrun House", "Old Hunting Site", "Burned Convoy", "Forgotten Trail"
	};

	void ARMAZ_LocationDiscovery()
	{
		m_aLocations = new array<ref ARMAZ_DiscoveredLocation>();
		m_aGeneratedLocations = new array<ref ARMAZ_DiscoveredLocation>();
		m_aAllowedTypes = new array<EMapDescriptorType>();
		UpdateAllowedTypes();
	}

	void SetFilters(bool cities, bool towns, bool villages, bool bases, bool namedLocations)
	{
		m_bIncludeCities = cities;
		m_bIncludeTowns = towns;
		m_bIncludeVillages = villages;
		m_bIncludeMilitaryBases = bases;
		m_bIncludeNamedLocations = namedLocations;
		UpdateAllowedTypes();
	}

	void SetRandomLocationGeneration(bool enable, int count)
	{
		m_bGenerateRandomLocations = enable;
		m_iRandomLocationCount = count;
	}

	protected void UpdateAllowedTypes()
	{
		m_aAllowedTypes.Clear();

		if (m_bIncludeCities)
			m_aAllowedTypes.Insert(EMapDescriptorType.MDT_NAME_CITY);

		if (m_bIncludeTowns)
		{
			m_aAllowedTypes.Insert(EMapDescriptorType.MDT_NAME_TOWN);
			m_aAllowedTypes.Insert(EMapDescriptorType.MDT_NAME_SETTLEMENT);
		}

		if (m_bIncludeVillages)
			m_aAllowedTypes.Insert(EMapDescriptorType.MDT_NAME_VILLAGE);

		if (m_bIncludeMilitaryBases)
		{
			m_aAllowedTypes.Insert(EMapDescriptorType.MDT_BASE);
			m_aAllowedTypes.Insert(EMapDescriptorType.MDT_AIRPORT);
			m_aAllowedTypes.Insert(EMapDescriptorType.MDT_PORT);
		}

		if (m_bIncludeNamedLocations)
			m_aAllowedTypes.Insert(EMapDescriptorType.MDT_NAME_LOCAL);
	}

	int DiscoverLocations()
	{
		m_aLocations.Clear();
		m_aGeneratedLocations.Clear();

		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return 0;

		DiscoverFromMapDescriptors();

		if (m_bGenerateRandomLocations)
			GenerateRandomLocations();

		foreach (ARMAZ_DiscoveredLocation generated : m_aGeneratedLocations)
			m_aLocations.Insert(generated);

		Print(string.Format("[ARMAZ-Location] Discovery complete. Total=%1", m_aLocations.Count()), LogLevel.NORMAL);
		return m_aLocations.Count();
	}

	protected void DiscoverFromMapDescriptors()
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return;

		vector mins, maxs;
		world.GetBoundBox(mins, maxs);
		vector center = (mins + maxs) * 0.5;
		float radius = vector.Distance(mins, maxs);

		world.QueryEntitiesBySphere(center, radius, ProcessMapDescriptorEntity, FilterMapDescriptorEntity, EQueryEntitiesFlags.ALL);
	}

	protected bool FilterMapDescriptorEntity(IEntity entity)
	{
		return entity && entity.FindComponent(MapDescriptorComponent) != null;
	}

	protected bool ProcessMapDescriptorEntity(IEntity entity)
	{
		MapDescriptorComponent mapDesc = MapDescriptorComponent.Cast(entity.FindComponent(MapDescriptorComponent));
		if (!mapDesc)
			return true;

		MapItem mapItem = mapDesc.Item();
		if (!mapItem)
			return true;

		EMapDescriptorType type = mapItem.GetBaseType();
		if (!m_aAllowedTypes.Contains(type))
			return true;

		string name = mapItem.GetDisplayName();
		if (name.IsEmpty())
			return true;

		vector pos = entity.GetOrigin();

		foreach (ARMAZ_DiscoveredLocation existing : m_aLocations)
		{
			if (vector.Distance(existing.GetPosition(), pos) < 50)
				return true;
		}

		m_aLocations.Insert(new ARMAZ_DiscoveredLocation(name, pos, type, GetRadiusForType(type), entity, false));
		return true;
	}

	protected float GetRadiusForType(EMapDescriptorType type)
	{
		switch (type)
		{
			case EMapDescriptorType.MDT_NAME_CITY: return 450;
			case EMapDescriptorType.MDT_NAME_TOWN: return 300;
			case EMapDescriptorType.MDT_NAME_VILLAGE: return 180;
			case EMapDescriptorType.MDT_NAME_SETTLEMENT: return 180;
			case EMapDescriptorType.MDT_NAME_LOCAL: return 125;
			case EMapDescriptorType.MDT_BASE: return 350;
			case EMapDescriptorType.MDT_AIRPORT: return 500;
			case EMapDescriptorType.MDT_PORT: return 300;
		}

		return 175;
	}

	protected void GenerateRandomLocations()
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return;

		vector mins, maxs;
		world.GetBoundBox(mins, maxs);

		mins[0] = mins[0] + 300;
		mins[2] = mins[2] + 300;
		maxs[0] = maxs[0] - 300;
		maxs[2] = maxs[2] - 300;

		int generated = 0;
		int attempts = 0;
		int maxAttempts = m_iRandomLocationCount * 12;

		while (generated < m_iRandomLocationCount && attempts < maxAttempts)
		{
			attempts++;

			float x = Math.RandomFloat(mins[0], maxs[0]);
			float z = Math.RandomFloat(mins[2], maxs[2]);
			float y = world.GetSurfaceY(x, z);
			vector pos = Vector(x, y, z);

			if (!IsValidGroundPosition(pos))
				continue;

			if (IsTooCloseToKnownLocation(pos, 350))
				continue;

			string name = m_aGeneratedNames[Math.RandomInt(0, m_aGeneratedNames.Count())];
			m_aGeneratedLocations.Insert(new ARMAZ_DiscoveredLocation(name, pos, EMapDescriptorType.MDT_NAME_LOCAL, 150, null, true));
			generated++;
		}
	}

	protected bool IsTooCloseToKnownLocation(vector pos, float minDistance)
	{
		foreach (ARMAZ_DiscoveredLocation existing : m_aLocations)
		{
			if (vector.Distance(existing.GetPosition(), pos) < minDistance)
				return true;
		}

		foreach (ARMAZ_DiscoveredLocation generated : m_aGeneratedLocations)
		{
			if (vector.Distance(generated.GetPosition(), pos) < minDistance)
				return true;
		}

		return false;
	}

	static bool IsValidGroundPosition(vector pos)
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return false;

		TraceParam trace = new TraceParam();
		trace.Start = pos + Vector(0, 5, 0);
		trace.End = pos - Vector(0, 20, 0);
		trace.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		trace.LayerMask = EPhysicsLayerPresets.Projectile;

		float hit = world.TraceMove(trace, null);
		return hit < 1.0;
	}

	ARMAZ_DiscoveredLocation GetRandomAvailableLocation(array<vector> existingPositions, array<vector> playerPositions, float minObjectiveDistance, float minPlayerDistance)
	{
		array<ref ARMAZ_DiscoveredLocation> candidates = {};

		foreach (ARMAZ_DiscoveredLocation loc : m_aLocations)
		{
			if (loc.IsInUse())
				continue;

			vector pos = loc.GetPosition();
			bool valid = true;

			if (existingPositions)
			{
				foreach (vector objPos : existingPositions)
				{
					if (vector.Distance(pos, objPos) < minObjectiveDistance)
					{
						valid = false;
						break;
					}
				}
			}

			if (valid && playerPositions && minPlayerDistance > 0)
			{
				foreach (vector playerPos : playerPositions)
				{
					if (vector.Distance(pos, playerPos) < minPlayerDistance)
					{
						valid = false;
						break;
					}
				}
			}

			if (valid)
				candidates.Insert(loc);
		}

		if (candidates.Count() == 0)
			return null;

		ARMAZ_DiscoveredLocation selected = candidates[Math.RandomInt(0, candidates.Count())];
		selected.SetInUse(true);
		return selected;
	}

	void ReleaseLocation(ARMAZ_DiscoveredLocation location)
	{
		if (location)
			location.SetInUse(false);
	}

	int GetLocationCount() { return m_aLocations.Count(); }
	int GetAvailableLocationCount()
	{
		int count = 0;
		foreach (ARMAZ_DiscoveredLocation loc : m_aLocations)
		{
			if (!loc.IsInUse())
				count++;
		}
		return count;
	}

	void GetAllLocations(notnull array<ref ARMAZ_DiscoveredLocation> outLocations)
	{
		foreach (ARMAZ_DiscoveredLocation loc : m_aLocations)
			outLocations.Insert(loc);
	}
}
