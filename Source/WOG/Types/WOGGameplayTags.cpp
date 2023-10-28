#include "WOGGameplayTags.h"

//State tags
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Dead, "State.Dead");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Dodging, "State.Movement.Dodging");

UE_DEFINE_GAMEPLAY_TAG(TAG_State_Debuff_Stagger, "State.Debuff.Stagger");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Debuff_KO, "State.Debuff.KO");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Debuff_Knockback, "State.Debuff.Knockback");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Debuff_HitReact, "State.Debuff.HitReact");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Debuff_Stun, "State.Debuff.Stun");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Debuff_Burn, "State.Debuff.Burn");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Debuff_Freeze, "State.Debuff.Freeze");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Debuff_Shock, "State.Debuff.Shock");

UE_DEFINE_GAMEPLAY_TAG(TAG_State_Buff_DamageReduction, "State.Buff.DamageReduction");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Buff_Strength, "State.Buff.Strength");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Buff_HealthRegen, "State.Buff.HealthRegen");

UE_DEFINE_GAMEPLAY_TAG(TAG_State_Weapon_AttackLight, "State.Weapon.AttackLight");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Weapon_AttackHeavy, "State.Weapon.AttackHeavy");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Weapon_Block, "State.Weapon.Block");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Weapon_Parry, "State.Weapon.Parry");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Weapon_Ranged_Throw, "State.Weapon.Ranged.Throw");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Weapon_Ranged_AOE, "State.Weapon.Ranged.AOE");

UE_DEFINE_GAMEPLAY_TAG(TAG_State_Magic_AOE, "State.Magic.AOE");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Magic_Projectile, "State.Magic.Projectile");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Magic_Instant, "State.Magic.Instant");


//Event tags
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Elim, "Event.Elim");

UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Movement_Sprint_Stop, "Event.Movement.Sprint.Stop");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Movement_Forward , "Event.Movement.Forward");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Movement_Right, "Event.Movement.Right");

UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Weapon_Block_Impact_Light, "Event.Weapon.Block.Impact.Light");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Weapon_Block_Impact_Heavy, "Event.Weapon.Block.Impact.Heavy");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Weapon_Block_Stop, "Event.Weapon.Block.Stop");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Weapon_Block_Parry, "Event.Weapon.Block.Parry");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Weapon_HeavyAttackExecute, "Event.Weapon.HeavyAttack.Execute");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Weapon_HeavyAttackCancel, "Event.Weapon.HeavyAttack.Cancel");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Weapon_Ranged_Throw, "Event.Weapon.Ranged.Throw");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Weapon_Ranged_Catch, "Event.Weapon.Ranged.Catch");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Weapon_Equip, "Event.Weapon.Equip");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Weapon_Unequip, "Event.Weapon.Unequip");

UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Magic_Cast, "Event.Magic.Cast");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Magic_SpawnAOE, "Event.Magic.SpawnAOE");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Magic_SpawnInstant, "Event.Magic.SpawnInstant");

UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Debuff_Stagger, "Event.Debuff.Stagger");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Debuff_KO, "Event.Debuff.KO");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Debuff_HitReact, "Event.Debuff.HitReact");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Debuff_Knockback, "Event.Debuff.Knockback");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Debuff_Stun, "Event.Debuff.Stun");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Debuff_HealthDrain, "Event.Debuff.HealthDrain");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Debuff_Freeze, "Event.Debuff.Freeze");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Debuff_StaminaDrain, "Event.Debuff.StaminaDrain");

UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Spawn_Unpossess, "Event.Spawn.Unpossess");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Spawn_Raven_PlaceMarker, "Event.Spawn.Raven.PlaceMarker");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Spawn_Raven_RemoveMarker, "Event.Spawn.Raven.RemoveMarker");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Spawn_Raven_MoveLeft, "Event.Spawn.Raven.MoveLeft");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Spawn_Raven_MoveRight, "Event.Spawn.Raven.MoveRight");


//Inventory tags

//Resources
UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Resource_Gold, "Inventory.Resource.Gold");
UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Resource_Iron, "Inventory.Resource.Iron");
UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Resource_ChaosEssence, "Inventory.Resource.ChaosEssence");
UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Resource_Souls, "Inventory.Resource.Souls");
UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Resource_Wood, "Inventory.Resource.Wood");

//Weapons
UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Weapon_Shield, "Inventory.Weapon.Shield");
UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Weapon_TwoHanded, "Inventory.Weapon.TwoHanded");
UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Weapon_DualWield, "Inventory.Weapon.DualWield");
UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Weapon, "Inventory.Weapon");
UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Tool, "Inventory.Tool");

UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Weapon_Shield_Hammer, "Inventory.Weapon.Shield.Hammer");
UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Weapon_Shield_Sword, "Inventory.Weapon.Shield.Sword");
UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Weapon_Shield_Axe, "Inventory.Weapon.Shield.Axe");
UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Weapon_TwoHanded_Sword, "Inventory.Weapon.TwoHanded.Sword");
UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Weapon_TwoHanded_Hammer, "Inventory.Weapon.TwoHanded.Hammer");
UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Weapon_TwoHanded_Axe, "Inventory.Weapon.TwoHanded.Axe");
UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Weapon_DualWield_Axe, "Inventory.Weapon.DualWield.Axe");
UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Weapon_DualWield_Hammer, "Inventory.Weapon.DualWield.Hammer");
UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Weapon_DualWield_Sword, "Inventory.Weapon.DualWield.Sword");

