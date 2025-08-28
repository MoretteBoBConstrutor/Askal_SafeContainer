class CfgPatches
{
	class ASKAL_Scripts
	{
		requiredAddons[] = {"DZ_Data", "DZ_Scripts", "DZ_Characters"};
		units[] = {};
		weapons[] = {};
	};
};

class CfgMods
{
	class ASKAL_Scripts
	{
		dir = "SafeContainer";
		name = "DayZ Askal";
		type = "mod";
		dependencies[] = {"Game","World","Mission"};
		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = {"SafeContainer/scripts/3_game"};
			};
			class worldScriptModule
			{
				value = "";
				files[] = {"SafeContainer/scripts/4_world"};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {"SafeContainer/scripts/5_mission"};
			};
		};
	};
};

class CfgVehicles
{
	class Container_Base;
	class Askal_SecureContainer: Container_Base
	{
		scope = 2;
		displayName = "Container Seguro";
		descriptionShort = "Container seguro - itens nunca são perdidos na morte";
		model="\dz\gear\containers\Protector_Case.p3d";
		weight = 2000;
		itemSize[] = {8,8};
		itemBehaviour = 0;
        inventorySlot[] = {"StorageSlot"};
		attachments[] = {"VestPouch"};
		class Cargo
		{
			itemsCargoSize[] = {4,6}; // 24 slots (PADRÃO)
			allowOwnedCargoManipulation = 1;
			openable = 0;
		};
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints = 999999;
				};
			};
			class GlobalArmor
			{
				class Projectile { class Health { damage = 0.0; }; class Blood { damage = 0.0; }; class Shock { damage = 0.0; }; };
				class Melee     { class Health { damage = 0.0; }; class Blood { damage = 0.0; }; class Shock { damage = 0.0; }; };
				class Infected  { class Health { damage = 0.0; }; class Blood { damage = 0.0; }; class Shock { damage = 0.0; }; };
				class FragGrenade { class Health { damage = 0.0; }; class Blood { damage = 0.0; }; class Shock { damage = 0.0; }; };
			};
		};
	};
	class Askal_SecureContainer_Lv2: Askal_SecureContainer
	{
		displayName = "Container Seguro Premium";
		descriptionShort = "Container seguro premium - 30 slots";
		class Cargo
		{
			itemsCargoSize[] = {5,6}; // 30 slots
			allowOwnedCargoManipulation = 1;
			openable = 0;
		};
	};
	class Askal_SecureContainer_Lv3: Askal_SecureContainer
	{
		displayName = "Container Seguro VIP";
		descriptionShort = "Container seguro VIP - 36 slots";
		class Cargo
		{
			itemsCargoSize[] = {6,6}; // 36 slots
			allowOwnedCargoManipulation = 1;
			openable = 0;
		};
	};
	class Askal_SecureContainer_Lv4: Askal_SecureContainer
	{
		displayName = "Container Seguro Ultra";
		descriptionShort = "Container seguro ultra - 48 slots";
		class Cargo
		{
			itemsCargoSize[] = {6,8}; // 48 slots
			allowOwnedCargoManipulation = 1;
			openable = 0;
		};
	};

	class Man;
	class InventoryEquipment;
	class SurvivorBase: Man
	{
		attachments[] += {"StorageSlot"};
		class InventoryEquipment: InventoryEquipment
		{
			playerSlots[] += {"Slot_StorageSlot"};
		};
	};
};


class CfgSlots
{
	class Slot_StorageSlot
	{
		name = "StorageSlot";
		displayName = "StorageSlot";
		selection = "slot_StorageSlot";
		ghostIcon = "set:dayz_inventory image:back";
	};
};
	