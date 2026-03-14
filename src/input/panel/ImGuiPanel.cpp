#include "ImGuiPanel.hpp"
#include <filesystem>
#include <iostream>
#include <set>
#include <vector>

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

void ImGuiPanel::setVehicle(VehicleFourWheelDrive* v) {
    if (!v) return;
    vehicle = v;

    if (!defaultParams.isSet) {
        defaultParams.base = vehicle->getVehicleData().mBaseParams;
        defaultParams.engine = vehicle->getVehicleData().mEngineDriveParams;
        defaultParams.isSet = true;
    }
}

std::vector<std::string> GetAvailableConfigs(const std::string& directory) {
    std::set<std::string> baseConfigs;
    std::set<std::string> engineConfigs;
    std::vector<std::string> validConfigs;

    if (!std::filesystem::exists(directory)) return validConfigs;

    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        std::string filename = entry.path().filename().string();

        // Look for file suffixes
        size_t basePos = filename.find("-Base.json");
        size_t enginePos = filename.find("-EngineDrive.json");

        if (basePos != std::string::npos) {
            baseConfigs.insert(filename.substr(0, basePos));
        }
        else if (enginePos != std::string::npos) {
            engineConfigs.insert(filename.substr(0, enginePos));
        }
    }

    // Only keep names that have BOTH files
    for (const auto& name : baseConfigs) {
        if (engineConfigs.find(name) != engineConfigs.end()) {
            validConfigs.push_back(name);
        }
    }
    return validConfigs;
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
        ImGui::SeparatorText("Configurations Management");
        // --- ACTIONS ---
        /*
        if (ImGui::Button("Reset Defaults")) {
            current.mBaseParams = defaultParams.base;
            current.mEngineDriveParams = defaultParams.engine;

            auto* actor = vehicle->getRigidActor()->is<physx::PxRigidDynamic>();
            if (actor) {
                actor->setMass(current.mBaseParams.rigidBodyParams.mass);
                actor->setMassSpaceInertiaTensor(current.mBaseParams.rigidBodyParams.moi);
                actor->wakeUp();
            }

            printf("Vehicle parameters reset to defaults and actor woken up.\n");
        }
        */
        if (ImGui::Button("Reset Default Parameters")) {
            const char* path = "../../../assets/vehicledata";
            // We assume your "original" files are named Base.json and EngineDrive.json
            // Or you can change these names to "Default-Base.json", etc.
            const char* defaultBase = "Base.json";
            const char* defaultEngine = "EngineDrive.json";

            bool l1 = readBaseParamsFromJsonFile(path, defaultBase, current.mBaseParams);
            bool l2 = readEngineDrivetrainParamsFromJsonFile(path, defaultEngine, current.mEngineDriveParams);

            if (l1 && l2) {
                auto* actor = vehicle->getRigidActor()->is<physx::PxRigidDynamic>();
                if (actor) {
                    // 1. Re-apply Physical Properties
                    actor->setMass(current.mBaseParams.rigidBodyParams.mass);
                    actor->setMassSpaceInertiaTensor(current.mBaseParams.rigidBodyParams.moi);

                    // 2. Clear velocities to prevent the vehicle from flying away if mass changed drastically
                    actor->setLinearVelocity(physx::PxVec3(0, 0, 0));
                    actor->setAngularVelocity(physx::PxVec3(0, 0, 0));

                    // 3. Force Wake Up
                    actor->wakeUp();
                }
                printf("Brute Force Reset: Reloaded %s and %s successfully.\n", defaultBase, defaultEngine);
            }
            else {
                printf("Brute Force Reset FAILED: Could not find %s or %s in %s\n", defaultBase, defaultEngine, path);
            }
        }
        ImGui::SameLine();

        if (ImGui::Button("Export Params to Console")) {
            printf("\n====================================================");
            printf("\n   VEHICLE PHYSICS EXPORT - CURRENT CONFIG        ");
            printf("\n====================================================\n");

            // 1. ENGINE & GEARBOX
            printf("[ENGINE & GEARBOX]\n");
            printf("  MaxOmega:     %.3f (Redline: %.0f RPM)\n", current.mEngineDriveParams.engineParams.maxOmega, current.mEngineDriveParams.engineParams.maxOmega * (60.0f / 6.28318f));
            printf("  PeakTorque:   %.1f Nm\n", current.mEngineDriveParams.engineParams.peakTorque);
            printf("  FinalRatio:   %.2f\n", current.mEngineDriveParams.gearBoxParams.finalRatio);
            printf("  SwitchTime:   %.3f s\n\n", current.mEngineDriveParams.gearBoxParams.switchTime);

            // 2. TRANSMISSION
            printf("[TRANSMISSION & DIFFS]\n");
            auto& diff = current.mEngineDriveParams.fourWheelDifferentialParams;
            printf("  Torque Split (F/R): %.2f / %.2f\n", diff.torqueRatios[0] * 2.0f, diff.torqueRatios[2] * 2.0f);
            printf("  Rear Bias (LSD):    %.2f\n", diff.rearBias);
            printf("  Center Bias:        %.2f\n", diff.centerBias);
            printf("  Diff Rate:          %.2f\n\n", diff.rate);

            // 3. BRAKE & STEERING
            printf("[BRAKING & STEERING]\n");
            printf("  Max Brake Torque:   %.1f Nm\n", current.mBaseParams.brakeResponseParams[0].maxResponse);
            printf("  Max Steer Angle:    %.2f rad (%.1f deg)\n\n", current.mBaseParams.steerResponseParams.maxResponse, current.mBaseParams.steerResponseParams.maxResponse * (180.0f / 3.14159f));

            // 4. CHASSIS
            printf("[CHASSIS & PHYSICS]\n");
            printf("  Mass:               %.1f kg\n", current.mBaseParams.rigidBodyParams.mass);
            printf("  Inertia (MOI):      X:%.1f Y:%.1f Z:%.1f\n", current.mBaseParams.rigidBodyParams.moi.x, current.mBaseParams.rigidBodyParams.moi.y, current.mBaseParams.rigidBodyParams.moi.z);
            auto* actor = vehicle->getRigidActor()->is<physx::PxRigidDynamic>();
            if (actor) {
                physx::PxVec3 p = actor->getCMassLocalPose().p;
                printf("  COM Offset:         X:%.2f Y:%.2f Z:%.2f\n\n", p.x, p.y, p.z);
            }

            // 5 & 6. INDIVIDUAL WHEELS (0-3)
            for (int i = 0; i < 4; i++) {
                printf("[WHEEL %d - %s]\n", i, (i < 2 ? "FRONT" : "REAR"));

                // Wheel Geometry
                printf("  Wheel Radius:       %.2f m\n", current.mBaseParams.wheelParams[i].radius);
                printf("  Wheel Mass:         %.1f kg\n", current.mBaseParams.wheelParams[i].mass);
                printf("  Wheel Damping:      %.2f\n", current.mBaseParams.wheelParams[i].dampingRate);

                // Tire Force Params
                auto& t = current.mBaseParams.tireForceParams[i];
                printf("  LongStiff:          %.0f\n", t.longStiff);
                printf("  LatStiff Peak:      %.0f\n", t.latStiffY);
                printf("  CamberStiff:        %.0f\n", t.camberStiff);
                printf("  Friction Points:    [0]:%.2f, [1]:(Slip:%.2f, Val:%.2f), [2]:(Slip:%.2f, Val:%.2f)\n",
                    t.frictionVsSlip[0][1],
                    t.frictionVsSlip[1][0], t.frictionVsSlip[1][1],
                    t.frictionVsSlip[2][0], t.frictionVsSlip[2][1]);
                printf("  Load Filter:        Min(L:%.2f, M:%.2f) Max(L:%.2f, M:%.2f)\n",
                    t.loadFilter[0][0], t.loadFilter[0][1],
                    t.loadFilter[1][0], t.loadFilter[1][1]);

                // Suspension Params
                printf("  Susp. Travel:       %.2f m\n", current.mBaseParams.suspensionParams[i].suspensionTravelDist);
                printf("  Spring Stiffness:   %.0f\n", current.mBaseParams.suspensionForceParams[i].stiffness);
                printf("  Spring Damping:     %.0f\n", current.mBaseParams.suspensionForceParams[i].damping);
                printf("  Sprung Mass:        %.1f kg\n\n", current.mBaseParams.suspensionForceParams[i].sprungMass);
            }
            printf("====================================================\n\n");
        }

        // --- FILE SYSTEM STORAGE ---
        static char configName[64] = "MySnowmobileConfig";
        const char* configDir = "../../../assets/vehicledata";

        // 1. Dropdown Menu (Combo Box)
        static std::vector<std::string> availableConfigs;
        static int selectedConfigIdx = -1;

        if (ImGui::BeginCombo("Existing Configs", (selectedConfigIdx == -1) ? "Select a config..." : availableConfigs[selectedConfigIdx].c_str())) {
            // Refresh list when opening the combo
            availableConfigs = GetAvailableConfigs(configDir);

            for (int n = 0; n < (int)availableConfigs.size(); n++) {
                const bool is_selected = (selectedConfigIdx == n);
                if (ImGui::Selectable(availableConfigs[n].c_str(), is_selected)) {
                    selectedConfigIdx = n;
                    // Auto-fill the text input with the selected name
                    strncpy(configName, availableConfigs[n].c_str(), sizeof(configName));
                }
                if (is_selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        // 2. Manual Name Input
        ImGui::SetNextItemWidth(300.0f);
        ImGui::InputText("Config Name", configName, IM_ARRAYSIZE(configName));
        HelpMarker("Name used for saving/loading. Matches files: 'Name-Base.json' & 'Name-EngineDrive.json'");

        // 3. Action Buttons
        if (ImGui::Button("Save JSON Config")) {
            // Ensure directory exists
            std::filesystem::create_directories(configDir);

            std::string baseFile = std::string(configName) + "-Base.json";
            std::string engineFile = std::string(configName) + "-EngineDrive.json";

            if (writeBaseParamsToJsonFile(configDir, baseFile.c_str(), current.mBaseParams) &&
                writeEngineDrivetrainParamsToJsonFile(configDir, engineFile.c_str(), current.mEngineDriveParams)) {
                printf("SUCCESS: Config '%s' saved to disk.\n", configName);
                selectedConfigIdx = -1; // Reset selection to force refresh
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Load JSON Config")) {
            std::string baseFile = std::string(configName) + "-Base.json";
            std::string engineFile = std::string(configName) + "-EngineDrive.json";

            if (readBaseParamsFromJsonFile(configDir, baseFile.c_str(), current.mBaseParams) &&
                readEngineDrivetrainParamsFromJsonFile(configDir, engineFile.c_str(), current.mEngineDriveParams)) {

                // PHYSICS RESET: Apply changes to the actor immediately
                auto* actor = vehicle->getRigidActor()->is<physx::PxRigidDynamic>();
                if (actor) {
                    actor->setMass(current.mBaseParams.rigidBodyParams.mass);
                    actor->setMassSpaceInertiaTensor(current.mBaseParams.rigidBodyParams.moi);
                    actor->wakeUp(); // Forces PhysX to recalculate with new values
                }
                printf("SUCCESS: Config '%s' loaded and applied to physics actor!\n", configName);
            }
            else {
                printf("ERROR: Files for '%s' were not found in %s\n", configName, configDir);
            }
        }

        ImGui::SeparatorText("Reset Vehicle Position");

        if (ImGui::Button("To Mountain Top")) {
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
        ImGui::SameLine();
        if (ImGui::Button("To Origin")) {
            physx::PxTransform targetPose(physx::PxVec3(0.0f, 10.0f, 0.0f), physx::PxQuat(physx::PxIdentity));
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

            ImGui::Separator();

            auto* actor = vehicle->getRigidActor()->is<physx::PxRigidDynamic>();
            if (actor) {
                physx::PxVec3 comOffset = actor->getCMassLocalPose().p;

                if (ImGui::DragFloat3("COM Offset", &comOffset.x, 0.01f, -5.0f, 5.0f, "%.2f m")) {
                    physx::PxTransform comPose = actor->getCMassLocalPose();
                    comPose.p = comOffset;
                    actor->setCMassLocalPose(comPose);
                }

                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Lower Y (middle) to avoid car turning on itself.");
                }
            }
            ImGui::TreePop();
        }

        // Indices et Helpers
        std::vector<int> frontIndices = { 0, 1 };
        std::vector<int> rearIndices = { 2, 3 };
        auto ApplyToGroup = [&](const std::vector<int>& indices, auto modifier) {
            for (int i : indices) modifier(i);
            };

        // --- FRONT TUNING (Axle 0) ---
        if (ImGui::TreeNodeEx("FRONT TUNING (Wheels & Suspension)", ImGuiTreeNodeFlags_DefaultOpen)) {

            // Wheels & Tires
            ImGui::SeparatorText("Front Wheels & Tires");
            if (ImGui::SliderFloat("Front Radius", &current.mBaseParams.wheelParams[0].radius, 0.1f, 1.0f, "%.2f m")) {
                ApplyToGroup(frontIndices, [&](int i) { current.mBaseParams.wheelParams[i].radius = current.mBaseParams.wheelParams[0].radius; });
            }
            if (ImGui::SliderFloat("Front Wheel Mass", &current.mBaseParams.wheelParams[0].mass, 1.0f, 100.0f, "%.1f kg")) {
                ApplyToGroup(frontIndices, [&](int i) { current.mBaseParams.wheelParams[i].mass = current.mBaseParams.wheelParams[0].mass; });
            }
            if (ImGui::SliderFloat("Front Rotation Damping", &current.mBaseParams.wheelParams[0].dampingRate, 0.0f, 5.0f, "%.2f")) {
                ApplyToGroup(frontIndices, [&](int i) { current.mBaseParams.wheelParams[i].dampingRate = current.mBaseParams.wheelParams[0].dampingRate; });
            }

            ImGui::Spacing();
            ImGui::SliderFloat("Front LongStiff", &current.mBaseParams.tireForceParams[0].longStiff, 0.0f, 5000000.0f, "%.0f");
            ImGui::SliderFloat("Front LatStiff Peak", &current.mBaseParams.tireForceParams[0].latStiffY, 0.0f, 5000000.0f, "%.0f");
            ImGui::SliderFloat("Front CamberStiff", &current.mBaseParams.tireForceParams[0].camberStiff, 0.0f, 500000.0f, "%.0f");

            ImGui::SeparatorText("Front Grip & Load");
            ImGui::SliderFloat("Front Base Friction", &current.mBaseParams.tireForceParams[0].frictionVsSlip[0][1], 0.0f, 2.0f, "%.2f");
            ImGui::DragFloat2("Front Max Friction Point", current.mBaseParams.tireForceParams[0].frictionVsSlip[1], 0.01f, 0.0f, 2.0f);
            ImGui::DragFloat2("Front Max Load Filter", current.mBaseParams.tireForceParams[0].loadFilter[1], 0.01f, 0.0f, 5.0f);

            // Sync Tire Params Front
            ApplyToGroup(frontIndices, [&](int i) {
                current.mBaseParams.tireForceParams[i].longStiff = current.mBaseParams.tireForceParams[0].longStiff;
                current.mBaseParams.tireForceParams[i].latStiffY = current.mBaseParams.tireForceParams[0].latStiffY;
                current.mBaseParams.tireForceParams[i].camberStiff = current.mBaseParams.tireForceParams[0].camberStiff;
                current.mBaseParams.tireForceParams[i].frictionVsSlip[0][1] = current.mBaseParams.tireForceParams[0].frictionVsSlip[0][1];
                current.mBaseParams.tireForceParams[i].frictionVsSlip[1][0] = current.mBaseParams.tireForceParams[0].frictionVsSlip[1][0];
                current.mBaseParams.tireForceParams[i].frictionVsSlip[1][1] = current.mBaseParams.tireForceParams[0].frictionVsSlip[1][1];
                current.mBaseParams.tireForceParams[i].loadFilter[1][0] = current.mBaseParams.tireForceParams[0].loadFilter[1][0];
                current.mBaseParams.tireForceParams[i].loadFilter[1][1] = current.mBaseParams.tireForceParams[0].loadFilter[1][1];
                });

            // Suspension
            ImGui::SeparatorText("Front Suspension");
            ImGui::SliderFloat("Front Travel", &current.mBaseParams.suspensionParams[0].suspensionTravelDist, 0.01f, 1.5f, "%.2f m");
            ImGui::SliderFloat("Front Stiffness", &current.mBaseParams.suspensionForceParams[0].stiffness, 0.0f, 1000000.0f);
            ImGui::SliderFloat("Front Damping", &current.mBaseParams.suspensionForceParams[0].damping, 0.0f, 100000.0f);
            ImGui::SliderFloat("Front Sprung Mass", &current.mBaseParams.suspensionForceParams[0].sprungMass, 1.0f, 5000.0f);

            ApplyToGroup(frontIndices, [&](int i) {
                current.mBaseParams.suspensionParams[i].suspensionTravelDist = current.mBaseParams.suspensionParams[0].suspensionTravelDist;
                current.mBaseParams.suspensionForceParams[i].stiffness = current.mBaseParams.suspensionForceParams[0].stiffness;
                current.mBaseParams.suspensionForceParams[i].damping = current.mBaseParams.suspensionForceParams[0].damping;
                current.mBaseParams.suspensionForceParams[i].sprungMass = current.mBaseParams.suspensionForceParams[0].sprungMass;
                });
            ImGui::TreePop();
        }

        ImGui::Spacing();

        // --- REAR TUNING (Axle 1) ---
        if (ImGui::TreeNodeEx("REAR TUNING (Wheels & Suspension)", ImGuiTreeNodeFlags_DefaultOpen)) {

            // Wheels & Tires
            ImGui::SeparatorText("Rear Wheels & Tires");
            if (ImGui::SliderFloat("Rear Radius", &current.mBaseParams.wheelParams[2].radius, 0.1f, 1.0f, "%.2f m")) {
                ApplyToGroup(rearIndices, [&](int i) { current.mBaseParams.wheelParams[i].radius = current.mBaseParams.wheelParams[2].radius; });
            }
            if (ImGui::SliderFloat("Rear Wheel Mass", &current.mBaseParams.wheelParams[2].mass, 1.0f, 100.0f, "%.1f kg")) {
                ApplyToGroup(rearIndices, [&](int i) { current.mBaseParams.wheelParams[i].mass = current.mBaseParams.wheelParams[2].mass; });
            }
            if (ImGui::SliderFloat("Rear Rotation Damping", &current.mBaseParams.wheelParams[2].dampingRate, 0.0f, 5.0f, "%.2f")) {
                ApplyToGroup(rearIndices, [&](int i) { current.mBaseParams.wheelParams[i].dampingRate = current.mBaseParams.wheelParams[2].dampingRate; });
            }

            ImGui::Spacing();
            ImGui::SliderFloat("Rear LongStiff", &current.mBaseParams.tireForceParams[2].longStiff, 0.0f, 5000000.0f, "%.0f");
            ImGui::SliderFloat("Rear LatStiff Peak", &current.mBaseParams.tireForceParams[2].latStiffY, 0.0f, 5000000.0f, "%.0f");
            ImGui::SliderFloat("Rear CamberStiff", &current.mBaseParams.tireForceParams[2].camberStiff, 0.0f, 500000.0f, "%.0f");

            ImGui::SeparatorText("Rear Grip & Load");
            ImGui::SliderFloat("Rear Base Friction", &current.mBaseParams.tireForceParams[2].frictionVsSlip[0][1], 0.0f, 2.0f, "%.2f");
            ImGui::DragFloat2("Rear Max Friction Point", current.mBaseParams.tireForceParams[2].frictionVsSlip[1], 0.01f, 0.0f, 2.0f);
            ImGui::DragFloat2("Rear Max Load Filter", current.mBaseParams.tireForceParams[2].loadFilter[1], 0.01f, 0.0f, 5.0f);

            // Sync Tire Params Rear
            ApplyToGroup(rearIndices, [&](int i) {
                current.mBaseParams.tireForceParams[i].longStiff = current.mBaseParams.tireForceParams[2].longStiff;
                current.mBaseParams.tireForceParams[i].latStiffY = current.mBaseParams.tireForceParams[2].latStiffY;
                current.mBaseParams.tireForceParams[i].camberStiff = current.mBaseParams.tireForceParams[2].camberStiff;
                current.mBaseParams.tireForceParams[i].frictionVsSlip[0][1] = current.mBaseParams.tireForceParams[2].frictionVsSlip[0][1];
                current.mBaseParams.tireForceParams[i].frictionVsSlip[1][0] = current.mBaseParams.tireForceParams[2].frictionVsSlip[1][0];
                current.mBaseParams.tireForceParams[i].frictionVsSlip[1][1] = current.mBaseParams.tireForceParams[2].frictionVsSlip[1][1];
                current.mBaseParams.tireForceParams[i].loadFilter[1][0] = current.mBaseParams.tireForceParams[2].loadFilter[1][0];
                current.mBaseParams.tireForceParams[i].loadFilter[1][1] = current.mBaseParams.tireForceParams[2].loadFilter[1][1];
                });

            // Suspension
            ImGui::SeparatorText("Rear Suspension");
            ImGui::SliderFloat("Rear Travel", &current.mBaseParams.suspensionParams[2].suspensionTravelDist, 0.01f, 1.5f, "%.2f m");
            ImGui::SliderFloat("Rear Stiffness", &current.mBaseParams.suspensionForceParams[2].stiffness, 0.0f, 1000000.0f);
            ImGui::SliderFloat("Rear Damping", &current.mBaseParams.suspensionForceParams[2].damping, 0.0f, 100000.0f);
            ImGui::SliderFloat("Rear Sprung Mass", &current.mBaseParams.suspensionForceParams[2].sprungMass, 1.0f, 5000.0f);

            ApplyToGroup(rearIndices, [&](int i) {
                current.mBaseParams.suspensionParams[i].suspensionTravelDist = current.mBaseParams.suspensionParams[2].suspensionTravelDist;
                current.mBaseParams.suspensionForceParams[i].stiffness = current.mBaseParams.suspensionForceParams[2].stiffness;
                current.mBaseParams.suspensionForceParams[i].damping = current.mBaseParams.suspensionForceParams[2].damping;
                current.mBaseParams.suspensionForceParams[i].sprungMass = current.mBaseParams.suspensionForceParams[2].sprungMass;
                });
            ImGui::TreePop();
        }

        // --- LIVE DASHBOARD ---
        ImGui::SeparatorText("Live Stats");
        float speedKmh = current.mBaseState.rigidBodyState.linearVelocity.magnitude() * 3.6f;
        ImGui::Text("Velocity: %.1f km/h", speedKmh);
        ImGui::ProgressBar(speedKmh / 300.0f, ImVec2(-1, 15), "");
        ImGui::Text("RPM: %.0f", current.mEngineDriveState.engineState.rotationSpeed * (60.0f / 6.28318f));

        float rpm = current.mEngineDriveState.engineState.rotationSpeed * (60.0f / 6.28318f);
        float redline = current.mEngineDriveParams.engineParams.maxOmega * (60.0f / 6.28318f);
        float rpmRatio = rpm / redline;

        // Changement de couleur dynamique
        ImVec4 color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // Vert
        if (rpmRatio > 0.7f) color = ImVec4(1.0f, 0.6f, 0.0f, 1.0f); // Orange à 70%
        if (rpmRatio > 0.9f) color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Rouge à 90%

        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);
        ImGui::ProgressBar(rpmRatio, ImVec2(-1, 25), "ENGINE RPM");
        ImGui::PopStyleColor();
    }
}

/* Old individual wheels and suspensions:
        // --- 7. WHEEL GEOMETRY ---
        if (ImGui::TreeNodeEx("Wheel Geometry (Alignment)", ImGuiTreeNodeFlags_DefaultOpen)) {
            for (int i = 0; i < 4; i++) {
                ImGui::PushID(i + 800);
                // À mettre dans ta boucle "Individual Wheel"
                if (ImGui::TreeNode("Individual Wheel", "Wheel %d", i)) {
                    auto& w = current.mBaseParams.wheelParams[i];
                    ImGui::SliderFloat("Radius", &w.radius, 0.1f, 1.0f, "%.2f m");
                    ImGui::SliderFloat("Wheel Mass", &w.mass, 1.0f, 100.0f, "%.1f kg");
                    ImGui::SliderFloat("Wheel MOI", &w.moi, 0.1f, 20.0f, "%.2f");
                    ImGui::SliderFloat("Damping Rate", &w.dampingRate, 0.0f, 5.0f, "%.2f");
                    ImGui::TreePop();
                }
                ImGui::PopID();
            }
            ImGui::TreePop();
        }

        // --- 5. TIRES & GRIP ---
        if (ImGui::TreeNodeEx("Tires & Friction (Hyper Grip)", ImGuiTreeNodeFlags_DefaultOpen)) {

            // Helper pour appliquer des paramètres à un groupe de roues
            auto ApplyToWheels = [&](const std::vector<int>& indices, auto modifier) {
                for (int i : indices) modifier(current.mBaseParams.tireForceParams[i]);
                };

            std::vector<int> allIndices = { 0, 1, 2, 3 };
            std::vector<int> frontIndices = { 0, 1 };
            std::vector<int> rearIndices = { 2, 3 };

            auto RenderGroupTuning = [&](const char* label, const std::vector<int>& indices) {
                if (ImGui::TreeNode(label)) {
                    static float gLong = 1500000.0f;
                    static float gLatY = 2000000.0f;
                    static float gCamber = 50000.0f;
                    static float gFricZero = 1.0f;

                    if (ImGui::SliderFloat("LongStiff", &gLong, 0.0f, 5000000.0f))
                        ApplyToWheels(indices, [&](auto& t) { t.longStiff = gLong; });

                    if (ImGui::SliderFloat("LatStiff Peak", &gLatY, 0.0f, 5000000.0f))
                        ApplyToWheels(indices, [&](auto& t) { t.latStiffY = gLatY; });

                    if (ImGui::SliderFloat("CamberStiff", &gCamber, 0.0f, 500000.0f))
                        ApplyToWheels(indices, [&](auto& t) { t.camberStiff = gCamber; });

                    if (ImGui::SliderFloat("Zero Slip Fric", &gFricZero, 0.0f, 2.0f))
                        ApplyToWheels(indices, [&](auto& t) { t.frictionVsSlip[0][1] = gFricZero; });

                    ImGui::TreePop();
                }
                };

            // Sections de groupe
            RenderGroupTuning("ALL WHEELS", allIndices);
            RenderGroupTuning("FRONT WHEELS", frontIndices);
            RenderGroupTuning("REAR WHEELS", rearIndices);

            ImGui::Separator();

            // --- INDIVIDUAL WHEELS ---
            for (int i = 0; i < 4; i++) {
                ImGui::PushID(i + 500);
                if (ImGui::TreeNode("Individual Wheel", "Wheel %d", i)) {
                    auto& tire = current.mBaseParams.tireForceParams[i];

                    ImGui::SliderFloat("Longitudinal Stiffness", &tire.longStiff, 0.0f, 5000000.0f, "%.0f");
                    ImGui::SliderFloat("Lateral Stiffness (Peak)", &tire.latStiffY, 0.0f, 5000000.0f, "%.0f");
                    ImGui::SliderFloat("Camber Stiffness", &tire.camberStiff, 0.0f, 500000.0f, "%.0f");

                    ImGui::SeparatorText("Friction vs Slip Graph");
                    ImGui::SliderFloat("Zero Slip Friction", &tire.frictionVsSlip[0][1], 0.0f, 2.0f, "%.2f");
                    ImGui::DragFloat2("Max Friction Point (Slip, Val)", tire.frictionVsSlip[1], 0.01f, 0.0f, 2.0f);
                    ImGui::DragFloat2("Late Slip Point (Slip, Val)", tire.frictionVsSlip[2], 0.01f, 0.0f, 2.0f);

                    ImGui::SeparatorText("Load Filter");
                    ImGui::DragFloat2("Min Load Filter", tire.loadFilter[0], 0.01f, 0.0f, 5.0f);
                    ImGui::DragFloat2("Max Load Filter", tire.loadFilter[1], 0.01f, 0.0f, 5.0f);

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
        */
