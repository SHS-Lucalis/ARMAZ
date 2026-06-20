//------------------------------------------------------------------------------------------------
//! ARMAZ Zombie Spawn Manager
//! Passive spawner only: no waypoints, no forced targets, no custom combat AI controller.
//------------------------------------------------------------------------------------------------

class ARMAZ_ZombieSpawnManager
{
	protected ref array<IEntity> m_aActiveZombies = {};
	protected ref array<ResourceName> m_aZombiePrefabs = {};

	protected int m_iMaxZombies = 150;
	protected float m_fSpawnRadius = 850;
	protected float m_fSafeRadius = 150;
	protected float m_fCleanupDistance = 1800;
	protected float m_fObjectiveSpawnRadius = 125;
	protected int m_iMaxSpawnPerUpdate = 12;
	protected bool m_bEnabled = true;

	void ARMAZ_ZombieSpawnManager(array<ResourceName> zombiePrefabs, int maxZombies, float spawnRadius, float safeRadius)
	{
		m_aActiveZombies = new array<IEntity>();
		m_aZombiePrefabs = new array<ResourceName>();

		if (zombiePrefabs)
		{
			foreach (ResourceName prefab : zombiePrefabs)
			{
				if (!prefab.IsEmpty())
					m_aZombiePrefabs.Insert(prefab);
			}
		}

		m_iMaxZombies = maxZombies;
		m_fSpawnRadius = spawnRadius;
		m_fSafeRadius = safeRadius;
	}

	void Configure(int maxZombies, float spawnRadius, float safeRadius, float cleanupDistance, int maxSpawnPerUpdate)
	{
		m_iMaxZombies = maxZombies;
		m_fSpawnRadius = spawnRadius;
		m_fSafeRadius = safeRadius;
		m_fCleanupDistance = cleanupDistance;
		m_iMaxSpawnPerUpdate = maxSpawnPerUpdate;
	}

	void SetEnabled(bool enabled) { m_bEnabled = enabled; }
	int GetActiveZombieCount() { CleanupInvalidReferences(); return m_aActiveZombies.Count(); }
	int GetMaxZombies() { return m_iMaxZombies; }
	void SetMaxZombies(int maxZombies) { m_iMaxZombies = maxZombies; }

	void Update(array<ref ARMAZ_ObjectiveBase> activeObjectives = null)
	{
		CleanupZombies();

		if (!m_bEnabled)
			return;

		if (m_aZombiePrefabs.Count() == 0)
		{
			Print("[ARMAZ-Zombies] No zombie prefabs configured. Add zombie character/entity prefabs to ARMAZ_GameModeComponent.", LogLevel.WARNING);
			return;
		}

		if (m_aActiveZombies.Count() >= m_iMaxZombies)
			return;

		SpawnAmbientZombiesNearPlayers();

		if (activeObjectives)
			SpawnObjectivePressure(activeObjectives);
	}

	protected void SpawnAmbientZombiesNearPlayers()
	{
		array<vector> playerPositions = {};
		GetPlayerPositions(playerPositions);

		if (playerPositions.Count() == 0)
			return;

		int budget = Math.Min(m_iMaxSpawnPerUpdate, m_iMaxZombies - m_aActiveZombies.Count());
		for (int i = 0; i < budget; i++)
		{
			vector basePos = playerPositions[Math.RandomInt(0, playerPositions.Count())];
			vector spawnPos = FindSpawnPositionAround(basePos, m_fSafeRadius, m_fSpawnRadius);
			if (!IsZeroVector(spawnPos))
				SpawnZombie(spawnPos);
		}
	}

	protected void SpawnObjectivePressure(array<ref ARMAZ_ObjectiveBase> activeObjectives)
	{
		if (!activeObjectives || activeObjectives.Count() == 0)
			return;

		int budget = Math.Min(4, m_iMaxZombies - m_aActiveZombies.Count());
		for (int i = 0; i < budget; i++)
		{
			ARMAZ_ObjectiveBase obj = activeObjectives[Math.RandomInt(0, activeObjectives.Count())];
			if (!obj || !obj.IsActive())
				continue;

			vector spawnPos = FindSpawnPositionAround(obj.GetPosition(), 40, m_fObjectiveSpawnRadius);
			if (!IsZeroVector(spawnPos))
				SpawnZombie(spawnPos);
		}
	}

