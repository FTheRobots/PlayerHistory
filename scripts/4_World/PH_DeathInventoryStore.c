class PH_DeathSnapshotCartridge
{
	int MuzzleIdx;
	bool InternalMagazine;
	float AmmoDamage;
	string AmmoTypeName;
}

class PH_DeathSnapshotItem
{
	string ItemTypeName;
	string ItemPid;
	bool InHands;
	bool OnGround;
	int SlotId;
	int CargoIdx;
	int CargoRow;
	int CargoCol;
	bool CargoFlip;
	float ItemHealth;
	float ItemQuantity;
	int MagazineAmmoCount;
	float StoredEnergy;
	int StoredLiquidType;
	float StoredWetness;
	float StoredTemperature;
	int StoredEdibleStage;
	ref array<ref PH_DeathSnapshotCartridge> Cartridges;
	ref array<ref PH_DeathSnapshotItem> Attachments;
	ref array<ref PH_DeathSnapshotItem> Cargo;

	void PH_DeathSnapshotItem()
	{
		SlotId = -1;
		CargoIdx = -1;
		CargoRow = -1;
		CargoCol = -1;
		CargoFlip = false;
		ItemHealth = -1.0;
		ItemQuantity = -1.0;
		MagazineAmmoCount = -1;
		StoredEnergy = -1.0;
		StoredLiquidType = 0;
		StoredWetness = -1.0;
		StoredTemperature = -1.0;
		StoredEdibleStage = -1;
		Cartridges = new array<ref PH_DeathSnapshotCartridge>();
		Attachments = new array<ref PH_DeathSnapshotItem>();
		Cargo = new array<ref PH_DeathSnapshotItem>();
	}
}

class PH_DeathEntry
{
	string EntryId;
	string OwnerSteam64;
	string OwnerName;
	string DeathTimeText;
	vector DeathPosition;
	int WasRestored;
	int RootItemCount;
	ref array<ref PH_DeathSnapshotItem> RootItems;

	void PH_DeathEntry()
	{
		WasRestored = 0;
		RootItemCount = 0;
		RootItems = new array<ref PH_DeathSnapshotItem>();
	}
}

class PH_DeathStore
{
	ref array<ref PH_DeathEntry> Entries;

	void PH_DeathStore()
	{
		Entries = new array<ref PH_DeathEntry>();
	}
}

class PH_DeathInventoryStore
{
	private static const string PROFILE_ROOT = "$profile:" + PH_Constants.ROOT_FOLDER + "\\";
	private static const string DEATHS_FOLDER = PH_Constants.DEATHS_FOLDER + "\\";
	private static const string INVENTORY_SNAPSHOTS_FOLDER = PH_Constants.INVENTORY_SNAPSHOTS_FOLDER + "\\";
	private static const string DEATH_MANAGER_LOG_FILE = PROFILE_ROOT + "death_manager.log";

	static string CapturePlayerDeath(PlayerBase player)
	{
		if (!GetGame().IsServer() || !player)
			return "";

		PlayerIdentity identity = player.GetIdentity();
		if (!identity)
			return "";

		PH_DeathEntry entry = CreateEntryHeader(player, identity);
		int nearbyGroundCount = PopulateGearFromPlayer(entry, player, true);
		entry.RootItemCount = entry.RootItems.Count();
		SaveEntryToFolder(entry, DEATHS_FOLDER);
		LogLine("Captured death snapshot entry=" + entry.EntryId + " steam64=" + entry.OwnerSteam64 + " ownerName=" + entry.OwnerName + " rootItems=" + entry.RootItemCount.ToString() + " nearbyGround=" + nearbyGroundCount.ToString() + " pos=" + entry.DeathPosition.ToString());
		return entry.EntryId;
	}

	static string CaptureInventorySnapshot(PlayerBase player)
	{
		if (!GetGame().IsServer() || !player)
			return "";

		PlayerIdentity identity = player.GetIdentity();
		if (!identity)
			return "";

		PH_DeathEntry entry = CreateEntryHeader(player, identity);
		PopulateGearFromPlayer(entry, player, false);
		entry.RootItemCount = entry.RootItems.Count();
		SaveEntryToFolder(entry, INVENTORY_SNAPSHOTS_FOLDER);
		PruneInventorySnapshots(entry.OwnerSteam64);
		LogLine("Captured inventory snapshot entry=" + entry.EntryId + " steam64=" + entry.OwnerSteam64 + " ownerName=" + entry.OwnerName + " rootItems=" + entry.RootItemCount.ToString());
		return entry.EntryId;
	}

	static void GetInventorySnapshotEntriesForSteam64(string steam64, out array<ref PH_DeathEntry> outEntries)
	{
		GetEntriesFromFolder(steam64, INVENTORY_SNAPSHOTS_FOLDER, outEntries);
	}

	static bool RestoreInventorySnapshotToPlayer(string steam64, string entryId, PlayerBase targetPlayer, bool forceRestore = false)
	{
		return RestoreEntryFromFolder(steam64, entryId, targetPlayer, INVENTORY_SNAPSHOTS_FOLDER, forceRestore);
	}

	static bool DeleteInventorySnapshot(string steam64, string entryId)
	{
		return DeleteEntryFromFolder(steam64, entryId, INVENTORY_SNAPSHOTS_FOLDER);
	}

