#pragma once

#include "NativeGameplayTags.h"

//State tags
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Dead)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Dodging)

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Weapon_AttackLight)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Weapon_AttackHeavy)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Weapon_Block)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Weapon_Parry)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Weapon_Ranged_Throw)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Weapon_Ranged_AOE)

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Magic_AOE)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Magic_Projectile)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Magic_Instant)

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Debuff_Stagger)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Debuff_KO)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Debuff_Knockback)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Debuff_HitReact)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Debuff_Stun)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Debuff_Burn)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Debuff_Freeze)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Debuff_Shock)


//EventTags
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Movement_Sprint_Stop)

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Debuff_Stagger)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Debuff_KO)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Debuff_HitReact)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Debuff_Knockback)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Debuff_Stun)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Debuff_HealthDrain)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Debuff_Freeze)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Debuff_StaminaDrain)

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Elim)

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Weapon_Block_Impact_Light)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Weapon_Block_Impact_Heavy)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Weapon_Block_Parry)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Weapon_Block_Stop)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Weapon_Ranged_Throw)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Weapon_Ranged_Catch)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Weapon_HeavyAttackExecute)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Weapon_HeavyAttackCancel)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Weapon_Equip)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Weapon_Unequip)

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Magic_SpawnAOE)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Magic_SpawnInstant)

//InventoryItems

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Weapon_Shield)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Weapon_TwoHanded)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Weapon_DualWield)

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Weapon_Shield_Hammer)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Weapon_Shield_Sword)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Weapon_Shield_Axe)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Weapon_TwoHanded_Sword)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Weapon_TwoHanded_Axe)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Weapon_TwoHanded_Hammer)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Weapon_DualWield_Hammer)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Weapon_DualWield_Axe)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Weapon_DualWield_Sword)

//Magic
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Magic_AOE)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Magic_Projectile)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Magic_Instant)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Magic_Buff)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Magic_Debuff)


//PoseTags
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Pose_Relax)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Pose_Magic)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Pose_Shield_Sword)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Pose_Shield_Hammer)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Pose_Shield_Axe)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Pose_TwoHanded_Sword)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Pose_TwoHanded_Hammer)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Pose_TwoHanded_Axe)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Pose_DualWield_Sword)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Pose_DualWield_Hammer)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Pose_DualWield_Axe)

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Pose_Overlay_Block)

//Gameplay Cues
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cue_Weapon_BodyHit)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cue_Weapon_Block_Impact)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cue_KO)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cue_Elim)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cue_Movement_Dodge)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cue_Movement_Jump)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cue_Weapon_AttackLight)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cue_Weapon_AttackHeavy)

//Cooldown

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cooldown_Input)