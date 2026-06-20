//------------------------------------------------------------------------------------------------
//! ARMAZ Notification Helper
//------------------------------------------------------------------------------------------------

class ARMAZ_Notification
{
	static void ShowObjectiveDiscovered(string typeName, string locationName)
	{
		Show("ARMAZ OBJECTIVE", string.Format("%1 discovered at %2", typeName, locationName));
	}

	static void ShowObjectiveComplete(string typeName, string locationName)
	{
		Show("OBJECTIVE COMPLETE", string.Format("%1 secured at %2. Supplies located.", typeName, locationName));
	}

	static void ShowObjectiveFailed(string typeName, string locationName)
	{
		Show("OBJECTIVE LOST", string.Format("%1 at %2 is no longer viable.", typeName, locationName));
	}

	static void ShowVehicleFound(vector position)
	{
		Show("VEHICLE FOUND", "A usable vehicle has been found. Fuel and condition may be limited.");
	}

	static void ShowHordeNearby(string locationName)
	{
		Show("HORDE ACTIVITY", string.Format("Heavy infected movement near %1", locationName));
	}

	static void Show(string title, string message)
	{
		SCR_PopUpNotification popup = SCR_PopUpNotification.GetInstance();
		if (popup)
		{
			popup.PopupMsg(title + "\n" + message);
			return;
		}

		Print(string.Format("[%1] %2", title, message), LogLevel.NORMAL);
	}
}
