#pragma once

class Time
{
    
public:float cameraShakingSpeed = 0.0f;
    int secondsPassed = 0;
    int framesPerSecond = 0;    // This will store our fps
    int directionSpeed = 1;
    Time(const char* nithing)
    {
        int i = 0; //doing nothing
        i++;
    }
    void CalculateFrameRate(float currentFrame)
    {
       // static int framesPerSecond = 0;       // This will store our fps
        static float lastTime = 0.0f;       // This will hold the time from the last frame
        float currentTime = currentFrame;
        ++framesPerSecond;
        if (currentTime - lastTime > 1.0f)//one second has passed
        {
            addSeconds();
            lastTime = currentTime;
            fprintf(stderr, "\nCurrent Frames Per Second: %d\n\n", (int)framesPerSecond);
            framesPerSecond = 0;
        }
    }

    float addSeconds()
    {
        secondsPassed++;
        if (secondsPassed % 10 > 4)
        {
            cameraShakingSpeed -= 0.05f* directionSpeed;
        }
        else
        {
             cameraShakingSpeed += 0.05f* directionSpeed;
        }
        if (secondsPassed % 10 == 0)
        {
            directionSpeed *= -1;
        }
        std::cout << cameraShakingSpeed << std::endl;
        return cameraShakingSpeed;
    }

    float getSpeed() { return cameraShakingSpeed; }
};