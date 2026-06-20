//------------------------------------------------------------------------------------------------
//! ARMAZ Map Marker Manager placeholder
//! Actual marker creation APIs vary by project setup; this preserves clean call sites.
//------------------------------------------------------------------------------------------------

[ComponentEditorProps(category: "ARMAZ/Managers", description: "ARMAZ objective marker manager")]
class ARMAZ_MapMarkerManagerComponentClass : ScriptComponentClass
{
}

class ARMAZ_MapMarkerManagerComponent : ScriptComponent
{
	protected ref map<int, ref SCR_MapMarkerBase> m_mObjectiveMarkers = new map<int, ref SCR_MapMarkerBase>();

	void CreateObjectiveMarker(int objectiveId, vector position, string label)
	{
		Print(string.Format("[ARMAZ-Marker] Objective %1 marker requested at %2: %3", objectiveId, position.ToString(), label), LogLevel.NORMAL);
	}

	void RemoveObjectiveMarker(int objectiveId)
	{
		if (m_mObjectiveMarkers.Contains(objectiveId))
			m_mObjectiveMarkers.Remove(objectiveId);
	}

	void ClearAllMarkers()
	{
		m_mObjectiveMarkers.Clear();
	}

	static ARMAZ_MapMarkerManagerComponent GetInstance()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return null;

		return ARMAZ_MapMarkerManagerComponent.Cast(gameMode.FindComponent(ARMAZ_MapMarkerManagerComponent));
	}
}
