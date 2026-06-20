//------------------------------------------------------------------------------------------------
//! ARMAZ Loot Manager
//! Spawns configured reward entities around objective reward positions.
//------------------------------------------------------------------------------------------------

class ARMAZ_LootManager
{
	protected ref array<ResourceName> m_aCommonLoot = {};
	protected ref array<ResourceName> m_aUncommonLoot = {};
	protected ref array<ResourceName> m_aRareLoot = {};
	protected ref array<ResourceName> m_aMilitaryLoot = {};

	void ARMAZ_LootManager(array<ResourceName> commonLoot, array<ResourceName> uncommonLoot, array<ResourceName> rareLoot, array<ResourceName> militaryLoot)
	{
		CopyPrefabs(commonLoot, m_aCommonLoot);
		CopyPrefabs(uncommonLoot, m_aUncommonLoot);
		CopyPrefabs(rareLoot, m_aRareLoot);
		CopyPrefabs(militaryLoot, m_aMilitaryLoot);
	}

	protected void CopyPrefabs(array<ResourceName> source, notnull array<ResourceName> target)
	{
		if (!source)
			return;

		foreach (ResourceName prefab : source)
		{
			if (!prefab.IsEmpty())
				target.Insert(prefab);
		}
	}

	void SpawnObjectiveReward(ARMAZ_ObjectiveType type, vector position, notnull array<IEntity> spawnedEntities)
	{
		ARMAZ_LootTier tier = GetTierForObjective(type);
		int count = GetCountForTier(tier);

		for (int i = 0; i < count; i++)
		{
			ResourceName prefab = GetRandomLootPrefab(tier);
			if (prefab.IsEmpty())
				continue;

			vector spawnPos = GetScatteredPosition(position, 1.0, 6.0);
			IEntity loot = SpawnPrefab(prefab, spawnPos);
			if (loot)
				spawnedEntities.Insert(loot);
		}
	}

	protected ARMAZ_LootTier GetTierForObjective(ARMAZ_ObjectiveType type)
	{
		switch (type)
		{
			case ARMAZ_ObjectiveType.MEDICAL_SUPPLY:
			case ARMAZ_ObjectiveType.FUEL_DEPOT:
			case ARMAZ_ObjectiveType.VEHICLE_PARTS:
				return ARMAZ_LootTier.UNCOMMON;

			case ARMAZ_ObjectiveType.MILITARY_STASH:
			case ARMAZ_ObjectiveType.EXTRACTION_INTEL:
				return ARMAZ_LootTier.MILITARY;

			case ARMAZ_ObjectiveType.RADIO_TOWER:
			case ARMAZ_ObjectiveType.RESTORE_POWER:
				return ARMAZ_LootTier.RARE;
		}

		return ARMAZ_LootTier.COMMON;
	}

	protected int GetCountForTier(ARMAZ_LootTier tier)
	{
		switch (tier)
		{
			case ARMAZ_LootTier.COMMON: return Math.RandomInt(3, 7);
			case ARMAZ_LootTier.UNCOMMON: return Math.RandomInt(3, 6);
			case ARMAZ_LootTier.RARE: return Math.RandomInt(2, 5);
			case ARMAZ_LootTier.MILITARY: return Math.RandomInt(2, 4);
		}

		return 3;
	}

	protected ResourceName GetRandomLootPrefab(ARMAZ_LootTier tier)
	{
		array<ResourceName> pool = m_aCommonLoot;

		switch (tier)
		{
			case ARMAZ_LootTier.UNCOMMON:
				pool = m_aUncommonLoot;
				break;
			case ARMAZ_LootTier.RARE:
				pool = m_aRareLoot;
				break;
			case ARMAZ_LootTier.MILITARY:
				pool = m_aMilitaryLoot;
				break;
		}

		if (!pool || pool.Count() == 0)
			pool = m_aCommonLoot;

		if (!pool || pool.Count() == 0)
			return "";

		return pool[Math.RandomInt(0, pool.Count())];
	}

	protected vector GetScatteredPosition(vector center, float minDistance, float maxDistance)
	{
		BaseWorld world = GetGame().GetWorld();
		float angle = Math.RandomFloat(0, Math.PI2);
		float distance = Math.RandomFloat(minDistance, maxDistance);
		float x = center[0] + Math.Cos(angle) * distance;
		float z = center[2] + Math.Sin(angle) * distance;
		float y = center[1];
		if (world)
			y = world.GetSurfaceY(x, z) + 0.05;

		return Vector(x, y, z);
	}

	protected IEntity SpawnPrefab(ResourceName prefab, vector position)
	{
		Resource res = Resource.Load(prefab);
		if (!res)
			return null;

		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = position;

		return GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), params);
	}
}
