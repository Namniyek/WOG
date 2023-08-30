// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WOGHoldProgressBar.h"

float UWOGHoldProgressBar::GetHoldProgress()
{
    return TimeHeld/MaxHoldTime;
}