//Magic
UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Magic, "Inventory.Magic");
UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Magic_AOE, "Inventory.Magic.AOE");
UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Magic_Projectile, "Inventory.Magic.Projectile");
UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Magic_Instant, "Inventory.Magic.Instant");
UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Magic_Buff, "Inventory.Magic.Buff");
UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Magic_Debuff, "Inventory.Magic.Debuff");


//Pose Tags
UE_DEFINE_GAMEPLAY_TAG(TAG_Pose_Relax, "Pose.Relax");
UE_DEFINE_GAMEPLAY_TAG(TAG_Pose_Magic, "Pose.Magic");
UE_DEFINE_GAMEPLAY_TAG(TAG_Pose_Shield_Sword, "Pose.Shield.Sword");
UE_DEFINE_GAMEPLAY_TAG(TAG_Pose_Shield_Hammer, "Pose.Shield.Hammer");
UE_DEFINE_GAMEPLAY_TAG(TAG_Pose_Shield_Axe, "Pose.Shield.Axe");
UE_DEFINE_GAMEPLAY_TAG(TAG_Pose_TwoHanded_Sword, "Pose.TwoHanded.Sword");
UE_DEFINE_GAMEPLAY_TAG(TAG_Pose_TwoHanded_Hammer, "Pose.TwoHanded.Hammer");
UE_DEFINE_GAMEPLAY_TAG(TAG_Pose_TwoHanded_Axe, "Pose.TwoHanded.Axe");
UE_DEFINE_GAMEPLAY_TAG(TAG_Pose_DualWield_Sword, "Pose.DualWield.Sword");
UE_DEFINE_GAMEPLAY_TAG(TAG_Pose_DualWield_Hammer, "Pose.DualWield.Hammer");
UE_DEFINE_GAMEPLAY_TAG(TAG_Pose_DualWield_Axe, "Pose.DualWield.Axe");

UE_DEFINE_GAMEPLAY_TAG(TAG_Pose_Overlay_Block, "Pose.Overlay.Block");

//Gameplay Cue Tags
UE_DEFINE_GAMEPLAY_TAG(TAG_Cue_Weapon_BodyHit, "GameplayCue.Weapon.BodyHit");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cue_Weapon_Block_Impact, "GameplayCue.Weapon.Block.Impact");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cue_Weapon_AttackHeavy, "GameplayCue.Weapon.AttackHeavy");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cue_Weapon_AttackLight, "GameplayCue.Weapon.AttackLight");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cue_Movement_Dodge, "GameplayCue.Movement.Dodge");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cue_Movement_Jump, "GameplayCue.Movement.Jump");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cue_KO, "GameplayCue.KO");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cue_Elim, "GameplayCue.Elim");

UE_DEFINE_GAMEPLAY_TAG(TAG_Cue_Vocal_Weapon_BlockImpact, "GameplayCue.Vocal.Weapon.BlockImpact");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cue_Vocal_Weapon_AttackHeavy, "GameplayCue.Vocal.Weapon.AttackHeavy");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cue_Vocal_Weapon_AttackLight, "GameplayCue.Vocal.Weapon.AttackLight");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cue_Vocal_Weapon_AttackLong, "GameplayCue.Vocal.Weapon.AttackLong");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cue_Vocal_Weapon_AttackBuildUp, "GameplayCue.Vocal.Weapon.AttackBuildUp");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cue_Vocal_Weapon_RangedThrow, "GameplayCue.Vocal.Weapon.RangedThrow");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cue_Vocal_Magic_CastShort, "GameplayCue.Vocal.Magic.CastShort");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cue_Vocal_Magic_CastLong, "GameplayCue.Vocal.Magic.CastLong");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cue_Vocal_Movement_EffortShort, "GameplayCue.Vocal.Movement.EffortShort");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cue_Vocal_Movement_EffortLong, "GameplayCue.Vocal.Movement.EffortLong");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cue_Vocal_Movement_Tired, "GameplayCue.Vocal.Movement.Tired");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cue_Vocal_Hurt_Death, "GameplayCue.Vocal.Hurt.Death");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cue_Vocal_Hurt_PainShort, "GameplayCue.Vocal.Hurt.PainShort");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cue_Vocal_Hurt_PainLong, "GameplayCue.Vocal.Hurt.PainLong");

//Cooldown tags
UE_DEFINE_GAMEPLAY_TAG(TAG_Cooldown_Input, "Cooldown.Input");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cooldown_Input_AbilitySwitch, "Cooldown.Input.AbilitySwitch");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cooldown_Input_Magic_AOE, "Cooldown.Input.Magic.AOE");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cooldown_Input_Magic_Instant, "Cooldown.Input.Magic.Instant");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cooldown_Input_Magic_Buff, "Cooldown.Input.Magic.Buff");

//SetByCallerTag
UE_DEFINE_GAMEPLAY_TAG(TAG_Data_Cooldown, "Data.Cooldown");

//Aux tags
UE_DEFINE_GAMEPLAY_TAG(TAG_Aux_Weapon, "Aux.Weapon");
UE_DEFINE_GAMEPLAY_TAG(TAG_Aux_Weapon_Primary, "Aux.Weapon.Primary");
UE_DEFINE_GAMEPLAY_TAG(TAG_Aux_Weapon_Secondary, "Aux.Weapon.Secondary");

//Vendor tags
UE_DEFINE_GAMEPLAY_TAG(TAG_Vendor_Blacksmith_Defender, "Vendor.Blacksmith.Defender");