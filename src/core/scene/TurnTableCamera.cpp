#include "TurnTableCamera.hpp"

#include "utils/logger.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/scalar_constants.hpp>

TurnTableCamera::TurnTableCamera()
    : TurnTableCamera(Params{})
{
}

TurnTableCamera::TurnTableCamera(Params const& params)
    : TurnTableCamera(mFallbackTarget, params)
{
}

TurnTableCamera::TurnTableCamera(SceneTransform& target)
    : TurnTableCamera(target, Params{})
{
}

TurnTableCamera::TurnTableCamera(SceneTransform& target, Params const& params)
    : BaseCamera()
{
    mTarget = &target;

    mDistance = params.defaultDistance;
    mMinDistance = params.minDistance;
    mMaxDistance = params.maxDistance;
}

void TurnTableCamera::setTarget(SceneTransform& target)
{
    mTarget = &target;
    mIsDirty = true;
}

glm::mat4 TurnTableCamera::getViewMatrix()
{
    updateViewMatrix();
    return mViewMatrix;
}

glm::vec3 TurnTableCamera::getPosition()
{
    updateViewMatrix();
    return mPosition;
}

void TurnTableCamera::updateViewMatrix()
{
    if (mIsDirty || mTargetPosition != mTarget->getWorldPosition())
    {
        mIsDirty = false;

        auto const hRot = glm::rotate(glm::mat4(1.0f), mTheta, mUp);
        auto const vRot = glm::rotate(glm::mat4(1.0f), mPhi, math::transform::RightVec3);

        mPosition = glm::vec3(hRot * vRot * glm::vec4{math::transform::ForwardVec3, 0.0f}) * mDistance;

        // Center the camera around the target
        mTargetPosition = mTarget->getWorldPosition();
        mPosition += mTargetPosition;

        mViewMatrix = glm::lookAt(mPosition, mTarget->getWorldPosition(), math::transform::UpVec3);
    }
}

void TurnTableCamera::adjustTheta(float const deltaTheta)
{
    auto newTheta = mTheta + deltaTheta;
    // auto newTheta = std::fmod(mTheta + deltaTheta, 2*M_PI);
    if (newTheta != mTheta)
    {
        mTheta = newTheta;
        mIsDirty = true;
    }
}

void TurnTableCamera::adjustPhi(float const deltaPhi)
{
    float const newPhi = std::clamp(
        mPhi + deltaPhi,
        -glm::pi<float>() * 0.49f,
        glm::pi<float>() * 0.49f
    );
    if (newPhi != mPhi)
    {
        mIsDirty = true;
        mPhi = newPhi;
    }
}

void TurnTableCamera::adjustRadius(float const deltaRadius)
{
    float const newDistance = std::clamp(mDistance + deltaRadius, mMinDistance, mMaxDistance);
    if (newDistance != mDistance)
    {
        mIsDirty = true;
        mDistance = newDistance;
    }
}


std::string TurnTableCamera::toString() const
{
    return fmt::format(
        "Orbit Camera\n"
        "Pos: ({:.1f}, {:.1f}, {:.1f})\n"
        "Target: ({:.1f}, {:.1f}, {:.1f})\n"
        "Dist: {:.2f}  FOV: {:.1f}\n"
        "Yaw: {:.1f}  Pitch: {:.1f}",
        mPosition.x, mPosition.y, mPosition.z,
        mTargetPosition.x, mTargetPosition.y, mTargetPosition.z,
        mDistance, glm::degrees(mFov),
        glm::degrees(mTheta), glm::degrees(mPhi)
    );
}
