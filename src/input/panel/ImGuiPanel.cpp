#include "ImGuiPanel.hpp"

const glm::vec3 defaultBackgroundColor(0.47f, 0.82f, 1.0f);

ImGuiPanel::ImGuiPanel()
    : backgroundColor(defaultBackgroundColor)
{
}

void ImGuiPanel::update()
{
    // Main debug info window
    if (showDebugWindow) {
        renderDebugInfo();
    }

    // Settings window
    if (showSettingsWindow) {
        ImGui::SetNextWindowPos(ImVec2(600, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);
        ImGui::Begin("Settings", &showSettingsWindow);
        renderRenderSettings();
        renderCameraInfo();
        renderControls();
        renderVehiclePhysx();
        ImGui::End();
    }
}

void ImGuiPanel::renderDebugInfo()
{
    ImGui::SetNextWindowPos(ImVec2(10, 450), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiCond_FirstUseEver);
    ImGui::Begin("Debug Info", &showDebugWindow);
    
    // FPS counter
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("FPS: %.1f", io.Framerate);
    ImGui::Text("Frame time: %.3f ms", 1000.0f / io.Framerate);
    
    ImGui::Separator();
    
    // Renderer info
    ImGui::Text("Renderer: OpenGL");
    ImGui::Text("Triangles rendered: 1"); // Simple for now
    
    ImGui::End();
}

void ImGuiPanel::renderRenderSettings()
{
    if (ImGui::CollapsingHeader("Render Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::ColorEdit3("Background Color", &backgroundColor[0]);
        ImGui::Checkbox("Wireframe Mode", &showWireframe);
        ImGui::Checkbox("Show Normals", &showNormals);
        ImGui::SliderFloat("Animation Speed", &animationSpeed, 0.0f, 5.0f);
        
        if (ImGui::Button("Reset Defaults")) {
            backgroundColor = defaultBackgroundColor;
            showWireframe = false;
            showNormals = false;
            animationSpeed = 1.0f;
        }
    }
}

void ImGuiPanel::renderCameraInfo()
{
    if (ImGui::CollapsingHeader("Camera")) {
        glm::vec3 pos = cameraStats.position;
        glm::vec3 target = cameraStats.target;
        
        ImGui::Text("Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
        ImGui::Text("Target:   (%.2f, %.2f, %.2f)", target.x, target.y, target.z);
        ImGui::Text("Distance: %.2f", cameraStats.distance);
        ImGui::Text("FOV: %.2f", cameraStats.fov);
        ImGui::Text("Scale: %.2f", cameraStats.scale);
        ImGui::Text("Aspect: %.2f", cameraStats.aspect);
        ImGui::Text("Yaw: %.2f", cameraStats.yaw);
        ImGui::Text("Pitch: %.2f", cameraStats.pitch);
        
        ImGui::Separator();
        ImGui::SliderFloat("Camera Speed", &camSpeed, 1.0f, 10.0f);
        ImGui::SliderFloat("Zoom Speed", &camZoomSpeed, 0.1f, 20.0f);
    }
}

void ImGuiPanel::renderControls()
{
    if (ImGui::CollapsingHeader("Controls")) {
        ImGui::BulletText("ESC - Exit application");
        ImGui::BulletText("Right Mouse + Drag - Rotate camera");
        ImGui::BulletText("Mouse Wheel - Zoom in/out");
    }
}

/* BEFORE DIRECT DRIVE SWITCH
void ImGuiPanel::setVehicle(VehicleFourWheelDrive* v) {
    if (!v) return;
    vehicle = v;

    // On ne fait le snapshot qu'une seule fois pour garder les vraies valeurs JSON
    if (!defaultParams.isSet) {
        defaultParams.base = vehicle->getVehicleData().mBaseParams;
        //defaultParams.engine = vehicle->getVehicleData().mEngineDriveParams;
        defaultParams.engine = vehicle->getVehicleData().mDirectDriveParams;
        defaultParams.isSet = true;
    }
}


void ImGuiPanel::renderVehiclePhysx() {
    if (!vehicle) return;
    auto& current = vehicle->getVehicleData();

    auto HelpMarker = [](const char* desc) {
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted(desc);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
        };

    if (ImGui::CollapsingHeader("Vehicle Physx Tuning - EXHAUSTIVE", ImGuiTreeNodeFlags_DefaultOpen)) {

        // --- ACTIONS ---
        if (ImGui::Button("Reset Defaults")) {
            current.mBaseParams = defaultParams.base;
            current.mEngineDriveParams = defaultParams.engine;
        }
        ImGui::SameLine();
        if (ImGui::Button("Export to Console")) {
            printf("\n--- PHYSX EXPORT ---\nMaxOmega: %f\nPeakTorque: %f\nMass: %f\n",
                current.mEngineDriveParams.engineParams.maxOmega,
                current.mEngineDriveParams.engineParams.peakTorque,
                current.mBaseParams.rigidBodyParams.mass);
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset to Spawn")) {
            physx::PxTransform targetPose(physx::PxVec3(-730.0f, 670.4f, -400.0f), physx::PxQuat(physx::PxIdentity));
            auto* actor = vehicle->getRigidActor()->is<physx::PxRigidDynamic>();
            if (actor) {
                actor->setGlobalPose(targetPose);
                actor->setLinearVelocity(physx::PxVec3(0, 0, 0));
                actor->setAngularVelocity(physx::PxVec3(0, 0, 0));
                current.mBaseState.rigidBodyState.linearVelocity = physx::PxVec3(0, 0, 0);
                current.mBaseState.rigidBodyState.angularVelocity = physx::PxVec3(0, 0, 0);
                current.mEngineDriveState.engineState.rotationSpeed = 0.0f;
                current.mEngineDriveState.gearboxState.currentGear = 1;
            }
        }

        ImGui::Separator();

        // --- 1. ENGINE & BOX ---
        if (ImGui::TreeNodeEx("Engine & Gearbox", ImGuiTreeNodeFlags_DefaultOpen)) {
            float maxRPM = current.mEngineDriveParams.engineParams.maxOmega * (60.0f / 6.28318f);
            if (ImGui::SliderFloat("Engine Redline (RPM)", &maxRPM, 500.0f, 20000.0f, "%.0f RPM")) {
                current.mEngineDriveParams.engineParams.maxOmega = maxRPM * (6.28318f / 60.0f);
            }
            ImGui::SliderFloat("Peak Torque", &current.mEngineDriveParams.engineParams.peakTorque, 50.0f, 15000.0f, "%.0f Nm");
            ImGui::SliderFloat("Final Ratio", &current.mEngineDriveParams.gearBoxParams.finalRatio, 0.5f, 15.0f, "%.2f");
            ImGui::SliderFloat("Gear Switch Time", &current.mEngineDriveParams.gearBoxParams.switchTime, 0.0f, 1.0f, "%.3f s");
            ImGui::TreePop();
        }

        // --- 2. TRANSMISSION & DIFFS ---
        if (ImGui::TreeNodeEx("Transmission & Differentials", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& diff = current.mEngineDriveParams.fourWheelDifferentialParams;
            float currentSplit = diff.torqueRatios[0] / (diff.torqueRatios[0] + diff.torqueRatios[2] + 0.0001f);
            if (ImGui::SliderFloat("Torque Split", &currentSplit, 0.0f, 1.0f, "Rear <-> Front")) {
                diff.torqueRatios[0] = diff.torqueRatios[1] = currentSplit * 0.5f;
                diff.torqueRatios[2] = diff.torqueRatios[3] = (1.0f - currentSplit) * 0.5f;
            }
            ImGui::SliderFloat("Rear Bias (LSD)", &diff.rearBias, 0.0f, 20.0f, "%.1f");
            ImGui::SliderFloat("Center Bias", &diff.centerBias, 0.0f, 20.0f, "%.1f");
            ImGui::SliderFloat("Diff Rate", &diff.rate, 0.0f, 100.0f, "%.1f");
            ImGui::TreePop();
        }

        // --- 3. BRAKE & STEERING ---
        if (ImGui::TreeNodeEx("Braking & Steering", ImGuiTreeNodeFlags_DefaultOpen)) {
            static float maxBrakeForce = current.mBaseParams.brakeResponseParams[0].maxResponse;
            if (ImGui::SliderFloat("Max Brake Torque", &maxBrakeForce, 0.0f, 15000.0f, "%.0f Nm")) {
                for (int i = 0; i < 2; i++) current.mBaseParams.brakeResponseParams[i].maxResponse = maxBrakeForce;
            }

            float steerDeg = current.mBaseParams.steerResponseParams.maxResponse * (180.0f / 3.14159f);
            if (ImGui::SliderFloat("Max Steer Angle", &steerDeg, 5.0f, 90.0f, "%.1f deg")) {
                current.mBaseParams.steerResponseParams.maxResponse = steerDeg * (3.14159f / 180.0f);
            }
            ImGui::TreePop();
        }

        // --- 4. CHASSIS & PHYSIC ---
        if (ImGui::TreeNodeEx("Chassis & Physics", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::SliderFloat("Mass", &current.mBaseParams.rigidBodyParams.mass, 10.0f, 10000.0f, "%.0f kg");
            ImGui::DragFloat3("Inertia (MOI)", &current.mBaseParams.rigidBodyParams.moi.x, 10.0f, 10.0f, 100000.0f, "%.0f");
            ImGui::TreePop();
        }

        // --- 5. TIRES & GRIP ---
        if (ImGui::TreeNodeEx("Tires & Friction (Hyper Grip)", ImGuiTreeNodeFlags_DefaultOpen)) {
            static float allLong = 1500000.0f;
            static float allLat = 2000000.0f;

            if (ImGui::SliderFloat("ALL WHEELS: LongStiff", &allLong, 0.0f, 5000000.0f, "%.0f")) {
                for (int i = 0; i < 4; i++) current.mBaseParams.tireForceParams[i].longStiff = allLong;
            }
            if (ImGui::SliderFloat("ALL WHEELS: LatStiff", &allLat, 0.0f, 5000000.0f, "%.0f")) {
                for (int i = 0; i < 4; i++) current.mBaseParams.tireForceParams[i].latStiffX = allLat;
            }
            ImGui::Separator();

            for (int i = 0; i < 4; i++) {
                ImGui::PushID(i + 500);
                if (ImGui::TreeNode("Individual Wheel", "Wheel %d", i)) {
                    ImGui::SliderFloat("Longitudinal Stiffness", &current.mBaseParams.tireForceParams[i].longStiff, 0.0f, 5000000.0f);
                    ImGui::SliderFloat("Lateral Stiffness", &current.mBaseParams.tireForceParams[i].latStiffX, 0.0f, 5000000.0f);
                    ImGui::TreePop();
                }
                ImGui::PopID();
            }
            ImGui::TreePop();
        }

        // --- 6. SUSPENSIONS ---
        if (ImGui::TreeNodeEx("Suspensions & SprungMass", ImGuiTreeNodeFlags_DefaultOpen)) {
            static float allTravel = 0.1f;
            static float allStiff = 300000.0f;
            static float allDamp = 20000.0f;
            static float allSprung = current.mBaseParams.rigidBodyParams.mass / 4.0f;

            if (ImGui::SliderFloat("ALL: Travel Distance", &allTravel, 0.01f, 1.5f, "%.2f m")) {
                for (int i = 0; i < 4; i++) current.mBaseParams.suspensionParams[i].suspensionTravelDist = allTravel;
            }
            if (ImGui::SliderFloat("ALL: Spring Stiffness", &allStiff, 0.0f, 1000000.0f, "%.0f")) {
                for (int i = 0; i < 4; i++) current.mBaseParams.suspensionForceParams[i].stiffness = allStiff;
            }
            if (ImGui::SliderFloat("ALL: Spring Damping", &allDamp, 0.0f, 100000.0f, "%.0f")) {
                for (int i = 0; i < 4; i++) current.mBaseParams.suspensionForceParams[i].damping = allDamp;
            }
            if (ImGui::SliderFloat("ALL: Sprung Mass", &allSprung, 1.0f, 5000.0f, "%.1f kg")) {
                for (int i = 0; i < 4; i++) current.mBaseParams.suspensionForceParams[i].sprungMass = allSprung;
            }

            ImGui::Separator();
            for (int i = 0; i < 4; i++) {
                ImGui::PushID(i + 100);
                if (ImGui::TreeNode("Individual Suspension", "Suspension %d", i)) {
                    auto& sForce = current.mBaseParams.suspensionForceParams[i];
                    ImGui::SliderFloat("Travel Distance", &current.mBaseParams.suspensionParams[i].suspensionTravelDist, 0.01f, 1.5f);
                    ImGui::SliderFloat("Stiffness", &sForce.stiffness, 0.0f, 1000000.0f);
                    ImGui::SliderFloat("Damping", &sForce.damping, 0.0f, 100000.0f);
                    ImGui::SliderFloat("Sprung Mass", &sForce.sprungMass, 1.0f, 5000.0f);
                    ImGui::TreePop();
                }
                ImGui::PopID();
            }
            ImGui::TreePop();
        }

        // --- LIVE DASHBOARD ---
        ImGui::SeparatorText("Live Stats");
        float speedKmh = current.mBaseState.rigidBodyState.linearVelocity.magnitude() * 3.6f;
        ImGui::Text("Velocity: %.1f km/h", speedKmh);
        ImGui::ProgressBar(speedKmh / 300.0f, ImVec2(-1, 15), "");
        ImGui::Text("RPM: %.0f", current.mEngineDriveState.engineState.rotationSpeed * (60.0f / 6.28318f));
    }
}
*/

void ImGuiPanel::setVehicle(VehicleFourWheelDrive* v) {
    if (!v) return;
    vehicle = v;

    // On r�cup�re les donn�es actuelles du v�hicule
    auto& vehicleData = vehicle->getVehicleData();

    // On ne fait le snapshot qu'une seule fois au premier branchement
    if (!defaultParams.isSet) {
        // Copie des param�tres de base (Masse, Suspensions, Pneus)
        defaultParams.base = vehicleData.mBaseParams;

        // Copie des param�tres DirectDrive (Couple moteur, Multiplicateurs)
        // Note: mDirectDriveParams remplace mEngineDriveParams ici
        defaultParams.engine = vehicleData.mDirectDriveParams;

        defaultParams.isSet = true;

        // Optionnel : un petit log pour confirmer le switch en DD
        // logger::info("ImGuiPanel: DirectDrive parameters snapshotted.");
    }
}

void ImGuiPanel::renderVehiclePhysx() {
    if (!vehicle) return;
    auto& current = vehicle->getVehicleData();

    if (ImGui::CollapsingHeader("Vehicle Physx Tuning - DIRECT DRIVE", ImGuiTreeNodeFlags_DefaultOpen)) {

        // --- ACTIONS ---
        if (ImGui::Button("Reset Defaults")) {
            current.mBaseParams = defaultParams.base;
            current.mDirectDriveParams = defaultParams.engine; // Stock� dans defaultParams.engine via setVehicle
        }
        ImGui::SameLine();
        if (ImGui::Button("Export to Console")) {
            printf("\n--- PHYSX DIRECT DRIVE EXPORT ---\nMax Torque: %f\nMass: %f\n",
                current.mDirectDriveParams.directDriveThrottleResponseParams.maxResponse,
                current.mBaseParams.rigidBodyParams.mass);
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset to Spawn")) {
            physx::PxTransform targetPose(physx::PxVec3(-730.0f, 670.4f, -400.0f), physx::PxQuat(physx::PxIdentity));
            auto* actor = vehicle->getRigidActor()->is<physx::PxRigidDynamic>();
            if (actor) {
                actor->setGlobalPose(targetPose);
                actor->setLinearVelocity(physx::PxVec3(0, 0, 0));
                actor->setAngularVelocity(physx::PxVec3(0, 0, 0));
                current.mBaseState.rigidBodyState.linearVelocity = physx::PxVec3(0, 0, 0);
                current.mBaseState.rigidBodyState.angularVelocity = physx::PxVec3(0, 0, 0);
            }
        }

        ImGui::Separator();

        // --- 1. DIRECT DRIVE MOTOR ---
        if (ImGui::TreeNodeEx("Direct Drive Engine", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& drive = current.mDirectDriveParams.directDriveThrottleResponseParams;

            ImGui::SliderFloat("Peak Drive Torque", &drive.maxResponse, 50.0f, 20000.0f, "%.0f Nm");

            ImGui::Text("Torque Distribution (Multipliers):");
            // Rappel : 0=FrontL, 1=FrontR, 2=RearL, 3=RearR
            ImGui::SliderFloat("Front Multiplier", &drive.wheelResponseMultipliers[0], 0.0f, 1.0f, "%.2f");
            drive.wheelResponseMultipliers[1] = drive.wheelResponseMultipliers[0]; // Sym�trie avant

            ImGui::SliderFloat("Rear Multiplier", &drive.wheelResponseMultipliers[2], 0.0f, 1.0f, "%.2f");
            drive.wheelResponseMultipliers[3] = drive.wheelResponseMultipliers[2]; // Sym�trie arri�re

            ImGui::TreePop();
        }

        // --- 2. BRAKE & STEERING ---
        if (ImGui::TreeNodeEx("Braking & Steering", ImGuiTreeNodeFlags_DefaultOpen)) {
            // Dans PhysX DirectDrive, on r�gle souvent le maxResponse de l'index 0 (Freinage)
            auto& brake = current.mBaseParams.brakeResponseParams[0];
            ImGui::SliderFloat("Max Brake Torque", &brake.maxResponse, 0.0f, 15000.0f, "%.0f Nm");

            float steerDeg = current.mBaseParams.steerResponseParams.maxResponse * (180.0f / 3.14159f);
            if (ImGui::SliderFloat("Max Steer Angle", &steerDeg, 5.0f, 90.0f, "%.1f deg")) {
                current.mBaseParams.steerResponseParams.maxResponse = steerDeg * (3.14159f / 180.0f);
            }
            ImGui::TreePop();
        }

        // --- 3. CHASSIS ---
        if (ImGui::TreeNodeEx("Chassis & Physics", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::SliderFloat("Mass", &current.mBaseParams.rigidBodyParams.mass, 10.0f, 10000.0f, "%.0f kg");
            ImGui::DragFloat3("Inertia (MOI)", &current.mBaseParams.rigidBodyParams.moi.x, 10.0f, 10.0f, 100000.0f, "%.0f");
            ImGui::TreePop();
        }

        // --- 4. TIRES & GRIP (SNOWMOBILE OPTIMIZED) ---
        if (ImGui::TreeNodeEx("Tires & Friction (Hyper Grip)", ImGuiTreeNodeFlags_DefaultOpen)) {
            // On peut ici impl�menter les boutons "Quick Set" dont on a parl� avant
            if (ImGui::Button("Set Pro Snowmobile Grip")) {
                // Avant (Skis)
                current.mBaseParams.tireForceParams[0].longStiff = 800000.0f;
                current.mBaseParams.tireForceParams[0].latStiffX = 3500000.0f;
                current.mBaseParams.tireForceParams[1].longStiff = 800000.0f;
                current.mBaseParams.tireForceParams[1].latStiffX = 3500000.0f;
                // Arri�re (Chenille)
                current.mBaseParams.tireForceParams[2].longStiff = 2500000.0f;
                current.mBaseParams.tireForceParams[2].latStiffX = 1800000.0f;
                current.mBaseParams.tireForceParams[3].longStiff = 2500000.0f;
                current.mBaseParams.tireForceParams[3].latStiffX = 1800000.0f;
            }

            for (int i = 0; i < 4; i++) {
                ImGui::PushID(i + 500);
                const char* wheelName = (i < 2) ? "Front (Ski)" : "Rear (Track)";
                if (ImGui::TreeNode("Individual Wheel", "%s %d", wheelName, i)) {
                    ImGui::SliderFloat("Long. Stiffness", &current.mBaseParams.tireForceParams[i].longStiff, 0.0f, 5000000.0f);
                    ImGui::SliderFloat("Lat. Stiffness", &current.mBaseParams.tireForceParams[i].latStiffX, 0.0f, 5000000.0f);
                    ImGui::TreePop();
                }
                ImGui::PopID();
            }
            ImGui::TreePop();
        }

        // --- 5. SUSPENSIONS ---
        if (ImGui::TreeNodeEx("Suspensions", ImGuiTreeNodeFlags_DefaultOpen)) {
            for (int i = 0; i < 4; i++) {
                ImGui::PushID(i + 100);
                const char* suspName = (i < 2) ? "Front" : "Rear";
                if (ImGui::TreeNode("Individual Susp", "%s %d", suspName, i)) {
                    auto& sForce = current.mBaseParams.suspensionForceParams[i];
                    ImGui::SliderFloat("Travel", &current.mBaseParams.suspensionParams[i].suspensionTravelDist, 0.01f, 1.0f);
                    ImGui::SliderFloat("Stiffness", &sForce.stiffness, 0.0f, 1000000.0f);
                    ImGui::SliderFloat("Damping", &sForce.damping, 0.0f, 100000.0f);
                    ImGui::SliderFloat("Sprung Mass", &sForce.sprungMass, 1.0f, 5000.0f);
                    ImGui::TreePop();
                }
                ImGui::PopID();
            }
            ImGui::TreePop();
        }

        // --- LIVE DASHBOARD ---
        ImGui::SeparatorText("Live Stats");
        float speedKmh = current.mBaseState.rigidBodyState.linearVelocity.magnitude() * 3.6f;
        ImGui::Text("Velocity: %.1f km/h", speedKmh);
        ImGui::ProgressBar(speedKmh / 200.0f, ImVec2(-1, 15), "");
        // Plus de RPM moteur ici, mais on peut afficher la rotation moyenne des roues
        float avgWheelOmega = (current.mBaseState.wheelRigidBody1dStates[2].rotationSpeed + current.mBaseState.wheelRigidBody1dStates[3].rotationSpeed) * 0.5f;
        ImGui::Text("Track Speed: %.1f rad/s", avgWheelOmega);
    }
}
