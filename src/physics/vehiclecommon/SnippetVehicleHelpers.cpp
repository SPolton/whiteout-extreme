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


#include <ctype.h>

#include "SnippetVehicleHelpers.h"
#include "../common/Flags.hpp"
#include "utils/logger.h"

using namespace physx;


namespace snippetvehicle
{

PxFilterFlags VehicleFilterShader(
PxFilterObjectAttributes attributes0, PxFilterData filterData0,
PxFilterObjectAttributes attributes1, PxFilterData filterData1,
PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{
	PX_UNUSED(attributes0);
	PX_UNUSED(attributes1);
	PX_UNUSED(constantBlock);
	PX_UNUSED(constantBlockSize);

    // Only apply filtering if BOTH objects have filter data set (word0 != 0)
    // This ensures objects without explicit filtering (like ground) still collide normally
    if (filterData0.word0 != 0 && filterData1.word0 != 0) {
        // Check if the two objects should collide based on their filter data
        // word0 = what this object is
        // word1 = what it collides with
        bool shouldCollide = (filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1);
        
        if (!shouldCollide) {
            // Suppress collision if filtering explicitly prevents it
            return PxFilterFlag::eSUPPRESS;
        }
    }

    // Default collision behavior
    pairFlags = PxPairFlag::eCONTACT_DEFAULT;

    // Special notification for obstacle-chassis collisions
    if ((filterData0.word0 == COLLISION_FLAG_OBSTACLE && filterData1.word0 == COLLISION_FLAG_CHASSIS) ||
        (filterData0.word0 == COLLISION_FLAG_CHASSIS && filterData1.word0 == COLLISION_FLAG_OBSTACLE)) {
        pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
        logger::debug("Collision detected between OBSTACLE and CHASSIS");
    }

    // Special notification for chassis-chassis collisions
    if (filterData0.word0 == COLLISION_FLAG_CHASSIS && filterData1.word0 == COLLISION_FLAG_CHASSIS) {
        pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
        logger::debug("Collision detected between CHASSIS and CHASSIS");
    }

    // Apply additional pair flags from word2 (enums are bitwise)
    pairFlags |= physx::PxPairFlags(physx::PxU16(filterData0.word2 | filterData1.word2));
    
    return physx::PxFilterFlags();
}


bool parseVehicleDataPath(int argc, const char *const* argv, const char* snippetName,
	const char*& vehicleDataPath)
{
	if (argc != 2 || 0 != strncmp(argv[1], "--vehicleDataPath", strlen("--vehicleDataPath")))
	{
		printf("%s usage:\n"
			"%s "
			"--vehicleDataPath=<path to the [PHYSX_ROOT]/snippets/media/vehicledata folder containing the vehiclejson files to be loaded> \n",
			snippetName, snippetName);
		return false;
	}
	vehicleDataPath = argv[1] + strlen("--vehicleDataPath=");
	return true;
}

}//namespace snippetvehicle