	static bool CaptureLiveInventory(PlayerBase player)
	{
		if (!GetGame().IsServer() || !player)
			return false;

		PlayerIdentity identity = player.GetIdentity();
		if (!identity)
			return false;

		PH_DeathEntry entry = CreateEntryHeader(player, identity);
		PopulateGearFromPlayer(entry, player, false);
		entry.RootItemCount = entry.RootItems.Count();

		string playerDir = PROFILE_ROOT + entry.OwnerSteam64 + "\\";
		MakeDirectory(playerDir);
		string path = playerDir + PH_Constants.LIVE_INVENTORY_FILE;
		JsonFileLoader<PH_DeathEntry>.JsonSaveFile(path, entry);
		LogLine("Captured live inventory steam64=" + entry.OwnerSteam64 + " rootItems=" + entry.RootItemCount.ToString());
		return true;
	}

	static bool DeleteItemByPid(PlayerBase player, string targetPid)
	{
		if (!GetGame().IsServer() || !player || targetPid == "")
			return false;

		array<EntityAI> items = new array<EntityAI>();
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);

		foreach (EntityAI ent : items)
		{
			if (!ent)
				continue;

			if (PH_ItemSerializer.GetPersistentIdString(ent) == targetPid)
			{
				g_Game.ObjectDelete(ent);
				LogLine("Deleted item pid=" + targetPid + " from player inventory");
				return true;
			}
		}

