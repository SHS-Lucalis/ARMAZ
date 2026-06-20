//------------------------------------------------------------------------------------------------
//! ARMAZ Objective Base
//! Objective state, completion radius, reward spawning, and cleanup. No AI control.
//------------------------------------------------------------------------------------------------

class ARMAZ_ObjectiveBase
{
	protected int m_iId;
	protected ref ARMAZ_DiscoveredLocation m_Location;
	protected ARMAZ_ObjectiveState m_eState = ARMAZ_ObjectiveState.INITIALIZING;
	protected ARMAZ_ObjectiveType m_eType = ARMAZ_ObjectiveType.LOOT_CACHE;
	protected float m_fStartTime;
	protected float m_fCompletionTime;
	protected float m_fMaxActiveTime = 1800;
	protected float m_fCompletionGracePeriod = 180;
	protected float m_fActivationRadius = 65;
	protected bool m_bRewardsSpawned = false;
	protected ref array<IEntity> m_aSpawnedRewardEntities = {};

	void ARMAZ_ObjectiveBase(int id, ARMAZ_DiscoveredLocation location)
	{
		m_iId = id;
		m_Location = location;
		m_aSpawnedRewardEntities = new array<IEntity>();
	}

	int GetId() { return m_iId; }
	ARMAZ_DiscoveredLocation GetLocation() { return m_Location; }
	vector GetPosition()
	{
		if (m_Location)
			return m_Location.GetPosition();

		return "0 0 0";
	}
	string GetName()
	{
		if (m_Location)
			return m_Location.GetName();

		return "Unknown";
	}
	ARMAZ_ObjectiveState GetState() { return m_eState; }
	ARMAZ_ObjectiveType GetType() { return m_eType; }
	bool IsActive() { return m_eState == ARMAZ_ObjectiveState.ACTIVE; }
	bool IsComplete() { return m_eState == ARMAZ_ObjectiveState.COMPLETED; }
	bool IsFailed() { return m_eState == ARMAZ_ObjectiveState.FAILED; }

	bool Initialize(ARMAZ_ObjectiveType type, float maxActiveTime, float completionGracePeriod)
	{
		if (!m_Location)
			return false;

		m_eType = type;
		m_fMaxActiveTime = maxActiveTime;
		m_fCompletionGracePeriod = completionGracePeriod;
		m_fStartTime = GetGame().GetWorld().GetWorldTime() / 1000;
		m_eState = ARMAZ_ObjectiveState.ACTIVE;

		ARMAZ_Notification.ShowObjectiveDiscovered(GetTypeDisplayName(), m_Location.GetName());
		Print(string.Format("[ARMAZ-Objective] Created #%1 %2 at %3", m_iId, GetTypeDisplayName(), m_Location.GetName()), LogLevel.NORMAL);
		return true;
	}

	void Update(ARMAZ_LootManager lootManager)
	{
		if (m_eState != ARMAZ_ObjectiveState.ACTIVE)
			return;

		if (IsPlayerInsideCompletionRadius())
			CompleteObjective(lootManager);
		else if (ShouldFailFromTimeout())
			SetFailed();
	}

	protected bool IsPlayerInsideCompletionRadius()
	{
		array<vector> playerPositions = {};
		ARMAZ_ZombieSpawnManager.GetPlayerPositions(playerPositions);

		foreach (vector playerPos : playerPositions)
		{
			if (vector.Distance(playerPos, GetPosition()) <= m_fActivationRadius)
				return true;
		}

		return false;
	}

	protected bool ShouldFailFromTimeout()
	{
		if (m_fMaxActiveTime <= 0)
			return false;

		float now = GetGame().GetWorld().GetWorldTime() / 1000;
		return (now - m_fStartTime) > m_fMaxActiveTime;
	}

	protected void CompleteObjective(ARMAZ_LootManager lootManager)
	{
		m_eState = ARMAZ_ObjectiveState.COMPLETED;
		m_fCompletionTime = GetGame().GetWorld().GetWorldTime() / 1000;

		if (lootManager && !m_bRewardsSpawned)
		{
			lootManager.SpawnObjectiveReward(m_eType, GetPosition(), m_aSpawnedRewardEntities);
			m_bRewardsSpawned = true;
		}

		ARMAZ_Notification.ShowObjectiveComplete(GetTypeDisplayName(), GetName());
		Print(string.Format("[ARMAZ-Objective] Completed #%1 %2", m_iId, GetName()), LogLevel.NORMAL);
	}

	void SetFailed()
	{
		m_eState = ARMAZ_ObjectiveState.FAILED;
		ARMAZ_Notification.ShowObjectiveFailed(GetTypeDisplayName(), GetName());
	}

	bool ShouldCleanup()
	{
		if (m_eState == ARMAZ_ObjectiveState.FAILED)
			return true;

		if (m_eState != ARMAZ_ObjectiveState.COMPLETED)
			return false;

		float now = GetGame().GetWorld().GetWorldTime() / 1000;
		return (now - m_fCompletionTime) > m_fCompletionGracePeriod;
	}

	void Cleanup(bool deleteRewards = false)
	{
		if (deleteRewards)
		{
			foreach (IEntity entity : m_aSpawnedRewardEntities)
			{
				if (entity)
					SCR_EntityHelper.DeleteEntityAndChildren(entity);
			}
		}

		m_aSpawnedRewardEntities.Clear();
		m_eState = ARMAZ_ObjectiveState.CLEANUP;
	}

	string GetTypeDisplayName()
	{
		switch (m_eType)
		{
			case ARMAZ_ObjectiveType.LOOT_CACHE: return "Loot Cache";
			case ARMAZ_ObjectiveType.MEDICAL_SUPPLY: return "Medical Supply";
			case ARMAZ_ObjectiveType.FUEL_DEPOT: return "Fuel Depot";
			case ARMAZ_ObjectiveType.RADIO_TOWER: return "Radio Tower";
			case ARMAZ_ObjectiveType.RESTORE_POWER: return "Restore Power";
			case ARMAZ_ObjectiveType.VEHICLE_PARTS: return "Vehicle Parts";
			case ARMAZ_ObjectiveType.EXTRACTION_INTEL: return "Extraction Intel";
			case ARMAZ_ObjectiveType.MILITARY_STASH: return "Military Stash";
		}

		return "Objective";
	}
}
