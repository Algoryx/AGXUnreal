// Fill out your copyright notice in the Description page of Project Settings.


#include "AGX_Constraint1DOF.h"


AAGX_Constraint1DOF::AAGX_Constraint1DOF()
{
}


AAGX_Constraint1DOF::AAGX_Constraint1DOF(const TArray<EDofFlag> &LockedDofsOrdered)
	: AAGX_Constraint(LockedDofsOrdered)
{

}


AAGX_Constraint1DOF::~AAGX_Constraint1DOF()
{

}