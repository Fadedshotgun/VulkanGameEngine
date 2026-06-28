#pragma once

#include <limits>

namespace v
{
    struct TimingStats
    {
        float last{0.0f};
        float average{0.0f};
        float min{std::numeric_limits<float>::max()};
        float max{0.0f};
        uint64_t sampleCount{0};

        void addSample(float value)
        {
            last = value;
            if (value < min)
            {
                min = value;
            }
            if (value > max)
            {
                max = value;
            }
            average = (average * sampleCount + value) / (sampleCount + 1);
            sampleCount++;
        }
    };

    struct FrameTimings
    {
        TimingStats totalFrame;
        TimingStats cpuUpdate;
        TimingStats movementUpdate;
        TimingStats cameraSystemUpdate;
        TimingStats particleEmitterUpdate;
        TimingStats imguiDraw;
        TimingStats gpuWait;
        TimingStats renderPass;
        TimingStats gpuSubmit;
    };
}
