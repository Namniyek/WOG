// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#define ECC_DamageTrace ECC_GameTraceChannel6
#define ECC_Damageable ECC_GameTraceChannel10

static FName NAME_WeaponSlot_Primary = FName("Primary");
static FName NAME_WeaponSlot_BackMain = FName("BackMain");
static FName NAME_WeaponSlot_BackSecondary = FName("BackSecondary");
static FName NAME_MagicSlot_MagicPrimary = FName("MagicPrimary");
static FName NAME_MagicSlot_MagicBackMain = FName("MagicBackMain");
static FName NAME_MagicSlot_MagicBackSecondary = FName("MagicBackSecondary");
static FName NAME_ConsumableSlot_Consumable = FName("Consumable");

static FName WOG_SESSION_NAME = FName("WOG_default_session");

static FName WOG_Overhead_Widget_Location = FName("WidgetLocation");

DECLARE_LOG_CATEGORY_EXTERN(WOGLogUI, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(WOGLogCombat, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(WOGLogSpawn, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(WOGLogWorld, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(WOGLogInventory, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(WOGLogBuild, Log, All);
