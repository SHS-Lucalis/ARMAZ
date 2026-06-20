//------------------------------------------------------------------------------------------------
//! ARMAZ shared types
//------------------------------------------------------------------------------------------------

enum ARMAZ_ObjectiveState
{
	INITIALIZING,
	ACTIVE,
	COMPLETED,
	FAILED,
	CLEANUP
}

enum ARMAZ_ObjectiveType
{
	LOOT_CACHE,
	MEDICAL_SUPPLY,
	FUEL_DEPOT,
	RADIO_TOWER,
	RESTORE_POWER,
	VEHICLE_PARTS,
	EXTRACTION_INTEL,
	MILITARY_STASH
}

enum ARMAZ_LootTier
{
	COMMON,
	UNCOMMON,
	RARE,
	MILITARY
}

enum ARMAZ_VehicleCondition
{
	WRECK,
	DAMAGED_NO_FUEL,
	DAMAGED_LOW_FUEL,
	WORKING_LOW_FUEL,
	RARE_WORKING
}

enum ARMAZ_PerformanceState
{
	EXCELLENT,
	GOOD,
	WARNING,
	CRITICAL
}
