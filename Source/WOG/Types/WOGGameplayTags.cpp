#include "WOGGameplayTags.h"

//State tags
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Dead, "State.Dead");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Dodging, "State.Movement.Dodging");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Debuff_Stagger, "State.Debuff.Stagger");

//Event tags
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Movement_Sprint_Stop, "Event.Movement.Sprint.Stop");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Weapon_Block_Stop, "Event.Weapon.Block.Stop");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Elim, "Event.Elim");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Weapon_HeavyAttackExecute, "Event.Weapon.HeavyAttack.Execute");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Weapon_HeavyAttackCancel, "Event.Weapon.HeavyAttack.Cancel");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Weapon_Equip, "Event.Weapon.Equip");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Weapon_Unequip, "Event.Weapon.Unequip");

//Inventory tags
UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Weapon_1H_HammerShield, "Inventory.Weapon.1H.HammerShield");
UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Weapon_2H_GreatSword, "Inventory.Weapon.2H.GreatSword");