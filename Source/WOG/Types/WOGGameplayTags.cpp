#include "WOGGameplayTags.h"

//State tags
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Dead, "State.Dead");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Dodging, "State.Movement.Dodging");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Debuff_Stagger, "State.Debuff.Stagger");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Debuff_KO, "State.Debuff.KO");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Debuff_Knockback, "State.Debuff.Knockback");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Debuff_HitReact, "State.Debuff.HitReact");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Weapon_AttackLight, "State.Weapon.AttackLight");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Weapon_AttackHeavy, "State.Weapon.AttackHeavy");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Weapon_Block, "State.Weapon.Block");

//Event tags
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Movement_Sprint_Stop, "Event.Movement.Sprint.Stop");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Weapon_Block_Knockback, "Event.Weapon.Block.Knockback");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Weapon_Block_Impact, "Event.Weapon.Block.Impact");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Weapon_Block_Stop, "Event.Weapon.Block.Stop");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Weapon_Block_Parry, "Event.Weapon.Block.Parry");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Elim, "Event.Elim");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Weapon_HeavyAttackExecute, "Event.Weapon.HeavyAttack.Execute");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Weapon_HeavyAttackCancel, "Event.Weapon.HeavyAttack.Cancel");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Weapon_Equip, "Event.Weapon.Equip");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Weapon_Unequip, "Event.Weapon.Unequip");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Debuff_Stagger, "Event.Debuff.Stagger");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Debuff_KO, "Event.Debuff.KO");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Debuff_HitReact, "Event.Debuff.HitReact");

//Inventory tags
UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Weapon_Shield_Hammer, "Inventory.Weapon.Shield.Hammer");
UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Weapon_Shield_Sword, "Inventory.Weapon.Shield.Sword");
UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Weapon_Shield_Axe, "Inventory.Weapon.Shield.Axe");
UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Weapon_TwoHanded_Sword, "Inventory.Weapon.TwoHanded.Sword");
UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Weapon_TwoHanded_Hammer, "Inventory.Weapon.TwoHanded.Hammer");
UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Weapon_TwoHanded_Axe, "Inventory.Weapon.TwoHanded.Axe");
UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Weapon_DualWield_Axe, "Inventory.Weapon.DualWield.Axe");
UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Weapon_DualWield_Hammer, "Inventory.Weapon.DualWield.Hammer");
UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Weapon_DualWield_Sword, "Inventory.Weapon.DualWield.Sword");

//Pose Tags
UE_DEFINE_GAMEPLAY_TAG(TAG_Pose_Relax, "Pose.Relax");
UE_DEFINE_GAMEPLAY_TAG(TAG_Pose_Shield_Sword, "Pose.Shield.Sword");
UE_DEFINE_GAMEPLAY_TAG(TAG_Pose_Shield_Hammer, "Pose.Shield.Hammer");
UE_DEFINE_GAMEPLAY_TAG(TAG_Pose_Shield_Axe, "Pose.Shield.Axe");
UE_DEFINE_GAMEPLAY_TAG(TAG_Pose_TwoHanded_Sword, "Pose.TwoHanded.Sword");
UE_DEFINE_GAMEPLAY_TAG(TAG_Pose_TwoHanded_Hammer, "Pose.TwoHanded.Hammer");
UE_DEFINE_GAMEPLAY_TAG(TAG_Pose_TwoHanded_Axe, "Pose.TwoHanded.Axe");
UE_DEFINE_GAMEPLAY_TAG(TAG_Pose_DualWield_Sword, "Pose.DualWield.Sword");
UE_DEFINE_GAMEPLAY_TAG(TAG_Pose_DualWield_Hammer, "Pose.DualWield.Hammer");
UE_DEFINE_GAMEPLAY_TAG(TAG_Pose_DualWield_Axe, "Pose.DualWield.Axe");

//Gameplay Cue Tags
UE_DEFINE_GAMEPLAY_TAG(TAG_Cue_Weapon_BodyHit, "GameplayCue.Weapon.BodyHit");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cue_Weapon_Block_Impact, "GameplayCue.Weapon.Block.Impact");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cue_Weapon_AttackHeavy, "GameplayCue.Weapon.AttackHeavy");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cue_Weapon_AttackLight, "GameplayCue.Weapon.AttackLight");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cue_Movement_Dodge, "GameplayCue.Movement.Dodge");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cue_Movement_Jump, "GameplayCue.Movement.Jump");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cue_KO, "GameplayCue.KO");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cue_Elim, "GameplayCue.Elim");