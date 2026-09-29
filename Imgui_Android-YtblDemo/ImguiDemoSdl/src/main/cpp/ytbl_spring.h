/**
 * 来自·默檐 -  2026 默檐
 * 2026/9/8 14:10
 *
 * 免责声明:本项目仅供学习、交流与合法用途,开发者本意不在于造成任何不良影响。
 * 使用者应遵守适用法律法规,并自行承担因不当使用所产生的责任。
 */
#pragma once

#include "ytbl_config.h"
#include <math.h>

namespace ytbl {

struct SpringFloat {
    float value;
    float velocity;
    float target;
    float dampingRatio;
    float stiffness;
    float threshold;

    SpringFloat(float v, float damping, float stiff, float thresh)
        : value(v), velocity(0.0f), target(v),
          dampingRatio(damping), stiffness(stiff),
          threshold(thresh > 0.000001f ? thresh : 0.000001f) {}

    void SnapTo(float v) { value = v; target = v; velocity = 0.0f; }
    void AnimateTo(float v) { target = v; }

    bool Step(float dt) {
        if (dt <= 0.0f) return false;
        if (dt > 0.064f) dt = 0.064f;

        double omega0 = sqrt(stiffness > 0.000001f ? stiffness : 0.000001f);
        double zeta = dampingRatio;
        double y0 = value - target;
        double v0 = velocity;
        double y, v;

        if (zeta < 1.0 - 1.0e-4) {
            double omegaD = omega0 * sqrt(1.0 - zeta * zeta);
            double envelope = exp(-zeta * omega0 * dt);
            double c = cos(omegaD * dt);
            double s = sin(omegaD * dt);
            double b = (v0 + zeta * omega0 * y0) / omegaD;
            double inner = y0 * c + b * s;
            y = envelope * inner;
            v = envelope * (-zeta * omega0 * inner + (-y0 * omegaD * s + b * omegaD * c));
        } else if (zeta > 1.0 + 1.0e-4) {
            double s = sqrt(zeta * zeta - 1.0);
            double r1 = -omega0 * (zeta - s);
            double r2 = -omega0 * (zeta + s);
            double c1 = (v0 - r2 * y0) / (r1 - r2);
            double c2 = y0 - c1;
            double e1 = exp(r1 * dt);
            double e2 = exp(r2 * dt);
            y = c1 * e1 + c2 * e2;
            v = r1 * c1 * e1 + r2 * c2 * e2;
        } else {
            double envelope = exp(-omega0 * dt);
            double c = v0 + omega0 * y0;
            y = envelope * (y0 + c * dt);
            v = envelope * (v0 - omega0 * c * dt);
        }

        value = target + (float)y;
        velocity = (float)v;

        if (!isfinite(value) || !isfinite(velocity)) {
            value = isfinite(target) ? target : 0.0f;
            if (!isfinite(target)) target = 0.0f;
            velocity = 0.0f;
        }

        if (fabsf(value - target) <= threshold
                && fabsf(velocity) <= threshold * 62.5f) {
            value = target;
            velocity = 0.0f;
        }
        return value != target || velocity != 0.0f;
    }
};

class DampedDragAnimation {
public:
    DampedDragAnimation(float initialValue, float rangeStart, float rangeEnd,
                        float visibilityThreshold, float initialScale, float pressedScale,
                        float valueStiffness = 1000.0f, float valueDamping = 1.0f)
        : initialValue_(initialValue), rangeStart_(rangeStart), rangeEnd_(rangeEnd),
          visibilityThreshold_(visibilityThreshold), initialScale_(initialScale),
          pressedScale_(pressedScale) {
        valueSpring_ = new SpringFloat(initialValue, valueDamping, valueStiffness, visibilityThreshold);
        velocitySpring_ = new SpringFloat(0.0f, 0.5f, 300.0f, visibilityThreshold * 10.0f);
        pressProgressSpring_ = new SpringFloat(0.0f, 1.0f, 1000.0f, 0.001f);
        scaleXSpring_ = new SpringFloat(initialScale, 0.6f, 250.0f, 0.001f);
        scaleYSpring_ = new SpringFloat(initialScale, 0.7f, 250.0f, 0.001f);
        lastObservedValue_ = initialValue;
    }

    ~DampedDragAnimation() {
        delete valueSpring_; delete velocitySpring_; delete pressProgressSpring_;
        delete scaleXSpring_; delete scaleYSpring_;
    }

    float GetValue() const { return valueSpring_->value; }
    float GetTargetValue() const { return valueSpring_->target; }
    void SetRange(float start, float end) {
        rangeStart_ = start;
        rangeEnd_ = end;
        if (valueSpring_->target < start) valueSpring_->target = start;
        if (valueSpring_->target > end) valueSpring_->target = end;
        if (valueSpring_->value < start) { valueSpring_->value = start; lastObservedValue_ = start; }
        if (valueSpring_->value > end) { valueSpring_->value = end; lastObservedValue_ = end; }
    }
    float GetProgress() const {
        float d = rangeEnd_ - rangeStart_;
        return d == 0.0f ? 0.0f : (valueSpring_->value - rangeStart_) / d;
    }
    float GetPressProgress() const { return Clamp01(pressProgressSpring_->value); }
    float GetScaleX() const { return scaleXSpring_->value; }
    float GetScaleY() const { return scaleYSpring_->value; }
    float GetVelocity() const { return velocitySpring_->value; }
    bool IsAnimating() const { return posted_ || releasePending_; }

