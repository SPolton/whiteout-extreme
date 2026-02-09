// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Copyright (c) 2008-2025 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

// ****************************************************************************
// This snippet illustrates simple use of the physx vehicle sdk and demonstrates
// how to simulate a vehicle with a fully featured drivetrain comprising engine,
// clutch, differential and gears.  The snippet uses only parameters, states and 
// components maintained by the PhysX Vehicle SDK.

// Vehicles are made of parameters, states and components.

// Parameters describe the configuration of a vehicle.  Examples are vehicle mass, wheel radius 
// and suspension stiffness.

// States describe the instantaneous dynamic state of a vehicle.  Examples are engine revs, wheel 
// yaw angle and tire slip angles.

// Components forward integrate the dynamic state of the vehicle, given the previous vehicle state 
// and the vehicle's parameterisation.
// Components update dynamic state by invoking reusable functions in a particular sequence. 
// An example component is a rigid body component that updates the linear and angular velocity of 
// the vehicle's rigid body given the instantaneous forces and torques of the suspension and tire 
// states.

// The pipeline of vehicle computation is a sequence of components that run in order.  For example, 
// one component might compute the plane under the wheel by performing a scene query against the 
// world geometry. The next component in the sequence might compute the suspension compression required 
// to place the wheel on the surface of the hit plane. Following this, another component might compute 
// the suspension force that arises from that compression.  The rigid body component, as discussed earlier, 
// can then forward integrate the rigid body's linear velocity using the suspension force.

// Custom combinations of parameter, state and component allow different behaviours to be simulated with 
// different simulation fidelities.  For example, a suspension component that implements a linear force 
// response with respect to its compression state could be replaced with one that imlements a non-linear
// response.  The replacement component would consume the same suspension compression state data and 
// would output the same suspension force data structure.  In this example, the change has been localised 
// to the  component that converts suspension compression to force and to the parameterisation that governs 
// that conversion.
// Another combination example could be the replacement of the tire component from a low fidelity model to 
// a high fidelty model such as Pacejka. The low and high fidelity components consume the same state data 
// (tire slip, load, friction) and  output the same state data  for the tire forces. Again, the 
// change has been localised to the component that converts slip angle to tire force and the 
// parameterisation that governs the conversion.

//The PhysX Vehicle SDK presents a maintained set of parameters, states and components.  The maintained 
//set of parameters, states and components may be combined on their own or combined with custom parameters, 
//states and components.

//This snippet breaks the vehicle into into three distinct models:
//1) a base vehicle model that describes the mechanical configuration of suspensions, tires, wheels and an 
//   associated rigid body.
//2) a drivetrain model that forwards input controls to wheel torques via a drivetrain model
//   that includes engine, clutch, differential and gears.
//3) a physx integration model that provides a representation of the vehicle in an associated physx scene.

// It is a good idea to record and playback with pvd (PhysX Visual Debugger).
// ****************************************************************************

#include <ctype.h>

#include "VehicleFourWheelDrive.hpp"
#include "vehiclecommon/enginedrivetrain/EngineDrivetrain.h"
#include "vehiclecommon/serialization/BaseSerialization.h"
#include "vehiclecommon/serialization/EngineDrivetrainSerialization.h"

#include "common/Flags.hpp"
#include "utils/logger.h"

// OK in cpp files, not in headers
using namespace physx;
using namespace physx::vehicle2;
using namespace snippetvehicle;

//The vehicle with engine drivetrain
EngineDriveVehicle gVehicle;

//Commands are issued to the vehicle in a pre-choreographed sequence.
struct Command
{
    PxF32 brake;
    PxF32 throttle;
    PxF32 steer;
    PxU32 gear;
    PxF32 duration;
};
const PxU32 gTargetGearCommand = PxVehicleEngineDriveTransmissionCommandState::eAUTOMATIC_GEAR;
Command gCommands[] =
{
    {0.5f, 0.0f, 0.0f, gTargetGearCommand, 2.0f},	//brake on and come to rest for 2 seconds
    {0.0f, 0.65f, 0.0f, gTargetGearCommand, 5.0f},	//throttle for 5 seconds
    {0.5f, 0.0f, 0.0f, gTargetGearCommand, 5.0f},	//brake for 5 seconds
    {0.0f, 0.75f, 0.0f, gTargetGearCommand, 5.0f},	//throttle for 5 seconds
    {0.0f, 0.25f, 0.5f, gTargetGearCommand, 5.0f}	//light throttle and steer for 5 seconds.
};
const PxU32 gNbCommands = sizeof(gCommands) / sizeof(Command);
PxReal gCommandTime = 0.0f;			//Time spent on current command
PxU32 gCommandProgress = 0;			//The id of the current command.


