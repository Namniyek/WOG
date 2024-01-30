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

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Buff_DamageReduction)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Buff_Strength)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Buff_HealthRegen)


//EventTags
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Movement_Sprint_Stop)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Movement_Forward)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Movement_Right)


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

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Magic_Cast)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Magic_SpawnAOE)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Magic_SpawnInstant)

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Spawn_Unpossess)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Spawn_Raven_PlaceMarker)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Spawn_Raven_RemoveMarker)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Spawn_Raven_MoveLeft)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Spawn_Raven_MoveRight)

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Order_Follow)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Order_Hold)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Order_Attack_Target)

//InventoryItems

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Weapon_Shield)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Weapon_TwoHanded)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Weapon_DualWield)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Tool)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Weapon)

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Weapon_Shield_Hammer)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Weapon_Shield_Sword)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Weapon_Shield_Axe)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Weapon_TwoHanded_Sword)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Weapon_TwoHanded_Axe)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Weapon_TwoHanded_Hammer)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Weapon_DualWield_Hammer)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Weapon_DualWield_Axe)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Weapon_DualWield_Sword)

//Resources
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Resource_Gold)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Resource_Iron)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Resource_ChaosEssence)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Resource_Souls)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Resource_Wood)

//Magic
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Magic)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Magic_AOE)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Magic_Projectile)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Magic_Instant)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Magic_Buff)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Magic_Debuff)

//Consumable
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Consumable)

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

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cue_Vocal_Weapon_AttackLight)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cue_Vocal_Weapon_AttackHeavy)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cue_Vocal_Weapon_AttackLong)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cue_Vocal_Weapon_AttackBuildUp)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cue_Vocal_Weapon_BlockImpact)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cue_Vocal_Weapon_RangedThrow)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cue_Vocal_Magic_CastShort)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cue_Vocal_Magic_CastLong)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cue_Vocal_Movement_EffortShort)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cue_Vocal_Movement_EffortLong)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cue_Vocal_Movement_Tired)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cue_Vocal_Hurt_Death)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cue_Vocal_Hurt_PainLong)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cue_Vocal_Hurt_PainShort)

//Cooldown
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cooldown_Input)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cooldown_Input_AbilitySwitch)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cooldown_Input_Magic_AOE)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cooldown_Input_Magic_Instant)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cooldown_Input_Magic_Buff)

//SetByCallerTag
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Data_Cooldown)

//Aux tags
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Aux_Weapon)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Aux_Weapon_Primary)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Aux_Weapon_Secondary)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Aux_Magic)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Aux_Magic_Primary)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Aux_Magic_Secondary)

//Vendor tags
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Vendor_Blacksmith_Defender)