    void Press() {
        releasePending_ = false;
        releaseWaitOneFrame_ = false;
        lastObservedValue_ = valueSpring_->value;
        pressProgressSpring_->AnimateTo(1.0f);
        scaleXSpring_->AnimateTo(pressedScale_);
        scaleYSpring_->AnimateTo(pressedScale_);
        posted_ = true;
    }

    void Release() {
        releasePending_ = true;
        releaseWaitOneFrame_ = true;
        posted_ = true;
    }

    void UpdateValue(float value) {
        trackValueVelocity_ = true;
        valueSpring_->AnimateTo(ClampRange(value));
        posted_ = true;
    }

    void SnapValue(float value) {
        trackValueVelocity_ = true;
        valueSpring_->SnapTo(ClampRange(value));
        UpdateMeasuredVelocity();
    }

    void AnimateToValue(float value) {
        Press();
        SettleToValue(value);
        Release();
        posted_ = true;
    }

    void SettleToValue(float value) {
        trackValueVelocity_ = false;
        valueSpring_->AnimateTo(ClampRange(value));
        if (velocitySpring_->value != 0.0f || velocitySpring_->target != 0.0f) {
            velocitySpring_->AnimateTo(0.0f);
        }
        posted_ = true;
    }

    void ReleasePressNow() {
        releasePending_ = false;
        releaseWaitOneFrame_ = false;
        pressProgressSpring_->AnimateTo(0.0f);
        scaleXSpring_->AnimateTo(initialScale_);
        scaleYSpring_->AnimateTo(initialScale_);
        posted_ = true;
    }

    bool Step(float dt) {
        lastStepDt_ = dt > 0.0001f ? dt : 1.0f / 60.0f;
        bool active = false;
        active |= valueSpring_->Step(dt);
        if (trackValueVelocity_) UpdateMeasuredVelocity();
        active |= velocitySpring_->Step(dt);

        if (releasePending_) {
            if (releaseWaitOneFrame_) {
                releaseWaitOneFrame_ = false;
            } else {
                float threshold = fabsf(rangeEnd_ - rangeStart_) * 0.025f;
                if (valueSpring_->value == valueSpring_->target
                        || fabsf(valueSpring_->value - valueSpring_->target) < threshold) {
                    pressProgressSpring_->AnimateTo(0.0f);
                    scaleXSpring_->AnimateTo(initialScale_);
                    scaleYSpring_->AnimateTo(initialScale_);
                    releasePending_ = false;
                }
            }
        }

        active |= pressProgressSpring_->Step(dt);
        active |= scaleXSpring_->Step(dt);
        active |= scaleYSpring_->Step(dt);
        if (releasePending_) active = true;

        const float safeRangeStart = isfinite(rangeStart_) ? rangeStart_ : 0.0f;
        const float safeTarget = isfinite(valueSpring_->target) ? valueSpring_->target : safeRangeStart;
        if (!isfinite(valueSpring_->value)) valueSpring_->value = safeTarget;
        if (!isfinite(valueSpring_->velocity)) valueSpring_->velocity = 0.0f;
        if (!isfinite(valueSpring_->target)) valueSpring_->target = safeRangeStart;
        if (!isfinite(velocitySpring_->value)) velocitySpring_->value = 0.0f;
        if (!isfinite(velocitySpring_->target)) velocitySpring_->target = 0.0f;
        if (!isfinite(pressProgressSpring_->value)) pressProgressSpring_->value = 0.0f;
        if (!isfinite(scaleXSpring_->value)) scaleXSpring_->value = initialScale_;
        if (!isfinite(scaleYSpring_->value)) scaleYSpring_->value = initialScale_;

        posted_ = active || releasePending_;
        return active;
    }

private:
    float ClampRange(float value) const {
        return value < rangeStart_ ? rangeStart_ : (value > rangeEnd_ ? rangeEnd_ : value);
    }

    void UpdateMeasuredVelocity() {
        const float range = rangeEnd_ - rangeStart_;
        if (fabsf(range) > 0.0001f) {
            const float perSecond = (valueSpring_->value - lastObservedValue_)
                                  / lastStepDt_ / range;
            velocitySpring_->AnimateTo(perSecond);
        }
        lastObservedValue_ = valueSpring_->value;
    }

    float initialValue_;
    float rangeStart_;
    float rangeEnd_;
    float visibilityThreshold_;
    float initialScale_;
    float pressedScale_;

    SpringFloat* valueSpring_;
    SpringFloat* velocitySpring_;
    SpringFloat* pressProgressSpring_;
    SpringFloat* scaleXSpring_;
    SpringFloat* scaleYSpring_;

    bool posted_ = false;
    float lastObservedValue_ = 0.0f;
    bool releasePending_ = false;
    bool releaseWaitOneFrame_ = false;
    bool trackValueVelocity_ = false;
    float lastStepDt_ = 1.0f / 60.0f;
};

}