VehicleFourWheelDrive::VehicleFourWheelDrive(ConstructData info)
    : PhysicsObject(info.vehicleName)
    , mVehicleDataPath(info.vehicleDataPath)
{
    if (!info.material || !info.physics || !info.scene) {
        logger::error("Invalid ConstructData provided to VehicleFourWheelDrive.");
        throw std::invalid_argument("ConstructData contains null pointers.");
    }

    //Check that we can read from the json file before continuing.
    BaseVehicleParams baseParams;
    if (!readBaseParamsFromJsonFile(mVehicleDataPath, "Base.json", baseParams)) {
        logger::error("Failed to read BaseVehicleParams from JSON file.");
        throw std::runtime_error("BaseVehicleParams JSON read failed.");
    }

    //Check that we can read from the json file before continuing.
    EngineDrivetrainParams engineDrivetrainParams;
    if (!readEngineDrivetrainParamsFromJsonFile(mVehicleDataPath, "EngineDrive.json",
        engineDrivetrainParams)) {
        logger::error("Failed to read EngineDrivetrainParams from JSON file.");
        throw std::runtime_error("EngineDrivetrainParams JSON read failed.");
    }

    // Assuming physX and ground plane are initialized elsewhere
    initMaterialFrictionTable(info);
    if (!initVehicles(info)) {
        logger::error("Failed to initialize VehicleFourWheelDrive.");
        throw std::runtime_error("VehicleFourWheelDrive initialization failed.");
    }
}

VehicleFourWheelDrive::~VehicleFourWheelDrive()
{
    cleanupVehicles();
}

void VehicleFourWheelDrive::initMaterialFrictionTable(ConstructData info)
{
	//Each physx material can be mapped to a tire friction value on a per tire basis.
	//If a material is encountered that is not mapped to a friction value, the friction value used is the specified default value.
	//In this snippet there is only a single material so there can only be a single mapping between material and friction.
	//In this snippet the same mapping is used by all tires.
	mPhysXMaterialFrictions[0].friction = 1.0f;
	mPhysXMaterialFrictions[0].material = info.material;
	mPhysXDefaultMaterialFriction = 1.0f;
	mNbPhysXMaterialFrictions = 1;
    logger::info("Material friction table initialized with {} entries.", mNbPhysXMaterialFrictions);
}

