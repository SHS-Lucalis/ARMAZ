//------------------------------------------------------------------------------------------------
//! ARMAZ Performance Manager
//! Adjusts zombie population caps based on sustained server frame rate.
//------------------------------------------------------------------------------------------------

class ARMAZ_PerformanceManager
{
	protected ARMAZ_ZombieSpawnManager m_ZombieManager;
	protected ref array<float> m_aFrameTimeSamples = {};
	protected int m_iSampleIndex = 0;
	protected int m_iMaxSamples = 60;

	protected float m_fAverageFPS = 60;
	protected float m_fTargetFPS = 30;
	protected float m_fWarningFPS = 25;
	protected float m_fCriticalFPS = 20;
	protected float m_fExcellentFPS = 40;

	protected int m_iBaseMaxZombies;
	protected int m_iCurrentMaxZombies;
	protected int m_iMinimumZombieCap = 35;
	protected int m_iAdjustStep = 15;
	protected float m_fLastAdjustTime = 0;
	protected float m_fAdjustCooldown = 45;
	protected ARMAZ_PerformanceState m_eState = ARMAZ_PerformanceState.GOOD;

	void ARMAZ_PerformanceManager(ARMAZ_ZombieSpawnManager zombieManager, int baseMaxZombies)
	{
		m_ZombieManager = zombieManager;
		m_iBaseMaxZombies = baseMaxZombies;
		m_iCurrentMaxZombies = baseMaxZombies;
		m_aFrameTimeSamples = new array<float>();

		for (int i = 0; i < m_iMaxSamples; i++)
			m_aFrameTimeSamples.Insert(0.016);
	}

	void Configure(float targetFPS, float warningFPS, float criticalFPS, int minimumZombieCap, int adjustStep, float adjustCooldown)
	{
		m_fTargetFPS = targetFPS;
		m_fWarningFPS = warningFPS;
		m_fCriticalFPS = criticalFPS;
		m_fExcellentFPS = targetFPS + 10;
		m_iMinimumZombieCap = minimumZombieCap;
		m_iAdjustStep = adjustStep;
		m_fAdjustCooldown = adjustCooldown;
	}

	void Update(float timeSlice)
	{
		RecordFrameTime(timeSlice);
		CalculateAverageFPS();
		UpdateState();
		TryAdjustZombieCap();
	}

	protected void RecordFrameTime(float timeSlice)
	{
		timeSlice = Math.Clamp(timeSlice, 0.001, 1.0);
		m_aFrameTimeSamples[m_iSampleIndex] = timeSlice;
		m_iSampleIndex = (m_iSampleIndex + 1) % m_iMaxSamples;
	}

	protected void CalculateAverageFPS()
	{
		float total = 0;
		foreach (float sample : m_aFrameTimeSamples)
			total += sample;

		float avg = total / m_aFrameTimeSamples.Count();
		if (avg > 0)
			m_fAverageFPS = 1.0 / avg;
	}

	protected void UpdateState()
	{
		if (m_fAverageFPS >= m_fExcellentFPS)
			m_eState = ARMAZ_PerformanceState.EXCELLENT;
		else if (m_fAverageFPS >= m_fTargetFPS)
			m_eState = ARMAZ_PerformanceState.GOOD;
		else if (m_fAverageFPS >= m_fCriticalFPS)
			m_eState = ARMAZ_PerformanceState.WARNING;
		else
			m_eState = ARMAZ_PerformanceState.CRITICAL;
	}

	protected void TryAdjustZombieCap()
	{
		if (!m_ZombieManager)
			return;

		float now = GetGame().GetWorld().GetWorldTime() / 1000;
		if (now - m_fLastAdjustTime < m_fAdjustCooldown)
			return;

		if (m_eState == ARMAZ_PerformanceState.CRITICAL)
		{
			m_iCurrentMaxZombies = Math.Max(m_iMinimumZombieCap, m_iCurrentMaxZombies - m_iAdjustStep);
			m_ZombieManager.SetMaxZombies(m_iCurrentMaxZombies);
			m_fLastAdjustTime = now;
			Print(string.Format("[ARMAZ-Performance] Critical FPS %1. Zombie cap reduced to %2", m_fAverageFPS, m_iCurrentMaxZombies), LogLevel.WARNING);
		}
		else if (m_eState == ARMAZ_PerformanceState.EXCELLENT && m_iCurrentMaxZombies < m_iBaseMaxZombies)
		{
			m_iCurrentMaxZombies = Math.Min(m_iBaseMaxZombies, m_iCurrentMaxZombies + m_iAdjustStep);
			m_ZombieManager.SetMaxZombies(m_iCurrentMaxZombies);
			m_fLastAdjustTime = now;
		}
	}

	bool ShouldPauseSpawning()
	{
		return m_eState == ARMAZ_PerformanceState.CRITICAL;
	}

	float GetAverageFPS() { return m_fAverageFPS; }
	ARMAZ_PerformanceState GetState() { return m_eState; }
	int GetCurrentZombieCap() { return m_iCurrentMaxZombies; }
}