		return false;
	}

	static bool GetInventorySnapshotItemLabels(string steam64, string entryId, out array<string> labels)
	{
		return GetEntryItemLabelsFromFolder(steam64, entryId, INVENTORY_SNAPSHOTS_FOLDER, labels);
	}

	static void GetEntriesForSteam64(string steam64, out array<ref PH_DeathEntry> outEntries)
	{
		GetEntriesFromFolder(steam64, DEATHS_FOLDER, outEntries);
	}

	static bool GetEntryItemLabels(string steam64, string entryId, out array<string> labels)
	{
		return GetEntryItemLabelsFromFolder(steam64, entryId, DEATHS_FOLDER, labels);
	}

	static bool DeleteEntry(string steam64, string entryId)
	{
		return DeleteEntryFromFolder(steam64, entryId, DEATHS_FOLDER);
	}

	static bool RestoreEntryToPlayer(string steam64, string entryId, PlayerBase targetPlayer, bool forceRestore = false)
	{
		return RestoreEntryFromFolder(steam64, entryId, targetPlayer, DEATHS_FOLDER, forceRestore);
	}

	private static PH_DeathEntry CreateEntryHeader(PlayerBase player, PlayerIdentity identity)
	{
		PH_DeathEntry entry = new PH_DeathEntry();
		entry.OwnerSteam64 = identity.GetPlainId();
		entry.OwnerName = identity.GetName();
		entry.DeathPosition = player.GetPosition();
		entry.DeathTimeText = PH_Timestamp.NowISO();
		entry.EntryId = BuildEntryId();
		entry.WasRestored = 0;
		return entry;
	}

	private static int PopulateGearFromPlayer(PH_DeathEntry entry, PlayerBase player, bool captureNearbyGround)
	{
		if (!entry || !player)
			return 0;

		array<EntityAI> capturedRoots = new array<EntityAI>();
		array<EntityAI> entities = new array<EntityAI>();
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, entities);
		foreach (EntityAI entity : entities)
		{
			if (!entity)
				continue;

			if (!IsPlayerRootInventoryItem(entity, player))
				continue;

			if (ContainsCapturedRoot(capturedRoots, entity))
				continue;

			PH_DeathSnapshotItem snap = SnapshotItemRecursive(entity, player);
			if (!snap)
				continue;

			entry.RootItems.Insert(snap);
			capturedRoots.Insert(entity);
		}

		EntityAI handsEntity = player.GetHumanInventory().GetEntityInHands();
		if (handsEntity && !ContainsCapturedRoot(capturedRoots, handsEntity))
		{
			PH_DeathSnapshotItem handsSnap = SnapshotItemRecursive(handsEntity, player);
			if (handsSnap)
			{
				handsSnap.InHands = true;
				entry.RootItems.Insert(handsSnap);
				capturedRoots.Insert(handsEntity);
			}
		}

		if (!captureNearbyGround)
			return 0;

		return CaptureNearbyGroundItems(entry, player, entry.DeathPosition, capturedRoots, handsEntity == null);
	}

	private static void GetEntriesFromFolder(string steam64, string folderSuffix, out array<ref PH_DeathEntry> outEntries)
	{
		if (!outEntries)
			outEntries = new array<ref PH_DeathEntry>();
		else
			outEntries.Clear();

		if (steam64 == "")
			return;

		LoadEntriesFromFolder(GetPlayerFolder(steam64, folderSuffix), steam64, outEntries);
	}

	private static bool GetEntryItemLabelsFromFolder(string steam64, string entryId, string folderSuffix, out array<string> labels)
	{
		if (!labels)
			labels = new array<string>();
		else
			labels.Clear();

		string path = "";
		PH_DeathEntry entry;
		if (!FindEntryInFolder(GetPlayerFolder(steam64, folderSuffix), steam64, entryId, path, entry))
			return false;

		if (!entry || !entry.RootItems)
			return false;

		foreach (PH_DeathSnapshotItem rootItem : entry.RootItems)
			AppendItemLabelRecursive(rootItem, 0, labels);

		return true;
	}

	private static bool DeleteEntryFromFolder(string steam64, string entryId, string folderSuffix)
	{
		string path = "";
		PH_DeathEntry entry;
		if (!FindEntryInFolder(GetPlayerFolder(steam64, folderSuffix), steam64, entryId, path, entry))
		{
			LogLine("Delete entry failed (not found) steam64=" + steam64 + " entry=" + entryId + " folder=" + folderSuffix);
			return false;
		}

		if (!FileExist(path))
		{
			LogLine("Delete entry failed (missing file) steam64=" + steam64 + " entry=" + entryId + " folder=" + folderSuffix);
			return false;
		}

		DeleteFile(path);
		LogLine("Deleted snapshot entry=" + entryId + " steam64=" + steam64 + " folder=" + folderSuffix);
		return true;
	}

	private static bool RestoreEntryFromFolder(string steam64, string entryId, PlayerBase targetPlayer, string folderSuffix, bool forceRestore)
	{
		if (!GetGame().IsServer() || !targetPlayer)
			return false;

		string path = "";
		PH_DeathEntry entry;
		if (!FindEntryInFolder(GetPlayerFolder(steam64, folderSuffix), steam64, entryId, path, entry))
		{
			LogLine("Restore failed (not found) steam64=" + steam64 + " entry=" + entryId + " folder=" + folderSuffix);
			return false;
		}

		if (!entry)
		{
			LogLine("Restore failed (entry null) steam64=" + steam64 + " entry=" + entryId + " folder=" + folderSuffix);
			return false;
		}

		if (!forceRestore && entry.WasRestored == 1)
		{
			LogLine("Restore blocked (already restored) steam64=" + steam64 + " entry=" + entryId + " folder=" + folderSuffix);
			return false;
		}

		DropCurrentTopLevelGear(targetPlayer);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(RestoreEntryItemsDelayed, 120, false, targetPlayer, entry);

		entry.WasRestored = 1;
		JsonFileLoader<PH_DeathEntry>.JsonSaveFile(path, entry);

		if (forceRestore)
			LogLine("Re-restored snapshot entry=" + entryId + " steam64=" + steam64 + " ownerName=" + entry.OwnerName + " folder=" + folderSuffix);
		else
			LogLine("Restored snapshot entry=" + entryId + " steam64=" + steam64 + " ownerName=" + entry.OwnerName + " folder=" + folderSuffix);

		return true;
	}

	private static void PruneInventorySnapshots(string steam64)
	{
		PH_ConfigData cfg = PH_Config.GetInstance().Get();
		if (!cfg || cfg.inventorySnapshotMaxRetained <= 0)
			return;

		array<ref PH_DeathEntry> entries = new array<ref PH_DeathEntry>();
		LoadEntriesFromFolder(GetPlayerFolder(steam64, INVENTORY_SNAPSHOTS_FOLDER), steam64, entries);

		while (entries.Count() > cfg.inventorySnapshotMaxRetained)
		{
			int oldestIndex = FindOldestEntryIndex(entries);
			if (oldestIndex < 0)
				break;

			PH_DeathEntry oldest = entries.Get(oldestIndex);
			if (!oldest)
			{
				entries.Remove(oldestIndex);
				continue;
			}

			string path = GetPlayerFolder(steam64, INVENTORY_SNAPSHOTS_FOLDER) + oldest.EntryId + ".json";
			if (FileExist(path))
				DeleteFile(path);

			entries.Remove(oldestIndex);
		}
	}

	private static int FindOldestEntryIndex(array<ref PH_DeathEntry> entries)
	{
		if (!entries || entries.Count() == 0)
			return -1;

		int oldestIndex = 0;
		string oldestTime = entries.Get(0).DeathTimeText;

		for (int i = 1; i < entries.Count(); i++)
		{
			PH_DeathEntry entry = entries.Get(i);
			if (!entry)
				continue;

			if (oldestTime == "" || (entry.DeathTimeText != "" && entry.DeathTimeText < oldestTime))
			{
				oldestTime = entry.DeathTimeText;
				oldestIndex = i;
			}
		}

		return oldestIndex;
	}

	private static string BuildEntryId()
	{
		string iso = PH_Timestamp.NowISO();
		iso.Replace(":", "-");
		iso.Replace("T", "_");
		iso.Replace("Z", "");
		return iso + "_" + GetGame().GetTime().ToString() + "_" + Math.RandomInt(1000, 9999).ToString();
	}

	private static bool FindEntry(string steam64, string entryId, out string outPath, out PH_DeathEntry outEntry)
	{
		return FindEntryInFolder(GetPlayerFolder(steam64, DEATHS_FOLDER), steam64, entryId, outPath, outEntry);
	}

	private static bool FindEntryInFolder(string folderPath, string steam64, string entryId, out string outPath, out PH_DeathEntry outEntry)
	{
		outPath = "";
		outEntry = null;

		if (!FileExist(folderPath))
			return false;

		string fileName = "";
		FileAttr attr = FileAttr.INVALID;
		FindFileHandle handle = FindFile(folderPath + "*.json", fileName, attr, FindFileFlags.ALL);
		if (handle == 0)
			return false;

		bool found = false;
		if (fileName != "" && !(attr & FileAttr.DIRECTORY))
			found = TryMatchEntryFile(folderPath, fileName, steam64, entryId, outPath, outEntry);

		while (!found && FindNextFile(handle, fileName, attr))
		{
			if (fileName == "" || (attr & FileAttr.DIRECTORY))
				continue;

			found = TryMatchEntryFile(folderPath, fileName, steam64, entryId, outPath, outEntry);
		}

		CloseFindFile(handle);
		return found;
	}

	private static bool TryMatchEntryFile(string folderPath, string fileName, string steam64, string entryId, out string outPath, out PH_DeathEntry outEntry)
	{
		outPath = "";
		outEntry = null;

		string fullPath = folderPath + fileName;
		PH_DeathEntry entry = new PH_DeathEntry();
		JsonFileLoader<PH_DeathEntry>.JsonLoadFile(fullPath, entry);
		if (!entry)
			return false;

		if (entry.OwnerSteam64 != steam64)
			return false;

		if (entry.EntryId != entryId)
			return false;

		outPath = fullPath;
		outEntry = entry;
		return true;
	}

	private static void RestoreEntryItemsDelayed(PlayerBase targetPlayer, PH_DeathEntry entry)
	{
		if (!targetPlayer || !entry || !entry.RootItems)
			return;

		array<ref PH_DeathSnapshotItem> bodyItems = new array<ref PH_DeathSnapshotItem>();
		array<ref PH_DeathSnapshotItem> handItems = new array<ref PH_DeathSnapshotItem>();
		array<ref PH_DeathSnapshotItem> otherItems = new array<ref PH_DeathSnapshotItem>();

		foreach (PH_DeathSnapshotItem classifyItem : entry.RootItems)
		{
			if (!classifyItem)
				continue;

			if (classifyItem.InHands)
			{
				handItems.Insert(classifyItem);
				continue;
			}

			if (classifyItem.SlotId >= 0)
			{
				bodyItems.Insert(classifyItem);
				continue;
			}

			otherItems.Insert(classifyItem);
		}

		foreach (PH_DeathSnapshotItem bodyItem : bodyItems)
		{
			EntityAI createdBody = CreateRootItemInBodySlot(targetPlayer, bodyItem);
			if (!createdBody)
				continue;

			ApplySnapshotToEntity(createdBody, bodyItem);
		}

		foreach (PH_DeathSnapshotItem otherItem : otherItems)
		{
			EntityAI createdOther = CreateRootItemInInventory(targetPlayer, otherItem);
			if (!createdOther)
				continue;

			ApplySnapshotToEntity(createdOther, otherItem);
		}

		foreach (PH_DeathSnapshotItem handItem : handItems)
		{
			EntityAI createdHand = CreateRootItemInHands(targetPlayer, handItem);
			if (!createdHand)
				continue;

			ApplySnapshotToEntity(createdHand, handItem);
		}
	}

	private static void LoadEntriesFromFolder(string folderPath, string steam64, inout array<ref PH_DeathEntry> outEntries)
	{
		if (!FileExist(folderPath))
			return;

		string fileName = "";
		FileAttr attr = FileAttr.INVALID;
		FindFileHandle handle = FindFile(folderPath + "*.json", fileName, attr, FindFileFlags.ALL);
		if (handle == 0)
			return;

		if (fileName != "" && !(attr & FileAttr.DIRECTORY))
			TryAppendEntryFile(folderPath + fileName, steam64, outEntries);

		while (FindNextFile(handle, fileName, attr))
		{
			if (fileName == "" || (attr & FileAttr.DIRECTORY))
				continue;

			TryAppendEntryFile(folderPath + fileName, steam64, outEntries);
		}

		CloseFindFile(handle);
	}

	private static void TryAppendEntryFile(string fullPath, string steam64, inout array<ref PH_DeathEntry> outEntries)
	{
		PH_DeathEntry entry = new PH_DeathEntry();
		JsonFileLoader<PH_DeathEntry>.JsonLoadFile(fullPath, entry);
		if (!entry)
			return;

		if (entry.OwnerSteam64 != steam64)
			return;

		if (entry.WasRestored == 1)
			return;

		if (entry.RootItemCount <= 0 && entry.RootItems)
			entry.RootItemCount = entry.RootItems.Count();

		outEntries.Insert(entry);
	}

	private static string GetPlayerFolder(string steam64, string folderSuffix)
	{
		return PROFILE_ROOT + steam64 + "\\" + folderSuffix;
	}

	private static string GetPlayerDeathsFolder(string steam64)
	{
		return GetPlayerFolder(steam64, DEATHS_FOLDER);
	}

	private static string GetEntryPath(string steam64, string entryId, string folderSuffix)
	{
		return GetPlayerFolder(steam64, folderSuffix) + entryId + ".json";
	}

	private static void SaveEntryToFolder(PH_DeathEntry entry, string folderSuffix)
	{
		if (!entry || entry.OwnerSteam64 == "")
			return;

		string folderPath = GetPlayerFolder(entry.OwnerSteam64, folderSuffix);
		MakeDirectory(folderPath);
		string entryPath = GetEntryPath(entry.OwnerSteam64, entry.EntryId, folderSuffix);
		JsonFileLoader<PH_DeathEntry>.JsonSaveFile(entryPath, entry);
	}

	private static void SaveEntry(PH_DeathEntry entry)
	{
		SaveEntryToFolder(entry, DEATHS_FOLDER);
	}

	static void LogLine(string text)
	{
		MakeDirectory(PROFILE_ROOT);

		FileHandle file = OpenFile(DEATH_MANAGER_LOG_FILE, FileMode.APPEND);
		if (file == 0)
			file = OpenFile(DEATH_MANAGER_LOG_FILE, FileMode.WRITE);

		if (file == 0)
			return;

		FPrintln(file, "[" + PH_Timestamp.NowISO() + "] " + text);
		CloseFile(file);
	}

	private static void AppendItemLabelRecursive(PH_DeathSnapshotItem item, int depth, inout array<string> labels)
	{
		if (!item || !labels)
			return;

		string label = "";
		for (int i = 0; i < depth; i++)
			label = label + "  ";

		string itemTypeLabel = item.ItemTypeName;
		if (itemTypeLabel == "")
			itemTypeLabel = "(unknown item class)";

		label = label + itemTypeLabel;

		if (item.MagazineAmmoCount >= 0)
		{
			label = label + " [" + item.MagazineAmmoCount.ToString() + "]";
		}
		else if (item.ItemQuantity >= 0.0)
		{
			int roundedQuantity = Math.Round(item.ItemQuantity);
			label = label + " [" + roundedQuantity.ToString() + "]";
		}

		labels.Insert(label);

		foreach (PH_DeathSnapshotItem attachment : item.Attachments)
			AppendItemLabelRecursive(attachment, depth + 1, labels);

		foreach (PH_DeathSnapshotItem cargoItem : item.Cargo)
			AppendItemLabelRecursive(cargoItem, depth + 1, labels);
	}

	private static bool IsPlayerRootInventoryItem(EntityAI entity, PlayerBase player)
	{
		if (!entity || !player)
			return false;

		EntityAI handsEntity = player.GetHumanInventory().GetEntityInHands();
		if (handsEntity && entity == handsEntity)
			return true;

		return entity.GetHierarchyParent() == player;
	}

	private static bool ContainsCapturedRoot(array<EntityAI> capturedRoots, EntityAI entity)
	{
		if (!capturedRoots || !entity)
			return false;

		return capturedRoots.Find(entity) != -1;
	}

	private static float GetDeathNearbyItemRadius()
	{
		PH_ConfigData cfg = PH_Config.GetInstance().Get();
		if (!cfg || cfg.deathNearbyItemRadius <= 0)
			return 2.5;

		return cfg.deathNearbyItemRadius;
	}

	private static int CaptureNearbyGroundItems(PH_DeathEntry entry, PlayerBase player, vector deathPos, array<EntityAI> capturedRoots, bool handsWereEmpty)
	{
		if (!entry || !player || !capturedRoots)
			return 0;

		float radius = GetDeathNearbyItemRadius();
		array<EntityAI> nearby = new array<EntityAI>();
		PH_EntityQuery.GetEntitiesInBox(deathPos, radius, 1.5, nearby);

		float closestDistSq = radius * radius;
		PH_DeathSnapshotItem closestSnap = null;

		int capturedCount = 0;
		foreach (EntityAI entity : nearby)
		{
			if (!IsNearbyDroppedRootItem(entity, player, deathPos, radius))
				continue;

			if (ContainsCapturedRoot(capturedRoots, entity))
				continue;

			PH_DeathSnapshotItem snap = SnapshotItemRecursive(entity, player);
			if (!snap)
				continue;

			snap.OnGround = true;

			vector itemPos = entity.GetPosition();
			float dx = itemPos[0] - deathPos[0];
			float dz = itemPos[2] - deathPos[2];
			float distSq = dx * dx + dz * dz;
			if (handsWereEmpty && distSq < closestDistSq)
			{
				closestDistSq = distSq;
				closestSnap = snap;
			}

			entry.RootItems.Insert(snap);
			capturedRoots.Insert(entity);
			capturedCount++;
		}

		if (handsWereEmpty && closestSnap && closestDistSq <= 1.2 * 1.2)
		{
			closestSnap.InHands = true;
			closestSnap.OnGround = true;
		}

		return capturedCount;
	}

	private static bool IsNearbyDroppedRootItem(EntityAI entity, PlayerBase player, vector deathPos, float maxRadius)
	{
		if (!entity || !player || entity == player)
			return false;

		if (entity.IsInherited(SurvivorBase))
			return false;

		if (PlayerBase.Cast(entity))
			return false;

		ItemBase item = ItemBase.Cast(entity);
		if (!item)
			return false;

		InventoryLocation il = new InventoryLocation();
		if (!item.GetInventory().GetCurrentInventoryLocation(il))
			return false;

		if (il.GetType() != InventoryLocationType.GROUND)
			return false;

		vector itemPos = item.GetPosition();
		float dx = itemPos[0] - deathPos[0];
		float dz = itemPos[2] - deathPos[2];
		float distSq = dx * dx + dz * dz;
		if (distSq > maxRadius * maxRadius)
			return false;

		float dy = Math.AbsFloat(itemPos[1] - deathPos[1]);
		if (dy > 2.0)
			return false;

		return true;
	}

	private static PH_DeathSnapshotItem SnapshotItemRecursive(EntityAI item, PlayerBase ownerPlayer)
	{
		if (!item)
			return null;

		PH_DeathSnapshotItem snap = new PH_DeathSnapshotItem();
		snap.ItemTypeName = item.GetType();
		snap.ItemHealth = item.GetHealth("", "");

		string itemPid = PH_ItemSerializer.GetPersistentIdString(item);
		if (itemPid != "")
			snap.ItemPid = itemPid;

		InventoryLocation il = new InventoryLocation();
		if (item.GetInventory().GetCurrentInventoryLocation(il))
		{
			if (il.GetType() == InventoryLocationType.HANDS)
			{
				snap.InHands = true;
			}
			else if (il.GetType() == InventoryLocationType.ATTACHMENT)
			{
				snap.SlotId = il.GetSlot();
			}
			else if (il.GetType() == InventoryLocationType.CARGO || il.GetType() == InventoryLocationType.PROXYCARGO)
			{
				snap.CargoIdx = il.GetIdx();
				snap.CargoRow = il.GetRow();
				snap.CargoCol = il.GetCol();
				snap.CargoFlip = il.GetFlip();
			}
		}

		ItemBase itemBase = ItemBase.Cast(item);
		if (itemBase && itemBase.HasQuantity())
			snap.ItemQuantity = itemBase.GetQuantity();
		if (itemBase && itemBase.HasEnergyManager())
			snap.StoredEnergy = itemBase.GetCompEM().GetEnergy();
		if (itemBase && itemBase.IsLiquidContainer())
			snap.StoredLiquidType = itemBase.GetLiquidType();
		if (itemBase)
			snap.StoredWetness = itemBase.GetWet();
		if (item.CanHaveTemperature())
			snap.StoredTemperature = item.GetTemperature();

		Edible_Base edible = Edible_Base.Cast(item);
		if (edible && edible.GetFoodStage())
			snap.StoredEdibleStage = edible.GetFoodStageType();

		Magazine mag = Magazine.Cast(item);
		if (mag)
			snap.MagazineAmmoCount = mag.GetAmmoCount();

		Weapon_Base weapon = Weapon_Base.Cast(item);
		if (weapon)
		{
			int muzzleCount = weapon.GetMuzzleCount();
			for (int mi = 0; mi < muzzleCount; mi++)
			{
				float chamberDamage = 0.0;
				string chamberType = "";
				if (!weapon.IsChamberEmpty(mi) && weapon.GetCartridgeInfo(mi, chamberDamage, chamberType))
				{
					PH_DeathSnapshotCartridge chamber = new PH_DeathSnapshotCartridge();
					chamber.MuzzleIdx = mi;
					chamber.InternalMagazine = false;
					chamber.AmmoDamage = chamberDamage;
					chamber.AmmoTypeName = chamberType;
					snap.Cartridges.Insert(chamber);
				}

				int internalCount = weapon.GetInternalMagazineCartridgeCount(mi);
				for (int ci = 0; ci < internalCount; ci++)
				{
					float internalDamage = 0.0;
					string internalType = "";
					if (!weapon.GetInternalMagazineCartridgeInfo(mi, ci, internalDamage, internalType))
						continue;

					PH_DeathSnapshotCartridge internalCartridge = new PH_DeathSnapshotCartridge();
					internalCartridge.MuzzleIdx = mi;
					internalCartridge.InternalMagazine = true;
					internalCartridge.AmmoDamage = internalDamage;
					internalCartridge.AmmoTypeName = internalType;
					snap.Cartridges.Insert(internalCartridge);
				}
			}
		}

		int attachmentCount = item.GetInventory().AttachmentCount();
		for (int attachmentIndex = 0; attachmentIndex < attachmentCount; attachmentIndex++)
		{
			EntityAI attachment = item.GetInventory().GetAttachmentFromIndex(attachmentIndex);
			if (!attachment)
				continue;

			PH_DeathSnapshotItem attachmentSnap = SnapshotItemRecursive(attachment, ownerPlayer);
			if (!attachmentSnap)
				continue;

			snap.Attachments.Insert(attachmentSnap);
		}

		CargoBase cargo = item.GetInventory().GetCargo();
		if (cargo)
		{
			int cargoCount = cargo.GetItemCount();
			for (int cargoIndex = 0; cargoIndex < cargoCount; cargoIndex++)
			{
				EntityAI cargoItem = cargo.GetItem(cargoIndex);
				if (!cargoItem)
					continue;

				PH_DeathSnapshotItem cargoSnap = SnapshotItemRecursive(cargoItem, ownerPlayer);
				if (!cargoSnap)
					continue;

				snap.Cargo.Insert(cargoSnap);
			}
		}

		return snap;
	}

	private static void DropCurrentTopLevelGear(PlayerBase player)
	{
		if (!player)
			return;

		player.DropAllItems();
	}

	private static bool MoveItemToGround(PlayerBase player, EntityAI item)
	{
		if (!player || !item)
			return false;

		InventoryLocation src = new InventoryLocation();
		if (!item.GetInventory().GetCurrentInventoryLocation(src))
			return false;

		InventoryLocation dst = new InventoryLocation();
		vector matrix[4];
		Math3D.MatrixIdentity4(matrix);

		vector pos = player.GetPosition();
		matrix[3][0] = pos[0] + Math.RandomFloat(-0.55, 0.55);
		matrix[3][2] = pos[2] + Math.RandomFloat(-0.55, 0.55);
		matrix[3][1] = g_Game.SurfaceY(matrix[3][0], matrix[3][2]) + 0.03;
		GameInventory.PrepareDropEntityPos(player, item, matrix, false, GameConstants.INVENTORY_ENTITY_DROP_OVERLAP_DEPTH);

		dst.SetGround(item, matrix);
		if (!player.ServerTakeToDst(src, dst))
			return false;

		ItemBase itemBase = ItemBase.Cast(item);
		if (itemBase)
			itemBase.PlaceOnSurface();

		return true;
	}

	private static EntityAI CreateRootItemInBodySlot(PlayerBase player, PH_DeathSnapshotItem snapshot)
	{
		if (!player || !snapshot || snapshot.ItemTypeName == "" || snapshot.SlotId == -1)
			return null;

		return player.GetInventory().CreateAttachmentEx(snapshot.ItemTypeName, snapshot.SlotId);
	}

	private static EntityAI CreateRootItemInHands(PlayerBase player, PH_DeathSnapshotItem snapshot)
	{
		if (!player || !snapshot || snapshot.ItemTypeName == "")
			return null;

		EntityAI created = player.GetHumanInventory().CreateInHands(snapshot.ItemTypeName);
		if (created)
			return created;

		EntityAI fallback = player.GetInventory().CreateInInventory(snapshot.ItemTypeName);
		if (!fallback)
			return null;

		player.ServerTakeEntityToHands(fallback);
		return fallback;
	}

	private static EntityAI CreateRootItemInInventory(PlayerBase player, PH_DeathSnapshotItem snapshot)
	{
		if (!player || !snapshot || snapshot.ItemTypeName == "")
			return null;

		if (snapshot.CargoIdx >= 0 && snapshot.CargoRow >= 0 && snapshot.CargoCol >= 0)
		{
			EntityAI cargoPlaced = player.GetInventory().CreateEntityInCargoEx(snapshot.ItemTypeName, snapshot.CargoIdx, snapshot.CargoRow, snapshot.CargoCol, snapshot.CargoFlip);
			if (cargoPlaced)
				return cargoPlaced;
		}

		return player.GetInventory().CreateInInventory(snapshot.ItemTypeName);
	}

	private static EntityAI CreateChildItem(EntityAI parent, PH_DeathSnapshotItem snapshot, bool attachment)
	{
		if (!parent || !snapshot || snapshot.ItemTypeName == "")
			return null;

		EntityAI created = null;

		PlayerBase rootPlayer = PlayerBase.Cast(parent.GetHierarchyRootPlayer());
		Weapon_Base parentWeapon = Weapon_Base.Cast(parent);
		bool isWeaponMagAttachment = parentWeapon && attachment && g_Game.IsKindOf(snapshot.ItemTypeName, "Magazine");
		if (isWeaponMagAttachment)
		{
			int magSlotId = snapshot.SlotId;
			if (magSlotId == -1)
				magSlotId = InventorySlots.GetSlotIdFromString("magazine");

			if (magSlotId != InventorySlots.INVALID)
				created = parent.GetInventory().CreateAttachmentEx(snapshot.ItemTypeName, magSlotId);

			if (!created && magSlotId != InventorySlots.INVALID)
			{
				if (rootPlayer)
				{
					EntityAI tempMag = rootPlayer.GetInventory().CreateInInventory(snapshot.ItemTypeName);
					Magazine tempMagCast = Magazine.Cast(tempMag);
					if (tempMagCast)
					{
						parent.GetInventory().TakeEntityAsAttachmentEx(InventoryMode.SERVER, tempMagCast, magSlotId);
						if (tempMagCast.GetHierarchyParent() == parent)
							created = tempMagCast;
						else
							g_Game.ObjectDelete(tempMagCast);
					}
				}
			}
		}

		if (!created && attachment && snapshot.SlotId != -1)
			created = parent.GetInventory().CreateAttachmentEx(snapshot.ItemTypeName, snapshot.SlotId);
		else if (!attachment && snapshot.CargoIdx >= 0 && snapshot.CargoRow >= 0 && snapshot.CargoCol >= 0)
			created = parent.GetInventory().CreateEntityInCargoEx(snapshot.ItemTypeName, snapshot.CargoIdx, snapshot.CargoRow, snapshot.CargoCol, snapshot.CargoFlip);

		if (!created && !attachment)
			created = parent.GetInventory().CreateEntityInCargo(snapshot.ItemTypeName);

		if (!created && isWeaponMagAttachment)
			return null;

		if (!created && !attachment)
			return null;

		if (!created)
			created = parent.GetInventory().CreateInInventory(snapshot.ItemTypeName);

		if (!created && rootPlayer)
		{
			EntityAI tempCreated = rootPlayer.GetInventory().CreateInInventory(snapshot.ItemTypeName);
			if (tempCreated)
			{
				bool moved = false;
				if (attachment)
					moved = MoveEntityToAttachment(rootPlayer, tempCreated, parent, snapshot.SlotId);
				else
					moved = MoveEntityToCargo(rootPlayer, tempCreated, parent, snapshot.CargoIdx, snapshot.CargoRow, snapshot.CargoCol, snapshot.CargoFlip);

				if (moved)
					created = tempCreated;
				else
					g_Game.ObjectDelete(tempCreated);
			}
		}

		Magazine createdMagazine = Magazine.Cast(created);
		if (createdMagazine && isWeaponMagAttachment)
		{
			int ensureMagSlotId = InventorySlots.GetSlotIdFromString("magazine");
			if (ensureMagSlotId != InventorySlots.INVALID)
				parent.GetInventory().TakeEntityAsAttachmentEx(InventoryMode.SERVER, createdMagazine, ensureMagSlotId);

			if (createdMagazine.GetHierarchyParent() != parent)
			{
				g_Game.ObjectDelete(createdMagazine);
				return null;
			}
		}

		return created;
	}

	private static void RestoreChildWithRetry(EntityAI parent, PH_DeathSnapshotItem snapshot, bool attachment, int retriesLeft = 8)
	{
		if (!parent || !snapshot)
			return;

		EntityAI created = CreateChildItem(parent, snapshot, attachment);
		if (created)
		{
			ApplySnapshotToEntity(created, snapshot);
			return;
		}

		if (retriesLeft <= 0)
		{
			string failureLog = "Restore child failed parent=" + parent.GetType();
			failureLog = failureLog + " item=" + snapshot.ItemTypeName;
			failureLog = failureLog + " attachment=" + attachment.ToString();
			failureLog = failureLog + " slot=" + snapshot.SlotId.ToString();
			failureLog = failureLog + " cargo=[" + snapshot.CargoIdx.ToString();
			failureLog = failureLog + "," + snapshot.CargoRow.ToString();
			failureLog = failureLog + "," + snapshot.CargoCol.ToString();
			failureLog = failureLog + "," + snapshot.CargoFlip.ToString() + "]";
			LogLine(failureLog);
			return;
		}

		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(RestoreChildWithRetry, 180, false, parent, snapshot, attachment, retriesLeft - 1);
	}

	private static bool MoveEntityToCargo(PlayerBase mover, EntityAI item, EntityAI parent, int idx, int row, int col, bool flip)
	{
		if (!mover || !item || !parent)
			return false;

		if (idx < 0 || row < 0 || col < 0)
			return false;

		InventoryLocation src = new InventoryLocation();
		if (!item.GetInventory().GetCurrentInventoryLocation(src))
			return false;

		InventoryLocation dst = new InventoryLocation();
		dst.SetCargo(parent, item, idx, row, col, flip);
		return mover.ServerTakeToDst(src, dst);
	}

	private static bool MoveEntityToAttachment(PlayerBase mover, EntityAI item, EntityAI parent, int slotId)
	{
		if (!mover || !item || !parent)
			return false;

		if (slotId == -1)
			return false;

		InventoryLocation src = new InventoryLocation();
		if (!item.GetInventory().GetCurrentInventoryLocation(src))
			return false;

		InventoryLocation dst = new InventoryLocation();
		dst.SetAttachment(parent, item, slotId);
		return mover.ServerTakeToDst(src, dst);
	}

	private static void ApplySnapshotToEntity(EntityAI entity, PH_DeathSnapshotItem snapshot)
	{
		if (!entity || !snapshot)
			return;

		if (snapshot.ItemHealth >= 0.0)
			entity.SetHealth("", "", snapshot.ItemHealth);

		ItemBase itemBase = ItemBase.Cast(entity);
		if (itemBase && itemBase.HasQuantity() && snapshot.ItemQuantity >= 0.0)
			itemBase.SetQuantity(snapshot.ItemQuantity);
		if (itemBase && itemBase.HasEnergyManager() && snapshot.StoredEnergy >= 0.0)
			itemBase.GetCompEM().SetEnergy(snapshot.StoredEnergy);
		if (itemBase && snapshot.StoredLiquidType != 0)
			itemBase.SetLiquidType(snapshot.StoredLiquidType);
		if (itemBase && snapshot.StoredWetness >= 0.0)
			itemBase.SetWet(snapshot.StoredWetness);
		if (entity.CanHaveTemperature() && snapshot.StoredTemperature >= 0.0)
			entity.SetTemperature(snapshot.StoredTemperature);

		if (itemBase && entity.GetInventory() && entity.GetInventory().GetCargo())
			itemBase.Open();

		Edible_Base edible = Edible_Base.Cast(entity);
		if (edible && snapshot.StoredEdibleStage != -1)
			edible.ChangeFoodStage(snapshot.StoredEdibleStage);

		Magazine mag = Magazine.Cast(entity);
		if (mag && snapshot.MagazineAmmoCount >= 0)
		{
			int ammoCount = snapshot.MagazineAmmoCount;
			if (ammoCount > mag.GetAmmoMax())
				ammoCount = mag.GetAmmoMax();
			mag.ServerSetAmmoCount(ammoCount);
		}

		foreach (PH_DeathSnapshotItem attachment : snapshot.Attachments)
		{
			RestoreChildWithRetry(entity, attachment, true, 6);
		}

		foreach (PH_DeathSnapshotItem cargoItem : snapshot.Cargo)
		{
			RestoreChildWithRetry(entity, cargoItem, false, 5);
		}

		Weapon_Base weapon = Weapon_Base.Cast(entity);
		if (weapon && snapshot.Cartridges && snapshot.Cartridges.Count() > 0)
		{
			foreach (PH_DeathSnapshotCartridge cartridge : snapshot.Cartridges)
			{
				if (!cartridge || cartridge.AmmoTypeName == "")
					continue;

				float dmg = cartridge.AmmoDamage;
				if (dmg <= 0.0)
					dmg = 1.0;

				if (cartridge.InternalMagazine)
					weapon.PushCartridgeToInternalMagazine(cartridge.MuzzleIdx, dmg, cartridge.AmmoTypeName);
				else
					weapon.PushCartridgeToChamber(cartridge.MuzzleIdx, dmg, cartridge.AmmoTypeName);
			}
		}

		Magazine restoredMagazine = Magazine.Cast(entity);
		Weapon_Base parentWeapon = Weapon_Base.Cast(entity.GetHierarchyParent());
		if (restoredMagazine && parentWeapon)
		{
			int magSlotId = InventorySlots.GetSlotIdFromString("magazine");
			if (magSlotId != InventorySlots.INVALID)
				parentWeapon.GetInventory().TakeEntityAsAttachmentEx(InventoryMode.SERVER, restoredMagazine, magSlotId);
		}
	}

}
