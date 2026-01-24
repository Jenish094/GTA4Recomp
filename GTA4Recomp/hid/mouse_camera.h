#pragma once

#include <cstdint>
// mouse camera handling

namespace MouseCamera
{
    void Initialize();
    
    /** 
     * @param deltaX mouse X movement in pixels
     * @param deltaY mouse Y movement in pixels
     * @param deltaTime
     */
    void Update(int32_t deltaX, int32_t deltaY, float deltaTime);
    
    /**
    Map mouse to right stick
     * @param outX output X axis value
     * @param outY output Y axis value
     */
    void GetAnalogValues(int16_t& outX, int16_t& outY);
    
    /**
     * Reset camera velocity if the mouse is inactive
     */
    void Reset();
    
    /**
     * check if mouse camera has moved recently/is active
     * @return true if mouse has moved recently
     */
    bool IsActive();
    
    /**
     * set sensitivity
     * @param sensitivityX Horizontal sensitivity
     * @param sensitivityY Vertical sensitivity
     */
    void SetSensitivity(float sensitivityX, float sensitivityY);
    
    /**
    invert y axis
     * @param invert
     */
    void SetInvertY(bool invert);
    
    /**
    mouse smoothing
     * @param smoothing between 0.0 and 1.0, higher means more smooth
     */
    void SetSmoothing(float smoothing);
}
