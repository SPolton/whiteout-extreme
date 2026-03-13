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

void ImGuiPanel::setVehicle(VehicleFourWheelDrive* v) {
    if (!v) return;
    vehicle = v;

    if (!defaultParams.isSet) {
        defaultParams.base = vehicle->getVehicleData().mBaseParams;
        defaultParams.engine = vehicle->getVehicleData().mEngineDriveParams;
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
                    ImGui::SetTooltip("Abaisser le Y (milieu) pour éviter que la voiture ne se retourne.");
                }
            }
            ImGui::TreePop();
        }

        std::vector<int> frontIndices = { 0, 1 };
        std::vector<int> rearIndices = { 2, 3 };

        auto ApplyToGroup = [&](const std::vector<int>& indices, auto modifier) {
            for (int i : indices) modifier(i);
            };

        // --- FRONT TUNING (Axle 0) ---
        if (ImGui::TreeNodeEx("FRONT TUNING (Wheels & Suspension)", ImGuiTreeNodeFlags_DefaultOpen)) {

            // --- Front Wheels & Tires ---
            ImGui::SeparatorText("Front Wheels & Tires");
            static float fRadius = current.mBaseParams.wheelParams[0].radius;
            if (ImGui::SliderFloat("Front Radius", &fRadius, 0.1f, 1.0f, "%.2f m")) {
                ApplyToGroup(frontIndices, [&](int i) { current.mBaseParams.wheelParams[i].radius = fRadius; });
            }

            static float fWheelMass = current.mBaseParams.wheelParams[0].mass;
            if (ImGui::SliderFloat("Front Wheel Mass", &fWheelMass, 1.0f, 100.0f, "%.1f kg")) {
                ApplyToGroup(frontIndices, [&](int i) { current.mBaseParams.wheelParams[i].mass = fWheelMass; });
            }

            ImGui::SliderFloat("Front LongStiff", &current.mBaseParams.tireForceParams[0].longStiff, 0.0f, 5000000.0f, "%.0f");
            ImGui::SliderFloat("Front LatStiff Peak", &current.mBaseParams.tireForceParams[0].latStiffY, 0.0f, 5000000.0f, "%.0f");
            ImGui::SliderFloat("Front CamberStiff", &current.mBaseParams.tireForceParams[0].camberStiff, 0.0f, 500000.0f, "%.0f");

            ApplyToGroup(frontIndices, [&](int i) {
                current.mBaseParams.tireForceParams[i].longStiff = current.mBaseParams.tireForceParams[0].longStiff;
                current.mBaseParams.tireForceParams[i].latStiffY = current.mBaseParams.tireForceParams[0].latStiffY;
                current.mBaseParams.tireForceParams[i].camberStiff = current.mBaseParams.tireForceParams[0].camberStiff;
                });

            ImGui::DragFloat2("Front Max Friction Point", current.mBaseParams.tireForceParams[0].frictionVsSlip[1], 0.01f, 0.0f, 2.0f);
            ApplyToGroup(frontIndices, [&](int i) {
                current.mBaseParams.tireForceParams[i].frictionVsSlip[1][0] = current.mBaseParams.tireForceParams[0].frictionVsSlip[1][0];
                current.mBaseParams.tireForceParams[i].frictionVsSlip[1][1] = current.mBaseParams.tireForceParams[0].frictionVsSlip[1][1];
                });

            // --- Front Suspension ---
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

            // --- Rear Wheels & Tires ---
            ImGui::SeparatorText("Rear Wheels & Tires");
            static float rRadius = current.mBaseParams.wheelParams[2].radius;
            if (ImGui::SliderFloat("Rear Radius", &rRadius, 0.1f, 1.0f, "%.2f m")) {
                ApplyToGroup(rearIndices, [&](int i) { current.mBaseParams.wheelParams[i].radius = rRadius; });
            }

            static float rWheelMass = current.mBaseParams.wheelParams[2].mass;
            if (ImGui::SliderFloat("Rear Wheel Mass", &rWheelMass, 1.0f, 100.0f, "%.1f kg")) {
                ApplyToGroup(rearIndices, [&](int i) { current.mBaseParams.wheelParams[i].mass = rWheelMass; });
            }

            ImGui::SliderFloat("Rear LongStiff", &current.mBaseParams.tireForceParams[2].longStiff, 0.0f, 5000000.0f, "%.0f");
            ImGui::SliderFloat("Rear LatStiff Peak", &current.mBaseParams.tireForceParams[2].latStiffY, 0.0f, 5000000.0f, "%.0f");
            ImGui::SliderFloat("Rear CamberStiff", &current.mBaseParams.tireForceParams[2].camberStiff, 0.0f, 500000.0f, "%.0f");

            ApplyToGroup(rearIndices, [&](int i) {
                current.mBaseParams.tireForceParams[i].longStiff = current.mBaseParams.tireForceParams[2].longStiff;
                current.mBaseParams.tireForceParams[i].latStiffY = current.mBaseParams.tireForceParams[2].latStiffY;
                current.mBaseParams.tireForceParams[i].camberStiff = current.mBaseParams.tireForceParams[2].camberStiff;
                });

            ImGui::DragFloat2("Rear Max Friction Point", current.mBaseParams.tireForceParams[2].frictionVsSlip[1], 0.01f, 0.0f, 2.0f);
            ApplyToGroup(rearIndices, [&](int i) {
                current.mBaseParams.tireForceParams[i].frictionVsSlip[1][0] = current.mBaseParams.tireForceParams[2].frictionVsSlip[1][0];
                current.mBaseParams.tireForceParams[i].frictionVsSlip[1][1] = current.mBaseParams.tireForceParams[2].frictionVsSlip[1][1];
                });

            // --- Rear Suspension ---
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