	protected vector FindSpawnPositionAround(vector center, float minDistance, float maxDistance)
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return "0 0 0";

		for (int attempt = 0; attempt < 16; attempt++)
		{
			float angle = Math.RandomFloat(0, Math.PI2);
			float distance = Math.RandomFloat(minDistance, maxDistance);
			float x = center[0] + Math.Cos(angle) * distance;
			float z = center[2] + Math.Sin(angle) * distance;
			float y = world.GetSurfaceY(x, z);
			vector pos = Vector(x, y, z);

			if (!ARMAZ_LocationDiscovery.IsValidGroundPosition(pos))
				continue;

			if (IsTooCloseToAnyPlayer(pos, m_fSafeRadius))
				continue;

			return pos;
		}

		return "0 0 0";
	}

	protected IEntity SpawnZombie(vector position)
	{
		if (m_aZombiePrefabs.Count() == 0)
			return null;

		ResourceName prefab = m_aZombiePrefabs[Math.RandomInt(0, m_aZombiePrefabs.Count())];
		Resource res = Resource.Load(prefab);
		if (!res)
		{
			Print(string.Format("[ARMAZ-Zombies] Failed to load zombie prefab: %1", prefab), LogLevel.WARNING);
			return null;
		}

		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = position;

		IEntity zombie = GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), params);
		if (zombie)
			m_aActiveZombies.Insert(zombie);

		return zombie;
	}

	protected void CleanupZombies()
	{
		for (int i = m_aActiveZombies.Count() - 1; i >= 0; i--)
		{
			IEntity zombie = m_aActiveZombies[i];
			if (!zombie)
			{
				m_aActiveZombies.Remove(i);
				continue;
			}

			if (IsEntityDead(zombie) || IsTooFarFromPlayers(zombie.GetOrigin()))
			{
				SCR_EntityHelper.DeleteEntityAndChildren(zombie);
				m_aActiveZombies.Remove(i);
			}
		}
	}

	protected void CleanupInvalidReferences()
	{
		for (int i = m_aActiveZombies.Count() - 1; i >= 0; i--)
		{
			if (!m_aActiveZombies[i])
				m_aActiveZombies.Remove(i);
		}
	}

	protected bool IsEntityDead(IEntity entity)
	{
		if (!entity)
			return true;

		SCR_DamageManagerComponent dmgMgr = SCR_DamageManagerComponent.Cast(entity.FindComponent(SCR_DamageManagerComponent));
		if (dmgMgr && dmgMgr.GetState() == EDamageState.DESTROYED)
			return true;

		SCR_CharacterControllerComponent charController = SCR_CharacterControllerComponent.Cast(entity.FindComponent(SCR_CharacterControllerComponent));
		if (charController && charController.IsDead())
			return true;

		return false;
	}

	protected bool IsTooFarFromPlayers(vector pos)
	{
		array<vector> playerPositions = {};
		GetPlayerPositions(playerPositions);

		if (playerPositions.Count() == 0)
			return false;

		foreach (vector playerPos : playerPositions)
		{
			if (vector.Distance(pos, playerPos) <= m_fCleanupDistance)
				return false;
		}

		return true;
	}

	protected bool IsTooCloseToAnyPlayer(vector pos, float minDistance)
	{
		array<vector> playerPositions = {};
		GetPlayerPositions(playerPositions);

		foreach (vector playerPos : playerPositions)
		{
			if (vector.Distance(pos, playerPos) < minDistance)
				return true;
		}

		return false;
	}

	static void GetPlayerPositions(notnull array<vector> outPositions)
	{
		array<int> playerIds = {};
		GetGame().GetPlayerManager().GetPlayers(playerIds);

		foreach (int playerId : playerIds)
		{
			IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
			if (player)
				outPositions.Insert(player.GetOrigin());
		}
	}
	protected bool IsZeroVector(vector pos)
	{
		return pos[0] == 0 && pos[1] == 0 && pos[2] == 0;
	}

}