bool VehicleFourWheelDrive::initVehicles(ConstructData info)
{
	//Load the params from json or set directly.
	readBaseParamsFromJsonFile(mVehicleDataPath, "Base.json", gVehicle.mBaseParams);
	setPhysXIntegrationParams(gVehicle.mBaseParams.axleDescription,
		mPhysXMaterialFrictions, mNbPhysXMaterialFrictions, mPhysXDefaultMaterialFriction,
		gVehicle.mPhysXParams);
	readEngineDrivetrainParamsFromJsonFile(mVehicleDataPath, "EngineDrive.json", 
		gVehicle.mEngineDriveParams);

	//Set the states to default.
	if (!gVehicle.initialize(*info.physics, PxCookingParams(PxTolerancesScale()), *info.material, EngineDriveVehicle::eDIFFTYPE_FOURWHEELDRIVE))
	{
        logger::error("EngineDriveVehicle initialization failed.");
		return false;
	}

	//Apply a start pose to the physx actor and add it to the physx scene.
	PxTransform pose(PxVec3(0.000000000f, -0.0500000119f, -10.59399998f), PxQuat(PxIdentity));
	gVehicle.setUpActor(*info.scene, pose, info.vehicleName);

	//Set the vehicle in 1st gear.
	gVehicle.mEngineDriveState.gearboxState.currentGear = gVehicle.mEngineDriveParams.gearBoxParams.neutralGear + 1;
	gVehicle.mEngineDriveState.gearboxState.targetGear = gVehicle.mEngineDriveParams.gearBoxParams.neutralGear + 1;

	//Set the vehicle to use the automatic gearbox.
	gVehicle.mTransmissionCommandState.targetGear = PxVehicleEngineDriveTransmissionCommandState::eAUTOMATIC_GEAR;

    // Collision filtering for the vehicle
    PxFilterData vehicleFilter(COLLISION_FLAG_CHASSIS, COLLISION_FLAG_CHASSIS_AGAINST, 0, 0);

    // Loop through each shape and set the query and simulation flags.
    // This is required for collision detection with other objects.
    PxU32 shapes = gVehicle.mPhysXState.physxActor.rigidBody->getNbShapes();
    for (PxU32 i = 0; i < shapes; i++) {
        PxShape* shape = NULL;
        gVehicle.mPhysXState.physxActor.rigidBody->getShapes(&shape, 1, i);

        // Add filter to our shader
        shape -> setSimulationFilterData(vehicleFilter);

        shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
        shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
        shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);
    }

	//Set up the simulation context.
	//The snippet is set up with
	//a) z as the longitudinal axis
	//b) x as the lateral axis
	//c) y as the vertical axis.
	//d) metres  as the lengthscale.
	mVehicleSimulationContext.setToDefault();
	mVehicleSimulationContext.frame.lngAxis = PxVehicleAxes::ePosZ;
	mVehicleSimulationContext.frame.latAxis = PxVehicleAxes::ePosX;
	mVehicleSimulationContext.frame.vrtAxis = PxVehicleAxes::ePosY;
	mVehicleSimulationContext.scale.scale = 1.0f;
	mVehicleSimulationContext.gravity = info.gravity;
	mVehicleSimulationContext.physxScene = info.scene;
	mVehicleSimulationContext.physxActorUpdateMode = PxVehiclePhysXActorUpdateMode::eAPPLY_ACCELERATION;

    logger::info("VehicleFourWheelDrive initialized successfully.");
	return true;
}

void VehicleFourWheelDrive::cleanupVehicles()
{
	gVehicle.destroy();
    logger::info("VehicleFourWheelDrive cleaned up successfully.");
}

void VehicleFourWheelDrive::stepPhysics(float deltaTime)
{
	if (gNbCommands == gCommandProgress)
		return;

	//Apply the brake, throttle and steer to the command state of the vehicle.
	const Command& command = gCommands[gCommandProgress];
	gVehicle.mCommandState.brakes[0] = command.brake;
	gVehicle.mCommandState.nbBrakes = 1;
	gVehicle.mCommandState.throttle = command.throttle;
	gVehicle.mCommandState.steer = command.steer;
	gVehicle.mTransmissionCommandState.targetGear = command.gear;

	//Forward integrate the vehicle by a single timestep.
	//Apply substepping at low forward speed to improve simulation fidelity.
	const PxVec3 linVel = gVehicle.mPhysXState.physxActor.rigidBody->getLinearVelocity();
	const PxVec3 forwardDir = gVehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q.getBasisVector2();
	const PxReal forwardSpeed = linVel.dot(forwardDir);
	const PxU8 nbSubsteps = (forwardSpeed < 5.0f ? 3 : 1);
	gVehicle.mComponentSequence.setSubsteps(gVehicle.mComponentSequenceSubstepGroupHandle, nbSubsteps);
	gVehicle.step(deltaTime, mVehicleSimulationContext);

    //Assume physx scene is stepped elsewhere 
	//Forward integrate the phsyx scene by a single timestep.
	// mScene->simulate(timestep);
	// mScene->fetchResults(true);

	//Increment the time spent on the current command.
	//Move to the next command in the list if enough time has lapsed.
	gCommandTime += deltaTime;
	if (gCommandTime > gCommands[gCommandProgress].duration)
	{
		gCommandProgress++;
		gCommandTime = 0.0f;
	}
}

physx::PxRigidActor* VehicleFourWheelDrive::getRigidActor()
{
	return gVehicle.mPhysXState.physxActor.rigidBody;
}

